#include <iostream>
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

    std::cout << "Testing self equality\n";
    EXPECT(slice, slice, "self equality");

    std::cout << "Testing unsafe indexing with safe indeces\n";
    for (int i = 0; i < (int)slice.size(); ++i)
        EXPECT(slice[i], i + 1, "unsafe indexing with safe indeces");

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
