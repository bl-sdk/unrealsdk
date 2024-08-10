# Changelog

## v1.2.0
- When an exception occurs during a hook, now mention what function it was under, to make debugging
  easier.

  [f2e21f60](https://github.com/bl-sdk/unrealsdk/commit/f2e21f60)

- Optimized performance of checking if to run hooks on functions which have none.

  [77bc3c54](https://github.com/bl-sdk/unrealsdk/commit/77bc3c54)

- Added support for `UByteAttributeProperty`, `UComponentProperty`, `UFloatAttributeProperty`,
  and `UIntAttributeProperty`.

  [45f07875](https://github.com/bl-sdk/unrealsdk/commit/45f07875),
  [adb5c986](https://github.com/bl-sdk/unrealsdk/commit/adb5c986)

- Added the `Enum` field to `UByteAttributeProperty`s.

  [fb9c043b](https://github.com/bl-sdk/unrealsdk/commit/fb9c043b)

- Added the `UNREALSDK_LOCKING_PROCESS_EVENT` env var, to help deal with games where it's not thread
  safe.

  **Note that this opens up the possibility for a deadlock in external code.**

  Both hooks and unreal function calls will attempt to acquire the "process event lock". It is
  possible to deadlock if a hook (which holds the process event lock) attempts to acquire another
  lock at the same time as the thread holding that lock tries to call an unreal function (which will
  attempt to acquire the process event lock).
  
  Swapped various unreal function calls with native equivalents to try reduce how many functions
  transitively have this behaviour - should only need to worry about calls to `BoundFunction::call`,
  or to `unrealsdk::process_event` directly.

  [35857adf](https://github.com/bl-sdk/unrealsdk/commit/35857adf),
  [b9469bbf](https://github.com/bl-sdk/unrealsdk/commit/b9469bbf),
  [d74ff4eb](https://github.com/bl-sdk/unrealsdk/commit/d74ff4eb),
  [91e3fcd5](https://github.com/bl-sdk/unrealsdk/commit/91e3fcd5)

- Several logging module reworks. *This breaks binary compatibility*, though existing code should
  work pretty much as is after a recompile.

  - *Changed the semantics of `unrealsdk::logging::init`.* The `callbacks_only` arg was removed in
    favour of passing an empty path to disable file output, and disabling console output separately.
    The never version has the exact same signature, so existing code which used both args may need
    to be updated.

  - Changed the `location` arg to take a string view rather than a raw pointer.

  - Moved to a message queue model, where all printing is done in its own thread. This helps avoid
    deadlocks when using locking process event, should mean filesystem access doesn't block logic
    threads.

  - Fixed that empty log messages would not be properly shown in the unreal console.

  [02b56f18](https://github.com/bl-sdk/unrealsdk/commit/02b56f18),
  [91e3fcd5](https://github.com/bl-sdk/unrealsdk/commit/91e3fcd5),
  [8ec285fc](https://github.com/bl-sdk/unrealsdk/commit/8ec285fc)

- Tweaked `Pattern::sigscan` to more explicitly deal with sigscans failing. Existing code will have
  to be updated to either call `sigscan_nullable` (if failing is ok), or to pass a name to use in
  case of error (if throwing is ok).

  [7135bdf3](https://github.com/bl-sdk/unrealsdk/commit/7135bdf3),
  [b849b0e8](https://github.com/bl-sdk/unrealsdk/commit/91e3fcd5)

- Made the `UnrealPointer` constructor explicit.

  [26a47713](https://github.com/bl-sdk/unrealsdk/commit/26a47713)


## v1.1.0
- Changed a number of interfaces to take a string view rather than a const string reference.

  [5e59b16b..2c9f3082](https://github.com/bl-sdk/unrealsdk/compare/5e59b16b..2c9f3082)

- Changed `unrealsdk::init` to take a getter function which returns an abstract hook, rather than
  taking the hook itself. This allows the getter to be run "lazily", after logging is initialized.

  [8db5c52f](https://github.com/bl-sdk/unrealsdk/commit/8db5c52f)

- Fixed `UClass::implements` always returning false. This prevented setting interface properties.

  [18ccdd87](https://github.com/bl-sdk/unrealsdk/commit/18ccdd87)

- Added a `UStruct::superfields` iterator.

  [18ccdd87](https://github.com/bl-sdk/unrealsdk/commit/18ccdd87)

- When accessing a weak object pointer, also check the upper bound of it's object index. Previously,
  it may have been possible to dereference an object beyond the end of GObjects, this now returns
  `nullptr`.

  [38529ae8](https://github.com/bl-sdk/unrealsdk/commit/38529ae8)

- No longer conditionally compile `FText` only under UE4. It's now compiled in all versions, but
  throws versions errors when not available.

  [bbf94676](https://github.com/bl-sdk/unrealsdk/commit/bbf94676)

- Add `unrealsdk::load_package`.

  [70245fac](https://github.com/bl-sdk/unrealsdk/commit/70245fac)

- Make all iterators default-constructable, equivalent to their past-the-end iterators.

  [f09949a4](https://github.com/bl-sdk/unrealsdk/commit/f09949a4)

- Add support for building using standard GCC-based MinGW. This is not tested in CI however, as it
  requires a newer version than that available in Github Actions.

  [1729c749](https://github.com/bl-sdk/unrealsdk/commit/1729c749)

## v1.0.0
- Initial Release
