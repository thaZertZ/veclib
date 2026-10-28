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
#include <bit> // std::bit_cast()
#include <concepts>

#if defined(VECLIB_NONCONSTRUCTOR_NEW) \
 || defined(VECLIB_NONDESTRUCTOR_DELETE) \
 || defined(VECLIB_EXPLICIT_NONDESTRUCTOR_DELETE) \
 || defined(VECLIB_FULLY_COPYABLE) \
 || defined(VECLIB_FULLY_MOVABLE) \
 || defined(VECLIB_FULLY_NOEXCEPT_COPYABLE) \
 || defined(VECLIB_FULLY_NOEXCEPT_MOVABLE) \
 || defined(VECLIB_NOEXCEPT_DEFAULT_CONSTRUCTIBLE) \
 || defined(VECLIB_NOEXCEPT_DESTRUCTIBLE) \
 || defined(VECLIB_NOEXCEPT_COPY_CONSTRUCTIBLE) \
 || defined(VECLIB_NOEXCEPT_COPY_ASSIGNABLE) \
 || defined(VECLIB_NOEXCEPT_MOVE_CONSTRUCTIBLE) \
 || defined(VECLIB_NOEXCEPT_MOVE_ASSIGNABLE) \
 || defined(VECLIB_NOEXCEPT_EQ_CMP) \
 || defined(VECLIB_VECTOR_DEFAULT_ALLOC) \
 || defined(VECLIB_TEMP_CHOOSE_MOVE_OR_COPY) \
 || defined(VECLIB_TEMP_SELECT_NOEXCEPT) \
 || defined(VECLIB_BYTESIZE) \
 || defined(VECLIB_BITSIZE) \
 || defined(VECLIB_COND_NOEXCEPT) \
 || defined(VECLIB_NOEXCEPT) \
 || defined(VECLIB_NOEXCEPT_IF)
#error "veclib can't be compiled if internal macros are already defined"
#endif // VECLIB_*

/// @brief A collection of array-like data structures (arrays, vectors, slices, lists...)
namespace veclib {


/// @brief Allocate objects on the heap without calling the
///        constructor of their type
/// @param type The type of elements to allocate The type to inspect
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
/// @param type The type of elements to deallocate The type to inspect
/// @param count The number of elements to deallocate
#define VECLIB_EXPLICIT_NONDESTRUCTOR_DELETE(data, type, count) \
    (::operator delete((data), (count) * sizeof(type)))

/// @brief Helper macro that checks if a given type can be copy-constructed and copy-assigned
/// @param type The type to inspect
#define VECLIB_FULLY_COPYABLE(type) \
    (std::is_copy_constructible_v<type> && std::is_copy_assignable_v<type>)

/// @brief Helper macro that checks if a given type can be move-constructed and move-assigned
/// @param type The type to inspect
#define VECLIB_FULLY_MOVABLE(type) \
    (std::is_move_constructible_v<type> && std::is_move_assignable_v<type>)

/// @brief Helper macro that checks if all copy operations of a given type are `noexcept`
/// @param type The type to inspect
#define VECLIB_FULLY_NOEXCEPT_COPYABLE(type) \
    (std::is_nothrow_copy_constructible_v<type> && std::is_nothrow_copy_assignable_v<type>)

/// @brief Helper macro that checks if all move operations of a given type are `noexcept`
/// @param type The type to inspect
#define VECLIB_FULLY_NOEXCEPT_MOVABLE(type) \
    (std::is_nothrow_move_constructible_v<type> && std::is_nothrow_move_assignable_v<type>)

/// @brief Expands to a conditional `noexcept` clause that evaluates to
///        `noexcept(true)` if a specified type's default constructor
///        is `noexcept`
/// @param type The type to inspect
#define VECLIB_NOEXCEPT_DEFAULT_CONSTRUCTIBLE(type) \
    noexcept(std::is_nothrow_default_constructible_v<type>)

/// @brief Expands to a conditional `noexcept` clause that evaluates to
///        `noexcept(true)` if a specified type's destructor is `noexcept`
/// @param type The type to inspect
#define VECLIB_NOEXCEPT_DESTRUCTIBLE(type) \
    noexcept(std::is_nothrow_destructible_v<type>)

/// @brief Expands to a conditional `noexcept` clause that evaluates to
///        `noexcept(true)` if a specified type's copy constructor is
///        `noexcept`
/// @param type The type to inspect
#define VECLIB_NOEXCEPT_COPY_CONSTRUCTIBLE(type) \
    noexcept(std::is_nothrow_copy_constructible_v<type>)
/// @brief Expands to a conditional `noexcept` clause that evaluates to
///        `noexcept(true)` if a specified type's copy assignment
///        operator is `noexcept`
/// @param type The type to inspect
#define VECLIB_NOEXCEPT_COPY_ASSIGNABLE(type) \
    noexcept(std::is_nothrow_copy_assignable_v<type>)

/// @brief Expands to a conditional `noexcept` clause that evaluates to
///        `noexcept(true)` if a specified type's move constructor is
///        `noexcept`
/// @param type The type to inspect
#define VECLIB_NOEXCEPT_MOVE_CONSTRUCTIBLE(type) \
    noexcept(std::is_nothrow_move_constructible_v<type>)
/// @brief Expands to a conditional `noexcept` clause that evaluates to
///        `noexcept(true)` if a specified type's move assignment
///        operator is `noexcept`
/// @param type The type to inspect
#define VECLIB_NOEXCEPT_MOVE_ASSIGNABLE(type) \
    noexcept(std::is_nothrow_move_assignable_v<type>)

/// @brief Expands to a conditional `noexcept` clause that evaluates to
///        `noexcept(true)` if a specified type's equality comparison is
///        `noexcept`
/// @param type The type to inspect
#define VECLIB_NOEXCEPT_EQ_CMP(type) \
    noexcept(noexcept(std::declval<type>() == std::declval<type>()))

/// @brief Expands to the minimum number of bytes required to fit the
///        provided number of bits
#define VECLIB_BYTESIZE(expr) (((expr) + (CHAR_BIT - 1) /* round up */) / CHAR_BIT)
/// @brief Expands to the number of bits contained in the provided
///        number of bytes
#define VECLIB_BITSIZE(expr) ((expr) * CHAR_BIT)

/// @brief Expands to a conditional `noexcept` clause into which is passed
///        an expression that calls a given function, evaluating to
///        `noexcept(false)` if it is not `noexcept` itself
/// @param ident The identifier of the function to check
/// @param __VA_ARGS__... The arguments to pass to the function, these can be
///                       `std::declval<>()` calls or just arbitrary values
#define VECLIB_COND_NOEXCEPT(ident, ...) \
    noexcept(noexcept(std::declval<decltype(ident)>()(__VA_ARGS__)))

#ifdef VECLIB_ASSERT_NOEXCEPT

/// @brief Expands to `noexcept` if `VECLIB_ASSERT_NOEXCEPT` is defined
#define VECLIB_NOEXCEPT noexcept(true)

#else // VECLIB_ASSERT_NOEXCEPT

/// @brief Expands to `noexcept(true)` if `VECLIB_ASSERT_NOEXCEPT`
///        is defined and `noexcept(false)` if it is not defined
#define VECLIB_NOEXCEPT noexcept(false)

#endif // VECLIB_ASSERT_NOEXCEPT

#define VECLIB_NOEXCEPT_IF(...) \
    noexcept(__VA_ARGS__)

/// @brief Type alias for the signed equivalent of `std::size_t`
using diff_t = std::make_signed_t<std::size_t>;

/// @brief Templated type alias for a function signature
/// @tparam Ret The return type of the function
/// @tparam ...Args The type of the arguments of the function
template <typename Ret, typename... Args>
using Function = Ret(*)(Args...);

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
public: // Public section for defining aliases in advance

    /// @brief Member type alias for accessing the referenced type
    using Item = Type;
    /// @brief Member type alias for the type of ourselves
    using Self = Slice<Type>;

private:
    /// @brief Data pointer
    Type* data = nullptr;
    /// @brief The number of referenced elements
    std::size_t count = 0;

    void bounds_check(const std::size_t i, const char* const str) const VECLIB_NOEXCEPT {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(i < count && str);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (i >= count) throw std::out_of_range(str);
        #endif // VECLIB_ASSERT_NOEXCEPT
    }
    void nullptr_check(const char* const str) const VECLIB_NOEXCEPT {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data != nullptr && str);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error(str);
        #endif // VECLIB_ASSERT_NOEXCEPT
    }
    void empty_check(const char* const str) const VECLIB_NOEXCEPT {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data != nullptr && str);
        assert(count != 0 && str);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr || count == 0) throw std::runtime_error(str);
        #endif // VECLIB_ASSERT_NOEXCEPT
    }
    void underflow_check(const std::size_t i, const char* const str) const VECLIB_NOEXCEPT {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(i <= count && str);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (i > count) throw std::underflow_error(str);
        #endif // VECLIB_ASSERT_NOEXCEPT
    }
    void range_check(const std::size_t i, const char* const str) const VECLIB_NOEXCEPT {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(i <= count && str);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (i > count) throw std::out_of_range(str);
        #endif // VECLIB_ASSERT_NOEXCEPT
    }
    void zero_check(const std::size_t i, const char* const str) const VECLIB_NOEXCEPT {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(i != 0 && str);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (i == 0) throw std::runtime_error(str);
        #endif // VECLIB_ASSERT_NOEXCEPT
    }

    inline constexpr Self internal_subslice(const std::size_t start, const std::size_t end) VECLIB_NOEXCEPT {
        this->nullptr_check("veclib::Slice.subslice(): The slice isn't referencing anything");
        this->range_check(start, "veclib::Slice.subslice(): Starting index exceeds slice size");
        this->range_check(end, "veclib::Slice.subslice(): Starting index exceeds slice size");
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(end >= start);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (end < start) throw std::logic_error("veclib::Slice.subslice(): Ending index is lower than starting index");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return Self(data + start, end - start);
    }
    inline constexpr const Self internal_subslice(const std::size_t start, const std::size_t end) const VECLIB_NOEXCEPT {
        this->nullptr_check("veclib::Slice.subslice(): The slice isn't referencing anything");
        this->range_check(start, "veclib::Slice.subslice(): Starting index exceeds slice size");
        this->range_check(end, "veclib::Slice.subslice(): Starting index exceeds slice size");
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(end >= start);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (end < start) throw std::logic_error("veclib::Slice.subslice(): Ending index is lower than starting index");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return Self(data + start, end - start);
    }

public:

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
        other.clear();
    }
    /// @brief Move assignment
    /// @param other The object to move
    /// @return A reference to the modified object
    inline constexpr Self& operator=(Self&& other) noexcept {
        *this = other;
        other.clear();
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
    /// @brief Put the slice into an empty state, by setting the data pointer
    ///        to `nullptr` and the size to `0`, such that calling `null()`
    ///        or `empty()` will return `true`. To check the cases of `null()`
    ///        and `empty()` simultaneously, define `VECLIB_EXTRA` and then
    ///        use `cleared()` or `moved()`
    /// @return A reference to the modified object
    inline constexpr Self& clear() noexcept {
        data = nullptr;
        count = 0;
        return *this;
    }

    /// @brief Returns `true` if `clear()` was called on the slice, or if the slice was moved
    inline constexpr bool cleared() const noexcept { return this->empty() && this->null(); }
    /// @brief Alias for `cleared()`
    inline constexpr bool moved() const noexcept { return this->cleared(); }

    /// @brief Access elements into the slice with no bounds checking
    /// @param i The index into the slice
    /// @return A reference to the indexed element
    inline constexpr Type& operator[](const std::size_t i) noexcept { return data[i]; }
    inline constexpr const Type& operator[](const std::size_t i) const noexcept { return data[i]; }

    /// @brief Access elements into the slice with bounds checking
    /// @param i The index into the slice
    /// @return A reference to the indexed element
    /// @throw `std::out_of_range` if the index is out of bounds and exceptions are not disabled
    inline constexpr Type& at(const std::size_t i) VECLIB_NOEXCEPT {
        this->bounds_check(i, "veclib::Slice.at(): Index is out of bounds");
        return data[i];
    }
    inline constexpr const Type& at(const std::size_t i) const VECLIB_NOEXCEPT {
        this->bounds_check(i, "veclib::Slice.at(): Index is out of bounds");
        return data[i];
    }

    #ifdef VECLIB_EXTRA

    inline constexpr Type& circular_at(const std::size_t i) noexcept { return data[i % count]; }
    inline constexpr const Type& circular_at(const std::size_t i) const noexcept { return data[i % count]; }

    #ifndef VECLIB_NO_OPERATOR_OVERLOADS

    inline constexpr Type& operator()(const std::size_t i) noexcept { return data[i % count]; }
    inline constexpr const Type& operator()(const std::size_t i) const noexcept { return data[i % count]; }

    #endif // VECLIB_NO_OPERATOR_OVERLOADS

    inline constexpr Type& clamped_at(const std::size_t i) noexcept { return data[i >= count ? count - 1 : i]; }
    inline constexpr const Type& clamped_at(const std::size_t i) const noexcept { return data[i >= count ? count - 1 : i]; }

    #endif // VECLIB_EXTRA

    /// @brief Check if a given index is valid in this slice
    /// @param i The index to check
    /// @return `true` if the index is valid
    inline constexpr bool inside_bounds(const std::size_t i) const noexcept { return i < count; }

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
    /// @throw `std::runtime_error` if the slice is empty, `std::out_of_range` if the
    ///        number of elements exceeds this slice's size, only if exceptions are not disabled
    inline constexpr Self first(const std::size_t x) VECLIB_NOEXCEPT {
        this->empty_check("veclib::Slice.first(): Slice is empty");
        this->range_check(x, "veclib::Slice.first(): Trying to extract too many first elements");
        return Self(data, x);
    }
    inline constexpr const Self first(const std::size_t x) const VECLIB_NOEXCEPT {
        this->empty_check("veclib::Slice.first(): Slice is empty");
        this->range_check(x, "veclib::Slice.first(): Trying to extract too many first elements");
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
    /// @throw `std::runtime_error` if the slice is empty, `std::out_of_range` if the
    ///        number of elements exceeds this slice's size, only if exceptions are not disabled
    inline constexpr Self last(const std::size_t x) VECLIB_NOEXCEPT {
        this->empty_check("veclib::Slice.last(): Slice is empty");
        this->range_check(x, "veclib::Slice.last(): Trying to extract too many last elements");
        return Self(data + (count - x), x);
    }
    inline constexpr const Self last(const std::size_t x) const VECLIB_NOEXCEPT {
        this->empty_check("veclib::Slice.last(): Slice is empty");
        this->range_check(x, "veclib::Slice.last(): Trying to extract too many last elements");
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
            VECLIB_COND_NOEXCEPT(fn, std::declval<Type&>()) {
        if (this->empty()) return *this;
        for (std::size_t i = 0; i < count; ++i)
            fn(data[i]);
        return *this;
    }
    inline constexpr Self& map(Function<void, Type&, std::size_t> fn)
            VECLIB_COND_NOEXCEPT(fn, std::declval<Type&>(), 0) {
        if (this->empty()) return *this;
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
    inline constexpr Type fold(Function<void, Type&, const Type&> fn, Type acc = {})
            const VECLIB_COND_NOEXCEPT(fn, std::declval<Type&>(), std::declval<const Type&>())
            requires (std::is_default_constructible_v<Type>) {
        if (this->empty()) return acc;
        for (std::size_t i = 0; i < count; ++i)
            fn(acc, data[i]); // How can we make `acc` a `const Type&`?!?
        return acc;
    }
    inline constexpr Type fold(Function<void, Type&, const Type&, std::size_t> fn, Type acc = {})
            const VECLIB_COND_NOEXCEPT(fn, std::declval<Type&>(), std::declval<const Type&>(), 0)
            requires (std::is_default_constructible_v<Type>) {
        if (this->empty()) return acc;
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
    inline constexpr bool all(Function<bool, const Type&> fn) const
            VECLIB_COND_NOEXCEPT(fn, std::declval<const Type&>()) {
        if (this->empty()) return false;
        for (std::size_t i = 0; i < count; ++i)
            if (!fn(data[i])) return false;
        return true;
    }
    inline constexpr bool all(Function<bool, const Type&, std::size_t> fn) const
            VECLIB_COND_NOEXCEPT(fn, std::declval<const Type&>(), 0) {
        if (this->empty()) return false;
        for (std::size_t i = 0; i < count; ++i)
            if (!fn(data[i], i)) return false;
        return true;
    }

    /// @brief Return `true` if at least an element in the slice passes a custom check function.
    ///        The check function must return `bool`, accept a `const Type&` parameter
    ///        and optionally an `std::size_t` parameter into which will be passed the
    ///        index into the slice of the element currently being processed
    /// @param fn The check function
    inline constexpr bool any(Function<bool, const Type&> fn) const
            VECLIB_COND_NOEXCEPT(fn, std::declval<const Type&>()) {
        if (this->empty()) return false;
        for (std::size_t i = 0; i < count; ++i)
            if (fn(data[i])) return true;
        return false;
    }
    inline constexpr bool any(Function<bool, const Type&, std::size_t> fn) const
            VECLIB_COND_NOEXCEPT(fn, std::declval<const Type&>(), 0) {
        if (this->empty()) return false;
        for (std::size_t i = 0; i < count; ++i)
            if (fn(data[i], i)) return true;
        return false;
    }

    /// @brief Return `true` if all elements in the slice don't pass a custom check function.
    ///        The check function must return `bool`, accept a `const Type&` parameter
    ///        and optionally an `std::size_t` parameter into which will be passed the
    ///        index into the slice of the element currently being processed
    /// @param fn The check function
    inline constexpr bool none(Function<bool, const Type&> fn) const
            VECLIB_COND_NOEXCEPT(fn, std::declval<const Type&>()) {
        if (this->empty()) return false;
        for (std::size_t i = 0; i < count; ++i)
            if (fn(data[i])) return false;
        return true;
    }
    inline constexpr bool none(Function<bool, const Type&, std::size_t> fn) const
            VECLIB_COND_NOEXCEPT(fn, std::declval<const Type&>(), 0) {
        if (this->empty()) return false;
        for (std::size_t i = 0; i < count; ++i)
            if (fn(data[i], i)) return false;
        return true;
    }

    #endif // VECLIB_EXTRA

    /// @brief Fill the slice with a specified value
    /// @param value The value to use
    /// @return A reference to the modified object
    inline constexpr Self& fill(const Type& value) VECLIB_NOEXCEPT_COPY_ASSIGNABLE(Type)
            requires (std::is_copy_assignable_v<Type>) {
        for (std::size_t i = 0; i < count; ++i)
            data[i] = value;
        return *this;
    }

    inline constexpr Self copy() noexcept { Self self = *this; return self; }
    inline constexpr const Self copy() const noexcept { Self self = *this; return self; }

    /// @brief Increase the size of this slice by a specified amount
    /// @param x The number of elements by which the size should be increased
    /// @return A referenced to the modified object
    inline constexpr Self& grow(const std::size_t x) VECLIB_NOEXCEPT {
        this->nullptr_check("veclib::Slice.grow(): Slice isn't referencing anything");
        count += x;
        return *this;
    }

    /// @brief Decrease the size of this slice by a specified amount
    /// @param x The number of elements by which the size should be decreased
    /// @return A referenced to the modified object
    /// @throw `std::runtime_error` if the slice isn't referencing anything,
    ///        `std::underflow_error` if `x` is trying to shrink
    ///        the slice by too much, only if exceptions are not disabled
    inline constexpr Self& shrink(const std::size_t x) VECLIB_NOEXCEPT {
        this->nullptr_check("veclib::Slice.shrink(): Slice isn't referencing anything");
        this->underflow_check(x, "veclib::Slice.shrink(): Trying to shrink the slice by too much");
        count -= x;
        return *this;
    }

    /// @brief Use a signed parameter to act like `grow()` when it is positive and
    ///        `shrink` when it is negative
    /// @param x The signed parameter
    /// @return A reference to the modified object
    /// @throw `std::runtime_error` if the slice isn't referencing anything,
    ///        `std::underflow_error` if `x` is negative and it is trying to shrink
    ///        the slice by too much, only if exceptions are not disabled
    inline constexpr Self& delta(const diff_t x) VECLIB_NOEXCEPT {
        this->nullptr_check("veclib::Slice.delta(): Slice isn't referencing anything");
        if (x < 0) {
            this->underflow_check(-x, "veclib::Slice.delta(): Trying to shrink the slice by too much");
            count -= -x; // Just to avoid signed vs unsigned errors
        } else count += x;
        return *this;
    }

    /// @brief Slide the whole view of the slice backward
    /// @param x The number of elements to slide the view by
    /// @return A reference to the modified object
    /// @throw `std::runtime_error` if the slice isn't referencing anything and
    ///        exceptions are not disabled
    inline constexpr Self& slide_backw(const std::size_t x) VECLIB_NOEXCEPT {
        this->nullptr_check("veclib::Slice.slide_backw(): Slice isn't referencing anything");
        data -= x;
        return *this;
    }

    /// @brief Slide the whole view of the slice forward
    /// @param x The number of elements to slide the view by
    /// @return A reference to the modified object
    /// @throw `std::runtime_error` if the slice isn't referencing anything and
    ///        exceptions are not disabled
    inline constexpr Self& slide_forw(const std::size_t x) VECLIB_NOEXCEPT {
        this->nullptr_check("veclib::Slice.slide_forw(): Slice isn't referencing anything");
        data += x;
        return *this;
    }

    /// @brief Slide the whole view of the slice by using a signed parameter,
    ///        when it is negative the slide is backward and when it is
    ///        positive the slide is forward
    /// @param x The number of elements to slide the view by
    /// @return A reference to the modified object
    /// @throw `std::runtime_error` if the slice isn't referencing anything and
    ///        exceptions are not disabled
    inline constexpr Self& slide(const diff_t x) VECLIB_NOEXCEPT {
        this->nullptr_check("veclib::Slice.slide(): Slice isn't referencing anything");
        data += x; // Signed arithmetic does its job
        return *this;
    }

    /// @brief Trim a specified number of elements from the beginning of the slice
    /// @param x The number of elements to trim
    /// @return A reference to the modified object
    /// @throw `std::runtime_error` if the slice is empty, `std::out_of_range` if `x`
    ///        exceeds the number of elements in this slice, only if exceptions
    ///        are not disabled
    inline constexpr Self& trim(const std::size_t x) VECLIB_NOEXCEPT {
        this->empty_check("veclib::Slice.trim(): Slice is empty");
        this->range_check(x, "veclib::Slice.trim(): Trying to trim too many elements");
        data += x;
        count -= x;
        return *this;
    }

    /// @brief Change the view of this slice to reference the specified number of elements
    ///        at its beginning
    /// @param x The number of new elements at the beginning
    /// @return A reference to the modified object
    /// @throw `std::runtime_error` if the slice isn't referencing anything, only if
    ///        exceptions are not disabled
    inline constexpr Self& extend(const std::size_t x) VECLIB_NOEXCEPT {
        this->nullptr_check("veclib::Slice.extend(): Slice isn't referencing anything");
        data -= x;
        count += x;
        return *this;
    }

    /// @brief Change the view of this slice with a signed parameter, acting as `trim` if it is positive
    ///        and acting as `extend` if it is negative
    /// @param x The signed parameter
    /// @return A reference to the modified object
    /// @throw `std::runtime_error` if the slice isn't referencing anything or if the slice is empty,
    ///        `std::out_of_range` if `x` is positive and tries to trim too many elements, only
    ///        if exceptions are not disabled
    inline constexpr Self& nudge(const diff_t x) VECLIB_NOEXCEPT {
        if (x < 0) {
            this->nullptr_check("veclib::Slice.nudge(): Slice isn't referencing anything");
            count += -x; // Just to avoid signed vs unsigned errors
        } else {
            this->empty_check("veclib::Slice.nudge(): Slice is empty");
            this->range_check(x, "veclib::Slice.nudge(): Trying to trim too many elements");
            count += x;
        }
        data += x;
        return *this;
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

    /// @brief Trim the last element off this slice (same as running `shrink(1)`)
    /// @return A reference to the modified object
    /// @throw `std::runtime_error` if the slice is empty
    inline constexpr Self& consume_back() VECLIB_NOEXCEPT {
        this->empty_check("veclib::Slice.consume_front(): Slice is empty");
        --count;
        return *this;
    }

    /// @brief Change the size of this slice
    /// @param x The new size
    /// @return A reference to the modified object
    inline constexpr Self& resize(const std::size_t x) noexcept {
        count = x;
        return *this;
    }

    /// @brief Compare two slices for in-memory equality (check for memory ranges to be the same)
    /// @param other The other slice to compare
    inline constexpr bool operator==(const Self& other) const noexcept { return (data == other.data) && (count == other.count); }
    /// @brief Compare a slice against a scalar value for equality
    /// @param value The scalar value
    inline constexpr bool operator==(const Type& value) const VECLIB_NOEXCEPT
            requires (std::equality_comparable<Type>) {
        this->nullptr_check("veclib::Slice.operator==(): The slice isn't referencing anything");
        for (std::size_t i = 0; i < count; ++i)
            if (data[i] != value) return false;
        return true;
    }
    /// @brief Compare two slices for inequality
    /// @param other The other slice ot compare
    inline constexpr bool operator!=(const Self& other) const noexcept { return !(*this == other); }
    /// @brief Compare a slice against a scalar value for inequality
    /// @param value The scalar value
    inline constexpr bool operator!=(const Type& value) const noexcept { return !(*this == value); }
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

        const Type* const a_end = a.get() + a.size();
        const Type* const b_end = b.get() + b.size();

        // Check if slices overlap or are adjacent in memory
        const bool overlap = (a.get() <= b_end) && (b.get() <= a_end);

        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(overlap);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (!overlap) throw std::logic_error("veclib::join(): Can't join non-adjacent slices");
        #endif // VECLIB_ASSERT_NOEXCEPT

        // The join starts at the lowest starting pointer and ends at the highest ending pointer
        const Type* const start = a.get() < b.get() ? a.get() : b.get(); // Minimum
        const Type* const end = a_end > b_end ? a_end : b_end; // Maximum

        return Self(start, end - start);
    }
    /// @brief Change this slice to be the union of the memory regions referenced by
    ///        itself and another slice
    /// @param other The other slice
    /// @return A reference to the modified object
    /// @throw `std::logic_error` if the two slices are not adjacent or do not overlap,
    ///        only if exceptions are not disabled
    inline constexpr Self& join(const Self& other) VECLIB_NOEXCEPT { return *this = ::veclib::join(*this, other); }

    /// @brief Check if a provided pointer falls inside the memory region referenced by this slice
    /// @param ptr The pointer to check
    /// @return `true` if the pointer is included in the memory region referenced by this slice
    inline constexpr bool contains(const Type* const ptr) const noexcept { return (ptr >= data) && (ptr < data + count); }
    /// @brief Check if a provided value is contained in this slice
    /// @param value The value to search for
    /// @throw `std::runtime_error` if the slice isn't referencing anything
    inline constexpr bool contains(const Type& value) const VECLIB_NOEXCEPT
            requires (std::equality_comparable<Type>) {
        this->nullptr_check("veclib::Slice.contains(): The slice isn't referencing anything");
        if (count == 0) return false; // Systematically false
        for (std::size_t i = 0; i < count; ++i)
            if (data[i] == value) return true;
        return false;
    }
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
    /// @throw `std::out_of_range` if the provided index exceeds this slice's size
    inline constexpr Self split_at(const std::size_t i) VECLIB_NOEXCEPT {
        this->range_check(i, "veclib::Slice.split_at(): Trying to split too far");
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
    inline constexpr Self& align_to(const std::size_t alignment) noexcept {
        // Truncate the front of the slice
        const std::uintptr_t raw_addr = reinterpret_cast<std::uintptr_t>(data);
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
    inline constexpr Self align_to_copy(const std::size_t alignment) const noexcept {
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

    /// @brief Type-pun this slice into a slice of a given type
    /// @tparam NewType The type of the punned slice
    /// @note This function doesn't take into account alignment and
    ///       type size compatibility, if you want a safer
    ///       alternative you should use `strict_into`
    template <typename NewType>
    inline constexpr Slice<NewType> into() const noexcept {
        if (this->empty()) return Slice<NewType>(); // Zeroed by default

        // Calculate total byte size of current slice
        const std::size_t bytes = count * sizeof(Type);
        // Calculate how many elements of NewType fit into that byte size,
        // using integer division we ensure that there are no half elements
        const std::size_t new_count = bytes / sizeof(NewType);

        return Slice<NewType>(reinterpret_cast<NewType*>(data), new_count);
    }
    /// @brief Safely type-pun this slice into a slice of a given type
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
        const std::size_t bytes = count * sizeof(Type);

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

        const std::size_t new_count = bytes / sizeof(NewType);
        return Slice<NewType>(reinterpret_cast<NewType*>(data), new_count);
    }

    #endif // VECLIB_EXTRA

    /// @brief Construct a slice referencing a portion of this slice's memory region.
    /// @param start The index into this slice at which the new slice should start.
    ///              Omitting this argument defaults it to `0`
    /// @param end The exclusive index into this slice at which the new slice should end.
    ///            Omitting this argument defaults it to this slice's size minus the
    ///            starting index, i.e. always reaching the end of the slice
    /// @throw `std::runtime_error` if this slice isn't referencing anything,
    ///        `std::out_of_range` if the starting or ending indeces exceed this slice's size.
    ///        only if exceptions are not disabled
    inline constexpr Self subslice(const std::size_t start = 0) VECLIB_NOEXCEPT {
        return this->internal_subslice(start, count - start); // Omit exclusive index and specify the maximum
    }
    inline constexpr const Self subslice(const std::size_t start = 0) const VECLIB_NOEXCEPT {
        return this->internal_subslice(start, count - start);
    }
    inline constexpr Self subslice(const std::size_t start, const std::size_t end) VECLIB_NOEXCEPT {
        return this->internal_subslice(start, end);
    }
    inline constexpr const Self subslice(const std::size_t start, const std::size_t end) const VECLIB_NOEXCEPT {
        return this->internal_subslice(start, end);
    }
    /// @brief Construct a slice referencing a portion of this slice's memory region
    /// @param start The index into this slice at which the new slice should start.
    ///              Omitting this argument defaults it to `0`
    /// @param len The length of the new slice, if it exceeds the maximum size of this
    ///            slice, the value is capped and by default it equals `~0ULL`
    ///            (maximum value for `std::size_t`)
    /// @throw `std::runtime_error` if this slice isn't referencing anything,
    ///        `std::out_of_range` if the offset exceeds the memory region of this slice
    ///        or if the offset plus the length exceed it, only if exceptions are not disabled
    inline constexpr Self subslice_len(const std::size_t start = 0, const std::size_t len = ~0ULL) VECLIB_NOEXCEPT {
        this->nullptr_check("veclib::Slice.subslice_len(): The slice isn't referencing anything");
        this->bounds_check(start, "veclib::Slice.subslice_len(): Starting index is out of bounds");
        // Clamp the value with a ternary check so len can be `const`
        return Slice<Type>(data + start, (len >= count - start) ? (count - start) : len);
    }
    inline constexpr const Self subslice_len(const std::size_t start = 0, const std::size_t len = ~0ULL) const VECLIB_NOEXCEPT {
        this->nullptr_check("veclib::Slice.subslice_len(): The slice isn't referencing anything");
        this->bounds_check(start, "veclib::Slice.subslice_len(): Starting index is out of bounds");
        // Clamp the value with a ternary check so len can be `const`
        return Slice<Type>(data + start, (len >= count - start) ? (count - start) : len);
    }

    /// @brief Keep trimming elements from the beginning of the slice while a custom
    ///        function returns `true`. The custom function must return `bool` and
    ///        must accept a `const Type&` parameter, into which will be passed the
    ///        next element to check for trimming
    /// @param fn The custom function
    /// @return A reference to the modified object
    inline constexpr Self& trim_while(Function<bool, const Type&> fn) VECLIB_COND_NOEXCEPT(fn, std::declval<const Type&>()) {
        // We have no overload for passing the index because this is a while loop
        while (count > 0 && fn(data)) this->consume_front();
        return *this;
    }
    /// @brief Keep trimming elements while the first element is equal to a provided value
    /// @param value The value to check against for equality
    /// @return A reference to the modified object
    inline constexpr Self& trim_leading(const Type& value) noexcept
            requires (std::equality_comparable<Type>) {
        return this->trim_while([&value] (const Type& x) noexcept { return x == value; });
    }
    /// @brief Keep trimming elements from the beginning of the slice while a custom
    ///        function returns `false`. The custom function must return `bool` and
    ///        must accept a `const Type&` parameter, into which will be passed the
    ///        next element to check for trimming
    /// @param fn The custom function
    /// @return A reference to the modified object
    inline constexpr Self& trim_until(Function<bool, const Type&> fn) VECLIB_COND_NOEXCEPT(fn, std::declval<const Type&>()) {
        while (count > 0 && !fn(data)) this->consume_front();
        return *this;
    }

    #ifndef VECLIB_NO_OPERATOR_OVERLOADS // We create extra operator overloads unless it's not requested

    /// @brief Operator overload for calling `grow(x)`
    inline constexpr Self& operator+=(const std::size_t x) VECLIB_NOEXCEPT { return this->grow(x); }
    /// @brief Operator overload for calling `copy().grow(x)`
    inline constexpr Self operator+(const std::size_t x) const VECLIB_NOEXCEPT { return this->copy().grow(x); }
    /// @brief Operator overload for calling `shrink(x)`
    inline constexpr Self& operator-=(const std::size_t x) { return this->shrink(x); }
    /// @brief Operator overload for calling `copy().shrink(x)`
    inline constexpr Self operator-(const std::size_t x) const { return this->copy().shrink(x); }

    /// @brief Operator overload for calling `slide_backw(x)`
    inline constexpr Self& operator<<=(const std::size_t x) VECLIB_NOEXCEPT { return this->slide_backw(x); }
    /// @brief Operator overload for calling `copy().slide_backw(x)`
    inline constexpr Self operator<<(const std::size_t x) const VECLIB_NOEXCEPT { return this->copy().slide_backw(x); }
    /// @brief Operator overload for calling `slide_forw(x)`
    inline constexpr Self& operator>>=(const std::size_t x) VECLIB_NOEXCEPT { return this->slide_forw(x); }
    /// @brief Operator overload for calling `copy().slide_forw(x)`
    inline constexpr Self operator>>(const std::size_t x) const VECLIB_NOEXCEPT { return this->copy().slide_forw(x); }

    /// @brief Operator overload for calling `consume_front()`
    inline constexpr Self& operator++() { return this->consume_front(); }
    /// @brief Operator overload for calling `copy().consume_front()`
    inline constexpr Self operator+() const { return this->copy().consume_front(); }
    /// @brief Operator overload for calling `consume_back()`
    inline constexpr Self& operator--() { return this->consume_back(); }
    /// @brief Operator overload for calling `copy().consume_back()`
    inline constexpr Self operator-() const { return this->copy().consume_back(); }

    /// @brief Operator overload for calling `intersect(other)`
    inline constexpr Self& operator&=(const Self& other) noexcept { return this->intersect(other); }
    /// @brief Operator overload for calling `copy().intersect(other)`
    inline constexpr Self operator&(const Self& other) const noexcept { return ::veclib::intersect(*this, other); }
    /// @brief Operator overload for calling `join(other)`
    inline constexpr Self& operator|=(const Self& other) VECLIB_NOEXCEPT { return this->join(other); }
    /// @brief Operator overload for calling `copy().join(other)`
    inline constexpr Self operator|(const Self& other) const VECLIB_NOEXCEPT { return ::veclib::join(*this, other); }

    #endif // VECLIB_NO_OPERATOR_OVERLOADS

    /// @brief It's an iterator what do you expect -_-
    inline constexpr Type* begin() noexcept { return data; }
    inline constexpr const Type* begin() const noexcept { return data; }
    /// @brief It's an iterator what do you expect -_-
    inline constexpr Type* end() noexcept { return data ? data + count : nullptr; }
    inline constexpr const Type* end() const noexcept { return data ? data + count : nullptr; }
};

/// @brief Alias for a `Slice` object referencing `const` memory
/// @tparam Type The type the slice should reference
template <typename Type>
using Span = Slice<const Type>;


/// @brief A static array class
/// @tparam Type The type held by the array
/// @tparam Size The size of the array
template <typename Type, std::size_t Size>
class Array {
private:

    /// @brief The static array of `Type`
    Type data[Size] {};

    void bounds_check(const std::size_t i, const char* const str) const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(i < Size && str);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (i >= Size) throw std::out_of_range(str);
        #endif // VECLIB_ASSERT_NOEXCEPT
    }
    void range_check(const std::size_t i, const char* const str) const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(i <= Size && str);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (i > Size) throw std::out_of_range(str);
        #endif // VECLIB_ASSERT_NOEXCEPT
    }

    inline constexpr Slice<Type> internal_slice(const std::size_t start, const std::size_t end) VECLIB_NOEXCEPT {
        this->range_check(start, "veclib::Array.slice(): Starting index exceeds array size");
        this->range_check(end, "veclib::Array.slice(): Ending index exceeds array size");
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(end >= start);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (end < start) throw std::logic_error("veclib::Array.slice(): Ending index is lower than starting index");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return Slice<Type>(data + start, end - start);
    }
    inline constexpr const Slice<Type> internal_slice(const std::size_t start, const std::size_t end) const VECLIB_NOEXCEPT {
        this->range_check(start, "veclib::Array.slice(): Starting index exceeds array size");
        this->range_check(end, "veclib::Array.slice(): Ending index exceeds array size");
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(end >= start);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (end < start) throw std::logic_error("veclib::Array.slice(): Ending index is lower than starting index");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return Slice<Type>(data + start, end - start);
    }

public:

    /// @brief Member type alias for accessing the contained type
    using Item = Type;
    /// @brief Member type alias for referencing this same type
    using Self = Array<Type, Size>;

    /// @brief Default constructor. This is `noexcept` only if `Type`'s default constructor is
    Array() VECLIB_NOEXCEPT_DEFAULT_CONSTRUCTIBLE(Type) requires (std::is_default_constructible_v<Type>) = default;
    /// @brief Default destructor. This is `noexcept` only if `Type`'s default destructor is
    ~Array() VECLIB_NOEXCEPT_DESTRUCTIBLE(Type) requires (std::is_destructible_v<Type>) = default;

    /// @brief Create an array with an initializer list
    /// @throw `std::out_of_range` if there are too many initializer list values
    Array(const std::initializer_list<Type>& args) VECLIB_NOEXCEPT
            requires (std::is_copy_assignable_v<Type>) {
        this->range_check(args.size(), "veclib::Array.Array(): Too many initializer list values");
        for (std::size_t i = 0; i < args.size(); ++i)
            data[i] = args.begin()[i];
    }
    /// @brief Assign this array to an initializer list
    /// @throw `std::out_of_range` if there are too many initializer list values
    inline constexpr Self& operator=(const std::initializer_list<Type>& args) VECLIB_NOEXCEPT
            requires (std::is_copy_assignable_v<Type>) {
        this->range_check(args.size(), "veclib::Array.Array(): Too many initializer list values");
        for (std::size_t i = 0; i < args.size(); ++i)
            data[i] = args.begin()[i];
        return *this;
    }

    /// @brief Scalar copy constructor. This is `noexcept` only if `Type`'s copy constructor is
    Array(const Type& value) VECLIB_NOEXCEPT_COPY_ASSIGNABLE(Type)
            requires (std::is_copy_assignable_v<Type>) {
        for (std::size_t i = 0; i < Size; ++i)
            data[i] = value;
    }
    /// @brief Scalar copy assignment. This is `noexcept` only if `Type`'s copy assignment operator is
    inline constexpr Self& operator=(const Type& value) VECLIB_NOEXCEPT_COPY_ASSIGNABLE(Type)
            requires (std::is_copy_assignable_v<Type>) {
        for (std::size_t i = 0; i < Size; ++i)
            data[i] = value;
        return *this;
    }

    /// @brief Copy constructor. This is `noexcept` only if `Type`'s copy constructor is
    Array(const Self& other) VECLIB_NOEXCEPT_COPY_ASSIGNABLE(Type)
            requires (std::is_copy_assignable_v<Type>) {
        for (std::size_t i = 0; i < Size; ++i)
            data[i] = other.data[i];
    }
    /// @brief Copy assignment. This is `noexcept` only if `Type`'s copy assignment operator is
    inline constexpr Self& operator=(const Self& other) VECLIB_NOEXCEPT_COPY_ASSIGNABLE(Type)
            requires (std::is_copy_assignable_v<Type>) {
        for (std::size_t i = 0; i < Size; ++i)
            data[i] = other.data[i];
        return *this;
    }

    /// @brief Move constructor. This is `noexcept only if `Type`'s move constructor is
    Array(Self&& other) VECLIB_NOEXCEPT_MOVE_ASSIGNABLE(Type)
            requires (std::is_move_assignable_v<Type>) {
        for (std::size_t i = 0; i < Size; ++i)
            data[i] = std::move(other.data[i]);
    }
    /// @brief Move assignment. This is `noexcept only if `Type`'s move assignment operator is
    inline constexpr Self& operator=(Self&& other) VECLIB_NOEXCEPT_MOVE_ASSIGNABLE(Type)
            requires (std::is_move_assignable_v<Type>) {
        for (std::size_t i = 0; i < Size; ++i)
            data[i] = std::move(other.data[i]);
        return *this;
    }

    /// @brief Return a `const` pointer to the internal array
    inline constexpr const Type* get() const noexcept { return data; }
    /// @brief Return the size of the array
    inline consteval std::size_t size() const noexcept { return Size; }
    /// @brief Check if this array is empty
    inline consteval bool empty() const noexcept { return Size == 0; }

    /// @brief Compare two arrays for equality
    inline constexpr bool operator==(const Self& other) const noexcept
            requires (std::equality_comparable<Type>) {
        for (std::size_t i = 0; i < Size; ++i)
            if (data[i] != other.data[i]) return false;
        return true;
    }
    /// @brief Compare this array for equality against a scalar value
    inline constexpr bool operator==(const Type& value) const noexcept
            requires (std::equality_comparable<Type>) {
        for (std::size_t i = 0; i < Size; ++i)
            if (data[i] != value) return false;
        return true;
    }
    /// @brief Compare two arrays for inequality
    /// @throw `std::runtime_error` if the array was moved, only if exceptions are not disabled
    inline constexpr bool operator!=(const Self& other) const VECLIB_NOEXCEPT
        requires (std::equality_comparable<Type>) { return !(*this == other); }
    /// @brief Compare this array for inequality against a scalar value
    /// @throw `std::runtime_error` if the array was moved, only if exceptions are not disabled
    inline constexpr bool operator!=(const Type& value) const VECLIB_NOEXCEPT
        requires (std::equality_comparable<Type>) { return !(*this == value); }
    /// @brief Implicitly check if this array wasn't moved or isn't empty
    inline constexpr operator bool() const noexcept { return data != nullptr && Size != 0; }

    /// @brief Create a `Slice` object referencing a portion of this array's memory,
    ///        optionally specifying starting and ending indeces.
    ///        The ending index is exclusive
    /// @throw `std::runtime_error` if the array was moved and `std::out_of_range`
    ///        if either index is out of bounds, only if exceptions are not disabled
    inline constexpr Slice<Type> slice(const std::size_t start = 0) VECLIB_NOEXCEPT {
        return this->internal_slice(start, Size - start);
    }
    inline constexpr const Slice<Type> slice(const std::size_t start = 0) const VECLIB_NOEXCEPT {
        return this->internal_slice(start, Size - start);
    }
    inline constexpr Slice<Type> slice(const std::size_t start, const std::size_t end) VECLIB_NOEXCEPT {
        return this->internal_slice(start, end);
    }
    inline constexpr const Slice<Type> slice(const std::size_t start, const std::size_t end) const VECLIB_NOEXCEPT {
        return this->internal_slice(start, end);
    }
    /// @brief Create a `Slice` object referencing a portion of this array's memory,
    ///        optionally specifying a starting index and a length. If the length
    ///        overflows it is clamped before creating the slice
    /// @throw `std::out_of_range` if the starting index is out of bounds, only if exceptions are not disabled
    inline constexpr Slice<Type> slice_len(const std::size_t start = 0, const std::size_t len = ~0ULL) VECLIB_NOEXCEPT {
        this->bounds_check(start, "veclib::Array.slice_len(): Starting index is out of bounds");
        // Clamp the value with a ternary check so len can be `const`
        return Slice<Type>(data + start, (len >= Size - start) ? (Size - start) : len);
    }inline constexpr const Slice<Type> slice_len(const std::size_t start = 0, const std::size_t len = ~0ULL) const VECLIB_NOEXCEPT {
        this->bounds_check(start, "veclib::Array.slice_len(): Starting index is out of bounds");
        // Clamp the value with a ternary check so len can be `const`
        return Slice<Type>(data + start, (len >= Size - start) ? (Size - start) : len);
    }

    /// @brief Return the first element in the array
    inline constexpr Type& first() noexcept requires (Size != 0) { return data[0]; }
    inline constexpr const Type& first() const noexcept requires (Size != 0) { return data[0]; }

    /// @brief Return a slice referencing a given amount of elements at the beginning of this array
    /// @param x The number of elements to include in the slice
    /// @throw `std::out_of_range` if too many elements are trying to be extracted,
    ///        only if exceptions are not disabled
    inline constexpr Slice<Type> first(const std::size_t x) VECLIB_NOEXCEPT requires (Size != 0) {
        this->range_check(x, "veclib::Array.first(): The number of first elements to extract exceeds array size");
        return Slice<Type>(data, x);
    }
    inline constexpr const Slice<Type> first(const std::size_t x) const VECLIB_NOEXCEPT requires (Size != 0) {
        this->range_check(x, "veclib::Array.first(): The number of first elements to extract exceeds array size");
        return Slice<Type>(data, x);
    }

    /// @brief Return the last element in the array
    inline constexpr Type& last() noexcept requires (Size != 0) { return data[Size - 1]; }
    inline constexpr const Type& last() const noexcept requires (Size != 0) { return data[Size - 1]; }

    /// @brief Return a slice referencing a given amount of elements at the end of this array
    /// @param x The number of elements to include in the slice
    /// @throw `std::out_of_range` if too many elements are trying to be extracted,
    ///        only if exceptions are not disabled
    inline constexpr Slice<Type> last(const std::size_t x) VECLIB_NOEXCEPT requires (Size != 0) {
        this->range_check(x, "veclib::Array.last(): The number of last elements to extract exceeds array size");
        return Slice<Type>(data + Size - x, x);
    }
    inline constexpr const Slice<Type> last(const std::size_t x) const VECLIB_NOEXCEPT requires (Size != 0) {
        this->range_check(x, "veclib::Array.last(): The number of last elements to extract exceeds array size");
        return Slice<Type>(data + Size - x, x);
    }

    /// @brief Index into this array without bounds checking
    /// @param i The index into the array
    inline constexpr Type& operator[](const std::size_t i) noexcept { return data[i]; }
    inline constexpr const Type& operator[](const std::size_t i) const noexcept { return data[i]; }

    /// @brief Index into this array with bounds checking
    /// @param i The index into the array
    /// @throw `std::out_of_range` if the index is out of bounds,
    ///        only if exceptions are not disabled
    inline constexpr Type& at(const std::size_t i) VECLIB_NOEXCEPT {
        this->bounds_check(i, "veclib::Array.at(): Index is out of bounds");
        return data[i];
    }
    inline constexpr const Type& at(const std::size_t i) const VECLIB_NOEXCEPT {
        this->bounds_check(i, "veclib::Array.at(): Index is out of bounds");
        return data[i];
    }

    #ifdef VECLIB_EXTRA

    /// @brief Index circularly into this array, preventing any out-of-bounds access
    /// @param i The index into the array
    inline constexpr Type& circular_at(const std::size_t i) noexcept { return data[i % Size]; }
    inline constexpr const Type& circular_at(const std::size_t i) const noexcept { return data[i % Size]; }

    #ifndef VECLIB_NO_OPERATOR_OVERLOADS

    /// @brief Operator overload for calling `circular_at()`
    inline constexpr Type& operator()(const std::size_t i) noexcept { return data[i % Size]; }
    inline constexpr const Type& operator()(const std::size_t i) const noexcept { return data[i % Size]; }

    #endif // VECLIB_NO_OPERATOR_OVERLOADS

    /// @brief Index into this array clamping the index at the maximum value,
    ///        preventing any out-of-bounds access
    /// @param i The index into the array
    inline constexpr Type& clamped_at(const std::size_t i) noexcept { return data[i >= Size ? Size - 1 : i]; }
    inline constexpr const Type& clamped_at(const std::size_t i) const noexcept { return data[i >= Size ? Size - 1 : i]; }

    #endif // VECLIB_EXTRA

    /// @brief Check if a given index is inside the valid bounds for this array
    /// @param i The index to check
    inline constexpr bool inside_bounds(const std::size_t i) const noexcept { return i < Size; }

    /// @brief Check if a given pointer is inside the memory region of this array
    /// @param ptr The pointer to check
    inline constexpr bool contains(const Type* const ptr) const noexcept { return (ptr >= data) && (ptr < data + Size); }
    /// @brief Check if a given value of `Type` is contained anywhere in this array
    /// @param value The value to check for
    inline constexpr bool contains(const Type& value) const noexcept
            requires (std::equality_comparable<Type>) {
        for (std::size_t i = 0; i < Size; ++i)
            if (data[i] == value) return true;
        return false;
    }

    /// @brief Apply a custom function to each element of the array.
    ///        The function must return `void` and must take a `Type&` parameter,
    ///        into which will be passed the current element in the array,
    ///        and can optionally accept an `std::size_t` parameter, into which
    ///        the index of the current element will be passed
    /// @param fn The function to apply
    inline constexpr Self& map(Function<void, Type&> fn) VECLIB_COND_NOEXCEPT(fn, std::declval<Type&>()) {
        // We don't require Size to be non-zero, since the loop will just never run
        for (std::size_t i = 0; i < Size; ++i)
            fn(data[i]);
        return *this;
    }
    inline constexpr Self& map(Function<void, Type&, std::size_t> fn) VECLIB_COND_NOEXCEPT(fn, std::declval<Type&>(), 0) {
        for (std::size_t i = 0; i < Size; ++i)
            fn(data[i], i);
        return *this;
    }

    /// @brief Combine all the values of the elements in this array with a custom
    ///        function, that must return `void` and must take a `Type&` parameter,
    ///        which will be the accumulated value, a `const Type&` parameter, which
    ///        will be the current element, and optionally an `std::size_t` parameter
    ///        into which the index of the current element will be passed
    /// @param fn The transformation function
    /// @param acc An optional starting value for the accumulated value. By default
    ///            it is assigned a default-constructed value of `Type`
    inline constexpr Type fold(Function<void, Type&, const Type&> fn, Type acc = {})
            VECLIB_COND_NOEXCEPT(fn, std::declval<Type&>(), std::declval<const Type&>())
            requires (std::is_default_constructible_v<Type>) {
        for (std::size_t i = 0; i < Size; ++i)
            fn(acc, data[i]);
        return acc;
    }
    inline constexpr Type fold(Function<void, Type&, const Type&, std::size_t> fn, Type acc = {})
            VECLIB_COND_NOEXCEPT(fn, std::declval<Type&>(), std::declval<const Type&>(), 0)
            requires (std::is_default_constructible_v<Type>) {
        for (std::size_t i = 0; i < Size; ++i)
            fn(acc, data[i], i);
        return acc;
    }

    #ifdef VECLIB_EXTRA

    /// @brief Return `true` if all elements in the array pass a custom check function.
    ///        The check function must return `bool`, accept a `const Type&` parameter
    ///        and optionally an `std::size_t` parameter into which will be passed the
    ///        index into the array of the element currently being processed
    /// @param fn The check function
    inline constexpr bool all(Function<bool, const Type&> fn) const
            VECLIB_COND_NOEXCEPT(fn, std::declval<const Type&>()) {
        for (std::size_t i = 0; i < Size; ++i)
            if (!fn(data[i])) return false;
        return true;
    }
    inline constexpr bool all(Function<bool, const Type&, std::size_t> fn) const
            VECLIB_COND_NOEXCEPT(fn, std::declval<const Type&>(), 0) {
        for (std::size_t i = 0; i < Size; ++i)
            if (!fn(data[i], i)) return false;
        return true;
    }

    /// @brief Return `true` if at least an element in the array passes a custom check function.
    ///        The check function must return `bool`, accept a `const Type&` parameter
    ///        and optionally an `std::size_t` parameter into which will be passed the
    ///        index into the array of the element currently being processed
    /// @param fn The check function
    inline constexpr bool any(Function<bool, const Type&> fn) const
            VECLIB_COND_NOEXCEPT(fn, std::declval<const Type&>()) {
        for (std::size_t i = 0; i < Size; ++i)
            if (fn(data[i])) return true;
        return false;
    }
    inline constexpr bool any(Function<bool, const Type&, std::size_t> fn) const
            VECLIB_COND_NOEXCEPT(fn, std::declval<const Type&>(), 0) {
        for (std::size_t i = 0; i < Size; ++i)
            if (fn(data[i], i)) return true;
        return false;
    }

    /// @brief Return `true` if all elements in the array don't pass a custom check function.
    ///        The check function must return `bool`, accept a `const Type&` parameter
    ///        and optionally an `std::size_t` parameter into which will be passed the
    ///        index into the array of the element currently being processed
    /// @param fn The check function
    inline constexpr bool none(Function<bool, const Type&> fn) const
            VECLIB_COND_NOEXCEPT(fn, std::declval<const Type&>()) requires (Size != 0) {
        for (std::size_t i = 0; i < Size; ++i)
            if (fn(data[i])) return false;
        return true;
    }
    inline constexpr bool none(Function<bool, const Type&, std::size_t> fn) const
            VECLIB_COND_NOEXCEPT(fn, std::declval<const Type&>(), 0) requires (Size != 0) {
        for (std::size_t i = 0; i < Size; ++i)
            if (fn(data[i], i)) return false;
        return true;
    }

    #endif // VECLIB_EXTRA

    /// @brief Fill the array with a specified value
    /// @param value The value to use
    inline constexpr Self& fill(const Type& value) VECLIB_NOEXCEPT_COPY_ASSIGNABLE(Type)
            requires (std::is_copy_assignable_v<Type> && Size != 0) {
        for (std::size_t i = 0; i < Size; ++i)
            data[i] = value;
        return *this;
    }

    /// @brief It's an iterator what do you expect -_-
    inline constexpr Type* begin() noexcept { return data; }
    inline constexpr const Type* begin() const noexcept { return data; }
    /// @brief It's an iterator what do you expect -_-
    inline constexpr Type* end() noexcept { return data + Size; }
    inline constexpr const Type* end() const noexcept { return data + Size; }
};


template <std::size_t Size>
class Bitset;

template <std::size_t Size>
class BitsetItr;

/// @brief A proxy object for modifying a bit inside a `Bitset`
/// @tparam Size The `Size` of the bitset
template <std::size_t Size>
class BitProxy {
private:

    /// @brief A reference to the parent bitset
    Bitset<Size>& parent;
    /// @brief The bit index into the `Bitset`
    std::size_t idx = 0;

public:

    /// @brief Member type alias for referencing this type
    using Self = BitProxy<Size>;

    BitProxy() noexcept = delete; // No default constructor due to reference
    /// @brief Default destructor
    ~BitProxy() noexcept = default;

    /// @brief Construct a proxy object with a reference to a `Bitset` and a bit index into it
    /// @param p The reference to the bitset
    /// @param i The index into the bitset
    BitProxy(Bitset<Size>& p, std::size_t i) noexcept
        : parent(p), idx(i) {}

    /// @brief Implicitly cast this object to a `bool`, retrieving the value stored at the
    ///        referenced bit in the bitset
    inline constexpr operator bool() const noexcept {
        return parent.get_at(idx);
    }

    /// @brief Assign a `bool` value to this proxy object, setting the value stored at the
    ///        referenced bit in the bitset
    inline constexpr Self& operator=(bool value) noexcept {
        parent.set_at(idx, value);
        return *this;
    }

    /// @brief Return a `const` pointer to the referenced `Bitset`
    inline constexpr const Bitset<Size>* get() const noexcept { return &parent; }
    /// @brief Return a `const` reference to the referenced `Bitset`
    inline constexpr const Bitset<Size>& bitset() const noexcept { return parent; }
    /// @brief Return the bit index that this proxy object is referencing
    inline constexpr std::size_t index() const noexcept { return idx; }

    /// @brief Return a `BitsetItr` object, simulating a pointer
    inline constexpr BitsetItr<Size> operator&() const noexcept {
        return BitsetItr<Size>(parent, idx); // Fake pointer-like object
    }
};

/// @brief Another proxy object for the `Bitset` class, functioning as a pointer
/// @tparam Size The `Size` of the bitset
template <std::size_t Size>
class BitsetItr {
private:

    /// @brief A reference to the parent bitset
    Bitset<Size>& parent;
    /// @brief The index into the bitset
    std::size_t index = 0;

public:

    /// @brief Member type alias for referencing this type
    using Self = BitsetItr<Size>;

    BitsetItr() noexcept = delete; // No default constructor due to reference
    /// @brief Default destructor
    ~BitsetItr() noexcept = default;

    /// @brief Construct this object with a reference to the parent `Bitset` and a bit index into it
    /// @param p The parent `Bitset`
    /// @param i The index into the bitset
    BitsetItr(Bitset<Size>& p, std::size_t i) noexcept
        : parent(p), index(i) {}

    /// @brief Pointer-like prefix increment
    inline constexpr Self& operator++() noexcept { ++index; return *this; }
    /// @brief Pointer-like postfix increment
    inline constexpr Self operator++(int) noexcept { Self self = *this; ++*this; return self; }
    /// @brief Pointer-like prefix decrement
    inline constexpr Self& operator--() noexcept { --index; return *this; }
    /// @brief Pointer-like postfix decrement
    inline constexpr Self operator--(int) noexcept { Self self = *this; --*this; return self; }

    /// @brief Pointer-like compound addition
    inline constexpr Self& operator+=(const std::size_t x) noexcept { index += x; return *this; }
    /// @brief Pointer-like addition
    inline constexpr Self operator+(const std::size_t x) noexcept { return Self(parent, index + x); }
    /// @brief Pointer-like compound subtraction
    inline constexpr Self& operator-=(const std::size_t x) noexcept { index -= x; return *this; }
    /// @brief Pointer-like subtraction
    inline constexpr Self operator-(const std::size_t x) noexcept { return Self(parent, index - x); }

    /// @brief Pointer-like dereference, returning a `BitProxy` object
    inline constexpr BitProxy<Size> operator*() noexcept { return BitProxy<Size>(parent, index); }
    inline constexpr const BitProxy<Size> operator*() const noexcept { return BitProxy<Size>(parent, index); }

    /// @brief Compare two iterator objects for equality
    /// @param other The other iterator
    inline constexpr bool operator==(const Self& other) const noexcept { return &parent == &other.parent && index == other.index; }
    /// @brief Compare two iterator objects for inequality
    /// @param other The other iterator
    inline constexpr bool operator!=(const Self& other) const noexcept { return !(*this == other); }
    /// @brief Implicitly cast this iterator object to check if the reference to the parent bitset is dangling or if the bit
    ///        index into the bitset is less than the size of the bitset
    inline constexpr operator bool() const noexcept { return std::bit_cast<Bitset<Size>*>(parent) != nullptr && index < Size; }
};

/// @brief A static bitset class
/// @tparam Size The number of bits held by this bitset
template <std::size_t Size>
class Bitset {

friend BitProxy<Size>;
friend BitsetItr<Size>;

private:

    /// @brief Byte array for storing bits
    std::uint8_t data[VECLIB_BYTESIZE(Size)] = {};

    /// @brief Helper for retrieving the value of a bit at an index
    /// @param i The index into the bitset
    inline constexpr bool get_at(const std::size_t i) const noexcept {
        return (data[i / CHAR_BIT] & (1 << (i % CHAR_BIT))) != 0;
    }
    /// @brief Helper for setting the value of a bit at an index
    /// @param i The index into the bitset
    /// @param value The new value
    inline constexpr void set_at(const std::size_t i, const bool value) noexcept {
        if (value) data[i / CHAR_BIT] |= 1 << (i % CHAR_BIT);
        else data[i / CHAR_BIT] &= ~(1 << (i % CHAR_BIT));
    }

    inline constexpr void bounds_check(const std::size_t i, const char* const str) const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(i < Size);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (i >= Size) throw std::out_of_range(str);
        #endif // VECLIB_ASSERT_NOEXCEPT
    }

public:

    /// @brief Member type alias for referencing the proxy object returned by element-wise operations
    using Item = BitProxy<Size>;
    /// @brief Member type alias for referencing this type
    using Self = Bitset<Size>;

    /// @brief Default constructor
    Bitset() noexcept = default;
    /// @brief Default destructor
    ~Bitset() noexcept = default;

    /// @brief Construct a bitset out of an `Array` object of the same size in bytes
    /// @param array The `Array object`
    Bitset(const Array<std::uint8_t, VECLIB_BYTESIZE(Size)>& array) noexcept {
        for (std::size_t i = 0; i < VECLIB_BYTESIZE(Size); ++i)
            data[i] = array[i];
    }
    /// @brief Construct a bitset out of an `Array` object of the same size in bytes
    /// @param array The `Array object`
    inline constexpr Self& operator=(const Array<std::uint8_t, VECLIB_BYTESIZE(Size)>& array) noexcept {
        for (std::size_t i = 0; i < VECLIB_BYTESIZE(Size); ++i)
            data[i] = array[i];
        return *this;
    }

    /// @brief Construct a bitset out of an `Array` object containing `bool` values
    ///        that represent individual bits
    /// @param array The `Array object`
    Bitset(const Array<bool, Size>& array) noexcept {
        for (std::size_t i = 0; i < Size; ++i)
            this->set_at(i, array[i]);
    }
    /// @brief Construct a bitset out of an `Array` object containing `bool` values
    ///        that represent individual bits
    /// @param array The `Array object`
    inline constexpr Self& operator=(const Array<bool, Size>& array) noexcept {
        for (std::size_t i = 0; i < Size; ++i)
            this->set_at(i, array[i]);
        return *this;
    }

    /// @brief Construct a bitset out of an integer or boolean variable, copying its bits
    /// @tparam Integral The integer type or `bool`
    /// @param x A value of that type
    template <std::integral Integral>
    requires (
        VECLIB_BITSIZE(sizeof(Integral)) == Size && // Require the exact same size
        !std::is_floating_point_v<Integral> // Require integers or `bool`
    )
    Bitset(const Integral x) noexcept {
        for (std::size_t i = 0; i < Size; ++i)
            this->set_at(i, (x & (1 << i)) != 0);
    }
    /// @brief Construct a bitset out of an integer or boolean variable, copying its bits
    /// @tparam Integral The integer type or `bool`
    /// @param x A value of that type
    template <std::integral Integral>
    requires (
        VECLIB_BITSIZE(sizeof(Integral)) == Size &&
        !std::is_floating_point_v<Integral>
    )
    inline constexpr Self& operator=(const Integral x) noexcept {
        for (std::size_t i = 0; i < Size; ++i)
            this->set_at(i, (x & (1 << i)) != 0);
        return *this;
    }

    /// @brief Construct a bitset out of a floating point variable, copying its bits.
    ///        The supported floating point sizes are the same as their integer counterparts.
    ///        No 128bi floats (or beyond) are supported
    /// @tparam Float The floating point type
    /// @param f A value of that type
    template <std::floating_point Float>
    requires (VECLIB_BITSIZE(sizeof(Float)) == Size)
    Bitset(const Float f) noexcept {
        if constexpr (sizeof(Float) == sizeof(std::uint8_t))
            *this = Self(std::bit_cast<std::uint8_t>(f));
        else if (sizeof(Float) == sizeof(std::uint16_t))
            *this = Self(std::bit_cast<std::uint16_t>(f));
        else if (sizeof(Float) == sizeof(std::uint32_t))
            *this = Self(std::bit_cast<std::uint32_t>(f));
        else if (sizeof(Float) == sizeof(std::uint64_t))
            *this = Self(std::bit_cast<std::uint64_t>(f));
        else
            static_assert(false && "veclib::Bitset: Unsupported floating point type"); // Sorry
    }
    /// @brief Construct a bitset out of a floating point variable, copying its bits.
    ///        The supported floating point sizes are the same as their integer counterparts.
    ///        No 128bi floats (or beyond) are supported
    /// @tparam Float The floating point type
    /// @param f A value of that type
    template <std::floating_point Float>
    requires (VECLIB_BITSIZE(sizeof(Float)) == Size)
    inline constexpr Self& operator=(const Float f) noexcept {
        if constexpr (sizeof(Float) == sizeof(std::uint8_t))
            *this = Self(std::bit_cast<std::uint8_t>(f));
        else if (sizeof(Float) == sizeof(std::uint16_t))
            *this = Self(std::bit_cast<std::uint16_t>(f));
        else if (sizeof(Float) == sizeof(std::uint32_t))
            *this = Self(std::bit_cast<std::uint32_t>(f));
        else if (sizeof(Float) == sizeof(std::uint64_t))
            *this = Self(std::bit_cast<std::uint64_t>(f));
        else
            static_assert(false && "veclib::Bitset: Unsupported floating point type"); // Sorry
        return *this;
    }

    /// @brief Fill this bitset by parsing the contents of a string. The number of filled bits is
    ///        the minimum value between this bitset's size and the string's size. The string parsing
    ///        stops at the first character that isn't `'0'` or `'1'`. All the untouched bits are set to `0`
    /// @param str The string to parse
    Bitset(const std::string& str) noexcept {
        this->clear(); // Make sure to clear everything
        std::size_t bits = (Size < str.size()) ? Size : str.size(); // Min
        for (std::size_t i = 0; i < bits; ++i) {
            if (str[i] == '1') this->set_at(i, true);
            else if (str[i] != '0') break; // We break if it's not `0` since everything is `0` already
        }
    }
    /// @brief Fill this bitset by parsing the contents of a string. The number of filled bits is
    ///        the minimum value between this bitset's size and the string's size. The string parsing
    ///        stops at the first character that isn't `'0'` or `'1'`. All the untouched bits are set to `0`
    /// @param str The string to parse
    inline constexpr Self& operator=(const std::string& str) noexcept {
        this->clear();
        std::size_t bits = (Size < str.size()) ? Size : str.size();
        for (std::size_t i = 0; i < bits; ++i) {
            if (str[i] == '1') this->set_at(i, true);
            else if (str[i] != '0') break;
        }
        return *this;
    }

    // We have no get() method since we aren't really holding any typed data
    //inline constexpr const std::uint8_t* get() const noexcept { return data; }

    /// @brief Return the number of bits in this bitset
    inline consteval std::size_t size() const noexcept { return Size; }
    /// @brief Return the number of bytes that this bitset holds
    inline consteval std::size_t bytesize() const noexcept { return VECLIB_BYTESIZE(Size); }

    /// @brief Set all bits to `0`
    inline constexpr void clear() noexcept {
        for (std::size_t i = 0; i < VECLIB_BYTESIZE(Size); ++i)
            data[i] = 0;
    }

    /// @brief Index into this bitset without bounds checking
    /// @param i The bit index into the bitset
    inline constexpr Item operator[](const std::size_t i) noexcept { return Item(*this, i); }
    inline constexpr const Item operator[](const std::size_t i) const noexcept { return Item(*this, i); }

    /// @brief Index into this bitset with bounds checking
    /// @param i The bit index into the bitset
    /// @throw `std::out_of_range` if the index is out of bounds
    inline constexpr Item at(const std::size_t i) VECLIB_NOEXCEPT {
        this->bounds_check(i, "veclib::Bitset.at(): Index is out of bounds");
        return Item(*this, i);
    }
    inline constexpr const Item at(const std::size_t i) const VECLIB_NOEXCEPT {
        this->bounds_check(i, "veclib::Bitset.at(): Index is out of bounds");
        return Item(*this, i);
    }

    #ifdef VECLIB_EXTRA

    /// @brief Index circularly into this bitset, preventing any out-of-bounds access
    /// @param i The bit index into the bitset
    inline constexpr Item circular_at(const std::size_t i) noexcept { return Item(*this, i % Size); }
    inline constexpr const Item circular_at(const std::size_t i) const noexcept { return Item(*this, i % Size); }

    #ifndef VECLIB_NO_OPERATOR_OVERLOADS

    /// @brief Operator overload for calling `circular_at()`
    inline constexpr Item operator()(const std::size_t i) noexcept { return Item(*this, i % Size); }
    inline constexpr const Item operator()(const std::size_t i) const noexcept { return Item(*this, i % Size); }

    #endif // VECLIB_NO_OPERATOR_OVERLOADS

    /// @brief Index into this bitset clamping the index at its maximum value,
    ///        preventing any out-of-bounds access
    /// @param i The bit index into the bitset
    inline constexpr Item& clamped_at(const std::size_t i) noexcept { return Item(*this, i >= Size ? Size - 1 : i); }
    inline constexpr const Item& clamped_at(const std::size_t i) const noexcept { return Item(*this, i >= Size ? Size - 1 : i); }

    #endif // VECLIB_EXTRA

    /// @brief Create an array containing the same bytes as this bitset
    inline constexpr Array<std::uint8_t, VECLIB_BYTESIZE(Size)> array() noexcept {
        Array<std::uint8_t, VECLIB_BYTESIZE(Size)> array;
        for (std::size_t i = 0; i < VECLIB_BYTESIZE(Size); ++i)
            array[i] = data[i];
        return array;
    }
    inline constexpr const Array<std::uint8_t, VECLIB_BYTESIZE(Size)> array() const noexcept {
        Array<std::uint8_t, VECLIB_BYTESIZE(Size)> array;
        for (std::size_t i = 0; i < VECLIB_BYTESIZE(Size); ++i)
            array[i] = data[i];
        return array;
    }

    /// @brief Create a `bool` array holding one of the bits of this
    ///        bitset for each element inside the array
    inline constexpr Array<bool, Size> bool_array() noexcept {
        Array<bool, Size> array;
        for (std::size_t i = 0; i < Size; ++i)
            array[i] = (data[i / 8] & (1 << (i % 8))) != 0;
        return array;
    }
    inline constexpr const Array<bool, Size> bool_array() const noexcept {
        Array<bool, Size> array;
        for (std::size_t i = 0; i < Size; ++i)
            array[i] = (data[i / 8] & (1 << (i % 8))) != 0;
        return array;
    }

    inline constexpr Self& map(Function<void, BitProxy<Size>> fn)
            VECLIB_COND_NOEXCEPT(fn, std::declval<BitProxy<Size>>()) {
        for (std::size_t i = 0; i < Size; ++i)
            fn(BitProxy<Size>(*this, i));
        return *this;
    }
    inline constexpr Self& map(Function<void, BitProxy<Size>, std::size_t> fn)
            VECLIB_COND_NOEXCEPT(fn, std::declval<BitProxy<Size>>(), 0) {
        for (std::size_t i = 0; i < Size; ++i)
            fn(BitProxy<Size>(*this, i), i);
        return *this;
    }

    inline constexpr bool fold(Function<void, bool&, const bool> fn, bool acc = false) const
            VECLIB_COND_NOEXCEPT(fn, std::declval<bool&>(), false) {
        for (std::size_t i = 0; i < Size; ++i)
            fn(acc, this->get_at(i));
        return acc;
    }
    inline constexpr bool fold(Function<void, bool&, const bool, std::size_t> fn, bool acc = false) const
            VECLIB_COND_NOEXCEPT(fn, std::declval<bool&>(), false, 0) {
        for (std::size_t i = 0; i < Size; ++i)
            fn(acc, this->get_at(i), i);
        return acc;
    }

    /// @brief Return the number of bits that are on
    inline constexpr std::size_t on() const noexcept {
        std::size_t count = 0;
        for (std::size_t i = 0; i < Size; ++i)
            if (this->get_at(i)) ++count;
        return count;
    }

    /// @brief Return the number of bits that are off
    inline constexpr std::size_t off() const noexcept {
        std::size_t count = 0;
        for (std::size_t i = 0; i < Size; ++i)
            if (!this->get_at(i)) ++count;
        return count;
    }

    /// @brief Return `true` if all bits are active
    inline constexpr bool all() const noexcept {
        for (std::size_t i = 0; i < Size; ++i)
            if (!this->get_at(i)) return false;
        return true;
    }

    /// @brief Return `true` if all bits are inactive
    inline constexpr bool none() const noexcept {
        for (std::size_t i = 0; i < Size; ++i)
            if (this->get_at(i)) return false;
        return true;
    }

    /// @brief Return `true` if any bit is active
    inline constexpr bool any() const noexcept {
        for (std::size_t i = 0; i < Size; ++i)
            if (this->get_at(i)) return true;
        return false;
    }

    /// @brief Fill this bitset with a given `bool` value
    /// @param value The value to use
    inline constexpr Self& fill(bool value) noexcept {
        std::uint8_t v = value ? ~0 : 0;
        for (std::size_t i = 0; i < VECLIB_BYTESIZE(Size); ++i)
            data[i] = v;
        return *this;
    }

    /// @brief Implicit cast returning `true` if any bit is active
    inline constexpr operator bool() const noexcept { return this->any(); }

    /// @brief Compare two bitsets for equality
    /// @param other The other bitset
    inline constexpr bool operator==(const Self& other) const noexcept {
        for (std::size_t i = 0; i < VECLIB_BYTESIZE(Size); ++i)
            if (data[i] != other.data[i]) return false;
        return true;
    }
    /// @brief Compare two bitsets for inequality
    /// @param other The other bitset
    inline constexpr bool operator!=(const Self& other) const noexcept { return !(*this == other); }
    /// @brief Compare this bitset against a scalar `bool` value for equality
    /// @param value The scalar value
    inline constexpr bool operator==(bool value) const noexcept {
        std::uint8_t v = value ? ~0 : 0;
        for (std::size_t i = 0; i < VECLIB_BYTESIZE(Size); ++i)
            if (data[i] != v) return false;
        return true;
    }
    /// @brief Compare this bitset against a scalar `bool` value for inequality
    /// @param value The scalar value
    inline constexpr bool operator!=(bool value) const noexcept { return !(*this == value); }

    #ifndef VECLIB_NO_OPERATOR_OVERLOADS

    /// @brief Compound bitwise AND assignment with another bitset
    /// @param other The other bitset
    inline constexpr Self& operator&=(const Self& other) noexcept {
        for (std::size_t i = 0; i < VECLIB_BYTESIZE(Size); ++i)
            data[i] &= other.data[i];
        return *this;
    }
    /// @brief Compound bitwise AND assignment with a scalar `bool` value
    /// @param value The scalar value
    inline constexpr Self& operator&=(const bool value) noexcept {
        const std::uint8_t v = value ? ~0 : 0;
        for (std::size_t i = 0; i < VECLIB_BYTESIZE(Size); ++i)
            data[i] &= v;
        return *this;
    }

    /// @brief Compound bitwise OR assignment with another bitset
    /// @param other The other bitset
    inline constexpr Self& operator|=(const Self& other) noexcept {
        for (std::size_t i = 0; i < VECLIB_BYTESIZE(Size); ++i)
            data[i] |= other.data[i];
        return *this;
    }
    /// @brief Compound bitwise OR assignment with a scalar `bool` value
    /// @param value The scalar value
    inline constexpr Self& operator|=(const bool value) noexcept {
        const std::uint8_t v = value ? ~0 : 0;
        for (std::size_t i = 0; i < VECLIB_BYTESIZE(Size); ++i)
            data[i] |= v;
        return *this;
    }

    /// @brief Compound bitwise XOR assignment with another bitset
    /// @param other The other bitset
    inline constexpr Self& operator^=(const Self& other) noexcept {
        for (std::size_t i = 0; i < VECLIB_BYTESIZE(Size); ++i)
            data[i] ^= other.data[i];
        return *this;
    }
    /// @brief Compound bitwise XOR assignment with a scalar `bool` value
    /// @param value The scalar value
    inline constexpr Self& operator^=(const bool value) noexcept {
        const std::uint8_t v = value ? ~0 : 0;
        for (std::size_t i = 0; i < VECLIB_BYTESIZE(Size); ++i)
            data[i] ^= v;
        return *this;
    }

    /// @brief Bitwise AND this bitset with another bitset
    /// @param other The other bitset
    inline constexpr Self operator&(const Self& other) const noexcept {
        Self self = *this;
        for (std::size_t i = 0; i < VECLIB_BYTESIZE(Size); ++i)
            self.data[i] &= other.data[i];
        return self;
    }
    /// @brief Bitwise AND this bitset with a scalar `bool` value
    /// @param value The scalar value
    inline constexpr Self operator&(const bool value) const noexcept {
        Self self = *this;
        self &= value;
        return self;
    }

    /// @brief Bitwise OR this bitset with another bitset
    /// @param other The other bitset
    inline constexpr Self operator|(const Self& other) const noexcept {
        Self self = *this;
        for (std::size_t i = 0; i < VECLIB_BYTESIZE(Size); ++i)
            self.data[i] |= other.data[i];
        return self;
    }
    /// @brief Bitwise OR this bitset with a scalar `bool` value
    /// @param value The scalar value
    inline constexpr Self operator|(const bool value) const noexcept {
        Self self = *this;
        self |= value;
        return self;
    }

    /// @brief Bitwise XOR this bitset with another bitset
    /// @param other The other bitset
    inline constexpr Self operator^(const Self& other) const noexcept {
        Self self = *this;
        for (std::size_t i = 0; i < VECLIB_BYTESIZE(Size); ++i)
            self.data[i] ^= other.data[i];
        return self;
    }
    /// @brief Bitwise XOR this bitset with a scalar `bool` value
    /// @param value The scalar value
    inline constexpr Self operator^(const bool value) const noexcept {
        Self self = *this;
        self ^= value;
        return self;
    }

    /// @brief Negate this bitset
    inline constexpr Self operator~() const noexcept {
        Self self;
        for (std::size_t i = 0; i < VECLIB_BYTESIZE(Size); ++i)
            self.data[i] = ~data[i];
        return self;
    }

    #endif // VECLIB_NO_OPERATOR_OVERLOADS

    /// @brief It's an iterator what do you expect -_-
    inline constexpr BitsetItr<Size> begin() noexcept { return BitsetItr<Size>(*this, 0); }
    inline constexpr const BitsetItr<Size> begin() const noexcept { return BitsetItr<Size>(*this, 0); }
    /// @brief It's an iterator what do you expect -_-
    inline constexpr BitsetItr<Size> end() noexcept { return BitsetItr<Size>(*this, Size); }
    inline constexpr const BitsetItr<Size> end() const noexcept { return BitsetItr<Size>(*this, Size); }
};


/// @brief Enum class representing the growing strategy used by a `Vector` object
enum class GrowType : std::uint8_t {
    AddHalf, // Grow the capacity by adding on half of it. This is the default behaviour
    Double, // Grow the capacity by doubling it
};

/// @brief A dynamic array class, similar to `std::vector` with additional utility methods
/// @tparam Type The type held by the vector
/// @tparam Grow The growing strategy used when reallocating
template <typename Type, GrowType Grow = GrowType::AddHalf>
class Vector {
public:

    /// @brief Member type alias for referencing this type
    using Self = Vector<Type, Grow>;
    /// @brief Member type alias for referencing the contained type
    using Item = Type;
    /// @brief Static member for referencing the chosen growing strategy
    static constexpr const GrowType GrowType = Grow;

private:

    /// @brief The data pointer
    Type* data = nullptr;
    /// @brief The number of constructed elements
    std::size_t count = 0;
    /// @brief The number of allocated elements
    std::size_t cap = 0;

    void ptr_check(const void* const ptr, const char* const str) const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(ptr && str);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (!ptr) throw std::runtime_error(str);
        #endif // VECLIB_ASSERT_NOEXCEPT
    }
    void empty_check(const char* const str) const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data && count != 0 && str);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr || count == 0) throw std::runtime_error(str);
        #endif // VECLIB_ASSERT_NOEXCEPT
    }
    void bounds_check(const std::size_t i, const char* const str) const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(i < count && str);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (i >= count) throw std::out_of_range(str);
        #endif // VECLIB_ASSERT_NOEXCEPT
    }
    void range_check(const std::size_t i, const char* const str) const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(i <= count && str);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (i > count) throw std::out_of_range(str);
        #endif // VECLIB_ASSERT_NOEXCEPT
    }

    #define VECLIB_VECTOR_DEFAULT_ALLOC 8

    /// @brief Reallocate the internal buffer using the chosen growing strategy
    ///        and move or copy old elements to the new buffer. This only grows the vector
    void realloc() {
        Type* const old = data;
        std::size_t new_cap = 0;
        // Choose by how much to grow
        if constexpr (Grow == GrowType::AddHalf)
            // We set it to 8 elements if we were empty before
            new_cap = !old ? VECLIB_VECTOR_DEFAULT_ALLOC : (cap + cap / 2);
        else
            new_cap = !old ? VECLIB_VECTOR_DEFAULT_ALLOC : (cap * 2);

        // Get a new buffer
        data = VECLIB_NONCONSTRUCTOR_NEW(Type, new_cap);
        if (old) {
            // Move elements if there were any
            for (std::size_t i = 0; i < count; ++i) {
                if constexpr (std::is_move_constructible_v<Type>)
                    new(&data[i]) Type(std::move(old[i]));
                else
                    new(&data[i]) Type(old[i]); // Resort to copying
                // Destroy manually and then free everything
                old[i].~Type();
            }
            VECLIB_NONDESTRUCTOR_DELETE(old, cap);
        }

        // Update capacity
        cap = new_cap;
    }

    /// @brief Reallocate the internal buffer by specifying an explicit new capacity and
    ///        then move or copy the old elements to the new buffer.
    ///        This allows for the vector to be shrunk
    /// @param new_cap The new explicit capacity for the vector
    void manual_realloc(const std::size_t new_cap) {
        if (new_cap == 0) this->clear(); // Take the easy route

        Type* const old = data;
        data = VECLIB_NONCONSTRUCTOR_NEW(Type, new_cap);
        if (old) {
            std::size_t to_move = new_cap < count ? new_cap : count; // Min
            for (std::size_t i = 0; i < to_move; ++i) {
                if constexpr (std::is_move_constructible_v<Type>)
                    new(&data[i]) Type(std::move(old[i]));
                else
                    new(&data[i]) Type(old[i]); // Resort to copying
            }
            for (std::size_t i = 0; i < count; ++i)
                // We need this separate when shrinking because we always need to destroy the whole old vector anyways
                old[i].~Type();
            VECLIB_NONDESTRUCTOR_DELETE(old, cap);
        }
        cap = new_cap;
    }

    /// @brief Slide elements forward into the vector, allocating new memory if necessary,
    ///        and destroy the objects in the emptied region of memory
    /// @param index The index at which the slide should begin
    /// @param slots The number of slots to slide elements by
    void slide_forw(const std::size_t index, const std::size_t slots) {
        if (slots == 0) return;

        const std::size_t new_count = count + slots;
        if (new_count > cap) {
            // Manually reallocate
            if constexpr (Grow == GrowType::AddHalf)
                this->manual_realloc(new_count + new_count / 2);
            else
                this->manual_realloc(new_count * 2);
        }

        // Start doing the real work
        for (std::size_t i = count; i > index; --i) {
            const std::size_t source = i - 1;
            const std::size_t dest = source + slots;

            if (dest >= count) { // We are modifying raw memory
                if constexpr (std::is_move_constructible_v<Type>) // We need to construct in-place and not assign
                    new(&data[dest]) Type(std::move(data[source]));
                else
                    new(&data[dest]) Type(data[source]); // Resort to copying
            } else { // We are modifying live objects
                if constexpr (std::is_move_assignable_v<Type>)
                    data[dest] = std::move(data[source]);
                else if (std::is_copy_assignable_v<Type>)
                    data[dest] = data[source]; // Resort to copying
                else { // If we can't assign, we just destroy and re-construct in place
                    if constexpr (std::is_move_constructible_v<Type>) {
                        data[dest].~Type();
                        new(&data[dest]) Type(std::move(data[source]));
                    } else {
                        data[dest].~Type();
                        new(&data[dest]) Type(data[source]); // Resort to copying
                    }
                }
            }
        }

        // We now need to destroy the objects in the gap, but only the ones that were occupied and now aren't
        // Same reason for the ternary check
        const std::size_t gap_end = (index + slots) > count ? count : (index + slots);
        //if (gap_end > count) gap_end = count; // This is still the old count
        for (std::size_t i = index; i < gap_end; ++i)
            data[i].~Type(); // Destroy
        count = new_count; // Update size
    }

    /// @brief Slide elements backward into the vector, destroying the ones that are
    ///        now beyond the new size of the vector
    /// @param index The index at which the slide should begin
    /// @param slots The number of slots to slide elements by
    void slide_backw(const std::size_t index, const std::size_t slots) VECLIB_NOEXCEPT_DESTRUCTIBLE(Type) {
        const std::size_t new_count = count - slots;
        for (std::size_t i = index; i < new_count; ++i) {
            if constexpr (std::is_move_assignable_v<Type>)
                data[i] = std::move(data[i + slots]);
            else
                data[i] = data[i + slots]; // Resort to copying
        }
        for (std::size_t i = new_count; i < count; ++i)
            data[i].~Type();
        count = new_count; // Update our size
    }

    inline constexpr Slice<Type> internal_slice(const std::size_t start, const std::size_t end) VECLIB_NOEXCEPT {
        this->empty_check("veclib::Vector.slice(): The vector is empty or has been moved");
        this->range_check(start, "veclib::Vector.slice(): Starting index exceeds vector size");
        this->range_check(end, "veclib::Vector.slice(): Ending index exceeds vector size");
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(end >= start);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (end < start) throw std::logic_error("veclib::Vector.slice(): Ending index is lower than starting index");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return Slice<Type>(data + start, end - start);
    }
    inline constexpr const Slice<Type> internal_slice(const std::size_t start, const std::size_t end) const VECLIB_NOEXCEPT {
        this->empty_check("veclib::Vector.slice(): The vector is empty or has been moved");
        this->range_check(start, "veclib::Vector.slice(): Starting index exceeds vector size");
        this->range_check(end, "veclib::Vector.slice(): Ending index exceeds vector size");
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(end >= start);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (end < start) throw std::logic_error("veclib::Vector.slice(): Ending index is lower than starting index");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return Slice<Type>(data + start, end - start);
    }

public:

    /// @brief Default constructor
    Vector() noexcept = default;
    /// @brief Default destructor. This isn't `noexcept` because `clear()` is called,
    ///        which involves calling `delete`
    ~Vector() { this->clear(); }

    /// @brief Construct a vector out of a size and an optionally specified value
    ///        of `Type` to copy across the created slots
    /// @param size The size of the vector
    /// @param value The value to copy
    Vector(const std::size_t size, const Type& value = {}) {
        this->resize(size, value);
        count = size;
    }
    /// @brief Construct a vector out of a size and `Type` constructor arguments
    ///        for the allocated elements
    /// @tparam ...Args The type pack for the constructor arguments
    /// @param size The size of the vector
    /// @param ...args The constructor arguments
    template <typename... Args>
    Vector(const std::size_t size, Args&&... args) {
        this->resize(size, std::forward<Args>(args)...);
        count = size;
    }

    /// @brief Construct a vector by specifying two pointers that represent the start and
    ///        the end of the range that should be copied into the new vector. If the ending
    ///        pointer points to a lower address, the elements are copied in the opposide order.
    ///        The beginning pointer references the first element to be included, like an iterator,
    ///        and the ending pointer references the memory right past the last element to
    ///        be included, again just like an iterator. When copying in reverse order,
    /// @param begin The pointer referencing the first element
    /// @param end The pointer referencing memory past the last element, like iterators
    /// @throw `std::runtime_error` if any of the pointers are `nullptr`,
    ///        only if exceptions are not disabled
    Vector(const Type* const begin, const Type* const end) VECLIB_NOEXCEPT_COPY_CONSTRUCTIBLE(Type)
            requires (std::is_copy_constructible_v<Type>) : data(nullptr), count(0), cap(0) {
        // If both are equal we have no range to copy and there is no reason to throw if they are nullptr
        if (begin == end) return; // We already initialized with member initializer lists

        // Check if they are nullptr alone
        this->ptr_check(begin, "veclib::Vector(): Begin pointer is nullptr");
        this->ptr_check(end, "veclib::Vector(): End pointer is nullptr");

        // Pointer range shenanigans
        const std::size_t range_size = begin <= end ? (end - begin) : (begin - end);
        this->reserve(range_size);

        for (std::size_t i = 0; i < range_size; ++i) {
            if (begin <= end)
                new(&data[i]) Type(begin[i]);
            else
                new(&data[i]) Type(*(begin - i));
            ++count; // Increment every time so that if `Type`'s constructor throws we still have a valid size
        }
    }

    /// @brief Construct a vector out of an initializer list
    /// @param args The initializer list
    Vector(const std::initializer_list<Type>& args) VECLIB_NOEXCEPT_COPY_CONSTRUCTIBLE(Type)
            requires (std::is_copy_constructible_v<Type>) {
        this->reserve(args.size());
        count = cap; // We are full
        for (std::size_t i = 0; i < count; ++i)
            new(&data[i]) Type(args.begin()[i]);
    }
    /// @brief Assign this vector to an initializer list
    /// @param args The initializer list
    inline constexpr Self& operator=(const std::initializer_list<Type>& args)
            VECLIB_NOEXCEPT_COPY_CONSTRUCTIBLE(Type)
            requires (std::is_copy_constructible_v<Type>) {
        this->clear();
        this->reserve(args.size());
        count = cap;
        for (std::size_t i = 0; i < count; ++i)
            new(&data[i]) Type(args.begin()[i]);
        return *this;
    }

    /// @brief Copy constructor
    /// @param other The vector to copy
    Vector(const Self& other) requires (std::is_copy_constructible_v<Type>) {
        this->clear();
        this->reserve(other.count);
        count = cap; // We are full
        for (std::size_t i = 0; i < count; ++i)
            new(&data[i]) Type(other[i]);
    }
    /// @brief Copy assignment
    /// @param other The vector to copy
    inline constexpr Self& operator=(const Self& other)
            requires (std::is_copy_constructible_v<Type>) {
        this->clear();
        this->reserve(other.count);
        count = cap; // We are full
        for (std::size_t i = 0; i < count; ++i)
            new(&data[i]) Type(other[i]);
        return *this;
    }

    /// @brief Move constructor
    /// @param other The vector to move
    Vector(Self&& other) {
        this->clear(); // We can't use member initializer lists because we need to clear first
        data = other.data;
        count = other.count;
        cap = other.cap;
        other.data = nullptr;
        other.count = 0;
        other.cap = 0;
    }
    /// @brief Move assignment
    /// @param other The vector to move
    inline constexpr Self& operator=(Self&& other) {
        this->clear();
        data = other.data;
        count = other.count;
        cap = other.cap;
        other.data = nullptr;
        other.count = 0;
        other.cap = 0;
        return *this;
    }

    // Make more constructors?

    /// @brief Clear all elements of this vector, destroying them
    ///        and deallocating the internal buffer. To avoid deallocating
    ///        the internal buffer, call `destroy()`
    void clear() /* this can throw due to `delete` */ {
        if (this->cleared()) return; // Safe check
        if (!this->empty())
            for (std::size_t i = 0; i < count; ++i)
                data[i].~Type(); // Destroy manually
        // Free everything
        VECLIB_NONDESTRUCTOR_DELETE(data, cap);

        data = nullptr;
        count = 0;
        cap = 0;
    }

    /// @brief Destroy all elements inside the vector while keeping
    ///        the allocated buffer
    void destroy() {
        if (this->cleared()) return;
        if (!this->empty())
            for (std::size_t i = 0; i < count; ++i)
                data[i].~Type();
        count = 0; // We are empty but we still have an allocated buffer and we keep the capacity
    }

    /// @brief Return a `const` pointer to the internal buffer
    inline constexpr const Type* get() const noexcept { return data; }
    /// @brief Return the number of constructed elements in the vector
    inline constexpr std::size_t size() const noexcept { return count; }
    /// @brief Return the number of allocated elements in the vector
    inline constexpr std::size_t capacity() const noexcept { return cap; }
    /// @brief Check if this vector's size is `0`
    inline constexpr bool empty() const noexcept { return count == 0; }
    /// @brief Check if this vector was cleared
    inline constexpr bool cleared() const noexcept { return data == nullptr && count == 0 && cap == 0; }
    /// @brief Alias for calling `cleared()`
    inline constexpr bool moved() const noexcept { return this->cleared(); }

    /// @brief Reserve space for a given amount of elements. If the current
    ///        capacity already satisfies the requirement, nothing is done.
    ///        To instead reserve space for a given amount of additional
    ///        elements, call `grow()`
    /// @param new_cap The new capacity
    /// @return The number of added slots
    inline constexpr std::size_t reserve(const std::size_t new_cap) {
        if (new_cap < cap) return 0;
        // How many slots were added
        std::size_t added = new_cap - cap;
        this->manual_realloc(new_cap);
        return added;
    }

    /// @brief Resize the vector to a given size, constructing new elements by
    ///        copying an optionally provided value of `Type`. If the requested size
    ///        is less than the current size, elements at the end of the vector
    ///        are destroyed
    /// @param new_count The new size of the vector
    /// @param value The value to copy
    /// @return The number of added, if positive, or removed slots, if negative
    inline constexpr diff_t resize(const std::size_t new_count, const Type& value = {}) {
        if (new_count == 0) { // Easy route
            diff_t current = -count;
            this->clear();
            return current;
        }
        this->manual_realloc(new_count);
        // Construct the new objects if we have any
        if (new_count > count)
            for (std::size_t i = count; i < cap; ++i)
                new(&data[i]) Type(value);
        const diff_t diff = new_count - count; // Negative if we shrunk
        count = new_count;
        return diff;
    }
    /// @brief Resize the vector to a given size, constructing new elements with
    ///        provided constructor arguments. If the requested size is less than
    ///        the current size, elements at the end of the vector are destroyed
    /// @tparam ...Args The type pack for the constructor arguments
    /// @param new_count The new size
    /// @param ...args The constructor arguments
    /// @return The number of added, if positive, or removed slots, if negative
    template <typename... Args>
    inline constexpr diff_t resize(const std::size_t new_count, Args&&... args) {
        if (new_count == 0) {
            diff_t current = -count;
            this->clear();
            return current;
        }
        this->manual_realloc(new_count);
        if (new_count > count)
            for (std::size_t i = count; i < cap; ++i)
                new(&data[i]) Type(std::forward<Args>(args)...);
        const diff_t diff = new_count - count;
        count = new_count;
        return diff;
    }

    /// @brief Resize the vector to hold a given number of additional elements.
    ///        This works essentially like `resize()` but with additional space
    ///        instead of absolute space
    /// @param new_slots The number of additional elements to add
    /// @param value An optional value to copy across the new slots
    /// @return The new size of the vector
    inline constexpr std::size_t grow(const std::size_t new_slots, const Type& value = {})
            /* not noexcept because of allocations */ requires (std::is_copy_constructible_v<Type>) {
        this->resize(count + new_slots, value);
        return count;
    }
    /// @brief Resize the vector to hold a given number of additional elements.
    ///        This works essentially like `resize()` but with additional space
    ///        instead of absolute space
    /// @tparam ...Args The type pack for the constructor arguments
    /// @param new_slots The number of additional elements to add
    /// @param args Constructor arguments for the elements in the new slots
    /// @return The new size of the vector
    template <typename... Args>
    inline constexpr std::size_t grow(const std::size_t new_slots, Args&&... args) {
        this->resize(count + new_slots, std::forward<Args>(args)...);
        return count;
    }

    /// @brief Reserve space for a given amount of additional elements.
    ///        This works essentially like `reserve()` but with additional
    ///        space instead of absoulute space
    /// @param new_slots The number of additional elements to reserve space for
    /// @return The vector's new capacity
    inline constexpr bool grow_reserve(const std::size_t new_slots) {
        this->reserve(count + new_slots);
        return cap;
    }

    /// @brief Shrink this vector's allocated buffer to fit the number of constructed
    ///        elements, making the size and the capacity equal. This is done by creating
    ///        a vector with the minimum capacity possible, and then moving or copying old elements
    ///        over, to then swap the two vectors and let the older one get destroyed safely
    /// @return The number of slots that were deallocated
    inline constexpr std::size_t shrink_to_fit()
            VECLIB_NOEXCEPT_IF(!std::is_nothrow_move_assignable_v<Type> ? VECLIB_NOEXCEPT_COPY_ASSIGNABLE(Type) : false) {
            // The `false` is because we could throw while allocating
        const std::size_t cut = cap - count;
        if (cut == 0) return 0;
        Self self(count); // Make it have the exact size
        for (std::size_t i = 0; i < count; ++i) {
            // Choose the best option
            if constexpr (std::is_move_assignable_v<Type>)
                self[i] = std::move(data[i]);
            else
                self[i] = data[i]; // Resort to copy
        }
        this->swap(self); // We get the new vector and our old self gets destroyed safely
        return cut;
    }

    /// @brief Shrink this vector by a given amount of elements
    /// @param slots The number of elements to shrink by
    /// @return The new size of the vector
    /// @throw `std::runtime_error` if the vector is empty or has been moved
    ///        and `std::out_of_range` if the requested slots to shrink by exceed
    ///        the vector's size, only if exceptions are not disabled
    inline constexpr std::size_t shrink(const std::size_t slots) {
        this->empty_check("veclib::Vector.shrink(): The vector is empty or has been moved");
        this->range_check(slots, "veclib::Vector.shrink(): Trying to shrink by too much");
        this->resize(count - slots);
        return count;
    }

    /// @brief Swap the contents of two vectors
    /// @param other The other vector
    inline constexpr void swap(Self& other) {
        // Smart little trick right here
        data ^= other.data;
        other.data ^= data;
        data ^= other.data;

        count ^= other.count;
        other.count ^= count;
        count ^= other.count;

        cap ^= other.cap;
        other.cap ^= cap;
        cap ^= other.cap;
    }

    // Evaluates to `true` if move is chosen and `false` if copy is chosen using this criteria:
    // - If we only have move we choose that
    // - If we only have copy we choose that
    // - If copy can throw we choose move
    // - If copy is noexcept, move isn't and value is true, we choose copy
    // - We choose move
    #define VECLIB_TEMP_CHOOSE_MOVE_OR_COPY(value, type) ( \
        (VECLIB_FULLY_MOVABLE(type) && !VECLIB_FULLY_COPYABLE(type)) ? true : ( \
        (!VECLIB_FULLY_MOVABLE(type) && VECLIB_FULLY_COPYABLE(type)) ? false : ( \
        !VECLIB_FULLY_NOEXCEPT_COPYABLE(type) ? true : ( \
        (VECLIB_FULLY_NOEXCEPT_COPYABLE(type) && !VECLIB_FULLY_NOEXCEPT_MOVABLE(type) && (value)) ? false : true \
    ))))

    // Choose the right `noexcept` check and add the destructor being `noexcept` as a requirement,
    // due to the temporary variable
    #define VECLIB_TEMP_SELECT_NOEXCEPT(value, type) (( \
        VECLIB_TEMP_CHOOSE_MOVE_OR_COPY(value, type) ? \
        VECLIB_FULLY_NOEXCEPT_MOVABLE(type) : VECLIB_FULLY_NOEXCEPT_COPYABLE(type) \
    ) && std::is_nothrow_destructible_v<type> /* require this because we have our temporary variable being destroyed */)

    /// @brief Reverse this vector in-place. This method requires `Type` to be fully movable or fully copyable
    ///        (constructor and assignment), and is `noexcept` only if operation chosen between moving or copying is.
    ///        When available, moving is always preferred, but to instead prefer the `noexcept` method while still
    ///        giving priority to moving, template this function with `true`. This means that if copying is `noexcept`
    ///        but moving isn't, copying will be chosen, otherwise moving is always selected.
    ///        If any of the chosen operations throw, some elements may not be left in a valid state, meaning partially
    ///        moved, reversed or both
    /// @tparam UseNoexcept If set to `true`, this makes the function prefer the non-throwing path between copying
    ///         or moving while always giving priority to moving
    template <bool UseNoexcept = false>
    inline constexpr void reverse() VECLIB_NOEXCEPT_IF(VECLIB_TEMP_SELECT_NOEXCEPT(UseNoexcept, Type))
            // Require full moving or full copying as a last resort
            requires (VECLIB_FULLY_MOVABLE(Type) || VECLIB_FULLY_COPYABLE(Type)) {

        if (count <= 1) return; // Easy path for empty and one-element vectors

        for (std::size_t left = 0; left < count / 2; ++left) {
            const std::size_t right = count - 1 - left;

            // Choose the preferred path
            if constexpr (VECLIB_TEMP_CHOOSE_MOVE_OR_COPY(UseNoexcept, Type)) {
                Type temp = std::move(data[left]); // This is actually construction
                data[left] = std::move(data[right]); // These are assignments
                data[right] = std::move(temp);
            } else { // Resort to copying
                Type temp = data[left]; // Again this is construction
                data[left] = data[right]; // And these are assignments
                data[right] = temp;
            }
        }
    }

    #undef VECLIB_TEMP_CHOOSE_MOVE_OR_COPY
    #undef VECLIB_TEMP_SELECT_NOEXCEPT

    /// @brief Remove an element given a pointer to it
    /// @param ptr The pointer to the element to remove
    /// @return The new vector size
    /// @throw `std::runtime_error` if the pointer is `nullptr` or `std::out_of_range`
    ///        if the pointer doesn't belong to the vector's address space
    inline constexpr std::size_t remove(const Type* ptr) {
        this->ptr_check(ptr, "veclib::Vector.remove(): Element pointer is nullptr");
        if (ptr < data) ptr = nullptr; // Hijack ourselves, kinda
        this->ptr_check(ptr, "veclib::Vector.remove(): Element pointer points before valid memory");
        const std::size_t index = ptr - data;
        this->bounds_check(index, "veclib::Vector.remove(): Element pointer is out of range");
        // We call this even if it's one element so we have no constraints and we can directly return this
        return this->pop_at(index, 1);
    }
    /// @brief Check if a given index is inside the valid bounds for this vector
    /// @param i The index into the vector
    inline constexpr bool inside_bounds(const std::size_t i) const noexcept { return i < count; }
    /// @brief Check if a given pointer is inside the valid bounds for this vector
    /// @param ptr The pointer to an element in the vector
    /// @return `false` if the pointer is `nullptr` or not in the valid bounds
    inline constexpr bool inside_bounds(const Type* const ptr) const noexcept { return !ptr ? false : (data <= ptr && ptr < (data + count)); }

    /// @brief Create a `Slice` object referencing a portion of this vector's memory,
    ///        optionally specifying starting and ending indeces. The ending index is
    ///        exclusive. To instead use a length for the slice, call `slice_len()`
    /// @param start The starting index
    /// @param end The ending exclusive index
    /// @throw `std::runtime_error` if the vector is empty or has been moved and `std::out_of_range`
    ///        if either index is out of bounds, only if exceptions are not disabled
    inline constexpr Slice<Type> slice(const std::size_t start = 0) VECLIB_NOEXCEPT {
        return this->internal_slice(start, count - start);
    }
    inline constexpr const Slice<Type> slice(const std::size_t start = 0) const VECLIB_NOEXCEPT {
        return this->internal_slice(start, count - start);
    }
    inline constexpr Slice<Type> slice(const std::size_t start, const std::size_t end) VECLIB_NOEXCEPT {
        return this->internal_slice(start, end);
    }
    inline constexpr const Slice<Type> slice(const std::size_t start, const std::size_t end) const VECLIB_NOEXCEPT {
        return this->internal_slice(start, end);
    }
    /// @brief Create a `Slice` object referencing a portiona of this vector's memory,
    ///        optionally specifying a starting index and a length. If the length exceeds
    ///        the valid bounds for this vector, it is clamped before creating the slice
    /// @param start The starting index
    /// @param len The slice length
    /// @throw `std::runtime_error` if the vector is empty or has been moved and `std::out_of_range`
    ///        if the starting index is out of bounds, only if exceptions are not disabled
    inline constexpr Slice<Type> slice_len(const std::size_t start = 0, const std::size_t len = ~0ULL) VECLIB_NOEXCEPT {
        this->empty_check("veclib::Vector.slice_len(): The vector is empty or has been moved");
        this->bounds_check(start, "veclib::Vector.slice_len(): Starting index is out of bounds");
        // Clamp the value with a ternary check so len can be `const`
        return Slice<Type>(data + start, (len >= count - start) ? (count - start) : len);
    }
    inline constexpr const Slice<Type> slice_len(const std::size_t start = 0, const std::size_t len = ~0ULL) const VECLIB_NOEXCEPT {
        this->empty_check("veclib::Vector.slice_len(): The vector is empty or has been moved");
        this->bounds_check(start, "veclib::Vector.slice_len(): Starting index is out of bounds");
        // Clamp the value with a ternary check so len can be `const`
        return Slice<Type>(data + start, (len >= count - start) ? (count - start) : len);
    }

    /// @brief Return a reference to the first element in the vector
    /// @throw `std::runtime_error` if the vector is empty or has been moved, only if
    ///        exceptions are not disabled
    inline constexpr Type& first() VECLIB_NOEXCEPT {
        this->empty_check("veclib::Vector.first(): The vector is empty or has been moved");
        return data[0];
    }
    inline constexpr const Type& first() const VECLIB_NOEXCEPT {
        this->empty_check("veclib::Vector.first(): The vector is empty or has been moved");
        return data[0];
    }
    /// @brief Return a slice referencing a given number of first elements in the vector
    /// @param x The number of elements to extract
    /// @throw `std::runtime_error` if the vector is empty or has been moved and `std::out_of_range`
    ///        if the number of elements to extract exceeds the vector's size, only if exceptions are not disabled
    inline constexpr Slice<Type> first(const std::size_t x) VECLIB_NOEXCEPT {
        this->empty_check("veclib::Vector.first(): The vector is empty or has been moved");
        this->range_check(x, "veclib::Vector.first(): Trying to extract too many first elements");
        return Slice<Type>(data, x);
    }
    inline constexpr const Slice<Type> first(const std::size_t x) const VECLIB_NOEXCEPT {
        this->empty_check("veclib::Vector.first(): The vector is empty or has been moved");
        this->range_check(x, "veclib::Vector.first(): Trying to extract too many first elements");
        return Slice<Type>(data, x);
    }

    /// @brief Return a reference to the last element in the vector
    /// @throw `std::runtime_error` if the vector is empty or has been moved, only if
    ///        exceptions are not disabled
    inline constexpr Type& last() VECLIB_NOEXCEPT {
        this->empty_check("veclib::Vector.first(): The vector is empty or has been moved");
        return data[count - 1];
    }
    inline constexpr const Type& last() const VECLIB_NOEXCEPT {
        this->empty_check("veclib::Vector.first(): The vector is empty or has been moved");
        return data[count - 1];
    }
    /// @brief Return a slice referencing a given number of last elements in the vector
    /// @param x The number of elements to extract
    /// @throw `std::runtime_error` if the vector is empty or has been moved and `std::out_of_range`
    ///        if the number of elements to extract exceeds the vector's size, only if exceptions are not disabled
    inline constexpr Slice<Type> last(const std::size_t x) VECLIB_NOEXCEPT {
        this->empty_check("veclib::Vector.last(): The vector is empty or has been moved");
        this->range_check(x, "veclib::Vector.last(): Trying to extract too many last elements");
        return Slice<Type>(data + count - x, x);
    }
    inline constexpr const Slice<Type> last(const std::size_t x) const VECLIB_NOEXCEPT {
        this->empty_check("veclib::Vector.last(): The vector is empty or has been moved");
        this->range_check(x, "veclib::Vector.last(): Trying to extract too many last elements");
        return Slice<Type>(data + count - x, x);
    }

    /// @brief Index into the vector without bounds checking
    /// @param i The index into the vector
    inline constexpr Type& operator[](const std::size_t i) noexcept { return data[i]; }
    inline constexpr const Type& operator[](const std::size_t i) const noexcept { return data[i]; }

    /// @brief Index into the vector with bounds checking
    /// @param i The index into the vector
    /// @throw `std::out_of_range` if the index is out of range, only if exceptions are not disabled
    inline constexpr Type& at(const std::size_t i) VECLIB_NOEXCEPT {
        this->bounds_check(i, "veclib::Vector.at(): Index is out of bounds");
        return data[i];
    }
    inline constexpr const Type& at(const std::size_t i) const VECLIB_NOEXCEPT {
        this->bounds_check(i, "veclib::Vector.at(): Index is out of bounds");
        return data[i];
    }

    #ifdef VECLIB_EXTRA

    /// @brief Index circularly into the vector, preventing any out-of-bounds access
    /// @param i The index into the vector
    inline constexpr Type& circular_at(const std::size_t i) noexcept { return data[i % count]; }
    inline constexpr const Type& circular_at(const std::size_t i) const noexcept { return data[i % count]; }

    #ifndef VECLIB_NO_OPERATOR_OVERLOADS

    /// @brief Operator overload for calling `circular_at()`
    inline constexpr Type& operator()(const std::size_t i) noexcept { return data[i % count]; }
    inline constexpr const Type& operator()(const std::size_t i) const noexcept { return data[i % count]; }

    #endif // VECLIB_NO_OPERATOR_OVERLOADS

    /// @brief Index into the vector clamping the provided index to the maximum value,
    ///        preventing any out-of-bounds access
    /// @param i The index into the vector
    inline constexpr Type& clamped_at(const std::size_t i) noexcept { return data[i >= count ? count - 1 : i]; }
    inline constexpr const Type& clamped_at(const std::size_t i) const noexcept { return data[i >= count ? count - 1 : i]; }

    #endif // VECLIB_EXTRA

    /// @brief Check if the vector contains a given element of `Type` anywhere in it
    /// @param value The value to search for
    inline constexpr bool contains(const Type& value) const VECLIB_NOEXCEPT_EQ_CMP(Type)
            requires (std::equality_comparable<Type>) {
        if (this->empty()) return false;
        for (std::size_t i = 0; i < count; ++i)
            if (data[i] == value) return true;
        return false;
    }

    /// @brief Count how many occurrences of a value are present in the vector
    /// @param value The value to search for
    inline constexpr std::size_t how_many(const Type& value) const VECLIB_NOEXCEPT_EQ_CMP(Type)
            requires (std::equality_comparable<Type>) {
        if (this->empty()) return 0;
        std::size_t num = 0;
        for (std::size_t i = 0; i < count; ++i)
            if (data[i] == value) ++num;
        return num;
    }

    /// @brief Apply a custom function to each element of the vector.
    ///        The function must return `void` and must take a `Type&` parameter,
    ///        into which will be passed the current element in the vector,
    ///        and can optionally accept an `std::size_t` parameter, into which
    ///        the index of the current element will be passed
    /// @param fn The function to apply
    inline constexpr Self& map(Function<void, Type&> fn)
            VECLIB_COND_NOEXCEPT(fn, std::declval<Type&>()) {
        if (this->empty()) return *this;
        for (std::size_t i = 0; i < count; ++i)
            fn(data[i]);
        return *this;
    }
    inline constexpr Self& map(Function<void, Type&, std::size_t> fn)
            VECLIB_COND_NOEXCEPT(fn, std::declval<Type&>(), 0) {
        if (this->empty()) return *this;
        for (std::size_t i = 0; i < count; ++i)
            fn(data[i], i);
        return *this;
    }

    /// @brief Combine all the values of the elements in this vector with a custom
    ///        function, that must return `void` and must take a `Type&` parameter,
    ///        which will be the accumulated value, a `const Type&` parameter, which
    ///        will be the current element, and optionally an `std::size_t` parameter
    ///        into which the index of the current element will be passed
    /// @param fn The transformation function
    /// @param acc An optional starting value for the accumulated value. By default
    ///            it is assigned a default-constructed value of `Type`
    inline constexpr Type fold(Function<void, Type&, const Type&> fn, Type acc = {}) const
            VECLIB_COND_NOEXCEPT(fn, std::declval<Type&>(), std::declval<const Type&>())
            requires (std::is_default_constructible_v<Type>) {
        if (this->empty()) return acc;
        for (std::size_t i = 0; i < count; ++i)
            fn(acc, data[i]);
        return acc;
    }
    inline constexpr Type fold(Function<void, Type&, const Type&, std::size_t> fn, Type acc = {})
            const VECLIB_COND_NOEXCEPT(fn, std::declval<Type&>(), std::declval<const Type&>(), 0)
            requires (std::is_default_constructible_v<Type>) {
        if (this->empty()) return acc;
        for (std::size_t i = 0; i < count; ++i)
            fn(acc, data[i], i);
        return acc;
    }

    /// @brief Filter elements in this vector using a custom function and return a new vector
    ///        with the elements that were chosen. The function must return `bool` and accept
    ///        a `const Type&` parameter, and could optionally also accept an `std::size_t` parameter
    ///        into which will be passed the index into the vector of the current element
    /// @tparam Preallocate If set to `true`, the buffer of the new vector is preallocated before
    ///         filtering. This is useful when the vector being filtered is large or it is known ahead
    ///         of time that the number of filtered-out elements will be small
    /// @param fn The custom function, return `true` if the current element should be included in
    ///           the new vector
    template <bool Preallocate = true>
    inline constexpr Self filter(Function<bool, const Type&> fn) const
            /* this isn't noexcept because of allocations */
            requires (std::is_default_constructible_v<Type>) {
        if (this->empty()) return Self();
        Self self;
        if constexpr (Preallocate) self.reserve(count); // Make it big enough when starting
        for (std::size_t i = 0; i < count; ++i)
            if (fn(data[i])) self.push_back(data[i]); // Copy it
        self.shrink_to_fit();
        return self;
    }
    template <bool Preallocate = true>
    inline constexpr Self filter(Function<bool, const Type&, std::size_t> fn) const
            requires (std::is_default_constructible_v<Type>) {
        if (this->empty()) return Self();
        Self self;
        if constexpr (Preallocate) self.reserve(count); // Make it big enough when starting
        for (std::size_t i = 0; i < count; ++i)
            if (fn(data[i], i)) self.push_back(data[i]); // Copy it
        self.shrink_to_fit();
        return self;
    }

    /// @brief Return `true` if all elements in the vector pass a custom check function.
    ///        The check function must return `bool`, accept a `const Type&` parameter
    ///        and optionally an `std::size_t` parameter into which will be passed the
    ///        index into the vector of the element currently being processed
    /// @param fn The check function
    inline constexpr bool all(Function<bool, const Type&> fn) const
            VECLIB_COND_NOEXCEPT(fn, std::declval<const Type&>()) {
        if (this->empty()) return false;
        for (std::size_t i = 0; i < count; ++i)
            if (!fn(data[i])) return false;
        return true;
    }
    inline constexpr bool all(Function<bool, const Type&, std::size_t> fn) const
            VECLIB_COND_NOEXCEPT(fn, std::declval<const Type&>(), 0) {
        if (this->empty()) return false;
        for (std::size_t i = 0; i < count; ++i)
            if (!fn(data[i], i)) return false;
        return true;
    }

    /// @brief Return `true` if at least an element in the vector passes a custom check function.
    ///        The check function must return `bool`, accept a `const Type&` parameter
    ///        and optionally an `std::size_t` parameter into which will be passed the
    ///        index into the vector of the element currently being processed
    /// @param fn The check function
    inline constexpr bool any(Function<bool, const Type&> fn) const
            VECLIB_COND_NOEXCEPT(fn, std::declval<const Type&>()) {
        if (this->empty()) return false;
        for (std::size_t i = 0; i < count; ++i)
            if (fn(data[i])) return true;
        return false;
    }
    inline constexpr bool any(Function<bool, const Type&, std::size_t> fn) const
            VECLIB_COND_NOEXCEPT(fn, std::declval<const Type&>(), 0) {
        if (this->empty()) return false;
        for (std::size_t i = 0; i < count; ++i)
            if (fn(data[i], i)) return true;
        return false;
    }

    /// @brief Return `true` if all elements in the vector don't pass a custom check function.
    ///        The check function must return `bool`, accept a `const Type&` parameter
    ///        and optionally an `std::size_t` parameter into which will be passed the
    ///        index into the vector of the element currently being processed
    /// @param fn The check function
    inline constexpr bool none(Function<bool, const Type&> fn) const
            VECLIB_COND_NOEXCEPT(fn, std::declval<const Type&>()) {
        if (this->empty()) return false;
        for (std::size_t i = 0; i < count; ++i)
            if (fn(data[i])) return false;
        return true;
    }
    inline constexpr bool none(Function<bool, const Type&, std::size_t> fn) const
            VECLIB_COND_NOEXCEPT(fn, std::declval<const Type&>(), 0) {
        if (this->empty()) return false;
        for (std::size_t i = 0; i < count; ++i)
            if (fn(data[i], i)) return false;
        return true;
    }

    /// @brief Fill the array with a specified value
    /// @param value The value to use
    inline constexpr Self& fill(const Type& value) VECLIB_NOEXCEPT_IF(
            std::is_copy_assignable_v<Type> ? std::is_nothrow_copy_assignable_v<Type> :
            std::is_nothrow_copy_constructible_v<Type>) // Choose `noexcept`ness based on the chosen copy operation
            requires (std::is_copy_assignable_v<Type> || std::is_copy_constructible_v<Type>) {
        if (this->empty()) return *this; // Easy route

        for (std::size_t i = 0; i < count; ++i) {
            if constexpr (std::is_copy_assignable_v<Type>)
                data[i] = value;
            else { // Choose destruction and copy construction
                data[i].~Type();
                new(&data[i]) Type(value);
            }
        }
        return *this;
    }
    /// @brief Fill the array by constructing elements in-place after destroying them
    /// @tparam ...Args The type parameter packs for the constructor
    /// @param args The constructor arguments
    template <typename... Args>
    inline constexpr Self& fill(Args&&... args) {
        if (this->empty()) return *this; // Easy route
        for (std::size_t i = 0; i < count; ++i) {
            data[i].~Type();
            new(&data[i]) Type(std::forward<Args>(args)...);
        }
        return *this;
    }

    /// @brief Copy a value of `Type` at the end of the vector, reallocating if necessary.
    ///        Unlike all other push or place operations, this one leaves the vector in a
    ///        valid state if `Type`'s copy constructor throws, since the vector's size is
    ///        not incremented until construction has finished, but the vector could still
    ///        end up in an invalid state during the internal buffer's reallocation
    /// @param value The value to copy
    /// @return A reference to the added value
    inline constexpr Type& push_back(const Type& value) requires (std::is_copy_constructible_v<Type>) {
        if (count + 1 > cap) realloc();
        new(&data[count]) Type(value); // Copy it
        return data[count++]; // Return a reference to it
    }
    /// @brief Move a value of `Type` at the end of the vector, reallocating if necessary.
    ///        Unlike all other push or place operations, this one leaves the vector in a
    ///        valid state if `Type`'s move constructor throws, since the vector's size is
    ///        not incremented until construction has finished, but the vector could still
    ///        end up in an invalid state during the internal buffer's reallocation
    /// @param value The value to move
    /// @return A reference to the added value
    inline constexpr Type& push_back(Type&& value) requires (std::is_move_constructible_v<Type>) {
        if (count + 1 > cap) realloc();
        new(&data[count]) Type(std::move(value)); // Move it
        return data[count++];
    }

    /// @brief Construct in-place a `Type` object at the end of the vector.
    ///        Unlike all other push or place operations, this one leaves the vector in a
    ///        valid state if `Type`'s called constructor throws, since the vector's size is
    ///        not incremented until construction has finished, but the vector could still
    ///        end up in an invalid state during the internal buffer's reallocation
    /// @tparam ...Args The type pack for the constructor arguments
    /// @param ...args The constructor arguments
    /// @return A reference to the added value
    template <typename... Args>
    inline constexpr Type& place_back(Args&&... args) {
        if (count + 1 > cap) realloc();
        new(&data[count]) Type(std::forward<Args>(args)...);
        return data[count++];
    }

    /// @brief Copy a value of `Type` at the front of the vector, reallocating if necessary.
    ///        If the copy constructor of `Type` throws while pushing or shifting elements,
    ///        the exception propagates and the vector may not be safe to use anymore
    /// @param value The value to copy
    /// @return A reference to the added value
    inline constexpr Type& push_front(const Type& value) requires (std::is_copy_constructible_v<Type>) {
        this->slide_forw(0, 1);
        new(data) Type(value);
        return data[0];
    }
    /// @brief Move a value of `Type` at the front of the vector, reallocating if necessary.
    ///        If the move constructor of `Type` throws while pushing or shifting elements,
    ///        the exception propagates and the vector may not be safe to use anymore
    /// @param value The value to move
    /// @return A reference to the added value
    inline constexpr Type& push_front(Type&& value) requires (std::is_move_constructible_v<Type>) {
        this->slide_forw(0, 1);
        new(data) Type(std::move(value));
        return data[0];
    }

    /// @brief Construct in-place a `Type` object at the front of the vector.
    ///        If the called constructor of `Type` throws while pushing or shifting elements,
    ///        the exception propagates and the vector may not be safe to use anymore
    /// @tparam ...Args The type pack for the constructor arguments
    /// @param ...args The constructor arguments
    /// @return A reference to the added value
    template <typename... Args>
    inline constexpr Type& place_front(Args&&... args) {
        this->slide_forw(0, 1);
        new(data) Type(std::forward<Args>(args)...);
        return data[0];
    }

    /// @brief Copy a value of `Type` at a given index in the vector, reallocating if necessary.
    ///        If the copy constructor of `Type` throws while pushing or shifting elements,
    ///        the exception propagates and the vector may not be safe to use anymore
    /// @param index The index at which the value should be inserted
    /// @param value The value to copy
    /// @return A reference to the added value
    /// @throw `std::out_of_range` if the given index is out of bounds, only if exceptions are not disabled
    inline constexpr Type& push_at(const std::size_t index, const Type& value) requires (std::is_copy_constructible_v<Type>) {
        this->range_check(index, "veclib::Vector.push_at(): Index is out of bounds");
        if (index == count) // Special case
            return this->push_back(value); // Very simple
        this->slide_forw(index, 1); // This already updates our size
        new(&data[index]) Type(value);
        return data[index];
    }
    /// @brief Move a value of `Type` at a given index in the vector, reallocating if necessary.
    ///        If the move constructor of `Type` throws while pushing or shifting elements,
    ///        the exception propagates and the vector may not be safe to use anymore
    /// @param index The index at which the value should be inserted
    /// @param value The value to move
    /// @return A reference to the added value
    /// @throw `std::out_of_range` if the given index is out of bounds, only if exceptions are not disabled
    inline constexpr Type& push_at(const std::size_t index, Type&& value) requires (std::is_move_constructible_v<Type>) {
        this->range_check(index, "veclib::Vector.push_at(): Index is out of bounds");
        if (index == count) // Special case
            return this->push_back(value); // Very simple
        this->slide_forw(index, 1);
        new(&data[index]) Type(std::move(value));
        return data[index];
    }

    /// @brief Construct in-place a `Type` object at a given index in the vector.
    ///        If the called constructor of `Type` throws while pushing or shifting elements,
    ///        the exception propagates and the vector may not be safe to use anymore
    /// @tparam ...Args The type pack for the constructor arguments
    /// @param index The index at which the object should be constructed
    /// @param ...args The constructor arguments
    /// @return A reference to the added value
    /// @throw `std::out_of_range` if the given index is out of bounds, only if exceptions are not disabled
    template <typename... Args>
    inline constexpr Type& place_at(const std::size_t index, Args&&... args) {
        this->range_check(index, "veclib::Vector.place_at(): Index is out of bounds");
        if (index == count)
            return this->place_back(std::forward<Args>(args)...);
        this->slide_forw(index, 1);
        new(&data[index]) Type(std::forward<Args>(args)...);
        return data[index];
    }

    /// @brief Remove an element at the end of the vector
    /// @return The removed element
    /// @throw `std::runtime_error` if the vector is empty or has been moved, only if
    ///        exceptions are not disabled
    inline constexpr Type pop_back() VECLIB_NOEXCEPT_DESTRUCTIBLE(Type)
            requires (std::is_move_constructible_v<Type> || std::is_copy_constructible_v<Type>) {
        this->empty_check("veclib::Vector.pop_back(): The vector is empty or has been moved");
        // Although we duplicate code here this allows us not to require Type to be default constructible
        if constexpr (std::is_move_constructible_v<Type>) {
            Type popped = std::move(data[--count]); // We are constructing even though we use `=`
            data[count].~Type();
            return popped;
        } else {
            Type popped = data[--count];
            data[count].~Type();
            return popped;
        }
    }
    /// @brief Remove a given amount of elements from the end of the vector
    /// @param slots The number of elements to remove
    /// @return The new size of the vector
    /// @throw `std::runtime_error` if the vector is empty or has been moved and `std::out_of_range`
    ///        if the given number of elements exceeds the vector's size, only if exceptions are not disabled
    inline constexpr std::size_t pop_back(const std::size_t slots) VECLIB_NOEXCEPT_DESTRUCTIBLE(Type) {
        this->empty_check("veclib::Vector.pop_back(): The vector is empty or has been moved");
        this->range_check(slots, "veclib::Vector.pop_back(): Can't pop this many slots");
        // Return the new size
        if (slots == 0) return count;
        for (std::size_t i = count - slots; i < count; ++i)
            data[i].~Type();
        count -= slots;
        return count;
    }

    /// @brief Remove an element at the front of the vector
    /// @return The removed element
    /// @throw `std::runtime_error` if the vector is empty or has been moved, only if
    ///        exceptions are not disabled
    inline constexpr Type pop_front() VECLIB_NOEXCEPT_DESTRUCTIBLE(Type)
            requires (std::is_move_constructible_v<Type> || std::is_copy_constructible_v<Type>) {
        this->empty_check("veclib::Vector.pop_front(): The vector is empty or has been moved");
        if constexpr (std::is_move_constructible_v<Type>) {
            Type popped = std::move(data[0]);
            this->slide_backw(0, 1);
            return popped;
        } else {
            Type popped = data[0]; // Resort to copying
            this->slide_backw(0, 1);
            return popped;
        }
    }
    /// @brief Remove a given amount of elements from the front of the vector
    /// @param slots The number of elements to remove
    /// @return The new size of the vector
    /// @throw `std::runtime_error` if the vector is empty or has been moved and `std::out_of_range`
    ///        if the given number of elements exceeds the vector's size, only if exceptions are not disabled
    inline constexpr std::size_t pop_front(const std::size_t slots) VECLIB_NOEXCEPT_DESTRUCTIBLE(Type) {
        this->empty_check("veclib::Vector.pop_front(): The vector is empty or has been moved");
        this->range_check(slots, "veclib::Vector.pop_front(): Can't pop this many slots");
        if (slots == 0) return count;
        this->slide_backw(0, slots);
        count -= slots;
        return count;
    }

    /// @brief Remove an element at a given index in the vector
    /// @param index The index into the vector
    /// @return The removed element
    /// @throw `std::runtime_error` if the vector is empty or has been moved and `std::out_of_range` if
    ///        the given index is out of bounds, only if exceptions are not disabled
    inline constexpr Type pop_at(const std::size_t index)
            requires (std::is_move_constructible_v<Type> || std::is_copy_constructible_v<Type>) {
        this->empty_check("veclib::Vector.pop_at(): The vector is empty or has been moved");
        this->bounds_check(index, "veclib::Vector.pop_at(): Index is out of bounds");
        if (index == count - 1) return this->pop_back();
        if constexpr (std::is_move_constructible_v<Type>) {
            Type popped = std::move(data[index]);
            this->slide_backw(index, 1);
            --count;
            return popped;
        } else {
            Type popped = data[index];
            this->slide_backw(index, 1); // Resort to copying
            --count;
            return popped;
        }
    }
    /// @brief Remove a given amount of elements at a given index in the vector
    /// @param index The index into the vector
    /// @param slots The number of elements to remove
    /// @return The new size of the vector
    /// @throw `std::runtime_error` if the vector is empty or has been moved and `std::out_of_range`
    ///        if the given index is out of bounds or the given number of elements exceeds the vector's size,
    ///        only if exceptions are not disabled
    inline constexpr std::size_t pop_at(const std::size_t index, const std::size_t slots) {
        this->empty_check("veclib::Vector.pop_at(): The vector is empty or has been moved");
        this->bounds_check(index, "veclib::Vector.pop_at(): Index is out of bounds");
        this->range_check(index + slots, "veclib::Vector.pop_at(): Can't pop this many slots");
        if (slots == 0) return count;
        this->slide_backw(index, slots);
        count -= slots;
        return count;
    }

    /// @brief Append the contents of another vector to this vector by copying them
    /// @param other The vector to append
    /// @return The number of newly added elements, equal to the size of the appended vector
    inline constexpr std::size_t append(const Self& other) requires (std::is_copy_constructible_v<Type>) {
        if (other.empty()) return count; // Easy path
        this->reserve(count + other.count);
        for (std::size_t i = 0; i < other.count; ++i)
            new(&data[i + count]) Type(other[i]); // Copy everything
        count += other.count;
        return other.count; // New size
    }
    /// @brief Append the contents of another vector to this vector by moving them
    /// @param other The vector to append
    /// @return The number of newly added elements, equal to the size of the appended vector
    inline constexpr std::size_t append(Self&& other) requires (std::is_move_constructible_v<Type>) {
        if (other.empty()) return count;
        this->reserve(count + other.count);
        for (std::size_t i = 0; i < other.count; ++i)
            new(&data[i + count]) Type(std::move(other[i]));
        count += other.count;
        std::size_t old_count = other.count;
        other.clear(); // We moved it now
        return old_count;
    }

    /// @brief Append the contents of an `Array` object by copying them
    /// @tparam Size The `Size` of the array
    /// @param array The array to append
    /// @return The number of newly added elements, equal to the size of the appended array
    template <std::size_t Size>
    inline constexpr std::size_t append(const Array<Type, Size>& array) requires (std::is_copy_constructible_v<Type>) {
        if (array.empty()) return count;
        this->reserve(count + Size);
        for (std::size_t i = 0; i < Size; ++i)
            new(&data[i + count]) Type(array[i]);
        count += Size;
        return Size;
    }
    /// @brief Append the contents of an `Array` object by moving them
    /// @tparam Size The `Size` of the array
    /// @param array The array to append
    /// @return The number of newly added elements, equal to the size of the appended array
    template <std::size_t Size>
    inline constexpr std::size_t append(Array<Type, Size>&& array) requires (std::is_move_constructible_v<Type>) {
        if (array.empty()) return count;
        this->reserve(count + Size);
        for (std::size_t i = 0; i < Size; ++i)
            new(&data[i + count]) Type(std::move(array[i]));
        count += Size;
        return Size;
    }

    /// @brief Compare two vectors for equality. If either vector is empty or the two vectors are of different sizes
    ///        they are automatically considered non-equal
    /// @param other The other vector
    inline constexpr bool operator==(const Self& other) const noexcept requires (std::equality_comparable<Type>) {
        // All dumb cases
        if (this->empty() || other.empty() || count != other.count) return false;
        for (std::size_t i = 0; i < count; ++i)
            if (data[i] != other[i]) return false;
        return true;
    }
    /// @brief Compare two vectors for inequality. If either vector is empty or the two vectors are of different sizes
    ///        they are automatically considered non-equal
    /// @param other The other vector
    inline constexpr bool operator!=(const Self& other) const noexcept requires (std::equality_comparable<Type>) {
        return !(*this == other);
    }
    /// @brief Compare this vector for equality against a scalar `Type` value
    /// @param value The value to compare
    inline constexpr bool operator==(const Type& value) const noexcept requires (std::equality_comparable<Type>) {
        if (this->empty()) return false;
        for (std::size_t i = 0; i < count; ++i)
            if (data[i] != value) return false;
        return true;
    }
    /// @brief Compare this vector for inequality against a scalar `Type` value
    /// @param value The value to compare
    inline constexpr bool operator!=(const Type& value) const noexcept requires (std::equality_comparable<Type>) {
        return !(*this == value);
    }
    /// @brief Implicitly cast this vector to check if it is empty or if it was cleared
    inline constexpr operator bool() const noexcept { return !this->empty() || !this->cleared(); }

    /// @brief It's an iterator what do you expect -_-
    inline constexpr Type* begin() noexcept { return data; }
    inline constexpr const Type* begin() const noexcept { return data; }
    /// @brief It's an iterator what do you expect -_-
    inline constexpr Type* end() noexcept { return data ? data + count : nullptr; }
    inline constexpr const Type* end() const noexcept { return data ? data + count : nullptr; }
};


} // namespace veclib

#endif // VECLIB_HPP
