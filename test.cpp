#include <iostream>
#include "veclib.hpp"

int main() {

    std::cout << "veclib::Vector:\n";
    veclib::Vector<int> vector = {1, 2, 3, 4, 5};
    veclib::Vector<int> layer = {2, 4, 6, 8, 10};

    std::cout << "  Vector at 3: " << vector[3] << '\n';
    std::cout << "  Vector.at(3): " << vector.at(3) << '\n';
    std::cout << "  Vector.first(): " << vector.first() << ", Vector.last(): " << vector.last() << '\n';
    std::cout << "  Vector.size(): " << vector.size() << '\n';

    std::cout << "  Vector iterator:\n";
    for (const int& x : vector) {
        std::cout << "   " << x << '\n';
    }
    std::cout << "  Added vectors:\n";
    for (const auto& x : (vector + layer)) {
        std::cout << "   " << x << '\n';
    }
    std::cout << "  Prefix incremented:\n";
    for (const int& x : ++vector) {
        std::cout << "   " << x << '\n';
    }
    --vector; // Decrement it

    veclib::MemSlice<int> slice = vector.slice(3); // 4, 5
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

    veclib::Vector<int> empty = {};
    std::cout << "  Empty vector:\n";
    for (const auto& x : empty) {
        std::cout << "   " << x << '\n';
    }

    return 0;
}