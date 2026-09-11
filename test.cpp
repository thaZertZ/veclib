#include <iostream>
#define VECLIB_EXTRA
#include "veclib.hpp"

#define EXPECT(expr, res, str) \
    do { \
        if ((expr) != (res)) throw std::runtime_error("'" str "' failed"); \
    } while (0)

#define ASSERT(expr, str) \
    do { \
        if (!(expr)) throw std::runtime_error("'" str "' failed"); \
    } while (0)

int main() {

    int array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    veclib::Slice<int> slice(array, 10);

    std::cout << "Testing slice length\n";
    EXPECT(slice.size(), 10, "slice length");

    std::cout << "Testing for non-empty\n";
    ASSERT(!slice.empty(), "slice is empty");

    std::cout << "Testing for non-null\n";
    ASSERT(!slice.null(), "slice is null");

    {
        auto slice2 = slice;
        slice2.clear();

        std::cout << "Testing for empty\n";
        ASSERT(slice2.empty(), "slice is not empty");

        std::cout << "Testing for null\n";
        ASSERT(slice2.null(), "slice isn't null");

        std::cout << "Testing for clearance\n";
        ASSERT(slice.cleared(), "slice wasn't cleared");
    }

    std::cout << "Testing unsafe indexing with safe indeces\n";
    for (int i = 0; i < (int)slice.size(); ++i)
        EXPECT(slice[i], i + 1, "unsafe indexing with safe indeces");

    std::cout << "Testing safe indexing with safe indeces\n";
    for (int i = 0; i < (int)slice.size(); ++i)
        EXPECT(slice.at(i), i + 1, "safe indexing with safe indeces");

    std::cout << "Testing circular indexing\n";
    EXPECT(slice[11], 2, "circular indexing");

    std::cout << "Testing clamped indexing\n";
    EXPECT(slice[11], 10, "clamped indexing");

    std::cout << "Testing bounds query\n";
    ASSERT(!slice.inside_bounds(10), "bounds query a");
    ASSERT(slice.inside_bounds(9), "bounds query b");

    std::cout << "Testing single first and last elements\n";
    EXPECT(slice.first(), 1, "single first element");
    EXPECT(slice.last(), 10, "single last element");

    {
        auto first = slice.first(3);
        auto last = slice.last(3);

        std::cout << "Testing multiple first and last elements\n";

        for (int i = 0; i < first.size(); ++i)
            EXPECT(first[i], i + 1, "multiple first elements");

        for (int i = 0; i < last.size(); ++i)
            EXPECT(last[i], i + 8, "multiple last elements");
    }

    {
        auto first = slice.first(0);
        auto last = slice.last(0);

        std::cout << "Testing empty first and last elements\n";

        ASSERT(first.empty(), "empty first elements are not empty");
        ASSERT(last.empty(), "empty last elements are not empty");

        EXPECT(first.get(), slice.get(), "empty first elements point to wrong memory");
        EXPECT(last.get(), slice.get() + slice.size(), "empty last elements point to wrong memory");
    }

    std::cout << "Testing non-indexed mapping\n";
    slice.map([] (int& x) { x *= 2; });
    for (int i = 0; i < slice.size(); ++i)
        EXPECT(slice[i], (i + 1) * 2, "non-indexed mapping");

    std::cout << "Testing self equality\n";
    EXPECT(slice, slice, "self equality");

    std::cout << "Testing unsafe punning with safe arguments\n";
    veclib::Slice<char> punned = slice.into<char>();
    if constexpr (std::endian::little == std::endian::native) {
        EXPECT(punned[0], slice[0], "unsafe punning with safe arguments"); // Endianness exists -_-
        EXPECT(punned[1], 0, "unsafe punning with safe arguments");
        EXPECT(punned[2], 0, "unsafe punning with safe arguments");
        EXPECT(punned[3], 0, "unsafe punning with safe arguments");
    } else {
        EXPECT(punned[0], 0, "unsafe punning with safe arguments");
        EXPECT(punned[1], 0, "unsafe punning with safe arguments");
        EXPECT(punned[2], 0, "unsafe punning with safe arguments");
        EXPECT(punned[3], slice[0], "unsafe punning with safe arguments");
    }

    return 0;
}
