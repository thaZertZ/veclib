#ifndef VECLIB_HPP
#define VECLIB_HPP

// For testing
//#define VECLIB_ASSERT_NOEXCEPT
//#define VECLIB_EXTRA

#include <cstdint>

#ifdef VECLIB_ASSERT_NOEXCEPT
#include <cassert>
#else // VECLIB_ASSERT_NOEXCEPT
#include <stdexcept>
#endif // VECLIB_ASSERT_NOEXCEPT

#include <initializer_list> // std::initializer_list
#include <utility> // std::move(), std::forward()
#include <concepts>

#if defined(VECLIB_NONCONSTRUCTOR_NEW) \
 || defined(VECLIB_NONDESTRUCTOR_DELETE) \
 || defined(VECLIB_EXPLICIT_NONDESTRUCTOR_DELETE) \
 || defined(VECLIB_NOEXCEPT) \
 || defined(VECLIB_COND_NOEXCEPT)
#error "veclib can't be compiled if internal macros are already defined"
#endif // VECLIB_*

/// @brief A collection of array-like data structures (arrays, vectors, slices, lists...)
namespace veclib {


/// @brief Allocate objects on the heap without calling the
///        constructor of their type
/// @param type The type of elements to allocate
/// @param count How many elements to allocate
#define VECLIB_NONCONSTRUCTOR_NEW(type, count) \
    (reinterpret_cast<type*>(::operator new((count) * sizeof(type))))

/// @brief Deallocate objects on the heap without calling the
///        destructor of their type, which is deduced by the
///        type of the given pointer
/// @param data A pointer to the elements to deallocate
/// @param count The number of elements to deallocate
/// @note The pointer must not be a `void*`, if it is,
///       use `VECLIB_EXPLICIT_NONDESTRUCTOR_DELETE`
///       and specify a type to cast it to
#define VECLIB_NONDESTRUCTOR_DELETE(data, count) \
    (::operator delete((data), (count) * sizeof(std::remove_pointer_t<decltype(data)>)))

/// @brief Deallocate objects on the heap without calling the
///        destructor of the explicitly specified type
/// @param data A pointer to the elements to deallocate
/// @param type The type of elements to deallocate
/// @param count The number of elements to deallocate
#define VECLIB_EXPLICIT_NONDESTRUCTOR_DELETE(data, type, count) \
    (::operator delete((data), (count) * sizeof(type)))

#ifdef VECLIB_ASSERT_NOEXCEPT
/// @brief Expands to `noexcept` if `VECLIB_ASSERT_NOEXCEPT` is defined
#define VECLIB_NOEXCEPT noexcept
/// @brief Expands to a conditional `noexcept` clause into which is passed
///        an expression that calls a given function, evaluating to
///        `noexcept(false)` if it is not `noexcept` itself. This always
///        expands to `noexcept` if `VECLIB_ASSERT_NOEXCEPT` is defined
/// @param ident The identifier of the function to check
/// @param __VA_ARGS__... The arguments to pass to the function, these can be
///                       `std::declval<>()` calls or just arbitrary values
#define VECLIB_COND_NOEXCEPT(ident, ...) noexcept
#else // VECLIB_ASSERT_NOEXCEPT
/// @brief Expands to `noexcept` if `VECLIB_ASSERT_NOEXCEPT` is defined
#define VECLIB_NOEXCEPT
/// @brief Expands to a conditional `noexcept` clause into which is passed
///        an expression that calls a given function, evaluating to
///        `noexcept(false)` if it is not `noexcept` itself. This always
///        expands to `noexcept` if `VECLIB_ASSERT_NOEXCEPT` is defined
/// @param ident The identifier of the function to check
/// @param __VA_ARGS__... The arguments to pass to the function, these can be
///                       `std::declval<>()` calls or just arbitrary values
#define VECLIB_COND_NOEXCEPT(ident, ...) \
    noexcept(std::declval<decltype(ident)>()(__VA_ARGS__))
#endif // VECLIB_ASSERT_NOEXCEPT

/// @brief Type alias for the signed equivalent of `std::size_t`
using diff_t = std::make_signed_t<std::size_t>;

/// @brief Templated type alias for a `noexcept` function signature
/// @tparam Ret The return type of the function
/// @tparam ...Args The type of the arguments of the function
template <typename Ret, typename... Args>
using Function = Ret(*)(Args...) noexcept;

template <typename Type>
using MemIterator = Type*;

/// @brief Helper concept for types used for indexing into containers
/// @tparam Type A type that satifies the following requirements:
///
///         - It is incrementable and decrementable through prefix operators
///
///         - It can be assigned with an increment and decrement value that
///           can be of type `std::size_t`, `Type` or both
///
///         - It can be compared for equality against an object of the same type
template <typename Type>
concept IndexerType = requires (std::size_t i) {
    requires (std::equality_comparable<Type>);
    ++std::declval<Type&>(); // Incrementable
    --std::declval<Type&>(); // Decrementable
    (std::declval<Type&>() += i) || (std::declval<Type&>() += std::declval<Type&>()); // Assignment-incrementable
    (std::declval<Type&>() -= i) || (std::declval<Type&>() -= std::declval<Type&>()); // Assignment-decrementable
};

/// @brief Helper concept for types that can be indexed by other types that
///        satisfy the requirements of the `IndexerType` concept
/// @tparam Type A type that provides an overload of
///         `operator[]` that accepts a type constrained by `IndexerType`
///         as a parameter
/// @tparam Item The type returned by the `operator[]` overload of `Type`
/// @tparam Index A type used for indexing into `Type` that satisfies the
///         requirements of `IndexerType`. It is used for the member function
///         parameter constraints on `Type`
template <typename Type, typename Item, typename Index>
concept IndexableViaOperator = requires () {
    requires (IndexerType<Index>);
    { std::declval<Type>()[std::declval<Index>()] } -> std::same_as<std::remove_const_t<Item&>>; // Works with const and non-const
};
/// @brief Helper concept for types that can be indexed by other types that
///        satisfy the requirements of the `IndexerType` concept
/// @tparam Type A type that provides an `at()` method that accepts a type
///         constrained by `IndexerType` as a parameter
/// @tparam Item The type returned by the `at()` method of `Type`
/// @tparam Index A type used for indexing into `Type` that satisfies the
///         requirements of `IndexerType`. It is used for the member function
///         parameter constraints on `Type`
template <typename Type, typename Item, typename Index>
concept IndexableViaAt = requires () {
    requires (IndexerType<Index>);
    { std::declval<Type>().at(std::declval<Index>()) } -> std::same_as<std::remove_const_t<Item&>>; // Works with const and non-const
};

/// @brief Helper concept for types that can be indexed by other types that
///        satisfy the requirements of the `IndexerType` concept
/// @tparam Type A type that provides an `at()` method taking a type
///         constrained by `IndexerType` as a parameter, or an overload of
///         `operator[]` that takes the same type as a parameter
/// @tparam Item The type returned by the indexing method of `Type`
/// @tparam Index A type used for indexing into `Type` that satisfies the
///         requirements of `IndexerType`. It is used for the member function
///         parameter constraints on `Type` and defaults to `std::size_t`
template <typename Type, typename Item, typename Index = std::size_t>
concept Indexable = requires () {
    requires (IndexerType<Index>);
    requires (IndexableViaOperator<Type, Item, Index>) || (IndexableViaAt<Type, Item, Index>);
};

template <typename Type>
inline constexpr Type intersect(const Type&, const Type&) noexcept;
// Should this throw when we can't create an intersection?

template <typename Type>
inline constexpr Type join(const Type&, const Type&) VECLIB_NOEXCEPT;


/// @brief A powerful and ergonomic slice class
/// @tparam Type The type being referenced by the slice
template <typename Type>
class Slice {
private:
    Type* data = nullptr;
    std::size_t count = 0;

    void bounds_check(std::size_t i, const char* str) const VECLIB_NOEXCEPT {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(i < count);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (i >= count) throw std::out_of_range(str);
        #endif // VECLIB_ASSERT_NOEXCEPT
    }
    void nullptr_check(const char* str) const VECLIB_NOEXCEPT {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data != nullptr);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error(str);
        #endif // VECLIB_ASSERT_NOEXCEPT
    }
    void empty_check(const char* str) const VECLIB_NOEXCEPT {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data != nullptr);
        assert(count != 0);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr || count == 0) throw std::runtime_error(str);
        #endif // VECLIB_ASSERT_NOEXCEPT
    }
    void underflow_check(std::size_t i, const char* str) const VECLIB_NOEXCEPT {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(i <= count);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (i > count) throw std::underflow_error(str);
        #endif // VECLIB_ASSERT_NOEXCEPT
    }
    void overflow_check(std::size_t i, const char* str) const VECLIB_NOEXCEPT {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(i <= count);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (i > count) throw std::overflow_error(str);
        #endif // VECLIB_ASSERT_NOEXCEPT
    }
    void zero_check(std::size_t i, const char* str) const VECLIB_NOEXCEPT {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(i != 0);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (i == 0) throw std::runtime_error(str);
        #endif // VECLIB_ASSERT_NOEXCEPT
    }

public:

    /// @brief Member type alias for accessing the referenced type
    using Item = Type;
    /// @brief Member type alias for the type of ourselves
    using Self = Slice<Type>;

    /// @brief Default constructor
    Slice() noexcept = default;
    /// @brief Default destructor
    ~Slice() noexcept = default;

    /// @brief Copy constructor
    /// @param other The object to copy
    Slice(const Self& other) noexcept
        : data(other.data), count(other.count) {}
    /// @brief Copy assignment
    /// @param other The object to copy
    /// @return A reference to the modified object
    inline constexpr Self& operator=(const Self& other) noexcept {
        data = other.data;
        count = other.count;
        return *this;
    }

    /// @brief Move constructor
    /// @param other The object to move
    Slice(Self&& other) noexcept
            : data(other.data), count(other.count) {
        other.data = nullptr;
        other.count = 0;
    }
    /// @brief Move assignment
    /// @param other The object to move
    /// @return A reference to the modified object
    inline constexpr Self& operator=(Self&& other) noexcept {
        data = other.data;
        count = other.count;
        other.data = nullptr;
        other.count = 0;
        return *this;
    }

    /// @brief Construct a slice out of a pointer and a length
    /// @param p The pointer
    /// @param s The length
    Slice(Type* p, std::size_t s = 0) noexcept
        : data(p), count(s) {}
    /// @brief Change this slice's view to start at a provided address
    /// @param p The pointer to the new view
    /// @return A reference to the modified object
    inline constexpr Self& operator=(Type* p) noexcept {
        data = p;
        return *this;
    }

    /// @brief Return a read-only pointer to the first element
    inline constexpr const Type* get() const noexcept { return data; }
    /// @brief Return the length of the slice
    inline constexpr std::size_t size() const noexcept { return count; }
    /// @brief Returns `true` if the length of the slice is `0`
    inline constexpr bool empty() const noexcept { return count == 0; }
    /// @brief Returns `true` if the slice isn't referencing anything
    inline constexpr bool null() const noexcept { return data == nullptr; }
    /// @brief Put the slice into an empty state
    /// @return A reference to the modified object
    inline constexpr Self& clear() noexcept {
        data = nullptr;
        count = 0;
        return *this;
    }

    /// @brief Access elements into the slice with no bounds checking
    /// @param i The index into the slice
    /// @return A reference to the indexed element
    inline constexpr Type& operator[](std::size_t i) noexcept { return data[i]; }
    inline constexpr const Type& operator[](std::size_t i) const noexcept { return data[i]; }

    /// @brief Access elements into the slice with bounds checking
    /// @param i The index into the slice
    /// @return A reference to the indexed element
    /// @throw `std::out_of_range` if the index is out of bounds and exceptions are not disabled
    inline constexpr Type& at(std::size_t i) VECLIB_NOEXCEPT {
        this->bounds_check(i, "veclib::Slice.at(): Index is out of bounds");
        return data[i];
    }
    inline constexpr const Type& at(std::size_t i) const VECLIB_NOEXCEPT {
        this->bounds_check(i, "veclib::Slice.at(): Index is out of bounds");
        return data[i];
    }

    /// @brief Return a reference to the first element in the slice
    /// @throw `std::runtime_error` if the slice is empty and exceptions are not disabled
    inline constexpr Type& first() VECLIB_NOEXCEPT {
        this->empty_check("veclib::Slice.first(): Slice is empty");
        return data[0];
    }
    inline constexpr const Type& first() const VECLIB_NOEXCEPT {
        this->empty_check("veclib::Slice.first(): Slice is empty");
        return data[0];
    }

    /// @brief Return a slice referencing the given number of first elements in this slice
    /// @param x The number of elements
    /// @throw `std::runtime_error` if the slice is empty, `std::overflow_error` if the
    ///        number of elements exceeds this slice's size, only if exceptions are not disabled
    inline constexpr Self first(std::size_t x) VECLIB_NOEXCEPT {
        this->empty_check("veclib::Slice.first(): Slice is empty");
        this->overflow_check("veclib::Slice.first(): Trying to extract too many first elements");
        return Self(data, x);
    }
    inline constexpr const Self first(std::size_t x) const VECLIB_NOEXCEPT {
        this->empty_check("veclib::Slice.first(): Slice is empty");
        this->overflow_check("veclib::Slice.first(): Trying to extract too many first elements");
        return Self(data, x);
    }

    /// @brief Return a reference to the last element in the slice
    /// @throw `std::runtime_error` if the slice is empty and exceptions are not disabled
    inline constexpr Type& last() VECLIB_NOEXCEPT {
        this->empty_check("veclib::Slice.last(): Slice is empty");
        return data[count - 1];
    }
    inline constexpr const Type& last() const VECLIB_NOEXCEPT {
        this->empty_check("veclib::Slice.last(): Slice is empty");
        return data[count - 1];
    }

    /// @brief Return a slice referencing the given number of last elements in this slice
    /// @param x The number of elements
    /// @throw `std::runtime_error` if the slice is empty, `std::overflow_error` if the
    ///        number of elements exceeds this slice's size, only if exceptions are not disabled
    inline constexpr Self last(std::size_t x) VECLIB_NOEXCEPT {
        this->empty_check("veclib::Slice.last(): Slice is empty");
        this->overflow_check("veclib::Slice.last(): Trying to extract too many last elements");
        return Self(data + (count - x), x);
    }
    inline constexpr const Self last(std::size_t x) const VECLIB_NOEXCEPT {
        this->empty_check("veclib::Slice.last(): Slice is empty");
        this->overflow_check("veclib::Slice.last(): Trying to extract too many last elements");
        return Self(data + (count - x), x);
    }

    /// @brief Apply a custom function to each element of the slice.
    ///        The function must return `void` and must take a `Type&` parameter,
    ///        into which will be passed the current element in the slice,
    ///        and can optionally accept an `std::size_t` parameter, into which
    ///        the index of the current element will be passed
    /// @param fn The function to apply
    /// @return A reference to the modified object
    inline constexpr Self& map(Function<void, Type&> fn)
            //VECLIB_COND_NOEXCEPT(fn, std::declval<Type&>())
            VECLIB_NOEXCEPT {
        this->empty_check("veclib::Slice.map(): Slice is empty");
        for (std::size_t i = 0; i < count; ++i)
            fn(data[i]);
        return *this;
    }
    inline constexpr Self& map(Function<void, Type&, std::size_t> fn)
            //VECLIB_COND_NOEXCEPT(fn, std::declval<Type&>(), 0)
            VECLIB_NOEXCEPT {
        this->empty_check("veclib::Slice.map(): Slice is empty");
        for (std::size_t i = 0; i < count; ++i)
            fn(data[i], i);
        return *this;
    }

    /// @brief Combine all the values of the elements in this slice with a custom
    ///        function, that must return `void` and must take a `Type&` parameter,
    ///        which will be the accumulated value, a `const Type&` parameter, which
    ///        will be the current element, and optionally an `std::size_t` parameter
    ///        into which the index of the current element will be passed
    /// @param fn The transformation function
    /// @param acc An optional starting value for the accumulated value. By default
    ///            it is assigned a default-constructed value of `Type`
    inline constexpr Type fold(Function<void, Type&, const Type&> fn, const Type& acc = Type())
            //VECLIB_COND_NOEXCEPT(fn, std::declval<Type&>(), std::declval<const Type&>())
            const VECLIB_NOEXCEPT requires (std::is_default_constructible_v<Type>) {
        this->empty_check("veclib::Slice.fold(): Slice is empty");
        for (std::size_t i = 0; i < count; ++i)
            fn(acc, data[i]);
        return acc;
    }
    inline constexpr Type fold(Function<void, Type&, const Type&, std::size_t> fn, const Type& acc = Type())
            //VECLIB_COND_NOEXCEPT(fn, std::declval<Type&>(), std::declval<const Type&>(), 0)
            const VECLIB_NOEXCEPT requires (std::is_default_constructible_v<Type>) {
        this->empty_check("veclib::Slice.fold(): Slice is empty");
        for (std::size_t i = 0; i < count; ++i)
            fn(acc, data[i], i);
        return acc;
    }

    #ifdef VECLIB_EXTRA

    /// @brief Return `true` if all elements in the slice pass a custom check function.
    ///        The check function must return `bool`, accept a `const Type&` parameter
    ///        and optionally an `std::size_t` parameter into which will be passed the
    ///        index into the slice of the element currently being processed
    /// @param fn The check function
    /// @throw `std::runtime_error` if the slice is empty and exceptions are not disabled
    inline constexpr bool all(Function<bool, const Type&> fn) const VECLIB_NOEXCEPT {
        this->empty_check("veclib::Slice.all(): Slice is empty");
        for (std::size_t i = 0; i < count; ++i)
            if (!fn(data[i])) return false;
        return true;
    }
    inline constexpr bool all(Function<bool, const Type&, std::size_t> fn) const VECLIB_NOEXCEPT {
        this->empty_check("veclib::Slice.all(): Slice is empty");
        for (std::size_t i = 0; i < count; ++i)
            if (!fn(data[i], i)) return false;
        return true;
    }

    /// @brief Return `true` if all values in the slice are equal to another value
    /// @param value The value to check for equality against
    /// @throw `std::runtime_error` if the slice is empty and exceptions are not disabled
    inline constexpr bool all_eq(const Type& value) const VECLIB_NOEXCEPT
            requires (std::equality_comparable<Type>) {
        return this->all([&value] (const Type& x) { return x == value; });
    }

    /// @brief Return `true` if at least an element in the slice passes a custom check function.
    ///        The check function must return `bool`, accept a `const Type&` parameter
    ///        and optionally an `std::size_t` parameter into which will be passed the
    ///        index into the slice of the element currently being processed
    /// @param fn The check function
    /// @throw `std::runtime_error` if the slice is empty and exceptions are not disabled
    inline constexpr bool any(Function<bool, const Type&> fn) const VECLIB_NOEXCEPT {
        this->empty_check("veclib::Slice.any(): Slice is empty");
        for (std::size_t i = 0; i < count; ++i)
            if (fn(data[i])) return true;
        return false;
    }
    inline constexpr bool any(Function<bool, const Type&, std::size_t> fn) const VECLIB_NOEXCEPT {
        this->empty_check("veclib::Slice.any(): Slice is empty");
        for (std::size_t i = 0; i < count; ++i)
            if (fn(data[i], i)) return true;
        return false;
    }

    /// @brief Return `true` if at least a value in the slice is equal to another value
    /// @param value The value to check for equality against
    /// @throw `std::runtime_error` if the slice is empty and exceptions are not disabled
    inline constexpr bool any_eq(const Type& value) const VECLIB_NOEXCEPT
            requires (std::equality_comparable<Type>) {
        return this->any([&value] (const Type& x) { return x == value; });
    }

    #endif // VECLIB_EXTRA

    /// @brief Increase the size of this slice by a specified amount
    /// @param x The number of elements by which the size should be increased
    /// @return A referenced to the modified object
    inline constexpr Self& grow(std::size_t x) VECLIB_NOEXCEPT {
        this->nullptr_check("veclib::Slice.grow(): Slice isn't referencing anything");
        count += x;
        return *this;
    }
    /// @brief Increase the size of this slice by a specified amount on a copy of this slice
    /// @param x The number of elements by which the size should be increased
    /// @return A copy of this slice, on which the operation was performed
    inline constexpr Self grow_copy(std::size_t x) const VECLIB_NOEXCEPT {
        Self self = *this;
        return self.grow(x);
    }

    /// @brief Decrease the size of this slice by a specified amount
    /// @param x The number of elements by which the size should be decreased
    /// @return A referenced to the modified object
    /// @throw `std::runtime_error` if the slice isn't referencing anything,
    ///        `std::underflow_error` if `x` is trying to shrink
    ///        the slice by too much, only if exceptions are not disabled
    inline constexpr Self& shrink(std::size_t x) VECLIB_NOEXCEPT {
        this->nullptr_check("veclib::Slice.shrink(): Slice isn't referencing anything");
        this->underflow_check(x, "veclib::Slice.shrink(): Trying to shrink the slice by too much");
        count -= x;
        return *this;
    }
    /// @brief Decrease the size of this slice by a specified amount on a copy of this slice
    /// @param x The number of elements by which the size should be decreased
    /// @return A copy of this slice, on which the operation was performed
    /// @throw `std::runtime_error` if the slice isn't referencing anything,
    ///        `std::underflow_error` if `x` is trying to shrink
    ///        the slice by too much, only if exceptions are not disabled
    inline constexpr Self shrink_copy(std::size_t x) const VECLIB_NOEXCEPT {
        Self self = *this;
        return self.shrink(x);
    }

    /// @brief Use a signed parameter to act like `grow()` when it is positive and
    ///        `shrink` when it is negative
    /// @param x The signed parameter
    /// @return A reference to the modified object
    /// @throw `std::runtime_error` if the slice isn't referencing anything,
    ///        `std::underflow_error` if `x` is negative and it is trying to shrink
    ///        the slice by too much, only if exceptions are not disabled
    inline constexpr Self& delta(diff_t x) VECLIB_NOEXCEPT {
        this->nullptr_check("veclib::Slice.delta(): Slice isn't referencing anything");
        if (x < 0) {
            this->underflow_check(-x, "veclib::Slice.delta(): Trying to shrink the slice by too much");
            count -= -x; // Just to avoid signed vs unsigned errors
        } else count += x;
        return *this;
    }
    /// @brief Use a signed parameter to act like `grow()` when it is positive and
    ///        `shrink` when it is negative on a copy of this slice
    /// @param x The signed parameter
    /// @return A copy if this slice, on which the operation was performed
    /// @throw `std::runtime_error` if the slice isn't referencing anything,
    ///        `std::underflow_error` if `x` is negative and it is trying to shrink
    ///        the slice by too much, only if exceptions are not disabled
    inline constexpr Self delta_copy(diff_t x) const VECLIB_NOEXCEPT {
        Self self = *this;
        return self.delta(x);
    }

    /// @brief Slide the whole view of the slice backward
    /// @param x The number of elements to slide the view by
    /// @return A reference to the modified object
    /// @throw `std::runtime_error` if the slice isn't referencing anything and
    ///        exceptions are not disabled
    inline constexpr Self& slide_backw(std::size_t x) VECLIB_NOEXCEPT {
        this->nullptr_check("veclib::Slice.slide_backw(): Slice isn't referencing anything");
        data -= x;
        return *this;
    }
    /// @brief Slide the whole view of a copy of this slice backward
    /// @param x The number of elements to slide the view by
    /// @return A copy of this slice, on which the operation was performed
    /// @throw `std::runtime_error` if the slice isn't referencing anything and
    ///        exceptions are not disabled
    inline constexpr Self slide_backw_copy(std::size_t x) const VECLIB_NOEXCEPT {
        Self self = *this;
        return self.slide_backw(x);
    }

    /// @brief Slide the whole view of the slice forward
    /// @param x The number of elements to slide the view by
    /// @return A reference to the modified object
    /// @throw `std::runtime_error` if the slice isn't referencing anything and
    ///        exceptions are not disabled
    inline constexpr Self& slide_forw(std::size_t x) VECLIB_NOEXCEPT {
        this->nullptr_check("veclib::Slice.slide_forw(): Slice isn't referencing anything");
        data += x;
        return *this;
    }
    /// @brief Slide the whole view of a copy of this slice forward
    /// @param x The number of elements to slide the view by
    /// @return A copy of this slice, on which the operation was performed
    /// @throw `std::runtime_error` if the slice isn't referencing anything and
    ///        exceptions are not disabled
    inline constexpr Self slide_forw_copy(std::size_t x) const VECLIB_NOEXCEPT {
        Self self = *this;
        return self.slide_forw(x);
    }

    /// @brief Slide the whole view of the slice by using a signed parameter,
    ///        when it is negative the slide is backward and when it is
    ///        positive the slide is forward
    /// @param x The number of elements to slide the view by
    /// @return A reference to the modified object
    /// @throw `std::runtime_error` if the slice isn't referencing anything and
    ///        exceptions are not disabled
    inline constexpr Self& slide(diff_t x) VECLIB_NOEXCEPT {
        this->nullptr_check("veclib::Slice.slide(): Slice isn't referencing anything");
        data += x; // Signed arithmetic does its job
        return *this;
    }
    /// @brief Slide the whole view of a copy of this slice by using a signed parameter,
    ///        when it is negative the slide is backward and when it is
    ///        positive the slide is forward
    /// @param x The number of elements to slide the view by
    /// @return A copy of this slice, on which the operation was performed
    /// @throw `std::runtime_error` if the slice isn't referencing anything and
    ///        exceptions are not disabled
    inline constexpr Self slide_copy(diff_t x) const VECLIB_NOEXCEPT {
        Self self = *this;
        return self.slide(x);
    }

    /// @brief Trim a specified number of elements from the beginning of the slice
    /// @param x The number of elements to trim
    /// @return A reference to the modified object
    /// @throw `std::runtime_error` if the slice is empty, `std::overflow_error` if `x`
    ///        exceeds the number of elements in this slice, only if exceptions
    ///        are not disabled
    inline constexpr Self& trim(std::size_t x) VECLIB_NOEXCEPT {
        this->empty_check("veclib::Slice.trim(): Slice is empty");
        this->overflow_check(x, "veclib::Slice.trim(): Trying to trim too many elements");
        data += x;
        count -= x;
        return *this;
    }
    /// @brief Trim a specified number of elements from the beginning of a copy of this slice
    /// @param x The number of elements to trim
    /// @return A copy of this slice, on which the operation was performed
    /// @throw `std::runtime_error` if the slice is empty, `std::overflow_error` if `x`
    ///        exceeds the number of elements in this slice, only if exceptions
    ///        are not disabled
    inline constexpr Self trim_copy(std::size_t x) const VECLIB_NOEXCEPT {
        Self self = *this;
        return self.trim(x);
    }

    /// @brief Change the view of this slice to reference the specified number of elements
    ///        at its beginning
    /// @param x The number of new elements at the beginning
    /// @return A reference to the modified object
    /// @throw `std::runtime_error` if the slice isn't referencing anything, only if
    ///        exceptions are not disabled
    inline constexpr Self& extend(std::size_t x) VECLIB_NOEXCEPT {
        this->nullptr_check("veclib::Slice.extend(): Slice isn't referencing anything");
        data -= x;
        count += x;
        return *this;
    }
    /// @brief Change the view of a copy of this slice to reference the specified number
    ///        of elements at its beginning
    /// @param x The number of new elements at the beginning
    /// @return A copy of this slice, on which the operation was performed
    /// @throw `std::runtime_error` if the slice isn't referencing anything, only if
    ///        exceptions are not disabled
    inline constexpr Self extend_copy(std::size_t x) const VECLIB_NOEXCEPT {
        Self self = *this;
        return self.extend(x);
    }

    /// @brief Change the view of this slice with a signed parameter, acting as `trim` if it is positive
    ///        and acting as `extend` if it is negative
    /// @param x The signed parameter
    /// @return A reference to the modified object
    /// @throw `std::runtime_error` if the slice isn't referencing anything or if the slice is empty,
    ///        `std::overflow_error` if `x` is positive and tries to trim too many elements, only
    ///        if exceptions are not disabled
    inline constexpr Self& nudge(diff_t x) VECLIB_NOEXCEPT {
        if (x < 0) {
            this->nullptr_check("veclib::Slice.nudge(): Slice isn't referencing anything");
            count += -x; // Just to avoid signed vs unsigned errors
        } else {
            this->empty_check("veclib::Slice.nudge(): Slice is empty");
            this->overflow_check(x, "veclib::Slice.nudge(): Trying to trim too many elements");
            count += x;
        }
        data += x;
        return *this;
    }
    /// @brief Change the view of a copy of this slice with a signed parameter, acting as `trim`
    ///        if it is positive and acting as `extend` if it is negative
    /// @param x The signed parameter
    /// @return A copy of this slice, on which the operation was performed
    /// @throw `std::runtime_error` if the slice isn't referencing anything or if the slice is empty,
    ///        `std::overflow_error` if `x` is positive and tries to trim too many elements, only
    ///        if exceptions are not disabled
    inline constexpr Self nudge_copy(diff_t x) const VECLIB_NOEXCEPT {
        Self self = *this;
        return self.nudge(x);
    }

    /// @brief Trim the first element off this slice (same as running `trim(1)`)
    /// @return A reference to the modified object
    /// @throw `std::runtime_error` if the slice is empty
    inline constexpr Self& consume_front() VECLIB_NOEXCEPT {
        this->empty_check("veclib::Slice.consume_front(): Slice is empty");
        ++data;
        --count;
        return *this;
    }
    /// @brief Trim the first element off a copy of this slice (same as running `trim_copy(1)`)
    /// @return A copy of this slice, on which the operation was performed
    /// @throw `std::runtime_error` if the slice is empty
    inline constexpr Self consume_front_copy() const VECLIB_NOEXCEPT {
        Self self = *this;
        return self.consume_front();
    }

    /// @brief Trim the last element off this slice (same as running `shrink(1)`)
    /// @return A reference to the modified object
    /// @throw `std::runtime_error` if the slice is empty
    inline constexpr Self& consume_back() VECLIB_NOEXCEPT {
        this->empty_check("veclib::Slice.consume_front(): Slice is empty");
        --count;
        return *this;
    }
    /// @brief Trim the last element off a copy of this slice (same as running `shrink(1)`)
    /// @return A copy of this slice, on which the operation was performed
    /// @throw `std::runtime_error` if the slice is empty
    inline constexpr Self consume_back_copy() const VECLIB_NOEXCEPT {
        Self self = *this;
        return self.consume_back();
    }

    /// @brief Change the size of this slice
    /// @param x The new size
    /// @return A reference to the modified object
    inline constexpr Self& resize(std::size_t x) noexcept {
        count = x;
        return *this;
    }
    /// @brief Change the size of a copy of this slice
    /// @param x The new size
    /// @return A copy of this slice, on which the operation was performed
    inline constexpr Self resize_copy(std::size_t x) const noexcept {
        Self self = *this;
        return self.resize(x);
    }

    /// @brief Compare two slices for equality
    /// @param other The other slice to compare
    inline constexpr bool operator==(const Self& other) const noexcept { return (data == other.data) && (count == other.count); }
    /// @brief Compare two slices for inequality
    /// @param other The other slice ot compare
    inline constexpr bool operator!=(const Self& other) const noexcept { return !(*this == other); }
    /// @brief Cast this slice to a boolean, evaluating to `true` if the slice is not empty
    inline constexpr operator bool() const noexcept { return !this->empty(); }

    /// @brief Return a new slice referencing the intersection of the memory
    ///        regions referenced by two slices
    /// @param a The first slice
    /// @param b The second slice
    friend inline constexpr Self intersect(const Self& a, const Self& b) noexcept {
        // If either slice is empty, the intersection will be empty
        if (a.empty() || b.empty()) return Self(a.get(), 0);

        // Find the boundaries in memory
        Type* a_end = a.get() + a.size();
        Type* b_end = b.get() + b.size();

        // The intersection starts at the highest starting pointer
        Type* start = a.get() > b.get() ? a.get() : b.get(); // Maximum
        // The intersection ends at the lowest ending pointer
        Type* end = a_end < b_end ? a_end : b_end; // Minimum

        Self output(start, 0);
        // If start is past or equal to end, there is no overlap
        if (start >= end) return output;
        // It already points to the correct location
        return output.resize(end - start); // Use the correct size
    }
    /// @brief Change this slice to be the intersection of the memory regions referenced by
    ///        itself and another slice
    /// @param other The other slice
    /// @return A reference to the modified object
    inline constexpr Self& intersect(const Self& other) VECLIB_NOEXCEPT { return *this = ::veclib::intersect(*this, other); }
    /// @brief Return a new slice referencing the intersection of the memory
    ///        regions referenced by this slice and another one
    /// @param other The other slice
    inline constexpr Self intersect_copy(const Self& other) VECLIB_NOEXCEPT {
        Self self = *this;
        return self.intersect(other);
    }

    /// @brief Return a new slice referencing the union of the memory
    ///        regions referenced by two slices
    /// @param a The first slice
    /// @param b The second slice
    /// @throw `std::logic_error` if the two slices are not adjacent or do not overlap,
    ///        only if exceptions are not disabled
    friend inline constexpr Self join(const Self& a, const Self& b) VECLIB_NOEXCEPT {
        // If either is empty the join is just the non-empty slice
        if (a.empty()) return b;
        if (b.empty()) return a;

        Type* a_end = a.get() + a.size();
        Type* b_end = b.get() + b.size();

        // Check if slices overlap or are adjacent in memory
        bool overlap = (a.get() <= b_end) && (b.get() <= a_end);

        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(overlap);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (!overlap) throw std::logic_error("veclib::join(): Can't join non-adjacent slices");
        #endif // VECLIB_ASSERT_NOEXCEPT

        // The join starts at the lowest starting pointer and ends at the highest ending pointer
        Type* start = a.get() < b.get() ? a.get() : b.get(); // Minimum
        Type* end = a_end > b_end ? a_end : b_end; // Maximum

        return Self(start, end - start);
    }
    /// @brief Change this slice to be the union of the memory regions referenced by
    ///        itself and another slice
    /// @param other The other slice
    /// @return A reference to the modified object
    /// @throw `std::logic_error` if the two slices are not adjacent or do not overlap,
    ///        only if exceptions are not disabled
    inline constexpr Self& join(const Self& other) VECLIB_NOEXCEPT { return *this = ::veclib::join(*this, other); }
    /// @brief Return a new slice referencing the union of the memory
    ///        regions referenced by this slice and another one
    /// @param other The other slice
    /// @throw `std::logic_error` if the two slices are not adjacent or do not overlap,
    ///        only if exceptions are not disabled
    inline constexpr Self join_copy(const Self& other) VECLIB_NOEXCEPT {
        Self self = *this;
        return self.join(other);
    }

    /// @brief Check if a provided pointer falls inside the memory region referenced by this slice
    /// @param ptr The pointer to check
    /// @return `true` if the pointer is included in the memory region referenced by this slice
    inline constexpr bool contains(Type* ptr) const noexcept { return (ptr >= data) && (ptr < data + count); }
    /// @brief Check if a whole slice falls inside the memory region referenced by this slice
    /// @param other The slice to check
    /// @return `true` if the starting address of the other slice is greater than or equal to the
    ///         starting address of this slice, and if the ending address of the other slice is
    ///         less than or equal to the ending address of this slice
    inline constexpr bool contains(const Self& other) const noexcept {
        if (this->contains(other.data) && other.count == 0) return true;
        return (other.data >= data) && (other.data + other.count <= data + count);
    }

    /// @brief Check if a whole slice's referenced memory region overlaps with this slice's
    /// @param other The other slice to check
    /// @return `true` if the starting address of this slice is less than the ending address
    ///         of the other slice, and the starting address of the other slice is less than
    ///         the ending address of this slice
    inline constexpr bool overlaps(const Self& other) const noexcept {
        if (count == 0 || other.count == 0) return false;
        return (data < other.data + other.count) && (other.data < data + count);
    }

    /// @brief Split this slice into two at the specified index, the returned slice is the
    ///        slice past the split and this slice is modified to reference the memory before
    ///        the split
    /// @param i The index at which to split this slice
    /// @return A slice referencing the memory of this slice starting at the specified index
    /// @throw `std::overflow_error` if the provided index exceeds this slice's size
    inline constexpr Self split_at(std::size_t i) VECLIB_NOEXCEPT {
        this->overflow_check(i, "veclib::Slice.split_at(): Trying to split too far");
        Self output(data + i, count - i); // Reference the correct range
        count = i; // Shrink ourselves
        return output;
    }

    #ifdef VECLIB_EXTRA

    /// @brief Align the region of memory referenced by this slice to a specified alignment.
    ///        This will potentially cut off elements from the beginning and/or end of the
    ///        slice to satisfy the provided alignment
    /// @param alignment The alignment to apply to the memory region of this slice
    /// @return A reference to the modified slice
    inline constexpr Self& align_to(std::size_t alignment) noexcept {
        // Truncate the front of the slice
        std::uintptr_t raw_addr = reinterpret_cast<std::uintptr_t>(data);
        std::uintptr_t misalignment = raw_addr % alignment;

        if (misalignment == 0) return *this;

        std::size_t bytes = alignment - misalignment;
        // Round up the number of elements to truncate
        std::size_t elements = (bytes + sizeof(Type) - 1) / sizeof(Type);

        if (elements > count) this->clear();
        else {
            data += elements;
            count -= elements;
        }

        // Truncate the end of the slice
        bytes = count * sizeof(Type);
        misalignment = bytes % alignment;

        if (misalignment == 0) return *this;

        // Calculate how many trailing elements we need to discard
        elements = misalignment / sizeof(Type);

        // If the misaligned bytes are smaller than a single element, we discard one full element
        if (elements == 0 && misalignment > 0) elements = 1;

        if (elements >= count) this->clear();
        else count -= elements;

        return *this;
    }
    /// @brief Create a copy of this slice, aligned to a provided alignment.
    ///        This will potentially cut off elements from the beginning and/or end of the
    ///        slice to satisfy the provided alignment
    /// @param alignment The alignment to apply to the memory region of this slice
    /// @return A copy of this slice, which was aligned to the specified alignment
    inline constexpr Self align_to_copy(std::size_t alignment) const noexcept {
        Self self = *this;
        return self.align_to(alignment);
    }
    /// @brief Same as running `align_to(alignof(AlignmentType))`
    /// @tparam AlignmentType The type of which to use the alignment to pass into `align_to`,
    ///         applied to this slice
    /// @return A reference to the modified object
    template <typename AlignmentType>
    inline constexpr Self& align() noexcept { return this->align_to(alignof(AlignmentType)); }
    /// @brief Same as running `align_to_copy(alignof(AlignmentType))`
    /// @tparam AlignmentType The type of which to use the alignment to pass into `align_to_copy`,
    ///         applied to this slice
    /// @return A copy of this slice, on which the operation was performed
    template <typename AlignmentType>
    inline constexpr Self align_copy() const noexcept {
        Self self = *this;
        return self.align<AlignmentType>();
    }
    /// @brief Same as running `align_to(alignof(Type))`. This essentially naturalizes the
    ///        alignment of this slice
    /// @return A reference to the modified object
    inline constexpr Self& self_align() noexcept { return this->align<Type>(); }
    /// @brief Same as running `align_to_copy(alignof(Type))`. This essentially naturalizes
    ///        the alignment of this slice
    /// @return A copy of this slice, on which the operation was performed
    inline constexpr Self self_align_copy() const noexcept {
        Self self = *this;
        return self.self_align();
    }

    #endif // VECLIB_EXTRA

    /// @brief Type-pun this slice into a slice of a chosen type
    /// @tparam NewType The type of the punned slice
    /// @note This function doesn't take into account alignment and
    ///       type size compatibility, if you want a safer
    ///       alternative you should use `strict_into`
    template <typename NewType>
    inline constexpr Slice<NewType> into() const noexcept {
        if (this->empty()) return Slice<NewType>(); // Zeroed by default

        // Calculate total byte size of current slice
        std::size_t bytes = count * sizeof(Type);
        // Calculate how many elements of NewType fit into that byte size
        std::size_t new_count = bytes / sizeof(NewType);

        return Slice<NewType>(reinterpret_cast<NewType*>(data), new_count);
    }
    /// @brief Safely type-pun this slice into a slice of a chosen type
    ///        by checking for const-correctness errors, too-tight alignment
    ///        requirements and compatible type sizes
    /// @tparam NewType The type of the punned slice
    template <typename NewType>
    requires (
        // Maintain const-correctness by requiring NewType to be const if Type is const
        (std::is_const_v<Type> ? std::is_const_v<NewType> : true) &&
        // Require a looser or equal alignment for NewType
        (alignof(NewType) <= alignof(Type)) &&
        // Require sizes to be evenly divisible across the two types
        (sizeof(Type) % sizeof(NewType) == 0 || sizeof(NewType) % sizeof(Type) == 0)
    )
    inline constexpr Slice<NewType> strict_into() const noexcept {
        if (this->empty()) return Slice<NewType>();

        // Calculate total byte size of current slice
        std::size_t bytes = count * sizeof(Type);

        // Runtime check for correct alignment and size
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(reinterpret_cast<std::uintptr_t>(data) % alignof(NewType) == 0);
        assert(bytes % sizeof(NewType) == 0);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (reinterpret_cast<std::uintptr_t>(data) % alignof(NewType) != 0)
            throw std::logic_error("veclib::Slice.strict_into(): Pointer alignment of the"
            "current slice doesn't allow creating a slice of the specified type");
        if (bytes % sizeof(NewType) != 0)
            throw std::logic_error("veclib::Slice.strict_into(): Can't create a slice of"
            "the specified type because byte truncation would occur");
        #endif // VECLIB_ASSERT_NOEXCEPT

        std::size_t new_count = bytes / sizeof(NewType);
        return Slice<NewType>(reinterpret_cast<NewType*>(data), new_count);
    }

    /// @brief Construct a slice referencing a portion of this slice's memory region
    /// @param offset The index into this slice at which the slice should start
    /// @param len The lenght of the new slice
    /// @throw `std::overflow_error` if the offset exceeds the memory region of this slice
    ///        or if the offset plus the length exceed it
    inline constexpr Self subslice(std::size_t offset, std::size_t len) const VECLIB_NOEXCEPT {
        this->overflow_check(offset, "veclib::Slice.subslice(): Offset exceeds slice boundaries");
        this->overflow_check(offset + len, "veclib::Slice.subslice(): Length exceeds slice boundaries");
        return Self(data + offset, len);
    }

    /// @brief Keep trimming elements from the beginning of the slice while a custom
    ///        function returns `true`. The custom function must return `bool` and
    ///        must accept a `const Type&` parameter, into which will be passed the
    ///        next element to check for trimming
    /// @param fn The custom function
    /// @return A reference to the modified object
    inline constexpr Self& trim_while(Function<bool, const Type&> fn) noexcept {
        while (count > 0 && fn(data)) this->consume_front();
        return *this;
    } // Should there be a copy variant?
    /// @brief Keep trimming elements while the first element is equal to a provided value
    /// @param value The value to check against for equality
    /// @return A reference to the modified object
    inline constexpr Self& trim_leading(const Type& value) noexcept
            requires (std::equality_comparable<Type>) {
        return this->trim_while([&value] (const Type& x) { return x == value; });
    } // And here too?
    /// @brief Keep trimming elements from the beginning of the slice while a custom
    ///        function returns `true`. The custom function must return `bool` and
    ///        must accept a `const Type&` parameter, into which will be passed the
    ///        next element to check for trimming
    /// @param fn The custom function
    /// @return A reference to the modified object
    inline constexpr Self& trim_until(Function<bool, const Type&> fn) noexcept {
        while (count > 0 && !fn(data)) this->consume_front();
        return *this;
    } // And here too?!?!?

    #ifndef VECLIB_NO_OPERATOR_OVERLOADS // We create extra operator overloads unless it's not requested

    /// @brief Operator overload for calling `grow`
    inline constexpr Self& operator+=(std::size_t x) VECLIB_NOEXCEPT { return this->grow(x); }
    /// @brief Operator overload for calling `grow_copy`
    inline constexpr Self operator+(std::size_t x) const VECLIB_NOEXCEPT { return this->grow_copy(x); }
    /// @brief Operator overload for calling `shrink`
    inline constexpr Self& operator-=(std::size_t x) { return this->shrink(x); }
    /// @brief Operator overload for calling `shrink_copy`
    inline constexpr Self operator-(std::size_t x) const { return this->shrink_copy(x); }

    /// @brief Operator overload for calling `slide_backw`
    inline constexpr Self& operator<<=(std::size_t x) VECLIB_NOEXCEPT { return this->slide_backw(x); }
    /// @brief Operator overload for calling `slide_backw_copy`
    inline constexpr Self operator<<(std::size_t x) const VECLIB_NOEXCEPT { return this->slide_backw_copy(x); }
    /// @brief Operator overload for calling `slide_forw`
    inline constexpr Self& operator>>=(std::size_t x) VECLIB_NOEXCEPT { return this->slide_forw(x); }
    /// @brief Operator overload for calling `slide_forw_copy`
    inline constexpr Self operator>>(std::size_t x) const VECLIB_NOEXCEPT { return this->slide_forw_copy(x); }

    /// @brief Operator overload for calling `consume_front`
    inline constexpr Self& operator++() { return this->consume_front(); }
    /// @brief Operator overload for calling `consume_front_copy`
    inline constexpr Self operator+() const { return this->consume_front_copy(); }
    /// @brief Operator overload for calling `consume_back`
    inline constexpr Self& operator--() { return this->consume_back(); }
    /// @brief Operator overload for calling `consume_back_copy`
    inline constexpr Self operator-() const { return this->consume_back_copy(); }

    /// @brief Operator overload for calling `intersect`
    inline constexpr Self& operator&=(const Self& other) noexcept { return this->intersect(other); }
    /// @brief Operator overload for calling `intersect_copy`
    inline constexpr Self operator&(const Self& other) const noexcept { return ::veclib::intersect(*this, other); }
    /// @brief Operator overload for calling `join`
    inline constexpr Self& operator|=(const Self& other) VECLIB_NOEXCEPT { return this->join(other); }
    /// @brief Operator overload for calling `join_copy`
    inline constexpr Self operator|(const Self& other) const VECLIB_NOEXCEPT { return ::veclib::join(*this, other); }

    #endif // VECLIB_NO_OPERATOR_OVERLOADS
};


} // namespace veclib

#endif // VECLIB_HPP
