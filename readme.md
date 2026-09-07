
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

- `VECLIB_NONCONSTRUCTOR_NEW(count, type)`: Allocate
  `count` elements on the heap without calling the constructor
  of their specified `type`
- `VECLIB_NONDESTRUCTOR_DELETE(data, count)`: Free `count`
  elements on the heap of the type pointed to by the `data` pointer

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

