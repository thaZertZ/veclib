#include <iostream>
#include "veclib.hpp"

int main() {

    std::cout << "veclib::Vector:\n";
    veclib::Vector<int> vector = {1, 2, 3, 4, 5};
    veclib::Vector<int> layer = {2, 4, 6, 8, 10};

    std::cout << "  Vector at 3: " << vector[3] << '\n';
    std::cout << "  Vector.at(3): " << vector.at(3) << '\n';
    std::cout << "  Vector circular at 7: " << vector(7) << '\n';
    std::cout << "  Vector.circular_at(7): " << vector.circular_at(7) << '\n';
    std::cout << "  Vector.first(): " << vector.first() << ", Vector.last(): " << vector.last() << '\n';
    std::cout << "  Vector.size(): " << vector.size() << '\n';

    std::cout << "  Vector iterator:\n";
    for (const int& x : vector) {
        std::cout << "   " << x << '\n';
    }
    std::cout << "  Added Vectors:\n";
    for (const auto& x : (vector + layer)) {
        std::cout << "   " << x << '\n';
    }
    std::cout << "  Prefix incremented:\n";
    for (const int& x : ++vector) {
        std::cout << "   " << x << '\n';
    }
    --vector; // Decrement it
    //std::cout << "  Pushing elements while iterating with index-based iterators:\n";
    //for (auto itr = vector.i_begin(); itr != vector.i_end(); ++itr) {
    //    if (*itr % 2 == 0) vector.push_back(*itr * 2);
    //    std::cout << *itr << '\n';
    //}

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