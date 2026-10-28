
# Changelog

- Added more global macros for checking full copy or move
  operations and if they are `noexcept`
- Removed all `_copy()` methods from `Slice` and added a
  `copy()` method to compensate
- Fixed a typo in `Slice.subslice_len()` documentation
- Removed `Array.moved()`
- Changed `Vector` iterator-based constructor to exclude the
  last element, whether it's the beginning or ending pointer
- Changed `Vector`'s previously called `grow()` method to
  `grow_reserve()` to better reflect behaviour and added a 
  `grow()` method working like `resize()` with the same two
  overloads
- Added an in-place `reverse()` method to `Vector` that chooses
  moving or copying based on a complex policy
- Changed some `noexcept` requirements in `Vector.contains()`
- Added a `how_many()` method in `Vector` to count occurrences
  of a value
- Overloaded `Vector.fill()` to allow copy assignment, destruction
  and copy construction, and destruction and designated
  construction using variadic template arguments like `resize()`
