
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
  of their specified `type`
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

**Note:** All types and aliases defined by veclib
are namespaced under `veclib`.

We can always get the size of any container in
veclib by calling the `.size()` method on it, and
receive a read-only pointer to its internal memory
by calling `.get()`.  
**Note:** This might be changed to `.data()` to mirror
the standard library in the future.

### `diff_t`

A type alias of `std::make_signed_t<std::size_t>`
used as a sized signed integer.

### `Array`

This simple static array class is similar to `std::array`,
with some extra functionality inspired by other languages like Rust:

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
    foo += bar; // Scalar values
    foo /= 2;
    veclib::Array<int, 5> baz = foo * 3;
    baz -= foo; // And individual elements
    foo += baz;

    // It also supports comparison
    if (foo == 0) std::cout << "All zeroes!\n"; // Scalar
    if (foo != baz) std::cout << "Not equal!\n";
}
```

The methods that take in lambdas all allow for them
to have their last parameter be `std::size_t`
(given that the previous arguments are the ones required
by the method) that will hold the index of the
element being processed.

<!--
Here put Vector
-->

### `MemSlice`

The `MemSlice` class references a set of elements of
an arbitrary type in contiguous memory, similar to `std::span`.

Here is some examples on how it can be used:
```cpp
#include <iostream>
#include "veclib.hpp"

int main() {
    veclib::Array<int, 10> foo;
    foo.map([] (int& x, std::size_t i) { x = i + 1; }); // Set it up

    veclib::MemSlice<int> slice = foo.slice(); // Covers the whole array
    slice = foo.slice(2); // From index 2 to the end
    slice = foo.slice(0, 5); // From index 0 to 5 exclusive

    slice.grow(2); // Inreases the number of referenced elements, same as `+=`
    slice.shrink(2) // Decreases the number of referenced elements, same as `-=`
    slice.slide_forw(2) // Shift the whole view forward, same as `>>=`
    slice.extend(2) // Add in-memory preceding elements to the view, same as `|=`
    slice.trim(2) // Remove front elements from the view, same as `/=`

    slice.consume_front(); // Consumes the first element, same as prefix `++`
    // These methods allow us to do chained operations
    slice.consume_back().slide_forw(1);
    //    ^^^ Consumes the last element, same as prefix `--`

    // No bounds checking
    slice[2] += 5;
    // With bounds checking
    //slice.at(100) = 0;

    // It of course supports iterators
    for (const int& x : slice)
        std::cout << x << '\n';
}
```

The `MemSlice` class is also aliased to `Slice` for simplicity.

As you might have deduced from this example, the
`MemSlice` class provides a handful of methods for manipulating
the view of the slice, which are also mapped to operator
overloads by default.
However, if you don't want to have extra cryptic overloads
you can define the `VECLIB_NO_OPERATOR_OVERLOADS` macro
mentioned [earlier](#macros);

Here is the full list of method names and their respective
operators:

| Method          | Overload    | Description |
| --------------- | ----------- | ----------- |
| `grow`          | `+=`        | Increase the number of referenced elements |
| `shrink`        | `-=`        | Decrease the number of referenced elements |
| `resize`        | *none*      | Uses `diff_t` to work as `grow` and `shrink` |
| `slide_forw`    | `>>=`       | Slide the whole view forward |
| `slide_backw`   | `<<=`       | Slide the whole view backward |
| `slide`         | *none*      | Uses `diff_t` to work as both `slide_` methods |
| `trim`          | `/=`        | Remove elements at the front of the slice from the view |
| `extend`        | `\|=`        | Add elements in front of the view
| `nudge`         | *none*      | Uses `diff_t` to work as `trim` and `extend` |
| `consume_front` | prefix `++` | Consume the first element |
| `consume_back`  | prefix `--` | Consume the last element |

All of these methods have a variant with `_copy`
appended to the name (eg. `grow_copy`) which return a
copy of the current slice on which the operation was
performed.  
In terms of operator overloads, all of the assignment
ones map the `_copy` variant to the binary version
of that operator (eg. `+=` becomes `+`).  
The prefix operators achieve this variant by using
the unary prefix version of them (prefix `++`
becomes unary `+` and prefix `--` becomes unary
`-`).
