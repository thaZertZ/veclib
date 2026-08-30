
# veclib

Veclib is a single-header C++ library for
array-like structures (arrays, vectors,
slices, lists...).

You can find your basic static array class
(`Array`), dynamic array class (`Vector`),
slice (`MemSlice`) and list (`LinkedList`),
but there are also some more complex data
structures like `LinkedArray` or `StillVector`.

## Macros

In veclib we can change the behaviour of certain features
by defining some macros:

- `VECLIB_ASSERT_NOEXCEPT`: disable exceptions and use
  asserts instead
- `VECLIB_NO_OPERATOR_OVERLOADS`: don't define extra
  operator overloads that may clutter code

Some macros are instead used as internal utilities
but are left defined for others to use too:

- `VECLIB_NONCONSTRUCTOR_NEW(count, type)`: Allocate
  `count` elements on the heap without calling the constructor
  of their specified type
- `VECLIB_NONDESTRUCTOR_DELETE(data, count, type)`: Free `count`
  elements of type `type` on the heap at the `data` pointer

Example use:
```cpp
#include "veclib.hpp"
// Imagine we have this class
class foo;

int main() {
    foo* bar = VECLIB_NONCONSTRUCTOR_NEW(1, foo);
    // Do something with it
    VECLIB_NONDESTRUCTOR_DELETE(bar, 1, foo);
}
```

## Details

Here we'll cover the functionality of every data
structure and type defined by veclib.

### `diff_t`

A type alias of `std::make_signed_t<std::size_t>`
used as a sized signed integer.

### `Array`

This simple static array class is similar to `std::array`,
with some extra functionality:

```cpp
#include <iostream>
#include "veclib.hpp"

int main() {
    veclib::Array<int, 5> foo = {1, 2, 3, 4, 5};

    // No bounds checking
    foo[2] += 5;
    // With bounds checking
    //foo.at(100) = 0;

    foo.first() = foo.last();

    // Iterators are of course supported
    for (const int& x : foo)
        std::cout << x << '\n';

    // And we have mapping functionality
    foo.map([] (int& x) {
        x *= 2;
    });

    // We can also receive an index into the array while we are mapping
    foo.map([] (int& x, std::size_t i) {
        x = i + 1;
    });

    // Sum all elements
    int bar = foo.fold([] (int& acc, const int& x) {
        // `acc` is the accumulated value
        acc += x;
    });

    // For numeric types only it supports arithmetic operations
    foo += bar;
    foo /= 2;
    veclib::Array<int, 5> baz = foo * 3;
}
```
