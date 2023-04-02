# unrealsdk
[![Developer Discord](https://img.shields.io/static/v1?label=&message=Developer%20Discord&logo=discord&color=222)](https://discord.gg/VJXtHvh)

A library to help interact with unreal engine objects from another module in the same address space.

# Usage Overview
To start, you need to initalize the sdk. This is called with a refernce to an `AbstractHook` - the
sdk can work with multiple (somewhat similar) UE versions from a single binary, so you need to tell
it how exactly to hook everything. The easiest way is to let it autodetect.

```cpp
unrealsdk::init(unrealsdk::game::select_based_on_executable());
```
If this doesn't work correctly, you can always implement your own version (and then merge it back
into this project).

After initalizing, you probably want to setup some hooks. The sdk can run callbacks whenever an
unreal function is hooked, allowing you to interact with it's args, and mess with it's execution.
Exact hook semantics are better documented in the `hook_manager.h` header.

```cpp
bool on_main_menu(HookDetails& hook) {
   LOG(INFO, "Reached main menu!");
}

hook_manager::hooks[L"WillowGame.FrontendGFxMovie:Start"].pre[L"main_menu_hook"] = &on_main_menu;
```

Once your hook runs, you start having access to unreal objects. You can generally interact with any
unreal value (such as the properties on an object) through the templated `get` and `set` functions.
These functions take the expected property type as a template arg (and will throw errors if it
doesn't appear to line up).

```cpp
auto paused = hook.args->get<UBoolProperty>(L"StartPaused"_fn);

auto idx = hook.obj->get<UIntProperty>(L"MessageOfTheDayIdx"_fn);
auto motd_array = hook.obj->get<UArrayProperty>(L"MessagesOfTheDay"_fn);
motd_array->get_at<UStructProperty>(idx)->set<UStrProperty>(L"Body"_fn, L"No MOTD today");

auto op_string = hook.obj->get<UFunction, BoundFunction>(L"BuildOverpowerPromptString")
                     .call<UStrProperty, UIntProperty, UIntProperty>(1, 10);
```

# Enviroment Variables
A few enviroment variables adjust the sdk's behaviour. Note that not all variables are used in all
build configurations.

| Enviroment Variable                       | Usage                                                                                                                           |
| :---------------------------------------- | :------------------------------------------------------------------------------------------------------------------------------ |
| `UNREALSDK_EXTERNAL_CONSOLE`              | If defined, creates an external console window mirroring what is written to the game's console. Always enabled in debug builds. |
| `UNREALSDK_LOG_LEVEL`                     | Changes the default logging level used in the unreal console. May use either the level names or their numerical values.         |
| `UNREALSDK_GAME_OVERRIDE`                 | Override the executable name used for game detection.                                                                           |
| `UNREALSDK_UPROPERTY_SIZE`                | Changes the size the `UProperty` class is assumed to have.                                                                      |
| `UNREALSDK_ALLOC_ALIGNMENT`               | Changes the alignment used when calling the unreal memory allocation functions.                                                 |
| `UNREALSDK_CONSOLE_KEY`                   | Changes the default console key which is set when one is not already bound.                                                     |
| `UNREALSDK_UCONSOLE_OUTPUT_TEXT_VF_INDEX` | Overrides the virtual function index used when calling `UConsole::OutputText`.                                                  |


# Linking against the sdk
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
- `UNREALSDK_STANDALONE` - If defined, also creates an additional standalone target creating a
  `unrealsdk.dll`. Generally useless if you're linking against it in your own project.

# Standalone Builds
As just mentioned, the sdk can be configured to create a small standalone dll. This just initalizes
itself when loaded, but does nothing else. All the CMake presets are set up to build this.

The standalone build is primarily useful when developing for the sdk itself, since it cuts out any
extra fluff. It's also used for CI.

To create a standalone build:

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
