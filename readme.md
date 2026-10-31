
# veclib

Veclib is a single-header C++ library for
array-like structures (arrays, vectors,
slices, lists...).

You can find your basic static array class
(`Array`), dynamic array class (`Vector`),
slice (`Slice`) and list (`LinkedList`),
but there are also some more complex data
structures like `LinkedArray` or `StillVector`.

## Macros

In veclib we can change the behaviour of certain features
by defining some macros:

- `VECLIB_ASSERT_NOEXCEPT`: disable exceptions and use
  asserts instead, this also defines `VECLIB_NOEXCEPT`,
  a macro that expands to `noexcept` if this is defined
- `VECLIB_NO_OPERATOR_OVERLOADS`: don't define extra
  operator overloads that may clutter code
- `VECLIB_EXTRA`: define extra methods for some data
  structures which could be used in niche cases but are
  not defined by default to avoid unnecessary clutter

Some macros are instead used as internal utilities
but are left defined for others to use too:

- `VECLIB_NONCONSTRUCTOR_NEW(type, count)`: allocate
  `count` elements on the heap without calling the constructor
  of their specified `type`
- `VECLIB_NONDESTRUCTOR_DELETE(data, count)`: free `count`
  elements on the heap of the type pointed to by the `data` pointer
- `VECLIB_EXPLICIT_NONDESTRUCTOR_DELETE(data, type, count)`: free
  `count` elements on the heap without calling the destructor of
  the explicitly specified `type`, pointed to by the `data` pointer

Example use:
```cpp
#include "veclib.hpp"
// Imagine we have this class
class foo;

int main() {
    foo* bar = VECLIB_NONCONSTRUCTOR_NEW(foo, 1);
    // Do something with it
    VECLIB_NONDESTRUCTOR_DELETE(bar, 1);
}
```

## `Slice`

The `Slice` type is a powerful and ergonomic slice class
that also provides various utility methods for niche cases.  
Here are a few uses of it with various of its methods:

```cpp
#define VECLIB_EXTRA
#include "veclib.hpp"

int main() {
    int array[5] = {1, 2, 3, 4, 5};
    veclib::Slice<int> slice(array, 5); // Provide a pointer and a length

    slice[4] = 0; // No bounds checking
    slice.at(4) = 5; // With bounds checking
    slice.first() = slice.last(); // Unlike standard containers' front() and back()
    slice.circular_at(5) = 1; // Prevent out-of-bounds access
    slice(9) = 0; // Operator overload for it
    slice.clamped_at(69) = 5; // Clamp the index to the maximum size

    const auto s = slice.first(3); // A slice to the first 3 elements
    const auto t = s.last(2); // A slice to the last 2 elements
    bool valid = slice.inside_bounds(42); // Check if an index is valid

    slice.map([] (int& x) { x *= 2; }); // Multiply each element by 2
    // Sum all elements with an initial accumulator value of 10
    int sum = slice.fold([] (int& acc, const int& x) { acc += x; }, 10);
    for (int& x : slice) x /= 2; // Iterator support of course

    // Check that every element is less than 42
    bool all = slice.all([] (const int& x) { return x < 42; });
    // Check if some elements are even
    bool any = slice.any([] (const int& x) { return x % 2 == 0; });
    // Check that no elements are greater than 69
    bool none = slice.none([] (const int& x) { return x > 69 });
    slice.fill(0); // Fill with one value
    bool scalar = slice == 0; // Check if all values are equal;
    // All functions that accept lambdas support passing the index of the element
    slice.map([] (int& x, std::size_t index) { x = index + 1; });
    bool elementwise = slice == slice; // Element-wise comparison

    // Check if a pointer to an element is in the memory range
    bool ptr = slice.contains(&slice[2]);
    bool value = slice.contains(2); // Check if a value is contained
    bool overlap = slice.overlaps(s); // Check if memory ranges overlap
    bool range = slice.contains(t); // Check if t's memory range is contained in slice's
    const auto intersection = s.intersect(t); // The intersection of two memory ranges
    const auto joined = s.join(t); // The union of two memory ranges
    // Split the slice, returning the right half and keeling the left one
    const auto split = slice.split_at(2);
    const auto punned = slice.into<char>(); // Type-pun the slice without checks
    const auto safer = slice.strict_into<char>(); // Same punning with alignment checks

    // Subslice with a starting index and an exclusive ending index
    const auto sub = slice.subslice(1, 4);
    const auto len = slice.subslice_len(1, 2); // Use a starting index and a capped length
    // Trim elements from the front while this function returns true
    sub.trim_while([] (const int&, std::size_t index) { return index == 0; });
    len.trim_leading(2); // Trim every leading occurrence of 2
    // Trim until x is 4
    sub.trim_until([] (const int& x) { return x == 4; });
}
```

