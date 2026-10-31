
# Changelog

- Added reverse iterators to all structures
- Added more macro helpers
- Added two more constructors to `Vector` that accept
  `Array` and `Slice` objects to copy
- Added more overloads to `Vector.append()` to support `Slice`
- Added `prepend()` and `insert()` to `Vector` that complement
  `append()` and that support `Vector`, `Array` and `Slice`
  with copy and move operations
- Added `remove_if()` to `Vector` to remove elements if a
  function returns `true`
- Added `find_if()` to `Vector` to return a pointer to an element
  if a function returns `true`. If the element isn't found it
  returns `nullptr`
- Added `extract()` and `extract_copy()` to `Vector` to extract and
  then remove, or return a copied portion of the vector
- Added `split_at()` to `Vector` as a wrapper for `extract` with
  one argument
- Added `rotl()`, `rotr()` and an arbirary `rotate()` to `Vector`
  to rotate elements in-place
- Added `unique()`, `unique_run()`, `unique_all()` and
  `unique_all_runs()` to `Vector` to remove any duplicates following
  various rules based on the method. Check code docs for more info
