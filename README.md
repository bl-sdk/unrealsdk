# unrealsdk
[![Developer Discord](https://img.shields.io/static/v1?label=&message=Developer%20Discord&logo=discord&color=222)](https://discord.gg/VJXtHvh)

A library to help interact with unreal engine objects from another module in the same address space.

# Usage Overview
To start, you need to initialize the sdk. This is called with a reference to an `AbstractHook` - the
sdk can work with multiple (somewhat similar) UE versions from a single binary, so you need to tell
it how exactly to hook everything. The easiest way is to let it autodetect.

```cpp
unrealsdk::init(unrealsdk::game::select_based_on_executable());
```
If this doesn't work correctly, you can always implement your own version (and then merge it back
into this project).

If you link against the sdk as a shared library, it automatically initializes like this for you.

After initializing, you probably want to setup some hooks. The sdk can run callbacks whenever an
unreal function is hooked, allowing you to interact with it's args, and mess with it's execution.
Exact hook semantics are better documented in the `hook_manager.h` header.

```cpp
bool on_main_menu(unrealsdk::hook_manager::Details& hook) {
    LOG(INFO, "Reached main menu!");
    return false;
}

unrealsdk::hook_manager::add_hook(L"WillowGame.FrontendGFxMovie:Start",
                                  unrealsdk::hook_manager::Type::PRE, L"main_menu_hook",
                                  &on_main_menu);
```

Once your hook runs, you start having access to unreal objects. You can generally interact with any
unreal value (such as the properties on an object) through the templated `get` and `set` functions.
These functions take the expected property type as a template arg (and will throw exceptions if it
doesn't appear to line up). All property accesses are evaluated at runtime, meaning you don't need
to generate an sdk specific to your game.

```cpp
auto paused = hook.args->get<UBoolProperty>(L"StartPaused"_fn);

auto idx = hook.obj->get<UIntProperty>(L"MessageOfTheDayIdx"_fn);
auto motd_array = hook.obj->get<UArrayProperty>(L"MessagesOfTheDay"_fn);
motd_array.get_at<UStructProperty>(idx).set<UStrProperty>(L"Body"_fn, L"No MOTD today");

auto op_string = hook.obj->get<UFunction, BoundFunction>(L"BuildOverpowerPromptString"_fn)
                    .call<UStrProperty, UIntProperty, UIntProperty>(1, 10);
```

# Environment Variables
A few environment variables adjust the sdk's behaviour. Note that not all variables are used in all
build configurations.

| Environment Variable                          | Usage                                                                                                                           |
| :-------------------------------------------- | :------------------------------------------------------------------------------------------------------------------------------ |
| `UNREALSDK_ENV_FILE`                          | A file containing enviroment variables to load. Defaults to `unrealsdk.env`. More below.                                        |
| `UNREALSDK_EXTERNAL_CONSOLE`                  | If defined, creates an external console window mirroring what is written to the game's console. Always enabled in debug builds. |
| `UNREALSDK_LOG_LEVEL`                         | Changes the default logging level used in the unreal console. May use either the level names or their numerical values.         |
| `UNREALSDK_GAME_OVERRIDE`                     | Override the executable name used for game detection.                                                                           |
| `UNREALSDK_UPROPERTY_SIZE`                    | Changes the size the `UProperty` class is assumed to have.                                                                      |
| `UNREALSDK_ALLOC_ALIGNMENT`                   | Changes the alignment used when calling the unreal memory allocation functions.                                                 |
| `UNREALSDK_CONSOLE_KEY`                       | Changes the default console key which is set when one is not already bound.                                                     |
| `UNREALSDK_UCONSOLE_CONSOLE_COMMAND_VF_INDEX` | Overrides the virtual function index used when hooking `UConsole::ConsoleCommand`.                                              |
| `UNREALSDK_UCONSOLE_OUTPUT_TEXT_VF_INDEX`     | Overrides the virtual function index used when calling `UConsole::OutputText`.                                                  |

You can also define any of these in an env file, which will automatically be loaded when the sdk
starts (excluding `UNREALSDK_ENV_FILE` of course). This file should contain lines of equals
seperated key-value pairs, noting that whitespace is *not* stripped (outside of the trailing
newline). A line is ignored if it does not contain an equals sign, or if it defines a variable which
already exists.

```ini
UNREALSDK_LOG_LEVEL=MISC
UNREALSDK_CONSOLE_KEY=Quote
```

You can also use this file to load enviroment variables for other plugins (assuming they don't check
them too early), it's not limited to just those used by the sdk.

# Linking Against the SDK
The sdk requires at least C++20, primarily for templated lambdas. It also makes great use of
`std::format`, though if this is not available it tries to fall back to using fmtlib. Linking
against the sdk thus requires your own projects to use at least C++20 too.

To link against the sdk, simply clone the repo (including submodules), add it as a subdirectory,
and link against the `unrealsdk` target.

```
git clone --recursive https://github.com/bl-sdk/unrealsdk.git
```
```cmake
add_submodule(path/to/unrealsdk)
target_link_libraries(MyProject PRIVATE unrealsdk)
```

You can configure the sdk by setting a few variables before including it:
- `UNREALSDK_UE_VERSION` - The unreal engine version to build the SDK for. One of `UE3` or `UE4`.
  These versions are different enough that supporting them from a single binary is difficult.
- `UNREALSDK_ARCH` - The architecture to build the sdk for. One of `x86` or `x64`. Will be double
  checked at compile time.
- `UNREALSDK_SHARED` - If set, compiles as a shared library instead of as an object.

## Shared Library
The sdk contains a decent amount of internal state, meaning it's not possible to inject twice into
the same process. At it's simplest, any detours on unreal functions will change their signatures, so
a second instance won't be able to find them again. If two programs both want to use the sdk in the
same game process, they will have to link against the shared library.

The included shared library initializes based on executable. If you need custom initialization, you
can create your own shared library by linking against the object library and defining the
`UNREALSDK_SHARED` and `UNREALSDK_EXPORTING` macros.

One of the goals of the shared library implementation is have a stable cross-compiler ABI - i.e.
allowing developing one program while also running another which you downloaded a precompiled
version of.

In order to do this, the exported functions try to use a pure C interface. Since the sdk heavily
relies on C++ features (e.g. all the templates), it's impractical to export everything this way.
Instead, it only exports the bare minimum functions which interact with internal state. Some of
these rely on private wrapper functions, which do things like decompose strings into pointer and
length, in which case the public functions are redirected as required. The remaining functions will
be linked statically.

While the function calls have a stable cross-compiler ABI, unfortuantly there's one other thing
which we can't guarantee: exceptions. MSVC and GNU have different exception ABIs, so if one travels
between two modules with different ABIs, the game will crash. While none of the shared functions
intentionally throw exceptions, it's impossible to completely avoid an exception travelling between
modules - we can't stop a client from throwing during a hook (which is called by the sdk). All
shared functions are compiled to try to allow exceptions to pass through them, but this will only
work properly if all modules share an exception ABI - though surely you write good code so it won't
be a problem :).

# Running Builds
As previously mentioned, the sdk can be configured to create a shared library. This is useful when
developing for the sdk itself, it's the minimal configuration to get it running. The CMake presets
are set up to build this.

Note that you will need to use some game specific plugin loader to get the dll loaded. It is not set
up to alias any system dlls (since when actually using it as a library you don't want that), you
can't just call it `d3d9.dll` and assume your game will load fine.

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
