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

/// @brief Expands to a conditional `noexcept` clause that evaluates to
///        `noexcept(true)` if a specified type's default constructor
///        is `noexcept`
#define VECLIB_NOEXCEPT_DEFAULT_CONSTRUCTIBLE(type) \
    noexcept(std::is_nothrow_default_constructible_v<type>)

/// @brief Expands to a conditional `noexcept` clause that evaluates to
///        `noexcept(true)` if a specified type's destructor is `noexcept`
#define VECLIB_NOEXCEPT_DESTRUCTIBLE(type) \
    noexcept(std::is_nothrow_destructible_v<type>)

/// @brief Expands to a conditional `noexcept` clause that evaluates to
///        `noexcept(true)` if a specified type's copy constructor is
///        `noexcept`
#define VECLIB_NOEXCEPT_COPY_CONSTRUCTIBLE(type) \
    noexcept(std::is_nothrow_copy_constructible_v<type>)
/// @brief Expands to a conditional `noexcept` clause that evaluates to
///        `noexcept(true)` if a specified type's copy assignment
///        operator is `noexcept`
#define VECLIB_NOEXCEPT_COPY_ASSIGNABLE(type) \
    noexcept(std::is_nothrow_copy_assignable_v<type>)

/// @brief Expands to a conditional `noexcept` clause that evaluates to
///        `noexcept(true)` if a specified type's move constructor is
///        `noexcept`
#define VECLIB_NOEXCEPT_MOVE_CONSTRUCTIBLE(type) \
    noexcept(std::is_nothrow_move_constructible_v<type>)
/// @brief Expands to a conditional `noexcept` clause that evaluates to
///        `noexcept(true)` if a specified type's move assignment
///        operator is `noexcept`
#define VECLIB_NOEXCEPT_MOVE_ASSIGNABLE(type) \
    noexcept(std::is_nothrow_move_assignable_v<type>)

/// @brief Expands to the minimum number of bytes required to fit the
///        provided number of bits
#define VECLIB_BYTESIZE(expr) (((expr) + (CHAR_BIT - 1) /* round up */) / CHAR_BIT)
/// @brief Expands to the number of bits contained in the provided
///        number of bytes
#define VECLIB_BITSIZE(expr) ((expr) * CHAR_BIT)

/// @brief Expands to a conditional `noexcept` clause into which is passed
///        an expression that calls a given function, evaluating to
///        `noexcept(false)` if it is not `noexcept` itself. This always
///        expands to `noexcept` if `VECLIB_ASSERT_NOEXCEPT` is defined
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

#define VECLIB_FN_NOEXCEPT(fn, ...) \
    noexcept(VECLIB_NOEXCEPT && VECLIB_COND_NOEXCEPT(fn, __VA_ARGS__))

/// @brief Type alias for the signed equivalent of `std::size_t`
using diff_t = std::make_signed_t<std::size_t>;

/// @brief Templated type alias for a function signature
/// @tparam Ret The return type of the function
/// @tparam ...Args The type of the arguments of the function
template <typename Ret, typename... Args>
using Function = Ret(*)(Args...);

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
public: // Public section for defining aliases in advance

    /// @brief Member type alias for accessing the referenced type
    using Item = Type;
    /// @brief Member type alias for the type of ourselves
    using Self = Slice<Type>;

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

    inline constexpr Self internal_subslice(std::size_t start, std::size_t end) const VECLIB_NOEXCEPT {
        this->nullptr_check("veclib::Slice.subslice(): The slice isn't referencing anything");
        this->overflow_check(start, "veclib::Slice.subslice(): Starting index exceeds slice size");
        this->overflow_check(end, "veclib::Slice.subslice(): Starting index exceeds slice size");
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

    #ifdef VECLIB_EXTRA

    /// @brief Returns `true` if `clear()` was called on the slice, or if the slice was moved
    inline constexpr bool cleared() const noexcept { return this->empty() && this->null(); }
    /// @brief Alias for `cleared()`
    inline constexpr bool moved() const noexcept { return this->cleared(); }

    #endif // VECLIB_EXTRA

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

    #ifdef VECLIB_EXTRA

    inline constexpr Type& circular_at(std::size_t i) noexcept { return data[i % count]; }
    inline constexpr const Type& circular_at(std::size_t i) const noexcept { return data[i % count]; }

    #ifndef VECLIB_NO_OPERATOR_OVERLOADS

    inline constexpr Type& operator()(std::size_t i) noexcept { return data[i % count]; }
    inline constexpr const Type& operator()(std::size_t i) const noexcept { return data[i % count]; }

    #endif // VECLIB_NO_OPERATOR_OVERLOADS

    inline constexpr Type& clamped_at(std::size_t i) noexcept { return data[i >= count ? count - 1 : i]; }
    inline constexpr const Type& clamped_at(std::size_t i) const noexcept { return data[i >= count ? count - 1 : i]; }

    #endif // VECLIB_EXTRA

    /// @brief Check if a given index is valid in this slice
    /// @param i The index to check
    /// @return `true` if the index is valid
    inline constexpr bool inside_bounds(std::size_t i) const noexcept { return i < count; }

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
            VECLIB_FN_NOEXCEPT(fn, std::declval<Type&>()) {
        this->empty_check("veclib::Slice.map(): Slice is empty");
        for (std::size_t i = 0; i < count; ++i)
            fn(data[i]);
        return *this;
    }
    inline constexpr Self& map(Function<void, Type&, std::size_t> fn)
            VECLIB_NOEXCEPT(fn, std::declval<Type&>(), 0) {
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
            const VECLIB_FN_NOEXCEPT(fn, std::declval<Type&>(), std::declval<const Type&>())
            requires (std::is_default_constructible_v<Type>) {
        this->empty_check("veclib::Slice.fold(): Slice is empty");
        for (std::size_t i = 0; i < count; ++i)
            fn(acc, data[i]);
        return acc;
    }
    inline constexpr Type fold(Function<void, Type&, const Type&, std::size_t> fn, const Type& acc = Type())
            const VECLIB_FN_NOEXCEPT(fn, std::declval<Type&>(), std::declval<const Type&>(), 0)
            requires (std::is_default_constructible_v<Type>) {
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
    inline constexpr bool all(Function<bool, const Type&> fn) const
            VECLIB_FN_NOEXCEPT(fn, std::declval<const Type&>()) {
        this->empty_check("veclib::Slice.all(): Slice is empty");
        for (std::size_t i = 0; i < count; ++i)
            if (!fn(data[i])) return false;
        return true;
    }
    inline constexpr bool all(Function<bool, const Type&, std::size_t> fn) const
            VECLIB_FN_NOEXCEPT(fn, std::declval<const Type&>(), 0) {
        this->empty_check("veclib::Slice.all(): Slice is empty");
        for (std::size_t i = 0; i < count; ++i)
            if (!fn(data[i], i)) return false;
        return true;
    }

    // There is no more `all_eq` because it was replaced by a scalar overload of `operator==`

    /// @brief Return `true` if at least an element in the slice passes a custom check function.
    ///        The check function must return `bool`, accept a `const Type&` parameter
    ///        and optionally an `std::size_t` parameter into which will be passed the
    ///        index into the slice of the element currently being processed
    /// @param fn The check function
    /// @throw `std::runtime_error` if the slice is empty and exceptions are not disabled
    inline constexpr bool any(Function<bool, const Type&> fn) const
            VECLIB_FN_NOEXCEPT(fn, std::declval<const Type&>()) {
        this->empty_check("veclib::Slice.any(): Slice is empty");
        for (std::size_t i = 0; i < count; ++i)
            if (fn(data[i])) return true;
        return false;
    }
    inline constexpr bool any(Function<bool, const Type&, std::size_t> fn) const
            VECLIB_FN_NOEXCEPT(fn, std::declval<const Type&>(), 0) {
        this->empty_check("veclib::Slice.any(): Slice is empty");
        for (std::size_t i = 0; i < count; ++i)
            if (fn(data[i], i)) return true;
        return false;
    }

    // There is no more `any_eq` because it was replaced by an overload of `contains`

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
    /// @brief Check if a provided value is contained in this slice
    /// @param value The value to search for
    /// @throw `std::runtime_error` if the slice isn't referencing anything
    inline constexpr bool contains(const Type& value) const VECLIB_NOEXCEPT
            requires (std::equality_comparable<Type>) {
        this->nullptr_check("veclib::Slice.contains(): The slice isn't referencing anything");
        if (count == 0) return false; // Systematically false
        for (std::size_t i = 0; i < count; ++i)
            if (data[i] != value) return false;
        return true;
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

    /// @brief Construct a slice referencing a portion of this slice's memory region.
    /// @param start The index into this slice at which the new slice should start.
    ///              Omitting this argument defaults it to `0`
    /// @param end The exclusive index into this slice at which the new slice should end.
    ///            Omitting this argument defaults it to this slice's size minus the
    ///            starting index, i.e. always reaching the end of the slice
    /// @throw `std::runtime_error` if this slice isn't referencing anything,
    ///        `std::overflow_error` if the starting or ending indeces exceed this slice's size.
    ///        only if exceptions are not disabled
    inline constexpr Self subslice(std::size_t start = 0) const VECLIB_NOEXCEPT {
        return this->internal_subslice(start, count - start); // Omit exclusive index and specify the maximum
    }
    inline constexpr Self subslice(std::size_t start, std::size_t end) const VECLIB_NOEXCEPT {
        return this->internal_subslice(start, end);
    }
    /// @brief Construct a slice referencing a portion of this slice's memory region
    /// @param offset The index into this slice at which the new slice should start.
    ///               Omitting this argument defaults it to `0`
    /// @param len The length of the new slice, if it exceeds the maximum size of this
    ///            slice, the value is capped and by default it equals `~0ULL`
    ///            (maximum value for `std::size_t`)
    /// @throw `std::runtime_error` if this slice isn't referencing anything,
    ///        `std::overflow_error` if the offset exceeds the memory region of this slice
    ///        or if the offset plus the length exceed it, only if exceptions are not disabled
    inline constexpr Self subslice_len(std::size_t start = 0, std::size_t len = ~0ULL) const VECLIB_NOEXCEPT {
        this->nullptr_check("veclib::Slice.subslice_len(): The slice isn't referencing anything");
        this->bounds_check(start, "veclib::Slice.subslice_len(): Starting index is out of bounds");
        if (start + len >= count) len = count - start; // Clamp the value
        return Slice<Type>(data + start, len);
    }

    /// @brief Keep trimming elements from the beginning of the slice while a custom
    ///        function returns `true`. The custom function must return `bool` and
    ///        must accept a `const Type&` parameter, into which will be passed the
    ///        next element to check for trimming
    /// @param fn The custom function
    /// @return A reference to the modified object
    inline constexpr Self& trim_while(Function<bool, const Type&> fn) VECLIB_COND_NOEXCEPT(fn, std::declval<const Type&>()) {
        while (count > 0 && fn(data)) this->consume_front();
        return *this;
    } // Should there be a copy variant?
    /// @brief Keep trimming elements while the first element is equal to a provided value
    /// @param value The value to check against for equality
    /// @return A reference to the modified object
    inline constexpr Self& trim_leading(const Type& value) noexcept
            requires (std::equality_comparable<Type>) {
        return this->trim_while([&value] (const Type& x) noexcept { return x == value; });
    } // And here too?
    /// @brief Keep trimming elements from the beginning of the slice while a custom
    ///        function returns `true`. The custom function must return `bool` and
    ///        must accept a `const Type&` parameter, into which will be passed the
    ///        next element to check for trimming
    /// @param fn The custom function
    /// @return A reference to the modified object
    inline constexpr Self& trim_until(Function<bool, const Type&> fn) VECLIB_COND_NOEXCEPT(fn, std::declval<const Type&>()) {
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


template <typename Type, std::size_t Size>
class Array {
private:

    Type data[Size] {};

    void bounds_check(std::size_t i, const char* str) const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(i < Size);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (i >= Size) throw std::out_of_range(str);
        #endif // VECLIB_ASSERT_NOEXCEPT
    }
    void overflow_check(std::size_t i, const char* str) const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(i <= Size);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (i > Size) throw std::overflow_error(str);
        #endif // VECLIB_ASSERT_NOEXCEPT
    }
    void underflow_check(std::size_t i, const char* str) const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(i <= Size);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (i > Size) throw std::underflow_error(str);
        #endif // VECLIB_ASSERT_NOEXCEPT
    }
    void nullptr_check(const char* str) const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data != nullptr);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error(str);
        #endif // VECLIB_ASSERT_NOEXCEPT
    }
    consteval void empty_check(const char* str) const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        static_assert(Size != 0);
        #else // VECLIB_ASSERT_NOEXCEPT
        if constexpr (Size == 0) throw std::runtime_error(str);
        #endif // VECLIB_ASSERT_NOEXCEPT
    }

    inline constexpr Slice<Type> internal_slice(std::size_t start, std::size_t end) const VECLIB_NOEXCEPT {
        this->nullptr_check("veclib::Array.slice(): The array was moved");
        this->overflow_check(start, "veclib::Array.slice(): Starting index exceeds array size");
        this->overflow_check(end, "veclib::Array.slice(): Ending index exceeds array size");
        return Slice<Type>(data + start, end - start);
    }

public:

    using Item = Type;
    using Self = Array<Type, Size>;

    Array() VECLIB_NOEXCEPT_DEFAULT_CONSTRUCTIBLE(Type) requires (std::is_default_constructible_v<Type>) = default;
    ~Array() VECLIB_NOEXCEPT_DESTRUCTIBLE(Type) requires (std::is_destructible_v<Type>) = default;

    Array(std::initializer_list<const Type&> args) VECLIB_NOEXCEPT
            requires (std::is_copy_assignable_v<Type>) {
        this->overflow_check(args.size(), "veclib::Array.Array(): Too many initializer list values");
        for (std::size_t i = 0; i < args.size(); ++i)
            data[i] = args.begin()[i];
    }
    inline constexpr Self& operator=(std::initializer_list<const Type&> args) VECLIB_NOEXCEPT
            requires (std::is_copy_assignable_v<Type>) {
        this->overflow_check(args.size(), "veclib::Array.Array(): Too many initializer list values");
        for (std::size_t i = 0; i < args.size(); ++i)
            data[i] = args.begin()[i];
        return *this;
    }

    Array(const Type& value) VECLIB_NOEXCEPT_COPY_ASSIGNABLE(Type)
            requires (std::is_copy_assignable_v<Type>) {
        for (std::size_t i = 0; i < Size; ++i)
            data[i] = value;
    }
    inline constexpr Self& operator=(const Type& value) VECLIB_NOEXCEPT_COPY_ASSIGNABLE(Type)
            requires (std::is_copy_assignable_v<Type>) {
        for (std::size_t i = 0; i < Size; ++i)
            data[i] = value;
        return *this;
    }

    Array(const Self& other) VECLIB_NOEXCEPT_COPY_ASSIGNABLE(Type)
            requires (std::is_copy_assignable_v<Type>) {
        for (std::size_t i = 0; i < Size; ++i)
            data[i] = other.data[i];
    }
    inline constexpr Self& operator=(const Self& other) VECLIB_NOEXCEPT_COPY_ASSIGNABLE(Type)
            requires (std::is_copy_assignable_v<Type>) {
        for (std::size_t i = 0; i < Size; ++i)
            data[i] = other.data[i];
        return *this;
    }

    Array(Self&& other) VECLIB_NOEXCEPT_MOVE_ASSIGNABLE(Type)
            requires (std::is_move_assignable_v<Type>) {
        for (std::size_t i = 0; i < Size; ++i)
            data[i] = std::move(other.data[i]);
        other.data = nullptr;
    }
    inline constexpr Self& operator=(Self&& other) VECLIB_NOEXCEPT_MOVE_ASSIGNABLE(Type)
            requires (std::is_move_assignable_v<Type>) {
        for (std::size_t i = 0; i < Size; ++i)
            data[i] = std::move(other.data[i]);
        other.data = nullptr;
        return *this;
    }

    inline constexpr const Type* get() const noexcept { return data; }
    inline consteval std::size_t size() const noexcept { return Size; }
    inline consteval bool empty() const noexcept { return Size == 0; }
    inline constexpr bool moved() const noexcept { return data == nullptr; }

    inline constexpr bool operator==(const Self& other) const VECLIB_NOEXCEPT
            requires (std::equality_comparable<Type>) {
        this->nullptr_check("veclib::Array.operator==(): The array was moved");
        for (std::size_t i = 0; i < Size; ++i)
            if (data[i] != other.data[i]) return false;
        return true;
    }
    inline constexpr bool operator==(const Type& value) const VECLIB_NOEXCEPT
            requires (std::equality_comparable<Type>) {
        this->nullptr_check("veclib::Array.operator==(): The array was moved");
        for (std::size_t i = 0; i < Size; ++i)
            if (data[i] != value) return false;
        return true;
    }
    inline constexpr bool operator!=(const Self& other) const VECLIB_NOEXCEPT
        requires (std::equality_comparable<Type>) { return !(*this == other); }
    inline constexpr bool operator!=(const Type& value) const VECLIB_NOEXCEPT
        requires (std::equality_comparable<Type>) { return !(*this == value); }
    inline constexpr operator bool() const noexcept { return data != nullptr; }

    inline constexpr Slice<Type> slice(std::size_t start = 0) const VECLIB_NOEXCEPT {
        return this->internal_slice(start, Size - start);
    }
    inline constexpr Slice<Type> slice(std::size_t start, std::size_t end) const VECLIB_NOEXCEPT {
        return this->internal_slice(start, end);
    }
    inline constexpr Slice<Type> slice_len(std::size_t start = 0, std::size_t len = ~0ULL) const VECLIB_NOEXCEPT {
        this->nullptr_check("veclib::Array.slice_len(): The array was moved");
        this->bounds_check(start, "veclib::Array.slice_len(): Starting index is out of bounds");
        if (start + len >= Size) len = Size - start; // Clamp the value
        return Slice<Type>(data + start, len);
    }

    inline constexpr Type& first() noexcept
        requires (Size != 0) { return data[0]; }
    inline constexpr const Type& first() const noexcept
        requires (Size != 0) { return data[0]; }

    inline constexpr Slice<Type> first(std::size_t x) noexcept
            requires (Size != 0) {
        this->overflow_check(x, "veclib::Array.first(): Number of first elements to extract exceeds array size");
        return Slice<Type>(data, x);
    }
    inline constexpr const Slice<Type> first(std::size_t x) const noexcept
            requires (Size != 0) {
        this->overflow_check(x, "veclib::Array.first(): Number of first elements to extract exceeds array size");
        return Slice<Type>(data, x);
    }

    inline constexpr Type& last() noexcept
        requires (Size != 0) { return data[Size - 1]; }
    inline constexpr const Type& last() const noexcept
        requires (Size != 0) { return data[Size - 1]; }

    inline constexpr Slice<Type> last(std::size_t x) noexcept
            requires (Size != 0) {
        this->underflow_check(x, "veclib::Array.last(): Number of last elements to extract exceeds array size");
        return Slice<Type>(data + Size - x, x);
    }
    inline constexpr const Slice<Type> last(std::size_t x) const noexcept
            requires (Size != 0) {
        this->underflow_check(x, "veclib::Array.last(): Number of last elements to extract exceeds array size");
        return Slice<Type>(data + Size - x, x);
    }

    inline constexpr Type& operator[](std::size_t i) noexcept { return data[i]; }
    inline constexpr const Type& operator[](std::size_t i) const noexcept { return data[i]; }

    inline constexpr Type& at(std::size_t i) VECLIB_NOEXCEPT {
        this->bounds_check(i, "veclib::Array.at(): Index is out of bounds");
        return data[i];
    }
    inline constexpr const Type& at(std::size_t i) const VECLIB_NOEXCEPT {
        this->bounds_check(i, "veclib::Array.at(): Index is out of bounds");
        return data[i];
    }

    #ifdef VECLIB_EXTRA

    inline constexpr Type& circular_at(std::size_t i) noexcept { return data[i % Size]; }
    inline constexpr const Type& circular_at(std::size_t i) const noexcept { return data[i % Size]; }

    #ifndef VECLIB_NO_OPERATOR_OVERLOADS

    inline constexpr Type& operator()(std::size_t i) noexcept { return data[i % Size]; }
    inline constexpr const Type& operator()(std::size_t i) const noexcept { return data[i % Size]; }

    #endif // VECLIB_NO_OPERATOR_OVERLOADS

    inline constexpr Type& clamped_at(std::size_t i) noexcept { return data[i >= Size ? Size - 1 : i]; }
    inline constexpr const Type& clamped_at(std::size_t i) const noexcept { return data[i >= Size ? Size - 1 : i]; }

    #endif // VECLIB_EXTRA

    inline constexpr bool inside_bounds(std::size_t i) const noexcept { return i < Size; }

    inline constexpr bool contains(Type* ptr) const noexcept { return (ptr >= data) && (ptr < data + Size); }
    inline constexpr bool contains(const Type& value) const VECLIB_NOEXCEPT
            requires (std::equality_comparable<Type>) {
        this->nullptr_check("veclib::Array.contains(): The array was moved");
        for (std::size_t i = 0; i < Size; ++i)
            if (data[i] == value) return true;
        return false;
    }

    inline constexpr Self& map(Function<void, Type&> fn) VECLIB_FN_NOEXCEPT(fn, std::declval<Type&>()) {
        // We don't require Size to be non-zero, since the loop will just never run
        this->nullptr_check("veclib::Array.map(): The array was moved");
        for (std::size_t i = 0; i < Size; ++i)
            fn(data[i]);
        return *this;
    }
    inline constexpr Self& map(Function<void, Type&, std::size_t> fn) VECLIB_FN_NOEXCEPT(fn, std::declval<Type&>(), 0) {
        this->nullptr_check("veclib::Array.map(): The array was moved");
        for (std::size_t i = 0; i < Size; ++i)
            fn(data[i], i);
        return *this;
    }

    inline constexpr Type fold(Function<void, Type&, const Type&> fn, Type acc = {})
            VECLIB_FN_NOEXCEPT(fn, std::declval<Type&>(), std::declval<const Type&>())
            requires (std::is_default_constructible_v<Type>) {
        this->nullptr_check("veclib::Array.fold(): The array was moved");
        for (std::size_t i = 0; i < Size; ++i)
            fn(data[i], acc);
        return acc;
    }
    inline constexpr Type fold(Function<void, Type&, const Type&, std::size_t> fn, Type acc = {})
            VECLIB_FN_NOEXCEPT(fn, std::declval<Type&>(), std::declval<const Type&>(), 0)
            requires (std::is_default_constructible_v<Type>) {
        this->nullptr_check("veclib::Array.fold(): The array was moved");
        for (std::size_t i = 0; i < Size; ++i)
            fn(data[i], acc, i);
        return acc;
    }

    #ifdef VECLIB_EXTRA

    inline constexpr bool all(Function<bool, const Type&> fn) const
            VECLIB_FN_NOEXCEPT(fn, std::declval<const Type&>()) {
        this->nullptr_check("veclib::Array.all(): The array was moved");
        for (std::size_t i = 0; i < Size; ++i)
            if (!fn(data[i])) return false;
        return true;
    }
    inline constexpr bool all(Function<bool, const Type&, std::size_t> fn) const
            VECLIB_FN_NOEXCEPT(fn, std::declval<const Type&>(), 0) {
        this->nullptr_check("veclib::Array.all(): The array was moved");
        for (std::size_t i = 0; i < Size; ++i)
            if (!fn(data[i], i)) return false;
        return true;
    }

    inline constexpr bool any(Function<bool, const Type&> fn) const
            VECLIB_FN_NOEXCEPT(fn, std::declval<const Type&>()) {
        this->nullptr_check("veclib::Array.any(): The array was moved");
        for (std::size_t i = 0; i < Size; ++i)
            if (fn(data[i])) return true;
        return false;
    }
    inline constexpr bool any(Function<bool, const Type&, std::size_t> fn) const
            VECLIB_FN_NOEXCEPT(fn, std::declval<const Type&>(), 0) {
        this->nullptr_check("veclib::Array.any(): The array was moved");
        for (std::size_t i = 0; i < Size; ++i)
            if (fn(data[i], i)) return true;
        return false;
    }

    #endif // VECLIB_EXTRA
};


template <std::size_t Size>
class Bitset;

template <std::size_t Size>
class BitsetItr;

template <std::size_t Size>
class BitProxy {
private:

    Bitset<Size>& parent;
    std::size_t idx = 0; // This makes it the size of a regular pointer

public:

    using Self = BitProxy<Size>;

    BitProxy() noexcept = delete; // No default constructor due to reference
    ~BitProxy() noexcept = default;

    BitProxy(Bitset<Size>& p, std::size_t i) noexcept
        : parent(p), idx(i) {}

    inline constexpr operator bool() const noexcept {
        return parent.get_at(idx);
    }

    inline constexpr Self& operator=(bool value) noexcept {
        parent.set_at(idx, value);
        return *this;
    }

    inline constexpr std::size_t index() const noexcept { return idx; }

    inline constexpr BitsetItr<Size> operator&() const noexcept {
        return BitsetItr<Size>(parent, idx); // Fake pointer-like object
    }
};

template <std::size_t Size>
class BitsetItr {
private:

    Bitset<Size>& parent;
    std::size_t index = 0;

public:

    using Self = BitsetItr<Size>;

    BitsetItr() noexcept = delete; // No default constructor due to reference
    ~BitsetItr() noexcept = default;

    BitsetItr(Bitset<Size>& p, std::size_t i) noexcept
        : parent(p), index(i) {}

    
};

template <std::size_t Size>
class Bitset {

friend BitProxy<Size>;
friend BitsetItr<Size>;

private:

    std::uint8_t data[VECLIB_BYTESIZE(Size)] = {};

    inline constexpr bool get_at(std::size_t i) const noexcept {
        return (data[i / CHAR_BIT] & (1 << (i % CHAR_BIT))) != 0;
    }
    inline constexpr void set_at(std::size_t i, bool value) noexcept {
        if (value) data[i / CHAR_BIT] |= 1 << (i % CHAR_BIT);
        else data[i / CHAR_BIT] &= ~(1 << (i % CHAR_BIT));
    }

    inline constexpr void bounds_check(std::size_t i, const char* str) const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(i < Size);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (i >= Size) throw std::out_of_range(str);
        #endif // VECLIB_ASSERT_NOEXCEPT
    }

public:

    using Item = BitProxy<Size>;
    using Self = Bitset<Size>;

    Bitset() noexcept = default;
    ~Bitset() noexcept = default;

    Bitset(const Array<std::uint8_t, VECLIB_BYTESIZE(Size)>& array) noexcept {
        for (std::size_t i = 0; i < VECLIB_BYTESIZE(Size); ++i)
            data[i] = array[i];
    }
    inline constexpr Self& operator=(const Array<std::uint8_t, VECLIB_BYTESIZE(Size)>& array) noexcept {
        for (std::size_t i = 0; i < VECLIB_BYTESIZE(Size); ++i)
            data[i] = array[i];
        return *this;
    }

    // We have no get() method since we aren't really holding any typed data
    //inline constexpr const std::uint8_t* get() const noexcept { return data; }

    inline consteval std::size_t size() const noexcept { return Size; }
    inline consteval std::size_t bytesize() const noexcept { return VECLIB_BYTESIZE(Size); }

    inline constexpr Item operator[](std::size_t i) noexcept { return Item(*this, i); }
    inline constexpr const Item operator[](std::size_t i) const noexcept { return Item(*this, i); }

    inline constexpr Item at(std::size_t i) VECLIB_NOEXCEPT {
        this->bounds_check(i, "veclib::Bitset.at(): Index is out of bounds");
        return Item(*this, i);
    }
    inline constexpr const Item at(std::size_t i) const VECLIB_NOEXCEPT {
        this->bounds_check(i, "veclib::Bitset.at(): Index is out of bounds");
        return Item(*this, i);
    }

    #ifdef VECLIB_EXTRA

    inline constexpr Item circular_at(std::size_t i) noexcept { return Item(*this, i % Size); }
    inline constexpr const Item circular_at(std::size_t i) const noexcept { return Item(*this, i % Size); }

    #ifndef VECLIB_NO_OPERATOR_OVERLOADS

    inline constexpr Item operator()(std::size_t i) noexcept { return Item(*this, i % Size); }
    inline constexpr const Item operator()(std::size_t i) const noexcept { return Item(*this, i % Size); }

    #endif // VECLIB_NO_OPERATOR_OVERLOADS

    #endif // VECLIB_EXTRA

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

    inline constexpr Self& map(Function<void, BitProxy<Size>> fn)
            VECLIB_FN_NOEXCEPT(fn, std::declval<BitProxy<Size>>()) {
        for (std::size_t i = 0; i < Size; ++i)
            fn(BitProxy<Size>(*this, i));
        return *this;
    }
    inline constexpr Self& map(Function<void, BitProxy<Size>, std::size_t> fn)
            VECLIB_FN_NOEXCEPT(fn, std::declval<BitProxy<Size>>(), 0) {
        for (std::size_t i = 0; i < Size; ++i)
            fn(BitProxy<Size>(*this, i), i);
        return *this;
    }

    inline constexpr bool fold(Function<void, bool&, const bool> fn, bool acc = false) const
            VECLIB_FN_NOEXCEPT(fn, std::declval<bool&>(), false) {
        for (std::size_t i = 0; i < Size; ++i)
            fn(acc, this->get_at(i));
        return *this;
    }
    inline constexpr bool fold(Function<void, bool&, const bool, std::size_t> fn, bool acc = false) const
            VECLIB_FN_NOEXCEPT(fn, std::declval<bool&>(), false, 0) {
        for (std::size_t i = 0; i < Size; ++i)
            fn(acc, this->get_at(i), i);
        return *this;
    }

    inline constexpr bool on() const noexcept {
        std::size_t count = 0;
        for (std::size_t i = 0; i < Size; ++i)
            if (this->get_at(i)) ++count;
        return count;
    }

    inline constexpr bool off() const noexcept {
        std::size_t count = 0;
        for (std::size_t i = 0; i < Size; ++i)
            if (!this->get_at(i)) ++count;
        return count;
    }

    inline constexpr bool all() const noexcept {
        for (std::size_t i = 0; i < Size; ++i)
            if (!this->get_at(i)) return false;
        return true;
    }

    inline constexpr bool none() const noexcept {
        for (std::size_t i = 0; i < Size; ++i)
            if (this->get_at(i)) return false;
        return true;
    }

    inline constexpr bool any() const noexcept {
        for (std::size_t i = 0; i < Size; ++i)
            if (this->get_at(i)) return true;
        return false;
    }

    inline constexpr operator bool() const noexcept { return this->any(); }

    inline constexpr bool operator==(const Self& other) const noexcept {
        for (std::size_t i = 0; i < VECLIB_BYTESIZE(Size); ++i)
            if (data[i] != other.data[i]) return false;
        return true;
    }
    inline constexpr bool operator!=(const Self& other) const noexcept { return !(*this == other); }

    #ifndef VECLIB_NO_OPERATOR_OVERLOADS

    inline constexpr Self& operator&=(const Self& other) noexcept {
        for (std::size_t i = 0; i < VECLIB_BYTESIZE(Size); ++i)
            data[i] &= other.data[i];
        return *this;
    }

    inline constexpr Self& operator|=(const Self& other) noexcept {
        for (std::size_t i = 0; i < VECLIB_BYTESIZE(Size); ++i)
            data[i] |= other.data[i];
        return *this;
    }

    inline constexpr Self& operator^=(const Self& other) noexcept {
        for (std::size_t i = 0; i < VECLIB_BYTESIZE(Size); ++i)
            data[i] ^= other.data[i];
        return *this;
    }

    inline constexpr Self operator&(const Self& other) const noexcept {
        Self self = *this;
        for (std::size_t i = 0; i < VECLIB_BYTESIZE(Size); ++i)
            self.data[i] &= other.data[i];
        return self;
    }

    inline constexpr Self operator|(const Self& other) const noexcept {
        Self self = *this;
        for (std::size_t i = 0; i < VECLIB_BYTESIZE(Size); ++i)
            self.data[i] |= other.data[i];
        return self;
    }

    inline constexpr Self operator^(const Self& other) const noexcept {
        Self self = *this;
        for (std::size_t i = 0; i < VECLIB_BYTESIZE(Size); ++i)
            self.data[i] ^= other.data[i];
        return self;
    }

    inline constexpr Self operator~() const noexcept {
        Self self;
        for (std::size_t i = 0; i < VECLIB_BYTESIZE(Size); ++i)
            self.data[i] = ~data[i];
        return self;
    }

    #endif // VECLIB_NO_OPERATOR_OVERLOADS
};


} // namespace veclib

#endif // VECLIB_HPP
