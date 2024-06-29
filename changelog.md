# Changelog

## v1.2.0 (Upcoming)
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
  lock at the same time as the thread holding that lock tries to (transitively) call an unreal
  function (which will attempt to acquire the process event lock).

  [35857adf](https://github.com/bl-sdk/unrealsdk/commit/35857adf),
  [b9469bbf](https://github.com/bl-sdk/unrealsdk/commit/b9469bbf)


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
