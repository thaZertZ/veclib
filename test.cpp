#include <iostream>
#include "veclib.hpp"

int main() {

    std::cout << "veclib::Array:\n";
    veclib::Array<int, 5> array = {1, 2, 3, 4, 5};
    veclib::Array<int, 5> layer = {2, 4, 6, 8, 10};

    std::cout << "  Array at 3: " << array[3] << '\n';
    std::cout << "  Array.at(3): " << array.at(3) << '\n';
    std::cout << "  Array.first(): " << array.first() << ", Array.last(): " << array.last() << '\n';
    std::cout << "  Array.size(): " << array.size() << '\n';

    std::cout << "  Array iterator:\n";
    for (const int& x : array) {
        std::cout << "   " << x << '\n';
    }
    std::cout << "  Added arrays:\n";
    for (const auto& x : (array + layer)) {
        std::cout << "   " << x << '\n';
    }
    std::cout << "  Prefix incremented:\n";
    for (const int& x : ++array) {
        std::cout << "   " << x << '\n';
    }
    --array; // Decrement it

    veclib::MemSlice<int> slice = array.slice(3); // 4, 5
    std::cout << "  Sliced from 3:\n";
    for (const int& x : slice) {
        std::cout << "   " << x << '\n';
    }
    slice <<= 2; // 2, 3
    std::cout << "  Slid left by 2:\n";
    for (const int& x : slice) {
        std::cout << "   " << x << '\n';
    }
    slice += 1; // 2, 3, 4
    std::cout << "  Grown by 1:\n";
    for (const int& x : slice) {
        std::cout << "   " << x << '\n';
    }
    std::cout << "  Consumed front (copy):\n";
    for (const int& x : +slice) { // 3, 4
        std::cout << "   " << x << '\n';
    }
    std::cout << "  Consumed back (copy):\n";
    for (const int& x : -slice) { // 2, 3
        std::cout << "   " << x << '\n';
    }

    veclib::Array<int, 0> empty = {};
    std::cout << "  Empty array:\n";
    for (const auto& x : empty) {
        std::cout << "   " << x << '\n';
    }

    return 0;
}