#include "unrealsdk/pch.h"

#include "unrealsdk/config.h"
#include "unrealsdk/hook_manager.h"
#include "unrealsdk/unreal/classes/ufunction.h"
#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/structs/fframe.h"
#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"
#include "unrealsdk/unrealsdk.h"
#include "unrealsdk/utils.h"

using namespace unrealsdk::unreal;

#pragma region Implementation
#ifndef UNREALSDK_IMPORTING
namespace unrealsdk::hook_manager::impl {

/*
The fact that hooks run arbitrary user provided callbacks means our data structure can get modified
in a number of awkward ways while we're in the middle of iterating over it. There's the obvious case
of a hook can remove itself, but also tricker ones like a hook can invoke a nested hook on itself,
and only remove itself there, or the upper layer hook could remove itself and the nested one could
re-add it, etc. It gets even messier if we consider threading.

So the number one concern behind this design is robustness - we need to support essentially
arbitrary modification while we're in the middle of iterating through it. This means performance is
a secondary concern - though I still tried keeping it mind - I haven't done any benchmarking.

The easiest way to keep our iterators valid is to use linked lists. However, using a bunch of
`std::list`s isn't the best for performance, as we'll discuss in a bit, since need several layers
of them, so instead we use a bunch of intrusive linked lists on our own type.


The most basic form of the data structure we want is essentially a:
    map<FName, map<full_name, map<Type, collection<pair<identifier, callback>>>>>

Matching FNames first lets us discard function calls far quicker than needing to do a string
comparison on the full name. After matching both of them, we then need to split by hook type, which
finally gets us the collection of callbacks to run. The identifiers have no influence when matching
hooks, they're only used when adding/removing them.

The smallest unit we have to iterate over is the hooks of the same type on the same function - the
one labeled just "collection" above. This is the `next_in_collection` linked list.

Then we need to be able to jump between the collections on the same hook, but of different type. The
heads of the collection linked lists form into a second `next_type` linked list. We use an intrusive
linked list, so the exact same node from the above list also holds the pointers for this one.

Above that, we need to jump between groups of hooks which share the same fname, but have different
full function names. The heads of the type linked lists form the third `next_function` linked list.

Finally, we need to iterate through FNames. We actually do this using a hash table - but we still
need to deal with collisions. The heads of the function linked lists form our fourth and final
`next_collision` linked list.

Trying to roughly diagram an example, this is what might be in a single hash bucket:

Collision  | [A] -----------------------------> [F]
           |  :                                  :
Function   | [A] ---------------> [D]           [F] -> [G]
           |  :                    :             :      :
Type       | [A] --------> [C]     :             :      :
           |  :             :      :             :      :
Collection | [A] -> [B]    [C]    [D] -> [E]    [F]    [G]

[A] Class::Func, post-hook
[B] Class::Func, post-hook
[C] Class::Func, pre-hook
[D] OtherClass::Func
[E] OtherClass::Func
[F] ThirdClass::SomeOtherFunc, where `SomeOtherFunc` and `Func` happen to get a hash collision
[G] FourthClass::SomeOtherFunc

Each column is a single node, a node may be in multiple linked lists.
*/

struct Node {
   public:
    FName fname;
    std::wstring full_name;
    Type type;
    std::wstring identifier;
    DLLSafeCallback* callback;

    // Using shared pointers because it's easy
    // Since we use std::make_shared, we're not really wasting allocations, but as a future
    // optimization we could use an intrusive reference count to avoid repeating all the control
    // block pointers, and save a little memory.
    std::shared_ptr<Node> next_collision = nullptr;
    std::shared_ptr<Node> next_function = nullptr;
    std::shared_ptr<Node> next_type = nullptr;
    std::shared_ptr<Node> next_in_collection = nullptr;

    Node(FName fname,
         std::wstring_view full_name,
         Type type,
         std::wstring_view identifier,
         DLLSafeCallback* callback)
        : fname(fname),
          full_name(full_name),
          type(type),
          identifier(identifier),
          callback(callback) {}
    Node(const Node&) = default;
    Node(Node&&) noexcept = default;
    Node& operator=(const Node&) = default;
    Node& operator=(Node&&) noexcept = default;
    ~Node() {
        if (this->callback != nullptr) {
            this->callback->destroy();
            this->callback = nullptr;
        }
    }
};

namespace {

const constexpr auto HASH_TABLE_SIZE = 0x1000;
std::array<std::shared_ptr<Node>, HASH_TABLE_SIZE> hooks_hash_table;

/**
 * @brief Hashes the given fname, and returns which index of the table it goes in.
 *
 * @param name The name to check.
 * @return The hash table index.
 */
size_t get_table_index(FName fname) {
    static_assert(sizeof(unrealsdk::unreal::FName) == sizeof(uint64_t),
                  "FName is not same size as a uint64");
    uint64_t val{};
    memcpy(&val, &fname, sizeof(fname));

    // Identity seems mostly good enough - FNames already are just a relatively small integer value.
    // This throws away the number field, but that's very rarely used on functions to begin with.
    return val % HASH_TABLE_SIZE;
}

bool should_log_all_calls = false;
std::wofstream log_all_calls_stream{};
std::mutex log_all_calls_stream_mutex{};

void log_all_calls(bool should_log) {
    // Only keep this file stream open while we need it
    if (should_log) {
        const std::lock_guard<std::mutex> lock(log_all_calls_stream_mutex);
        log_all_calls_stream.open(
            utils::get_this_dll().parent_path()
                / config::get_str("unrealsdk.log_all_calls_file").value_or("unrealsdk.calls.tsv"),
            std::wofstream::trunc);
    }

    should_log_all_calls = should_log;

    if (!should_log) {
        const std::lock_guard<std::mutex> lock(log_all_calls_stream_mutex);
        log_all_calls_stream.close();
    }
}

thread_local bool should_inject_next_call = false;

void inject_next_call(void) {
    should_inject_next_call = true;
}

/**
 * @brief Extracts the object name from a hook function's full path name.
 *
 * @param func The full function path name.
 * @return The FName we expect it's object to use.
 */
FName extract_func_obj_name(std::wstring_view func) {
    auto idx = func.find_last_of(L".:");
    if (idx == std::wstring_view::npos) {
        return FName{std::wstring{func}};
    }
    return FName{std::wstring{func.substr(idx + 1)}};
}

bool add_hook(std::wstring_view func,
              Type type,
              std::wstring_view identifier,
              DLLSafeCallback* callback) {
    auto fname = extract_func_obj_name(func);

    auto hash_idx = get_table_index(fname);
    auto node = hooks_hash_table.at(hash_idx);
    if (node == nullptr) {
        // This function isn't in the hash table, can just add directly.
        hooks_hash_table.at(hash_idx) =
            std::make_shared<Node>(fname, func, type, identifier, callback);
        return true;
    }

    // Look through hash collisions
    while (node->fname != fname) {
        if (node->next_collision == nullptr) {
            // We found a collision, but nothing matched our name, so add it to the end
            node->next_collision = std::make_shared<Node>(fname, func, type, identifier, callback);
            return true;
        }
        node = node->next_collision;
    }

    // Look though full function names
    while (node->full_name != func) {
        if (node->next_function == nullptr) {
            // We found another function with the same fname, but nothing matches the full name
            node->next_function = std::make_shared<Node>(fname, func, type, identifier, callback);
            return true;
        }
        node = node->next_function;
    }

    // Look through hook types
    while (node->type != type) {
        if (node->next_type == nullptr) {
            // We found the right function, but it doesn't have any hooks of this type yet
            node->next_type = std::make_shared<Node>(fname, func, type, identifier, callback);
            return true;
        }
        node = node->next_type;
    }

    // Look through all remaining hooks to see if we can match the identifier
    while (node->identifier != identifier) {
        if (node->next_in_collection == nullptr) {
            // Didn't find a matching identifier, add our new hook at the end
            node->next_in_collection =
                std::make_shared<Node>(fname, func, type, identifier, callback);
        }
        node = node->next_in_collection;
    }

    // We already have this identifier, can't insert
    return false;
}

bool has_hook(std::wstring_view func, Type type, std::wstring_view identifier) {
    auto fname = extract_func_obj_name(func);

    auto hash_idx = get_table_index(fname);
    auto node = hooks_hash_table.at(hash_idx);
    if (node == nullptr) {
        // This function isn't even in the hash table
        return false;
    }

    // Look through hash collisions
    while (node->fname != fname) {
        if (node->next_collision == nullptr) {
            // We found a collision, but nothing matched our name
            return false;
        }
        node = node->next_collision;
    }

    // Look though full function names
    while (node->full_name != func) {
        if (node->next_function == nullptr) {
            // We found another function with the same fname, but nothing matches the full name
            return false;
        }
        node = node->next_function;
    }

    // Look through hook types
    while (node->type != type) {
        if (node->next_type == nullptr) {
            // We found the right function, but it doesn't have any hooks of this type
            return false;
        }
        node = node->next_type;
    }

    // Look through all remaining hooks to see if we can match the identifier
    while (node->identifier != identifier) {
        if (node->next_in_collection == nullptr) {
            // Didn't find a matching identifier
            return false;
        }
        node = node->next_in_collection;
    }

    return true;
}

bool remove_hook(std::wstring_view func, Type type, std::wstring_view identifier) {
    auto fname = extract_func_obj_name(func);

    auto hash_idx = get_table_index(fname);
    auto node = hooks_hash_table.at(hash_idx);
    if (node == nullptr) {
        // This function isn't even in the hash table
        return false;
    }

    // Look through hash collisions
    decltype(node) prev_collision = nullptr;
    while (node->fname != fname) {
        if (node->next_collision == nullptr) {
            // We found a collision, but nothing matched our name
            return false;
        }
        prev_collision = node;
        node = node->next_collision;
    }

    // Look though full function names
    decltype(node) prev_function = nullptr;
    while (node->full_name != func) {
        if (node->next_function == nullptr) {
            // We found another function with the same fname, but nothing matches the full name
            return false;
        }
        prev_function = node;
        node = node->next_function;
    }

    // Look through hook types
    decltype(node) prev_type = nullptr;
    while (node->type != type) {
        if (node->next_type == nullptr) {
            // We found the right function, but it doesn't have any hooks of this type
            return false;
        }
        prev_type = node;
        node = node->next_type;
    }

    // Look through all remaining hooks to see if we can match the identifier
    decltype(node) prev_in_collection = nullptr;
    while (node->identifier != identifier) {
        if (node->next_in_collection == nullptr) {
            // Didn't find a matching identifier
            return false;
        }
        prev_in_collection = node;
        node = node->next_in_collection;
    }

    /*
    We found a matching hook - 'node' is pointing at it.

    Consider the following diagram the diagram from above:

    Collision  | [A] -----------------------------> [F]
               |  :                                  :
    Function   | [A] ---------------> [D]           [F]
               |  :                    :             :
    Type       | [A]                   :            [F] --------> [H]
               |  :                    :             :             :
    Collection | [A] -> [B] -> [C]    [D] -> [E]    [F] -> [G]    [H]

    Lets say we want to remove B, D, and F. The new layout we need is:

    Collision  | [A] ------------------------------------> [G]
               |  :                                         :
    Function   | [A] ----------------------> [E]            :
               |  :                           :             :
    Type       | [A]                          :            [G] -> [H]
               |  :                           :             :      :
    Collection | [A] --------> [C]           [E]           [G]    [H]

    Removing B is easy, we simply set A->next_in_collection = C to bypass it.

    For D, we need to work bottom up. If our node is the head of the list, we need to promote our
    next node up a layer. If our node is also the head of the above layer, we need to recurse
    another layer up. So since D is the head of both the collection and type linked lists, we move
    up to the function linked list, and need to set A->next_function = E.

    F exposes a further complication on top of D, since it was pointing at multiple other nodes. As
    part of promoting a node up a layer, we need to insert it into that layer's linked list - that's
    how we keep then link to H. In D's case, these were all null.

    Since only the heads of the lower-layer lists are used in the higher layer ones, as soon as we
    come across a layer where we're not the head, we must be done, the lower layer nodes cannot have
    other references to upper ones.
    */

    if (prev_in_collection != nullptr) {
        prev_in_collection->next_in_collection = node->next_in_collection;
        return true;
    }
    if (node->next_in_collection) {
        node->next_in_collection->next_type = node->next_type;
        node->next_type = node->next_in_collection;
    }

    if (prev_type != nullptr) {
        prev_type->next_type = node->next_type;
        return true;
    }
    if (node->next_type != nullptr) {
        node->next_type->next_function = node->next_function;
        node->next_function = node->next_type;
    }

    if (prev_function != nullptr) {
        prev_function->next_function = node->next_function;
        return true;
    }
    if (node->next_function != nullptr) {
        node->next_function->next_collision = node->next_collision;
        node->next_collision = node->next_function;
    }

    if (prev_collision != nullptr) {
        prev_collision->next_collision = node->next_collision;
        return true;
    }
    // There's no higher layer linked list left. If we have following collision entries, set the
    // hash table to them. If we don't, we want to null it anyway.
    hooks_hash_table.at(hash_idx) = node->next_collision;
    return true;
}

}  // namespace

std::shared_ptr<Node> preprocess_hook(std::wstring_view source,
                                      const UFunction* func,
                                      const UObject* obj) {
    if (should_inject_next_call) {
        should_inject_next_call = false;
        return nullptr;
    }

    // Want to delay filling this, but if we're logging all calls we need it straight away
    std::wstring func_name{};

    if (should_log_all_calls) {
        // Extra safety check
        if (log_all_calls_stream.is_open()) {
            func_name = func->get_path_name();
            auto obj_name = obj->get_path_name();

            const std::lock_guard<std::mutex> lock(log_all_calls_stream_mutex);
            log_all_calls_stream << source << L'\t' << func_name << L'\t' << obj_name << L'\n';
        }
    }

    auto fname = func->Name();

    auto hash_idx = get_table_index(fname);
    auto node = hooks_hash_table.at(hash_idx);
    if (node == nullptr) {
        // This function isn't even in the hash table
        return nullptr;
    }

    // Look through hash collisions
    while (node->fname != fname) {
        if (node->next_collision == nullptr) {
            // We found a collision, but nothing matched our name
            return nullptr;
        }
        node = node->next_collision;
    }

    // At this point we need the full path name
    if (!should_log_all_calls) {
        func_name = func->get_path_name();
    }

    // Look though full function names
    while (node->full_name != func_name) {
        if (node->next_function == nullptr) {
            // We found another function with the same fname, but nothing matches the full name
            return nullptr;
        }
        node = node->next_function;
    }

    // Break off at this point - we know we have hooks on this function, so the hook processing will
    // need to start extracting args.
    return node;
}

bool has_post_hooks(std::shared_ptr<Node> node) {
    // We got the node from preprocess_hook, it's pointing to the start of the types linked list
    for (; node != nullptr; node = node->next_type) {
        if (node->type == Type::POST || node->type == Type::POST_UNCONDITIONAL) {
            return true;
        }
    }
    return false;
}

bool run_hooks_of_type(std::shared_ptr<Node> node, Type type, Details& hook) {
    // We got the node from preprocess_hook, it's pointing to the start of the types linked list

    // Look through hook types
    while (node->type != type) {
        if (node->next_type == nullptr) {
            // No hooks of this type - return false to not block
            return false;
        }
        node = node->next_type;
    }

    // We've got the final list of hooks, run them all
    bool ret = false;
    for (; node != nullptr; node = node->next_in_collection) {
        try {
            ret |= node->callback->operator()(hook);
        } catch (const std::exception& ex) {
            LOG(ERROR, "An exception occurred during hook processing");
            LOG(ERROR, L"Function: {}", hook.func.func->get_path_name());
            LOG(ERROR, "Exception: {}", ex.what());
        }
    }

    return ret;
}

}  // namespace unrealsdk::hook_manager::impl
#endif
#pragma endregion

// =================================================================================================

#pragma region Public Interface
namespace unrealsdk::hook_manager {

#ifdef UNREALSDK_SHARED
UNREALSDK_CAPI(void, log_all_calls, bool should_log);
#endif
#ifndef UNREALSDK_IMPORTING
UNREALSDK_CAPI(void, log_all_calls, bool should_log) {
    impl::log_all_calls(should_log);
}
#endif
void log_all_calls(bool should_log) {
    UNREALSDK_MANGLE(log_all_calls)(should_log);
}

#ifdef UNREALSDK_SHARED
UNREALSDK_CAPI(void, inject_next_call);
#endif
#ifndef UNREALSDK_IMPORTING
UNREALSDK_CAPI(void, inject_next_call) {
    impl::inject_next_call();
}
#endif
void inject_next_call(void) {
    UNREALSDK_MANGLE(inject_next_call)();
}

#ifdef UNREALSDK_SHARED
UNREALSDK_CAPI(bool,
               add_hook,
               const wchar_t* func,
               size_t func_size,
               Type type,
               const wchar_t* identifier,
               size_t identifier_size,
               DLLSafeCallback* callback);
#endif
#ifndef UNREALSDK_IMPORTING
UNREALSDK_CAPI(bool,
               add_hook,
               const wchar_t* func,
               size_t func_size,
               Type type,
               const wchar_t* identifier,
               size_t identifier_size,
               DLLSafeCallback* callback) {
    return impl::add_hook({func, func_size}, type, {identifier, identifier_size}, callback);
}
#endif

bool add_hook(std::wstring_view func,
              Type type,
              std::wstring_view identifier,
              const Callback& callback) {
    // NOLINTBEGIN(cppcoreguidelines-owning-memory)
    return UNREALSDK_MANGLE(add_hook)(func.data(), func.size(), type, identifier.data(),
                                      identifier.size(), new DLLSafeCallback(callback));
    // NOLINTEND(cppcoreguidelines-owning-memory)
}

#ifdef UNREALSDK_SHARED
UNREALSDK_CAPI(bool,
               has_hook,
               const wchar_t* func,
               size_t func_size,
               Type type,
               const wchar_t* identifier,
               size_t identifier_size);
#endif
#ifndef UNREALSDK_IMPORTING
UNREALSDK_CAPI(bool,
               has_hook,
               const wchar_t* func,
               size_t func_size,
               Type type,
               const wchar_t* identifier,
               size_t identifier_size) {
    return impl::has_hook({func, func_size}, type, {identifier, identifier_size});
}
#endif

bool has_hook(std::wstring_view func, Type type, std::wstring_view identifier) {
    return UNREALSDK_MANGLE(has_hook)(func.data(), func.size(), type, identifier.data(),
                                      identifier.size());
}

#ifdef UNREALSDK_SHARED
UNREALSDK_CAPI(bool,
               remove_hook,
               const wchar_t* func,
               size_t func_size,
               Type type,
               const wchar_t* identifier,
               size_t identifier_size);
#endif
#ifndef UNREALSDK_IMPORTING

UNREALSDK_CAPI(bool,
               remove_hook,
               const wchar_t* func,
               size_t func_size,
               Type type,
               const wchar_t* identifier,
               size_t identifier_size) {
    return impl::remove_hook({func, func_size}, type, {identifier, identifier_size});
}

#endif

bool remove_hook(std::wstring_view func, Type type, std::wstring_view identifier) {
    return UNREALSDK_MANGLE(remove_hook)(func.data(), func.size(), type, identifier.data(),
                                         identifier.size());
}

}  // namespace unrealsdk::hook_manager

#pragma endregion
