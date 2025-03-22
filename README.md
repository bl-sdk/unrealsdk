# unrealsdk
[![Developer Discord](https://img.shields.io/static/v1?label=&message=Developer%20Discord&logo=discord&color=222)](https://discord.gg/VJXtHvh)

A library to help interact with unreal engine objects from another module in the same address space.

What makes this library different?
- All property accesses are performed and validated at runtime. There's no need to dump an SDK, and 
  it's automatically compatible with any game updates adding new fields.
- No dependencies on Epic, so actually open source, all code independently reverse engineered.
- Supports multiple different compilers, cross compiling from Linux is a first class target.

Why shouldn't you use this library?
- Very limited support for different Unreal Engine versions, it's primarily focused on the
  Borderlands series.
- Some more complex property types (e.g. maps) are not supported, since they each need to be
  manually implemented.
- Only supports Windows executables (though will work under Proton).

# Usage Overview
Standard usage involves setting up hooks. The sdk can run callbacks whenever a given unreal function
is called, allowing you to interact with it's args, and mess with it's execution. Exact hook
semantics are better documented in the `hook_manager.h` header.

```cpp
bool on_main_menu(unrealsdk::hook_manager::Details& hook) {
    LOG(INFO, "Reached main menu!");
    return false;
}

unrealsdk::hook_manager::add_hook(L"WillowGame.FrontendGFxMovie:Start",
                                  unrealsdk::hook_manager::Type::PRE, L"main_menu_hook",
                                  &on_main_menu);
```

Once your hook runs, you start having access to unreal objects. You can interact with unreal values
through the `get` and `set` functions. These functions take the expected property type as a template
arg, and will throw exceptions if it doesn't appear to line up.

```cpp
auto paused = hook.args->get<UBoolProperty>(L"StartPaused"_fn);

auto idx = hook.obj->get<UIntProperty>(L"MessageOfTheDayIdx"_fn);
auto motd_array = hook.obj->get<UArrayProperty>(L"MessagesOfTheDay"_fn);
motd_array.get_at<UStructProperty>(idx).set<UStrProperty>(L"Body"_fn, L"No MOTD today");

auto op_string = hook.obj->get<UFunction, BoundFunction>(L"BuildOverpowerPromptString"_fn)
                    .call<UStrProperty, UIntProperty, UIntProperty>(1, 10);
```

# Integrating the SDK into your project
So there are a few extra steps to integrate the sdk into your project before you can start using
hooks like above.

The SDK requires at least C++20. This is primarily for templated lambdas - you may still be able to
use it if your compiler doesn't yet fully support it (e.g. it falls back to fmtlib if `std::format`
is not available).

The recommended way to link against the sdk is as a submodule.

```
git clone --recursive https://github.com/bl-sdk/unrealsdk.git
```
```cmake
add_submodule(path/to/unrealsdk)
target_link_libraries(MyProject PRIVATE unrealsdk)
```

You can configure the sdk by setting a few variables before including it:
- `UNREALSDK_UE_VERSION` - The unreal engine version to build the SDK for, one of `UE3` or `UE4`.
  These versions are different enough that supporting them from a single binary is difficult.
- `UNREALSDK_ARCH` - The architecture to build the sdk for. One of `x86` or `x64`. Will be double
  checked at compile time.
- `UNREALSDK_SHARED` - If set, compiles as a shared library instead of as an object.

If you want to be able to run multiple projects using the sdk in the same game process, you *must*
compile it as a shared library, there's a decent amount of internal state preventing initializing it
twice.

If you're linking against a static library, the easiest way to initialize it is:
```cpp
unrealsdk::init(unrealsdk::game::select_based_on_executable);
```
If you're linking against the shared library, it's automatically initialized in a thread. You'll
instead need to blocking wait on it finishing before continuing.
```cpp
while (!unrealsdk::is_initialized()) {}
```

At this point the SDK is ready, you can start setting your hooks.

You may want to further wait on the console being hooked, so that your log messages appear in the
game's console. They will still be written to the log file before this point.
```cpp
LOG(INFO, "Some log message");  // Only in the log file
while (!unrealsdk::is_console_ready()) {}
LOG(INFO, "Some log message");  // Both in the log file and console
```

## Configuration
There are a few pieces of sdk behaviour you can configure, via an `unrealsdk.toml`. By default this
should be placed next to the dll, though you can also specify a custom location via the
`UNREALSDK_CONFIG_FILE` environment variable. You are not expect to provide a full configuration,
some settings change behaviour simply by being defined, you should only set the ones you need. If
the defaults work you may not even need a config file to begin with.

Since it's somewhat expected your project may have to ship with some default settings, an
`unrealsdk.user.toml` can also be used to add some user specific settings. The values in the user
file overwrite those from the base, unless both values are tables, in which case they're merged
recursively.

[`supported_settings.toml`](supported_settings.toml) has full descriptions of all supported
settings. *Do not* try make a copy of this file as the basis of your config, some of the values
within it are intentionally bogus, as there's no sane default, and using them will likely crashes.

<br>

All the sdk's settings are under the top level `unrealsdk` table. In your own projects you may add
additional settings to the same file, under a different header. The sdk exposes a few functions to
let you read some basic values without needing to parse the files again.
```cpp
auto val = unrealsdk::config::get_str("my_project.my_field");
if (val.has_value()) {
    do_something(*val);
}
```

## Cross-Compiler ABI
One of the goals of the shared library implementation is have a stable cross-compiler ABI - i.e.
allowing developing one program while also running another which you downloaded a precompiled
version of.

In order to do this, the sdk uses a number of private exported functions (those in the headers are
automatically converted), which try to keep to a pure C interface. There is one assumption we still
rely on however, where we can't quite stick with pure C:

- Both dlls share the same exception ABI. While none of the exported functions intentionally throw,
  it's impossible to completely avoid an exception travelling between modules - we can't stop a
  client from throwing during a hook, meaning an exception would travel from the client dll through
  to the sdk.

This turns out to be a bit of a problem - MSVC and GNU have different exception ABIs. Clang supports
both. Practically, this means when cross compiling, you should either compile everything from
scratch, or setup Clang to build with the MSVC ABI. [See this blog post for more info](https://apple1417.dev/posts/2023-05-18-debugging-proton).

# Building the SDK by itself
The shared library is also useful when developing for the sdk itself, since it's the minimal
configuration to get it running. The CMake presets are set up to build this. There are currently
five supported toolchains, each of which have a few different sdk configurations:

- MSVC
- Clang (Windows)
- Clang (Cross Compile) <sup>*</sup>
- MinGW <sup>*</sup>
- LLVM MinGW <sup>*</sup>

The toolchains with an asterix are all cross compiling toolchains. These all also have an associated
dev container, which is the recommended way of building them. The `clang-cross-*` presets in
particular hardcode a path assuming they're running in the container.

Note that you will need to use some game specific plugin loader to get the `unrealsdk.dll` loaded.
It is not set up to alias any system dlls (since when actually using it as a library you don't want
that), you can't just rename it to `d3d9.dll` and assume your game will load fine.

To build:

1. Clone the repo (including submodules).
   ```
   git clone --recursive https://github.com/bl-sdk/unrealsdk.git
   ```

2. (OPTIONAL) Copy `postbuild.template`, and edit it to copy files to your game install directories.

3. Choose a preset, and run CMake. Most IDEs will be able to do this for you,
   ```
   cmake . --preset msvc-ue4-x64-debug
   cmake --build out/build/msvc-ue4-x64-debug
   ```

4. (OPTIONAL) If you're debugging a game on Steam, add a `steam_appid.txt` in the same folder as the
   executable, containing the game's Steam App Id.

   Normally, games compiled with Steamworks will call
   [`SteamAPI_RestartAppIfNecessary`](https://partner.steamgames.com/doc/sdk/api#SteamAPI_RestartAppIfNecessary),
   which will drop your debugger session when launching the exe directly - adding this file prevents
   that. Not only does this let you debug from entry, it also unlocks some really useful debugger
   features which you can't access from just an attach (i.e. Visual Studio's Edit and Continue).
