#ifndef VECLIB_HPP
#define VECLIB_HPP

#include <cstdint>
#ifdef VECLIB_ASSERT_NOEXCEPT // Use asserts instead of throwing exceptions
#include <cassert>
#else // VECLIB_ASSERT_NOEXCEPT
#include <stdexcept>
#endif // VECLIB_ASSERT_NOEXCEPT
#include <utility> // std::move(), std::forward()
#include <initializer_list>

/// @brief A collection of array data structures (arrays, vectors, lists...)
namespace veclib {

/// @brief Alias for the signed equivalent of `std::size_t`
using diff_t = std::make_signed_t<std::size_t>;

/// @brief Macro function to allocate heap memory without calling
///        the constructor of the specified type
/// @param count The number of items of the type to allocate
/// @param type The type of items to allocate
#define VECLIB_NONCONSTRUCTOR_NEW(count, type) \
    (reinterpret_cast<type*>(::operator new((count) * sizeof(type))))
/// @brief Macro function to deallocate heap memory without calling
///        the destructor of the specified type
/// @param data The pointer to the memory to deallocate
/// @param count The number of items to deallocate
/// @param type The type of the items to deallocate
#define VECLIB_NONDESTRUCTOR_DELETE(data, count, type) \
    (::operator delete((data), (count) * sizeof(type)))

/// @brief A reverse iterator for contiguous memory regions
/// @tparam Type The type begin iterated over
template <typename Type>
class ReverseMemIterator {
private:
    Type* data = nullptr;

public:
    /// @brief Default constructor
    ReverseMemIterator() noexcept = default;
    /// @brief Default destructor
    ~ReverseMemIterator() noexcept = default;

    /// @brief Construct a `ReverseMemIterator` object from
    ///        a pointer of the type being iterated over 
    /// @param d The pointer from which to construct the object
    ReverseMemIterator(Type* d) noexcept
        : data(d) {}

    /// @brief Receive the underlying pointer as read-only
    /// @return The underlying data pointer
    inline constexpr const Type* get() const noexcept { return data; }

    /// @brief Prefix-increment this iterator
    /// @return A reference to this iterator object once modified
    inline constexpr ReverseMemIterator<Type>& operator++() noexcept {
        --data;
        return *this; // Does this throw if data is nullptr?
    }
    /// @brief Postfix-increment this iterator 
    /// @return A copy of the iterator before being incremented
    inline constexpr ReverseMemIterator<Type> operator++(int) noexcept {
        ReverseMemIterator<Type> self = *this;
        --data;
        return self;
    }

    /// @brief Prefix-decrement this iterator
    /// @return A reference to this iterator object once modified
    inline constexpr ReverseMemIterator<Type>& operator--() noexcept {
        ++data;
        return *this; // Does this throw if data is nullptr?
    }
    /// @brief Postfix-increment this iterator 
    /// @return A copy of the iterator before being incremented
    inline constexpr ReverseMemIterator<Type> operator--(int) noexcept {
        ReverseMemIterator<Type> self = *this;
        ++data;
        return self;
    }

    /// @brief Increment this iterator
    /// @param x The number of items of the type being pointed
    ///          to to increment this iterator by
    /// @return A reference to the incremented iterator
    inline constexpr ReverseMemIterator<Type>& operator+=(std::size_t x) noexcept {
        data -= x;
        return *this;
    }

    /// @brief Decrement this iterator
    /// @param x The number of items of the type being pointed
    ///          to to decrement this iterator by
    /// @return A reference to the decremented iterator
    inline constexpr ReverseMemIterator<Type>& operator-=(std::size_t x) noexcept {
        data += x;
        return *this;
    }

    /// @brief Add a number of items of the type being pointed
    ///        to to this iterator
    /// @param x The number of items to add
    /// @return A copy of this iterator pointing to the new location
    ///         provided by `x`
    inline constexpr ReverseMemIterator<Type> operator+(std::size_t x) noexcept {
        ReverseMemIterator<Type> self = *this;
        self.data -= x;
        return self;
    }

    /// @brief Subtract a number of items of the type being pointed
    ///        to to this iterator
    /// @param x The number of items to subtract
    /// @return A copy of this iterator pointing to the new location
    ///         provided by `x`
    inline constexpr ReverseMemIterator<Type> operator-(std::size_t x) noexcept {
        ReverseMemIterator<Type> self = *this;
        self.data += x;
        return self;
    }

    /// @brief Access the element being pointed to
    /// @return A reference to the element being pointed to
    /// @throws `std::runtime_error` if the underlying data pointer is `nullptr`,
    ///         if exceptions are not disabled by defining `VECLIB_ASSERT_NOEXCEPT`,
    ///         otherwise an assert will fail
    inline constexpr Type& operator*() const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data != nullptr);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr)
            throw std::runtime_error("ReverseMemIterator<Type>::operator*(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return *data;
    }

    /// @brief Compare two iterators for equality
    /// @param other The other iterator to compare
    /// @return `true` if the data pointers of the two iterators are the same
    inline constexpr bool operator==(const ReverseMemIterator<Type>& other) const noexcept {
        return data == other.data;
    }
    /// @brief Compare two iterators for inequality
    /// @param other The other iterator to compare
    /// @return `true` if the data pointers of the two iterators are not the same
    inline constexpr bool operator!=(const ReverseMemIterator<Type>& other) const noexcept {
        return data != other.data;
    }

    /// @brief Convert an iterator object to a boolean value,
    ///        `true` if the data pointer is not `nullptr`
    inline constexpr operator bool() const noexcept {
        return data != nullptr;
    }
};

/// @brief Memory slice class
/// @tparam Type The type of element being referenced
template <typename Type>
class MemSlice {
private:
    Type* data = nullptr;
    std::size_t count = 0;

public:
    MemSlice() noexcept = default;
    ~MemSlice() noexcept = default;

    MemSlice(Type* p, std::size_t s) : data(p), count(s) {}

    inline constexpr const Type* get() const noexcept { return data; }
    inline constexpr std::size_t size() const noexcept { return count; }

    inline constexpr Type& operator[](std::size_t i) noexcept { return data[i]; }
    inline constexpr const Type& operator[](std::size_t i) const noexcept { return data[i]; }

    inline constexpr Type& at(std::size_t i) {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(i < count);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (i >= count) throw std::out_of_range("MemSlice<Type>.at(std::size_t): Index is out of bounds");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return data[i];
    }
    inline constexpr const Type& at(std::size_t i) const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(i < count);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (i >= count) throw std::out_of_range("MemSlice<Type>.at(std::size_t): Index is out of bounds");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return data[i];
    }

    // slice+=     change size            grow()              } -----+
    // slice+      new changed size       grow_copy()        } ----+ |---- resize() => universal and signed
    // slice-=     change size            shrink()            } --|--+
    // slice-      new changed size       shrink_copy()      } ----+------- resize_copy() => universal and signed

    // slice<<=    change ptr             slide_backw()        } -----+
    // slice<<     new changed ptr        slide_backw_copy()  } ----+ |---- slide() => universal and signed
    // slice>>=    change ptr             slide_forw()         } --|--+
    // slice>>     new changed ptr        slide_forw_copy()   } ----+------- slide_copy() => universal and signed

    // slice/=     move head              trim()
    // slice/      new moved head         trim_copy()

    // optional:
    // ++slice     move head by 1         consume_front()
    // +slice      new moved head by 1    consume_front_copy()
    // --slice     move tail by -1        consume_back()
    // -slice      new moved tail by -1   consume_back_copy()

    /// @brief Move the end of the slice forward while keeping the head still
    /// @param x The amount of new elements to widen the window by
    /// @return A reference to the grown slice
    inline constexpr MemSlice<Type>& grow(std::size_t x) noexcept {
        count += x;
        return *this;
    }
    /// @brief Move the end of the slice forward while keeping the head still
    /// @param x The amount of new elements to widen the window by
    /// @return A copy of this slice on which the operation was performed
    inline constexpr MemSlice<Type> grow_copy(std::size_t x) const noexcept {
        MemSlice<Type> self = *this;
        self.grow(x);
        return self;
    }
    /// @brief Move the end of the slice backward while keeping the head still
    /// @param x The amount of new elements to shrink the window by
    /// @return A reference to the grown slice
    /// @throws `std::underflow_error` if `x` is greater than the slice size
    inline constexpr MemSlice<Type>& shrink(std::size_t x) {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(x <= count);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (x > count)
            throw std::underflow_error("MemSlice<Type>.shrink(std::size_t): Shrinking by too much");
        #endif // VECLIB_ASSERT_NOEXCEPT
        count -= x;
        return *this;
    }
    /// @brief Move the end of the slice backward while keeping the head still
    /// @param x The amount of new elements to shrink the window by
    /// @return A copy of this slice on which the operation was performed
    /// @throws `std::underflow_error` if `x` is greater than the slice size
    inline constexpr MemSlice<Type> shrink_copy(std::size_t x) const {
        MemSlice<Type> self = *this;
        self.shrink(x);
        return self;
    }
    /// @brief Resize arbitrarily this slice by moving the end of the slice
    ///        forward or backward
    /// @param x The amount of elements to consider during the operation
    /// @return A reference to the modified slice
    /// @throws `std::underflow_error` if `x` is negative and its absolute
    ///         value is greater than the slice size
    inline constexpr MemSlice<Type>& resize(diff_t x) {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(count + x >= 0);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (count + x < 0)
            throw std::underflow_error("MemSlice<Type>.resize(diff_t): Shrinking by too much");
        #endif // VECLIB_ASSERT_NOEXCEPT
        count += x;
    }
    /// @brief Resize arbitrarily this slice by moving the end of the slice
    ///        forward or backward
    /// @param x The amount of elements to consider during the operation
    /// @return A copy of this slice on which the operation was performed
    /// @throws `std::underflow_error` if `x` is negative and its absolute
    ///         value is greater than the slice size
    inline constexpr MemSlice<Type> resize_copy(diff_t x) const {
        MemSlice<Type> self = *this;
        self.resize(x);
        return self;
    }

    /// @brief Slide the whole window backwards
    /// @param x The amount of elements to slide the window by
    /// @return A reference to the modified slice
    inline constexpr MemSlice<Type>& slide_backw(std::size_t x) noexcept {
        data -= x;
        return *this;
    }
    /// @brief Slide the whole window backwards
    /// @param x The amount of elements to slide the window by
    /// @return A copy of this slice on which the operation was performed
    inline constexpr MemSlice<Type> slide_backw_copy(std::size_t x) const noexcept {
        MemSlice<Type> self = *this;
        self.slide_backw(x);
        return self;
    }
    /// @brief Slide the whole window forward
    /// @param x The amount of elements to slide the window by
    /// @return A reference to the modified slice
    inline constexpr MemSlice<Type>& slide_forw(std::size_t x) noexcept {
        data += x;
        return *this;
    }
    /// @brief Slide the whole window backwards
    /// @param x The amount of elements to slide the window by
    /// @return A copy of this slice on which the operation was performed
    inline constexpr MemSlice<Type> slide_forw_copy(std::size_t x) const noexcept {
        MemSlice<Type> self = *this;
        self.slide_forw(x);
        return self;
    }
    /// @brief Slide the whole window forward or backward
    /// @param x The number of elements to slide the window by (signed)
    /// @return A reference to the modified slice
    inline constexpr MemSlice<Type>& slide(diff_t x) noexcept {
        data += x;
        return *this;
    }
    /// @brief Slide the whole window forward or backward
    /// @param x The number of elements to slide the window by (signed)
    /// @return A copy of this slice on which the operation was performed
    inline constexpr MemSlice<Type> slide_copy(diff_t x) const noexcept {
        MemSlice<Type> self = *this;
        self.slide(x);
        return self;
    }

    /// @brief Move the head of the slice while keeping the last element still
    /// @param x The number of elements to move the head by
    /// @return A reference to the modified slice
    inline constexpr MemSlice<Type>& trim(std::size_t x) {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(x <= count);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (x > count) throw std::overflow_error("MemSlice<Type>.trim(std::size_t): Can't trim past slice size");
        #endif // VECLIB_ASSERT_NOEXCEPT
        data += x;
        count -= x;
    }
    /// @brief Move the head of the slice while keeping the last element still
    /// @param x The number of elements to move the head by
    /// @return A copy of this slice on which the operation was performed
    inline constexpr MemSlice<Type> trim_copy(std::size_t x) {
        MemSlice<Type> self = *this;
        self.trim(x);
        return self;
    }

    /// @brief Consume the first element of the slice while keeping the last one still
    /// @return A reference to the modified slice
    inline constexpr MemSlice<Type>& consume_front() {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(count != 0);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (count == 0)
            throw std::underflow_error("MemSlice<Type>.consume_front(): Can't consume since size is 0");
        #endif // VECLIB_ASSERT_NOEXCEPT
        ++data;
        --count;
        return *this;
    }
    /// @brief Consume the first element of the slice while keeping the last one still
    /// @return A copy of this slice on which the operation was performed
    inline constexpr MemSlice<Type> consume_front_copy() {
        MemSlice<Type> self = *this;
        self.consume_front();
        return self;
    }
    /// @brief Consume the last element of the slice while keeping the first one still
    /// @return A reference to the modified slice
    inline constexpr MemSlice<Type>& consume_back() {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(count != 0);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (count == 0)
            throw std::underflow_error("MemSlice<Type>.consume_back(): Can't consume since size is 0");
        #endif // VECLIB_ASSERT_NOEXCEPT
        --count;
        return *this;
    }
    /// @brief Consume the last element of the slice while keeping the first one still
    /// @return A copy of this slice on which the operation was performed
    inline constexpr MemSlice<Type> consume_back_copy() {
        MemSlice<Type> self = *this;
        self.consume_back();
        return self;
    }

    /// @brief Operator overload for calling `grow(x)`
    inline constexpr MemSlice<Type>& operator+=(std::size_t x) noexcept { return grow(x); }
    /// @brief Operator overload for calling `grow_copy(x)`
    inline constexpr MemSlice<Type> operator+(std::size_t x) const noexcept { return grow_copy(x); }
    /// @brief Operator overload for calling `shrink(x)`
    inline constexpr MemSlice<Type>& operator-=(std::size_t x) { return shrink(x); }
    /// @brief Operator overload for calling `shrink_copy(x)`
    inline constexpr MemSlice<Type> operator-(std::size_t x) const { return shrink_copy(x); }

    /// @brief Operator overload for calling `slide_backw(x)`
    inline constexpr MemSlice<Type>& operator<<=(std::size_t x) noexcept { return slide_backw(x); }
    /// @brief Operator overload for calling `slide_backw_copy(x)`
    inline constexpr MemSlice<Type> operator<<(std::size_t x) const noexcept { return slide_backw_copy(x); }
    /// @brief Operator overload for calling `slide_forw(x)`
    inline constexpr MemSlice<Type>& operator>>=(std::size_t x) noexcept { return slide_forw(x); }
    /// @brief Operator overload for calling `slide_forw_copy(x)`
    inline constexpr MemSlice<Type> operator>>(std::size_t x) const noexcept { return slide_forw_copy(x); }

    /// @brief Operator overload for calling `trim(x)`
    inline constexpr MemSlice<Type>& operator/=(std::size_t x) { return trim(x); }
    /// @brief Operator overload for calling `trim_copy(x)`
    inline constexpr MemSlice<Type> operator/(std::size_t x) { return trim_copy(x); }

    /// @brief Operator overload for calling `consume_front()`
    inline constexpr MemSlice<Type>& operator++() { return consume_front(); }
    /// @brief Operator overload for calling `consume_front_copy()`
    inline constexpr MemSlice<Type> operator+() { return consume_front_copy(); }
    /// @brief Operator overload for calling `consume_back()`
    inline constexpr MemSlice<Type>& operator--() { return consume_back(); }
    /// @brief Operator overload for calling `consume_back_copy()`
    inline constexpr MemSlice<Type> operator-() { return consume_back_copy(); }

    /// @brief Receive a forward iterator to the first element
    /// @return A pointer to the first element
    inline constexpr Type* begin() noexcept { return data; }
    /// @brief Receive a const forward iterator to the first element
    /// @return A const pointer to the first element
    inline constexpr const Type* begin() const noexcept { return data; }

    /// @brief Receive a forward iterator past the last element
    /// @return A pointer pointing past the last element
    inline constexpr Type* end() noexcept { return data + count; }
    /// @brief Receive a const forward iterator past the last element
    /// @return A const pointer pointing past the last element
    inline constexpr const Type* end() const noexcept { return data + count; }

    /// @brief Receive a reverse iterator to the first element
    /// @return A reverse iterator object pointing to the first element
    inline constexpr ReverseMemIterator<Type> rbegin() noexcept {
        return ReverseMemIterator<Type>(data + count - 1);
    }
    /// @brief Receive a const reverse iterator to the first element
    /// @return A const reverse iterator object pointing to the first element
    inline constexpr const ReverseMemIterator<Type> rbegin() const noexcept {
        return ReverseMemIterator<Type>(data + count - 1);
    }

    /// @brief Receive a reverse iterator past the last element
    /// @return A reverse iterator object pointing past the last element
    inline constexpr ReverseMemIterator<Type> rend() noexcept {
        return ReverseMemIterator<Type>(data - 1);
    }
    /// @brief Receive a const reverse iterator past the last element
    /// @return A const reverse iterator object pointing past the last element
    inline constexpr const ReverseMemIterator<Type> rend() const noexcept {
        return ReverseMemIterator<Type>(data - 1);
    }

    /// @brief Receive a const forward iterator to the first element
    /// @return A const pointer to the first element
    inline constexpr const Type* cbegin() const noexcept { return data; }

    /// @brief Receive a const forward iterator past the last element
    /// @return A const pointer pointing past the last element
    inline constexpr const Type* cend() const noexcept { return data + count; }

    /// @brief Receive a const reverse iterator to the first element
    /// @return A const reverse iterator object pointing to the first element
    inline constexpr const ReverseMemIterator<Type> crbegin() const noexcept {
        return ReverseMemIterator<Type>(data + count - 1);
    }

    /// @brief Receive a const reverse iterator past the last element
    /// @return A const reverse iterator object pointing past the last element
    inline constexpr const ReverseMemIterator<Type> crend() const noexcept {
        return ReverseMemIterator<Type>(data - 1);
    }
};

/// @brief Static array class
/// @tparam Type The type of each element in the array
/// @tparam Size The size of the array
template <typename Type, std::size_t Size>
class Array {
private:
    Type data[Size] = {};

public:
    /// @brief Default constructor
    Array() noexcept = default;
    /// @brief Default destructor
    ~Array() noexcept = default;

    /// @brief Construct an `Array` object with a list of `Type` elements
    /// @param args The list of elements
    /// @throws `std::invalid_argument` if the size of the provided list is
    ///         greater than the size specified by the template parameter,
    ///         if exceptions are disabled (by defining VECLIB_ASSERT_NOEXCEPT)
    ///         an assert fails
    Array(const std::initializer_list<Type>& args) {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(args.size() <= Size);
        #else // VECLIB_ASSERT_NOEXCEPT
        // Maybe std::overflow_error instead?
        if (args.size() > Size) throw std::invalid_argument(
        "Array<Type, Size>::Array(const std::initializer_list<Type>&): Initializer list is too large");
        #endif // VECLIB_ASSERT_NOEXCEPT
        for (std::size_t i = 0; i < args.size(); ++i)
            data[i] = args.begin()[i];
    }
    /// @brief Construct an `Array` object with a list of `Type` elements
    /// @param args The list of elements
    /// @throws `std::invalid_argument` if the size of the provided list is
    ///         greater than the size specified by the template parameter,
    ///         if exceptions are disabled (by defining VECLIB_ASSERT_NOEXCEPT)
    ///         an assert fails
    /// @return A reference to the constructed object
    Array<Type, Size>& operator=(const std::initializer_list<const Type&>& args) {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(args.size() <= Size);
        #else // VECLIB_ASSERT_NOEXCEPT
        // Maybe std::overflow_error instead?
        if (args.size() > Size) throw std::invalid_argument(
        "Array<Type, Size>::Array(const std::initializer_list<Type>&): Initializer list is too large");
        #endif // VECLIB_ASSERT_NOEXCEPT
        for (std::size_t i = 0; i < args.size(); ++i)
            data[i] = args.begin()[i];
        return *this;
    }

    /// @brief Construct an `Array` object by copying a `value`
    ///        `Size` times throughout the array
    /// @param value The value to copy
    Array(const Type& value) noexcept {
        for (std::size_t i = 0; i < Size; ++i)
            data[i] = value;
    }
    /// @brief Construct an `Array` object by copying a `value`
    ///        `Size` times throughout the array
    /// @param value The value to copy
    /// @return A reference to the constructed object
    Array<Type, Size>& operator=(const Type& value) noexcept {
        for (std::size_t i = 0; i < Size; ++i)
            data[i] = value;
        return *this;
    }

    /// @brief Copy constructor
    /// @param other The object to copy
    Array(const Array<Type, Size>& other) noexcept {
        // We don't need to assert for the size
        for (std::size_t i = 0; i < Size; ++i)
            data[i] = other.data[i];
    }
    /// @brief Copy assigment
    /// @param other The object to copy
    /// @return A reference to the constructed object
    Array<Type, Size>& operator=(const Array<Type, Size>& other) noexcept {
        for (std::size_t i = 0; i < Size; ++i)
            data[i] = other.data[i];
        return *this;
    }

    /// @brief Move constructor
    /// @param other The object to move
    Array(Array<Type, Size>&& other) noexcept {
        for (std::size_t i = 0; i < Size; ++i) {
            data[i] = std::move(other.data[i]);
            //other.data[i].~Type(); // Needed?
        }
        // Or do this?
        //other.data = nullptr;
    }
    /// @brief Move assigment
    /// @param other The object to move
    /// @return A reference to the constructed object
    Array<Type, Size>& operator=(Array<Type, Size>& other) noexcept {
        for (std::size_t i = 0; i < Size; ++i) {
            data[i] = std::move(other.data[i]);
            //other.data[i].~Type();
        }
        //other.data = nullptr;
        return *this;
    }

    /// @brief Receive a read-only pointer to the underlying array
    /// @return A pointer to the start of the array
    inline constexpr const Type* get() const noexcept { return data; }
    /// @brief Evaluates to the `Size` template argument
    /// @return The size of the array
    inline constexpr std::size_t size() const noexcept { return Size; }

    /// @brief Construct a `MemSlice` object out of this array, optionally
    ///        specifying start and end indeces (end indeces are exclusive)
    /// @return A `MemSlice` object referencing the whole array or part of it
    inline constexpr MemSlice<Type> slice(std::size_t start = 0, std::size_t end = Size) {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(end >= start);
        assert(start < Size);
        assert(end <= Size);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (end < start) throw std::out_of_range(
            "Array<Type, Size>.slice(std::size_t, std::size_t): End index is smaller than start index");
        if (start >= Size) throw std::out_of_range(
            "Array<Type, Size>.slice(std::size_t, std::size_t): Start index is out of bounds");
        if (end > Size) throw std::out_of_range(
            "Array<Type, Size>.slice(std::size_t, std::size_t): End index is out of bounds");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return MemSlice<Type>(data + start, end - start);
    }

    /// @brief Receive a reference to the first element in the array
    /// @return A reference to the first element
    inline constexpr Type& first() noexcept requires (Size != 0) {
        return data[0];
    }
    /// @brief Receive a const reference to the first element in the array
    /// @return A reference to the first element
    inline constexpr const Type& first() const noexcept requires (Size != 0) {
        return data[0];
    }

    /// @brief Receive a reference to the last element in the array
    /// @return A reference to the last element
    inline constexpr Type& last() noexcept requires (Size != 0) {
        return data[Size - 1];
    }
    /// @brief Receive a const reference to the last element in the array
    /// @return A reference to the last element
    inline constexpr const Type& last() const noexcept requires (Size != 0) {
        return data[Size - 1];
    }

    /// @brief Index the array without bounds checking
    /// @param i The index into the array
    /// @return A reference to the element at the specified index
    inline constexpr Type& operator[](std::size_t i) noexcept { return data[i]; }
    /// @brief Index the array without bounds checking
    /// @param i The index into the array
    /// @return A const reference to the element at the specified index
    inline constexpr const Type& operator[](std::size_t i) const noexcept { return data[i]; }

    /// @brief Index the array with bounds checking
    /// @param i The index into the array
    /// @return A reference to the element at the specified index
    /// @throws `std::out_of_range` if the provided index is greater than
    ///         the `Size` template argument if exceptions are disabled
    ///         (by defining VECLIB_ASSERT_NOEXCEPT) an assert fails
    inline constexpr Type& at(std::size_t i) {
        // We have no `noexcept` because we don't know if
        // we could throw or not based on VECLIB_ASSERT_NOEXCEPT
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(i <= Size);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (i > Size) throw std::out_of_range("Array<Type, Size>.at(std::size_t): Index is out of bounds");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return data[i];
    }
    /// @brief Index the array with bounds checking
    /// @param i The index into the array
    /// @return A const reference to the element at the specified index
    /// @throws `std::out_of_range` if the provided index is greater than
    ///         the `Size` template argument if exceptions are disabled
    ///         (by defining VECLIB_ASSERT_NOEXCEPT) an assert fails
    inline constexpr const Type& at(std::size_t i) const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(i <= Size);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (i > Size) throw std::out_of_range("Array<Type, Size>.at(std::size_t): Index is out of bounds");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return data[i];
    }

    /// @brief Receive a forward iterator to the first element of the array
    /// @return A pointer to the first element
    inline constexpr Type* begin() noexcept {
        return data;
    }
    /// @brief Receive a const forward iterator to the first element of the array
    /// @return A const pointer to the first element
    inline constexpr const Type* begin() const noexcept {
        return data;
    }

    /// @brief Receive a forward iterator past the last element of the array
    /// @return A pointer pointing past the last element
    inline constexpr Type* end() noexcept {
        return &data[Size];
    }
    /// @brief Receive a const forward iterator past the last element of the array
    /// @return A const pointer pointing past the last element
    inline constexpr const Type* end() const noexcept {
        return &data[Size];
    }

    /// @brief Receive a reverse iterator to the first element of the array
    /// @return A reverse iterator object pointing to the first element
    inline constexpr ReverseMemIterator<Type> rbegin() noexcept {
        return ReverseMemIterator<Type>(&data[Size - 1]);
    }
    /// @brief Receive a const reverse iterator to the first element of the array
    /// @return A const reverse iterator object pointing to the first element
    inline constexpr const ReverseMemIterator<Type> rbegin() const noexcept {
        return ReverseMemIterator<Type>(&data[Size - 1]);
    }

    /// @brief Receive a reverse iterator past the last element of the array
    /// @return A reverse iterator object pointing past the last element
    inline constexpr ReverseMemIterator<Type> rend() noexcept {
        return ReverseMemIterator<Type>(data - 1);
    }
    /// @brief Receive a const reverse iterator past the last element of the array
    /// @return A const reverse iterator object pointing past the last element
    inline constexpr const ReverseMemIterator<Type> rend() const noexcept {
        return ReverseMemIterator<Type>(data - 1);
    }

    /// @brief Receive a const forward iterator to the first element of the array
    /// @return A const pointer to the first element
    inline constexpr const Type* cbegin() const {
        return data;
    }

    /// @brief Receive a const forward iterator past the last element of the array
    /// @return A const pointer pointing past the last element
    inline constexpr const Type* cend() const {
        return &data[Size];
    }

    /// @brief Receive a const reverse iterator to the first element of the array
    /// @return A const reverse iterator object pointing to the first element
    inline constexpr const ReverseMemIterator<Type> crbegin() const {
        return ReverseMemIterator<Type>(&data[Size - 1]);
    }

    /// @brief Receive a const reverse iterator past the last element of the array
    /// @return A const reverse iterator object pointing past the last element
    inline constexpr const ReverseMemIterator<Type> crend() const {
        return ReverseMemIterator<Type>(data - 1);
    }

    // Arithmetic operations for integral types

    inline constexpr Array<Type, Size>& operator++() noexcept requires std::integral<Type> {
        for (std::size_t i = 0; i < Size; ++i)
            ++data[i];
        return *this;
    }
    inline constexpr Array<Type, Size>& operator++(int) noexcept requires std::integral<Type> {
        Array<Type, Size> self = *this;
        for (std::size_t i = 0; i < Size; ++i)
            ++data[i];
        return self;
    }
    inline constexpr Array<Type, Size>& operator--() noexcept requires std::integral<Type> {
        for (std::size_t i = 0; i < Size; ++i)
            --data[i];
        return *this;
    }
    inline constexpr Array<Type, Size>& operator--(int) noexcept requires std::integral<Type> {
        Array<Type, Size> self = *this;
        for (std::size_t i = 0; i < Size; ++i)
            --data[i];
        return self;
    }

    inline constexpr Array<Type, Size>& operator+=(const Array<Type, Size>& other)
            noexcept requires std::integral<Type> {
        for (std::size_t i = 0; i < Size; ++i)
            data[i] += other.data[i];
        return *this;
    }
    inline constexpr Array<Type, Size>& operator-=(const Array<Type, Size>& other)
            noexcept requires std::integral<Type> {
        for (std::size_t i = 0; i < Size; ++i)
            data[i] -= other.data[i];
        return *this;
    }
    inline constexpr Array<Type, Size>& operator*=(const Array<Type, Size>& other)
            noexcept requires std::integral<Type> {
        for (std::size_t i = 0; i < Size; ++i)
            data[i] *= other.data[i];
        return *this;
    }
    inline constexpr Array<Type, Size>& operator/=(const Array<Type, Size>& other)
            noexcept requires std::integral<Type> {
        for (std::size_t i = 0; i < Size; ++i)
            data[i] /= other.data[i];
        return *this;
    }
    inline constexpr Array<Type, Size>& operator%=(const Array<Type, Size>& other)
            noexcept requires std::integral<Type> {
        for (std::size_t i = 0; i < Size; ++i)
            data[i] %= other.data[i];
        return *this;
    }

    inline constexpr Array<Type, Size> operator+(const Array<Type, Size>& other)
            noexcept requires std::integral<Type> {
        Array<Type, Size> output = *this;
        for (std::size_t i = 0; i < Size; ++i)
            output[i] += other.data[i];
        return output;
    }
    inline constexpr Array<Type, Size> operator-(const Array<Type, Size>& other)
            noexcept requires std::integral<Type> {
        Array<Type, Size> output = *this;
        for (std::size_t i = 0; i < Size; ++i)
            output[i] -= other.data[i];
        return output;
    }
    inline constexpr Array<Type, Size> operator*(const Array<Type, Size>& other)
            noexcept requires std::integral<Type> {
        Array<Type, Size> output = *this;
        for (std::size_t i = 0; i < Size; ++i)
            output[i] *= other.data[i];
        return output;
    }
    inline constexpr Array<Type, Size> operator/(const Array<Type, Size>& other)
            noexcept requires std::integral<Type> {
        Array<Type, Size> output = *this;
        for (std::size_t i = 0; i < Size; ++i)
            output[i] /= other.data[i];
        return output;
    }
    inline constexpr Array<Type, Size> operator%(const Array<Type, Size>& other)
            noexcept requires std::integral<Type> {
        Array<Type, Size> output = *this;
        for (std::size_t i = 0; i < Size; ++i)
            output[i] %= other.data[i];
        return output;
    }

    inline constexpr bool operator==(const Array<Type, Size>& other)
            const noexcept requires std::equality_comparable<Type> {
        for (std::size_t i = 0; i < Size; ++i)
            if (data[i] != other.data[i]) return false;
        return true;
    }
    inline constexpr bool operator!=(const Array<Type, Size>& other)
            const noexcept requires std::equality_comparable<Type> {
        return !(*this == other);
    }
};

/// @brief Compile-time enum used for specifying the formula
///        applied when reallocating array buffers
enum class GrowType : std::uint8_t {
    OneAndHalf, DoubleSize
};

/// @brief A dynamic array class similar to `std::vector`
/// @tparam Type The type of the elements held
/// @tparam Grow The growing formula applied when reallocating the internal buffer
template <typename Type, GrowType Grow = GrowType::OneAndHalf>
class Vector {
private:
    Type* data = nullptr;
    std::size_t count = 0;
    std::size_t cap = 0;

    /// @brief Reallocate the vector, doesn't update `count`, only `cap`
    /// @param new_count The new `count` value for the vector
    /// @param update_cap `true` by default if `cap` needs to be updated
    ///                   according to `new_count`
    /// @return The new `count` value without setting it
    std::size_t realloc(std::size_t new_count, bool update_cap = true) {
        Type* old = data;
        std::size_t old_cap = cap;
        if (update_cap) {
            if constexpr (Grow == GrowType::OneAndHalf) // Update capacity accordingly
                cap = new_count + new_count / 2;
            else // Grow == GrowType::DoubleSize
                cap = new_count * 2;
        } else cap = new_count; // We just make them equal

        data = VECLIB_NONCONSTRUCTOR_NEW(cap, Type);
        if (new_count < count && old) { // We do all of this only if the old buffer had slots
            for (std::size_t i = new_count; i < count; ++i)
                delete &old[i]; // We should be fine with destructive delete
            // And nondestructive delete for unoccupied slots in the old buffer
            VECLIB_NONDESTRUCTOR_DELETE(&old[count], old_cap, Type);
        }
        // We copy over the old elements if there were any
        if (old) for (std::size_t i = 0; i < count; ++i) {
            data[i] = std::move(old[i]);
            delete &old[i]; // And delete the old ones
        }
        return new_count;
    }

    /// @brief Slide elements forward inside the vector, doesn't call the
    ///        destructor of `Type` or free any of the old slots
    /// @param index The index from which the sliding should begin
    /// @param slots How many slots to slide the elements from the index by
    /// @return The new `count` value without setting it
    std::size_t slide_forw(std::size_t index, std::size_t slots) {
        std::size_t new_count = count + slots;
        if (new_count > cap) realloc(new_count);
        for (std::size_t i = 0; i < count - index; ++i)
            data[new_count - 1 - i] = std::move(data[new_count - 1 - i - slots]);
        count = new_count;
        return new_count;
    }

    /// @brief Slide elements backward inside the vector, doesn't free
    ///        the left over slots but it destroys the objects inside them
    ///        (elements before `index` remain unchanged)
    /// @param index The index from which the sliding should begin
    /// @param slots How many slots to slide the elements from the index by
    /// @return The new `count` value without setting it
    std::size_t slide_backw(std::size_t index, std::size_t slots) {
        std::size_t new_count = count - slots;
        for (std::size_t i = index; i < new_count; ++i)
            data[i] = std::move(data[i + slots]);
        for (std::size_t i = new_count; i < count; ++i)
            data[i].~Type();
        return new_count;
    }

public:
    /// @brief Default constructor
    Vector() noexcept = default;
    /// @brief Destructor
    ~Vector() noexcept { clear(); };

    /// @brief Construct a `Vector` with a specific size
    /// @param size The size of the vector
    Vector(std::size_t size) { count = realloc(size); }
    /// @brief Construct a `Vector` with a specific size
    ///        and a `Type` object to copy across the created slots
    /// @param size The size of the vector
    /// @param value The `Type` object to copy
    Vector(std::size_t size, const Type& value) {
        count = realloc(size);
        for (std::size_t i = 0; i < count; ++i)
            new(&data[i]) Type(value);
    }
    /// @brief Construct a `Vector` with a specific size
    ///        and pass arguments to the constructor of `Type`
    ///        across the created slots
    /// @tparam ...Args The variadic arguments' types for the constructor of `Type`
    /// @param size The size of the vector
    /// @param args The variadic arguments for the constructor of `Type`
    template <typename... Args>
    Vector(std::size_t size, Args&&... args) {
        count = realloc(size);
        for (std::size_t i = 0; i < count; ++i)
            new(&data[i]) Type(std::forward<Args>(args)...);
    }

    Vector(Type* begin, Type* end) {
        count = realloc(end - begin); // Allocate the correct range
        for (std::size_t i = 0; i < count; ++i)
            new(&data[i]) Type(begin[i]);
    }

    Vector(const std::initializer_list<const Type&>& args) {
        count = realloc(args.size());
        for (std::size_t i = 0; i < count; ++i)
            new(&data[i]) Type(args.begin()[i]); // Use placement new and `Type`'s copy constructor
    }
    inline constexpr Vector<Type, Grow>& operator=(const std::initializer_list<Type>& args) {
        count = realloc(args.size());
        for (std::size_t i = 0; i < count; ++i)
            new(&data[i]) Type(args.begin()[i]);
        return *this;
    }

    Vector(const Vector<Type, Grow>& other) : count(other.count), cap(other.cap) {
        realloc(cap, false); // Allocate the exact amount, fine since data was nullptr
        for (std::size_t i = 0; i < count; ++i)
            data[i] = other.data[i]; // Copy the slots that were occupied
    }
    inline constexpr Vector<Type, Grow>& operator=(const Vector<Type, Grow>& other) {
        realloc(cap, false); // Allocate the exact amount, fine since data was nullptr
        for (std::size_t i = 0; i < count; ++i)
            data[i] = other.data[i]; // Copy the slots that were occupied
        return *this;
    }

    Vector(Vector<Type, Grow>&& other) noexcept
            : count(other.count), cap(other.cap), data(other.data) {
        other.count = 0;
        other.cap = 0;
        other.data = nullptr;
    }
    inline constexpr Vector<Type, Grow>& operator=(Vector<Type, Grow>&& other) noexcept {
        count = other.count;
        cap = other.cap;
        data = other.data;
        other.count = 0;
        other.cap = 0;
        other.data = nullptr;
    }

    inline constexpr const Type* get() const noexcept { return data; }
    inline constexpr std::size_t size() const noexcept { return count; }
    inline constexpr std::size_t capacity() const noexcept { return cap; }
    inline constexpr bool empty() const noexcept { return count == 0; }

    inline constexpr void clear() noexcept {
        for (std::size_t i = 0; i < count; ++i)
            delete &data[i]; // Call the destructor of the placed elements
        VECLIB_NONDESTRUCTOR_DELETE(&data[count], cap, Type); // Don't call it for the extra slots
        data = nullptr; // The buffer is now empty
        count = 0;
        cap = 0;
    }

    inline constexpr std::size_t reserve(std::size_t new_cap) {
        std::size_t cut = 0;
        if (new_cap < count) cut = count - new_cap; // How many elements were cut off
        count = realloc(new_cap, false); // We pass false to avoid having extra capacity
        return cut;
    }

    inline constexpr std::size_t resize(std::size_t new_count, const Type& value = Type()) {
        std::size_t old_count = count;
        std::size_t cut = reserve(new_count);
        // Do this if we have extra slots
        if (cut == 0) for (std::size_t i = old_count; i < new_count; ++i)
            new(&data[i]) Type(value);
        return cut;
    }
    template <typename... Args>
    inline constexpr std::size_t resize(std::size_t new_count, Args&&... args) {
        std::size_t old_count = count;
        std::size_t cut = reserve(new_count);
        if (cut == 0) for (std::size_t i = old_count; i < new_count; ++i)
            new(&data[i]) Type(std::forward<Args>(args)...);
        return cut;
    }

    inline constexpr void swap(Vector<Type, Grow>& other) {
        Vector<Type, Grow> self = *this; // That one trick -_-
        *this = other;
        other = self;
    }

    inline constexpr std::size_t remove(Type* itr) {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data <= itr && itr <= data + count);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (itr < data || data + count <= itr)
            throw std::out_of_range("Vector<Type, Grow>.remove(Type*): Pointer is out of range");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return pop_at(itr - data); // Reuse code
    }

    inline constexpr Type& first() noexcept { return data[0]; }
    inline constexpr const Type& first() const noexcept { return data[0]; }

    inline constexpr Type& last() noexcept { return data[count - 1]; }
    inline constexpr const Type& last() const noexcept { return data[count - 1]; }

    inline constexpr Type& operator[](std::size_t i) noexcept { return data[i]; }
    inline constexpr const Type& operator[](std::size_t i) const noexcept { return data[i]; }

    inline constexpr Type& at(std::size_t i) {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(i < count && data != nullptr);
        #else // VECLIB_ASSERT_NOEXCEPT
        // We also cover the case in which data is nullptr since when it's cleared count is set to 0
        if (i >= count) throw std::out_of_range("Vector<Type, Grow>.at(std::size_t): Index is out of bounds");
        #endif // VECLIB_ASSERT_NOEXCEPT
    }
    inline constexpr const Type& at(std::size_t i) const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(i < count && data != nullptr);
        #else // VECLIB_ASSERT_NOEXCEPT
        // We also cover the case in which data is nullptr since when it's cleared count is set to 0
        if (i >= count) throw std::out_of_range("Vector<Type, Grow>.at(std::size_t): Index is out of bounds");
        #endif // VECLIB_ASSERT_NOEXCEPT
    }

    inline constexpr Type& push_back(const Type& value) {
        if (count + 1 > cap) realloc(count + 1);
        new(&data[count]) Type(value); // Copy the provided value
        return data[count++]; // Finally increment count
    }
    inline constexpr Type& push_back(Type&& value) {
        if (count + 1 > cap) realloc(count + 1);
        new(&data[count]) Type(std::move(value)); // Move it
        return data[count++];
    }

    template <typename... Args>
    inline constexpr Type& place_back(Args&&... args) {
        if (count + 1 > cap) realloc(count + 1);
        new(&data[count]) Type(std::forward<Args>(args)...);
        return data[count++];
    }

    inline constexpr Type& push_front(const Type& value) {
        count = slide_forw(0, 1);
        new(data) Type(value);
        return data[0];
    }
    inline constexpr Type& push_front(Type&& value) {
        count = slide_forw(0, 1);
        new(data) Type(std::move(value));
        return data[0];
    }

    template <typename... Args>
    inline constexpr Type& place_front(Args&&... args) {
        count = slide_forw(0, 1);
        new(data) Type(std::forward<Args>(args)...);
        return data[0];
    }

    inline constexpr Type& push_at(std::size_t index, const Type& value) {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(index <= count)
        #else // VECLIB_ASSERT_NOEXCEPT
        // We allow count as a value to just serve the purpose of push_back
        if (index > count) throw std::out_of_range(
            "Vector<Type, Grow>.push_at(std::size_t, const Type&): Index is out of bounds");
        #endif // VECLIB_ASSERT_NOEXCEPT
        if (index == count) // Special case
            count = realloc(count + 1);
        else
            count = slide_forw(index, 1);
        new(&data[index]) Type(value);
        return data[index];
    }
    inline constexpr Type& push_at(std::size_t index, Type&& value) {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(index <= count)
        #else // VECLIB_ASSERT_NOEXCEPT
        // We allow count as a value to just serve the purpose of push_back
        if (index > count) throw std::out_of_range(
            "Vector<Type, Grow>.push_at(std::size_t, Type&&): Index is out of bounds");
        #endif // VECLIB_ASSERT_NOEXCEPT
        if (index == count) // Special case
            count = realloc(count + 1);
        else
            count = slide_forw(index, 1);
        new(&data[index]) Type(std::move(value));
        return data[index];
    }

    template <typename... Args>
    inline constexpr Type& place_at(std::size_t index, Args&&... args) {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(index <= count)
        #else // VECLIB_ASSERT_NOEXCEPT
        // We allow count as a value to just serve the purpose of push_back
        if (index > count) throw std::out_of_range(
            "Vector<Type, Grow>.push_at(std::size_t, Type&&): Index is out of bounds");
        #endif // VECLIB_ASSERT_NOEXCEPT
        if (index == count) // Special case
            count = realloc(count + 1);
        else
            count = slide_forw(index, 1);
        new(&data[index]) Type(std::forward<Args>(args)...);
        return data[index];
    }

    inline constexpr std::size_t pop_back(std::size_t slots = 1) {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(slots < count);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (slots >= count)
            throw std::overflow_error("Vector<Type, Grow>.pop_back(std::size_t): Too many slots to pop");
        #endif // VECLIB_ASSERT_NOEXCEPT
        for (std::size_t i = 0; i < slots; ++i)
            data[count - 1 - i].~Type();
        count -= slots;
        return count;
    }

    inline constexpr std::size_t pop_front(std::size_t slots = 1) {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(slots < count);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (slots >= count)
            throw std::overflow_error("Vector<Type, Grow>.pop_front(std::size_t): Too many slots to pop");
        #endif // VECLIB_ASSERT_NOEXCEPT
        count = slide_backw(0, slots);
        return count;
    }

    inline constexpr std::size_t pop_at(std::size_t index, std::size_t slots = 1) {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(index + slots < count);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (index >= count)
            throw std::out_of_range("Vector<Type, Grow>.pop_at(std::size_t): Index is out of bounds");
        if (index + slots >= count)
            throw std::out_of_range("Vector<Type, Grow>.pop_at(std::size_t): Too many slots to pop");
        #endif // VECLIB_ASSERT_NOEXCEPT
        count = slide_backw(index, slots);
        return count;
    }

    inline constexpr Type* begin() noexcept { return data; }
    inline constexpr const Type* begin() const noexcept { return data; }

    inline constexpr Type* end() noexcept { return data + count; }
    inline constexpr const Type* end() const noexcept { return data + count; }

    inline constexpr ReverseMemIterator<Type> rbegin() noexcept {
        return ReverseMemIterator<Type>(data + count - 1);
    }
    inline constexpr const ReverseMemIterator<Type> rbegin() const noexcept {
        return ReverseMemIterator<Type>(data + count - 1);
    }

    inline constexpr ReverseMemIterator<Type> rend() noexcept {
        return ReverseMemIterator<Type>(data - 1);
    }
    inline constexpr const ReverseMemIterator<Type> rend() const noexcept {
        return ReverseMemIterator<Type>(data - 1);
    }

    inline constexpr const Type* cbegin() const noexcept { return data; }

    inline constexpr const Type* cend() const noexcept { return data + count; }

    inline constexpr const ReverseMemIterator<Type> crbegin() const noexcept {
        return ReverseMemIterator<Type>(data + count - 1);
    }

    inline constexpr const ReverseMemIterator<Type> crend() const noexcept {
        return ReverseMemIterator<Type>(data - 1);
    }

    // Arithmetic overloads for integral types

    inline constexpr Vector<Type, Grow>& operator++() noexcept requires std::integral<Type> {
        for (std::size_t i = 0; i < count; ++i)
            ++data[i];
        return *this;
    }
    inline constexpr Vector<Type, Grow>& operator++(int) noexcept requires std::integral<Type> {
        Vector<Type, Grow> self = *this;
        for (std::size_t i = 0; i < count; ++i)
            ++data[i];
        return self;
    }
    inline constexpr Vector<Type, Grow>& operator--() noexcept requires std::integral<Type> {
        for (std::size_t i = 0; i < count; ++i)
            --data[i];
        return *this;
    }
    inline constexpr Vector<Type, Grow>& operator--(int) noexcept requires std::integral<Type> {
        Vector<Type, Grow> self = *this;
        for (std::size_t i = 0; i < count; ++i)
            --data[i];
        return self;
    }

    inline constexpr Vector<Type, Grow>& operator+=(const Vector<Type, Grow>& other)
            noexcept requires std::integral<Type> {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(count == other.count);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (count != other.count) throw std::logic_error(
        "Vector<Type, Grow>::operator+=(const Vector<Type, Grow>&): The vectors are not of the same size");
        #endif // VECLIB_ASSERT_NOEXCEPT
        for (std::size_t i = 0; i < count; ++i)
            data[i] += other.data[i];
        return *this;
    }
    inline constexpr Vector<Type, Grow>& operator-=(const Vector<Type, Grow>& other)
            noexcept requires std::integral<Type> {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(count == other.count);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (count != other.count) throw std::logic_error(
        "Vector<Type, Grow>::operator-=(const Vector<Type, Grow>&): The vectors are not of the same size");
        #endif // VECLIB_ASSERT_NOEXCEPT
        for (std::size_t i = 0; i < count; ++i)
            data[i] -= other.data[i];
        return *this;
    }
    inline constexpr Vector<Type, Grow>& operator*=(const Vector<Type, Grow>& other)
            noexcept requires std::integral<Type> {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(count == other.count);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (count != other.count) throw std::logic_error(
        "Vector<Type, Grow>::operator*=(const Vector<Type, Grow>&): The vectors are not of the same size");
        #endif // VECLIB_ASSERT_NOEXCEPT
        for (std::size_t i = 0; i < count; ++i)
            data[i] *= other.data[i];
        return *this;
    }
    inline constexpr Vector<Type, Grow>& operator/=(const Vector<Type, Grow>& other)
            noexcept requires std::integral<Type> {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(count == other.count);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (count != other.count) throw std::logic_error(
        "Vector<Type, Grow>::operator/=(const Vector<Type, Grow>&): The vectors are not of the same size");
        #endif // VECLIB_ASSERT_NOEXCEPT
        for (std::size_t i = 0; i < count; ++i)
            data[i] /= other.data[i];
        return *this;
    }
    inline constexpr Vector<Type, Grow>& operator%=(const Vector<Type, Grow>& other)
            noexcept requires std::integral<Type> {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(count == other.count);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (count != other.count) throw std::logic_error(
        "Vector<Type, Grow>::operator%=(const Vector<Type, Grow>&): The vectors are not of the same size");
        #endif // VECLIB_ASSERT_NOEXCEPT
        for (std::size_t i = 0; i < count; ++i)
            data[i] %= other.data[i];
        return *this;
    }

    inline constexpr Vector<Type, Grow> operator+(const Vector<Type, Grow>& other)
            noexcept requires std::integral<Type> {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(count == other.count);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (count != other.count) throw std::logic_error(
        "Vector<Type, Grow>::operator+(const Vector<Type, Grow>&): The vectors are not of the same size");
        #endif // VECLIB_ASSERT_NOEXCEPT
        Vector<Type, Grow> output = *this;
        for (std::size_t i = 0; i < count; ++i)
            output[i] += other.data[i];
        return output;
    }
    inline constexpr Vector<Type, Grow> operator-(const Vector<Type, Grow>& other)
            noexcept requires std::integral<Type> {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(count == other.count);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (count != other.count) throw std::logic_error(
        "Vector<Type, Grow>::operator-(const Vector<Type, Grow>&): The vectors are not of the same size");
        #endif // VECLIB_ASSERT_NOEXCEPT
        Vector<Type, Grow> output = *this;
        for (std::size_t i = 0; i < count; ++i)
            output[i] -= other.data[i];
        return output;
    }
    inline constexpr Vector<Type, Grow> operator*(const Vector<Type, Grow>& other)
            noexcept requires std::integral<Type> {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(count == other.count);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (count != other.count) throw std::logic_error(
        "Vector<Type, Grow>::operator*(const Vector<Type, Grow>&): The vectors are not of the same size");
        #endif // VECLIB_ASSERT_NOEXCEPT
        Vector<Type, Grow> output = *this;
        for (std::size_t i = 0; i < count; ++i)
            output[i] *= other.data[i];
        return output;
    }
    inline constexpr Vector<Type, Grow> operator/(const Vector<Type, Grow>& other)
            noexcept requires std::integral<Type> {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(count == other.count);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (count != other.count) throw std::logic_error(
        "Vector<Type, Grow>::operator/(const Vector<Type, Grow>&): The vectors are not of the same size");
        #endif // VECLIB_ASSERT_NOEXCEPT
        Vector<Type, Grow> output = *this;
        for (std::size_t i = 0; i < count; ++i)
            output[i] /= other.data[i];
        return output;
    }
    inline constexpr Vector<Type, Grow> operator%(const Vector<Type, Grow>& other)
            noexcept requires std::integral<Type> {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(count == other.count);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (count != other.count) throw std::logic_error(
        "Vector<Type, Grow>::operator%(const Vector<Type, Grow>&): The vectors are not of the same size");
        #endif // VECLIB_ASSERT_NOEXCEPT
        Vector<Type, Grow> output = *this;
        for (std::size_t i = 0; i < count; ++i)
            output[i] %= other.data[i];
        return output;
    }

    inline constexpr bool operator==(const Vector<Type, Grow>& other)
            const noexcept requires std::equality_comparable<Type> {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(count == other.count);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (count != other.count) throw std::logic_error(
        "Vector<Type, Grow>::operator==(const Vector<Type, Grow>&): The vectors are not of the same size");
        #endif // VECLIB_ASSERT_NOEXCEPT
        for (std::size_t i = 0; i < count; ++i)
            if (data[i] != other.data[i]) return false;
        return true;
    }
    inline constexpr bool operator!=(const Vector<Type, Grow>& other)
            const noexcept requires std::equality_comparable<Type> {
        return !(*this == other);
    }
};

} // namespace veclib

#endif // ARRAY_HPP
