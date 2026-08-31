#ifndef VECLIB_HPP
#define VECLIB_HPP

#include <cstdint>
#include <cstring>
#ifdef VECLIB_ASSERT_NOEXCEPT // Use asserts instead of throwing exceptions
#include <cassert>
#else // VECLIB_ASSERT_NOEXCEPT
#include <stdexcept>
#endif // VECLIB_ASSERT_NOEXCEPT
#include <initializer_list>
#include <utility> // std::move(), std::forward()
#include <concepts> // std::integral, std::equality_comparable

// Guard against already defined macros
#if defined(VECLIB_NONCONSTRUCTOR_NEW) \
 || defined(VECLIB_NONDESTRUCTOR_DELETE)
#error "veclib can't be compiled if internal macros are already defined"
#endif // VECLIB_*

/// @brief A collection of array-like data structures (arrays, vectors, slices, lists...)
namespace veclib {


/// @brief Alias for the signed equivalent of `std::size_t`
using diff_t = std::make_signed_t<std::size_t>;

template <typename Ret, typename... Args>
using TransformFn = Ret(*)(Args...);

template <typename Type>
class Explicit {
private:
    Type data {};

public:
    Explicit() = delete;
    ~Explicit() = default;

    explicit Explicit(const Type& other)
        : data(other) {}
    explicit Explicit(Type&& other) noexcept
        : data(std::move(other)) {}

    inline constexpr operator Type() const noexcept { return data; }
};

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


// The following macros can be used inside veclib to change the behaviour of the API:
// VECLIB_ASSERT_NOEXCEPT           Use asserts instead of exceptions when checking runtime cases like null pointers
// VECLIB_NO_OPERATOR_OVERLOADS     Avoid overloading unnecessary operators for most types defined by veclib
// VECLIB_USE_INDEXED_ITERATORS     Use the `IndexedIterator` and `ReverseIndexedIterator` types as the default iterator type
//                                  All types that may require distinction between the two iterator types implement
//                                  iterator functions prefixed with `p_` for the pointer-based version and `i_` for the
//                                  index-based version
//                                  The regular non-prefixed versions are defined according to the state of this macro
//                                  (defined or not). Note that reverse iterators are not available for the index-based
//                                  methods, even though a type is defined for them. This is because the ending iterators
//                                  can't have an index that is less than 0, therefore no reverse methods are implemented.
//                                  This doesn't mean that a `ReverseIndexedIterator` can't be used: we can construct one
//                                  out of an `IndexedIterator`, to function like a regular reverse iterator, but without
//                                  any way of identifying an ending state


/// @brief A reverse iterator for contiguous memory regions
/// @tparam Type The type being iterated over
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
    /// @throws `std::runtime_error` if the underlying data
    ///         pointer is `nullptr
    inline constexpr ReverseMemIterator<Type>& operator++() {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr)
            throw std::runtime_error("ReverseMemIterator<Type>::operator++(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        --data;
        return *this; // Does this throw if data is nullptr?
    }
    /// @brief Postfix-increment this iterator 
    /// @return A copy of the iterator before being incremented
    inline constexpr ReverseMemIterator<Type> operator++(int) {
        ReverseMemIterator<Type> self = *this;
        --data;
        return self;
    }

    /// @brief Prefix-decrement this iterator
    /// @return A reference to this iterator object once modified
    /// @throws `std::runtime_error` if the underlying data
    ///         pointer is `nullptr
    inline constexpr ReverseMemIterator<Type>& operator--() {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr)
            throw std::runtime_error("ReverseMemIterator<Type>::operator++(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        ++data;
        return *this; // Does this throw if data is nullptr?
    }
    /// @brief Postfix-increment this iterator 
    /// @return A copy of the iterator before being incremented
    inline constexpr ReverseMemIterator<Type> operator--(int) {
        ReverseMemIterator<Type> self = *this;
        ++data;
        return self;
    }

    /// @brief Increment this iterator
    /// @param x The number of items of the type being pointed
    ///          to to increment this iterator by
    /// @return A reference to the incremented iterator
    /// @throws `std::runtime_error` if the underlying data
    ///         pointer is `nullptr
    inline constexpr ReverseMemIterator<Type>& operator+=(std::size_t x) {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr)
            throw std::runtime_error("ReverseMemIterator<Type>::operator++(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        data -= x;
        return *this;
    }

    /// @brief Decrement this iterator
    /// @param x The number of items of the type being pointed
    ///          to to decrement this iterator by
    /// @return A reference to the decremented iterator
    /// @throws `std::runtime_error` if the underlying data
    ///         pointer is `nullptr
    inline constexpr ReverseMemIterator<Type>& operator-=(std::size_t x) {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr)
            throw std::runtime_error("ReverseMemIterator<Type>::operator++(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        data += x;
        return *this;
    }

    /// @brief Add a number of items of the type being pointed
    ///        to to this iterator
    /// @param x The number of items to add
    /// @return A copy of this iterator pointing to the new location
    ///         provided by `x`
    /// @throws `std::runtime_error` if the underlying data
    ///         pointer is `nullptr
    inline constexpr ReverseMemIterator<Type> operator+(std::size_t x) const noexcept {
        ReverseMemIterator<Type> self = *this;
        self.data -= x;
        return self;
    }

    /// @brief Subtract a number of items of the type being pointed
    ///        to to this iterator
    /// @param x The number of items to subtract
    /// @return A copy of this iterator pointing to the new location
    ///         provided by `x`
    inline constexpr ReverseMemIterator<Type> operator-(std::size_t x) const noexcept {
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
concept IndexerType = requires (Type index, std::size_t i) {
    ++std::declval<Type&>(); // Incrementable
    --std::declval<Type&>(); // Decrementable
    (std::declval<Type&>() += i) || (std::declval<Type&>() += std::declval<Type&>()); // Assignment-incrementable
    (std::declval<Type&>() -= i) || (std::declval<Type&>() -= std::declval<Type&>()); // Assignment-decrementable
    std::declval<Type>() == std::declval<Type>(); // Equality comparable
};

template <typename Type, typename Item, typename Index>
concept IndexableViaOperator = requires () {
    requires (IndexerType<Index>);
    //Type::operator[](index) -> item;
    { std::declval<Type>()[std::declval<Index>()] } -> std::same_as<std::remove_const_t<Item&>>; // Works with const and non-const
};
template <typename Type, typename Item, typename Index>
concept IndexableViaAt = requires () {
    requires (IndexerType<Index>);
    //Type::at(index) -> item;
    { std::declval<Type>().at(std::declval<Index>()) } -> std::same_as<std::remove_const_t<Item&>>; // Works with const and non-const
};

/// @brief Helper concept for types that can be indexed by other types that
///        satisfy the requirements of the `IndexerType` concept
/// @tparam Type A type that provides an `at()` method taking a type
///         constrained by `IndexerType` as a parameter, or an overload of
///         `operator[]` that takes the same type as a parameter
/// @tparam Index A type used for indexing into `Type` that satisfies the
///         requirements of `IndexerType`. It is used for the member function
///         parameter constraints on `Type`
template <typename Type, typename Item, typename Index = std::size_t>
concept Indexable = requires () {
    requires (IndexerType<Index>);
    requires (IndexableViaOperator<Type, Item, Index>) || (IndexableViaAt<Type, Item, Index>);
};

/// @brief Compile-time enum used for choosing if to index elements with an
///        overload of `operator[]` or an `at()` method
enum class PreferredIndexing : std::uint8_t {
    Operator, AtMethod
};

/// @brief This kind of iterator aims to avoid iterator invalidation
///        caused by memory reallocations, for example in vectors,
///        by keeping a pointer to an container and an index into
///        that container
/// @tparam Container The type of the container, which must satisfy the
///         requirements of the `Indexable` concept
/// @tparam Item The type held inside the container, it defaults to a
///         type definition inside the specified container
/// @tparam IndexType The type used to index into the container, it defaults
///         to `std::size_t`, any type specified as a parameter in this field
///         must satisfy the requirements of the `IndexerType` concept
/// @tparam Pref The preferred indexing method, whether it is an overload of
///         `operator[]` or an `at()` method defined by `Container`. This is taken
///         into account only if `Container` provides both member functions
template <typename Container, typename Item = Container::Item, IndexerType IndexType = std::size_t, PreferredIndexing Pref = PreferredIndexing::Operator>
requires (Indexable<Container, Item, IndexType>) // This is the only way to add `Item` and `IndexType` as a template parameter for `Indexable`
class IndexedIterator {
private:
    Container* obj = nullptr;
    IndexType index = 0;

    /// @brief Utility type alias for not repeating ourselves
    using Self = IndexedIterator<Container, Item, IndexType>;

    inline constexpr Item& deref() requires (IndexableViaOperator<Container, Item, IndexType>) {
        return (*obj)[index]; // Do not care about nullptr since we do that at the public API level
    }
    inline constexpr Item& deref() requires (IndexableViaAt<Container, Item, IndexType>) {
        return obj->at(index);
    }
    inline constexpr Item& deref() requires (Indexable<Container, Item, IndexType>) {
        if constexpr (Pref == PreferredIndexing::Operator)
            return (*obj)[index];
        else // Pref == PreferredIndexing::AtMethod
            return obj->at(index);
    }

public:
    /// @brief Default constructor
    IndexedIterator() noexcept = default;
    /// @brief Default destructor
    ~IndexedIterator() noexcept = default;

    /// @brief Construct an `IndexedIterator` object with a pointer to
    ///        a `Container` and an index into it
    /// @param o The pointer to the container
    /// @param i The index into the container
    IndexedIterator(Container* o, const IndexType& i) noexcept
        : obj(o), index(i) {}

    /// @brief Receive a const pointer to the referenced item
    /// @return A const pointer to the item referenced by this iterator
    inline constexpr const Item* get() const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(obj != nullptr);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (obj == nullptr)
            throw std::runtime_error("IndexedIterator<Container, Item, IndexType>.get(): Container pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return &deref();
    }
    /// @brief Receive a const pointer to the referenced `Container` object
    /// @return A const pointer to the internally referenced container
    inline constexpr const Container* get_container() const noexcept { return obj; }

    // No noexcept in every increment operator because custom overloads could throw

    inline constexpr Self& operator++() {
        ++index;
        return *this;
    }
    inline constexpr Self operator++(int) {
        Self self = *this;
        ++(*this);
        return self;
    }
    inline constexpr Self& operator--() {
        --index;
        return *this;
    }
    inline constexpr Self operator--(int) {
        Self self = *this;
        --(*this);
        return self;
    }

    inline constexpr Self& operator+=(const IndexType& x) requires (!std::same_as<IndexType, std::size_t>) {
        // Maybe `same_as` could become `convertible_to`
        index += x;
        return *this;
    }
    inline constexpr Self& operator+=(std::size_t x) {
        index += x;
        return *this;
    }
    inline constexpr Self& operator-=(const IndexType& x) requires (!std::same_as<IndexType, std::size_t>) {
        index -= x;
        return *this;
    }
    inline constexpr Self& operator-=(std::size_t x) {
        index -= x;
        return *this;
    }

    inline constexpr Self operator+(const IndexType& x) requires (!std::same_as<IndexType, std::size_t>) {
        Self self = *this;
        self += x;
        return self;
    }
    inline constexpr Self operator+(std::size_t x) {
        Self self = *this;
        self += x;
        return self;
    }
    inline constexpr Self operator-(const IndexType& x) requires (!std::same_as<IndexType, std::size_t>) {
        Self self = *this;
        self -= x;
        return self;
    }
    inline constexpr Self operator-(std::size_t x) {
        Self self = *this;
        self -= x;
        return self;
    }

    inline constexpr Item& operator*() const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(obj != nullptr);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (obj == nullptr)
            throw std::runtime_error("IndexedIterator<Container, Item, IndexType::operator*(): Container pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return deref(); // We required that a `Container` object can be indexed by `index` and the returned value is of type `Item`
    }

    inline constexpr bool operator==(const Self& other) const noexcept {
        return obj == other.obj && index == other.index; // We required that the type of `index` is equality comparable
    }
    inline constexpr bool operator!=(const Self& other) const noexcept {
        return !(*this == other);
    }

    inline constexpr operator bool() const noexcept {
        return obj != nullptr;
    }
};

/// @brief The reverse counterpart of `IndexedIterator`.
///        This kind of iterator aims to avoid iterator invalidation
///        caused by memory reallocations, for example in vectors,
///        by keeping a pointer to an container and an index into
///        that container
/// @tparam Container The type of the container, which must satisfy the
///         requirements of the `Indexable` concept
/// @tparam Item The type held inside the container, it defaults to a
///         type definition inside the specified container
/// @tparam IndexType The type used to index into the container, it defaults
///         to `std::size_t`, any type specified as a parameter in this field
///         must satisfy the requirements of the `IndexerType` concept
/// @tparam Pref The preferred indexing method, whether it is an overload of
///         `operator[]` or an `at()` method defined by `Container`. This is taken
///         into account only if `Container` provides both member functions
template <typename Container, typename Item = Container::Item, IndexerType IndexType = std::size_t, PreferredIndexing Pref = PreferredIndexing::Operator>
requires (Indexable<Container, Item, IndexType>) // This is the only way to add `Item` and `IndexType` as a template parameter for `Indexable`
class ReverseIndexedIterator {
private:
    IndexedIterator<Container, Item, IndexType, Pref> itr = IndexedIterator<Container, Item, IndexType, Pref>();

    using Self = ReverseIndexedIterator<Container, Item, IndexType, Pref>;

public:
    /// @brief Default constructor
    ReverseIndexedIterator() noexcept = default;
    /// @brief Default destructor
    ~ReverseIndexedIterator() noexcept = default;

    /// @brief Construct a `ReverseIndexedIterator` object with a pointer to
    ///        a `Container` and an index into it
    /// @param o The pointer to the container
    /// @param i The index into the container
    ReverseIndexedIterator(Container* o, const IndexType& i) noexcept
        : itr(o, i) {}

    ReverseIndexedIterator(IndexedIterator<Container, Item, IndexType, Pref> i /*copy is not expensive*/) noexcept
        : itr(i) {}

    /// @brief Receive a const pointer to the referenced item
    /// @return A const pointer to the item referenced by this iterator
    inline constexpr const Item* get() const { return itr.get(); }
    /// @brief Receive a const pointer to the referenced `Container` object
    /// @return A const pointer to the internally referenced container
    inline constexpr const Container* get_container() const noexcept { return itr.get_container(); }

    inline constexpr Self& operator++() noexcept {
        --itr;
        return *this;
    }
    inline constexpr Self operator++(int) noexcept {
        Self self = *this;
        --(*this);
        return self;
    }
    inline constexpr Self& operator--() noexcept {
        ++itr;
        return *this;
    }
    inline constexpr Self operator--(int) noexcept {
        Self self = *this;
        ++(*this);
        return self;
    }

    inline constexpr Self& operator+=(const IndexType& x) requires (!std::same_as<IndexType, std::size_t>) {
        itr -= x;
        return *this;
    }
    inline constexpr Self& operator+=(std::size_t x) {
        itr -= x;
        return *this;
    }
    inline constexpr Self& operator-=(const IndexType& x) requires (!std::same_as<IndexType, std::size_t>) {
        itr += x;
        return *this;
    }
    inline constexpr Self& operator-=(std::size_t x) {
        itr += x;
        return *this;
    }

    inline constexpr Self operator+(const IndexType& x) requires (!std::same_as<IndexType, std::size_t>) {
        Self self = *this;
        self += x; // This should not be changed since it is already the reverse behaviour
        return self;
    }
    inline constexpr Self operator+(std::size_t x) {
        Self self = *this;
        self += x;
        return self;
    }
    inline constexpr Self operator-(const IndexType& x) requires (!std::same_as<IndexType, std::size_t>) {
        Self self = *this;
        self -= x;
        return self;
    }
    inline constexpr Self operator-(std::size_t x) {
        Self self = *this;
        self -= x;
        return self;
    }

    inline constexpr Item& operator*() const {
        return *itr;
    }

    inline constexpr bool operator==(const Self& other) const noexcept {
        return itr == other.itr;
    }
    inline constexpr bool operator!=(const Self& other) const noexcept {
        return !(*this == other);
    }

    inline constexpr operator bool() const noexcept {
        return (bool)itr; // Explicit cast
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

    /// @brief Make the template parameter accessible to iterators
    using Item = Type;

    /// @brief Default constructor
    MemSlice() noexcept = default;
    /// @brief Default destructor
    ~MemSlice() noexcept = default;

    /// @brief Construct a `MemSlice` with a pointer to a region
    ///        of memory and the number of elements the slice will reference
    /// @param p The pointer to memory to reference
    /// @param s The number of elements to reference
    MemSlice(Type* p, std::size_t s) noexcept
        : data(p), count(s) {}

    /// @brief Construct a `MemSlice` object by copying another one
    ///        that references a type convertible to the type of this slice
    /// @tparam Other The type of the slice to copy
    /// @param other The slice to copy
    template <typename Other>
    MemSlice(const MemSlice<Other>& other) noexcept requires std::convertible_to<Other*, Type*>
        : data(other.data), count(other.count) {}
    /// @brief Construct a `MemSlice` object by copying another one
    ///        that references a type convertible to the type of this slice
    /// @tparam Other The type of the slice to copy
    /// @param other The slice to copy
    /// @return A reference to the constructed object
    template <typename Other>
    inline constexpr MemSlice<Type>& operator=(const MemSlice<Other>& other) noexcept
            requires std::convertible_to<Other*, Type*> {
        data = other.data;
        count = other.count;
        return *this;
    }

    /// @brief Receive a read-only pointer to the data being referenced
    /// @return A pointer to the memory being referenced
    inline constexpr const Type* get() const noexcept { return data; }
    /// @brief Evaluates to the number of elements currently being
    ///        referenced by the slice
    /// @return The number of referenced elements
    inline constexpr std::size_t size() const noexcept { return count; }

    /// @brief Index into the referenced range with no bounds checking
    /// @param i The index into the range
    /// @return A reference to the indexed element
    inline constexpr Type& operator[](std::size_t i) noexcept { return data[i]; }
    /// @brief Index into the referenced range with no bounds checking
    /// @param i The index into the range
    /// @return A const reference to the indexed element
    inline constexpr const Type& operator[](std::size_t i) const noexcept { return data[i]; }

    /// @brief Index into the referenced range with bounds checking
    /// @param i The index into the range
    /// @return A reference to the indexed element
    inline constexpr Type& at(std::size_t i) {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(i < count);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (i >= count) throw std::out_of_range("MemSlice<Type>.at(std::size_t): Index is out of bounds");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return data[i];
    }
    /// @brief Index into the referenced range with bounds checking
    /// @param i The index into the range
    /// @return A const reference to the indexed element
    inline constexpr const Type& at(std::size_t i) const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(i < count);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (i >= count) throw std::out_of_range("MemSlice<Type>.at(std::size_t): Index is out of bounds");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return data[i];
    }

    inline constexpr Type* map(TransformFn<void, Type&> fn) noexcept {
        // Do nullptr checks later
        for (std::size_t i = 0; i < count; ++i)
            fn(data[i]);
        return data;
    }
    inline constexpr Type* map(TransformFn<void, Type&, std::size_t> fn) noexcept {
        // Do nullptr checks later
        for (std::size_t i = 0; i < count; ++i)
            fn(data[i], i);
        return data;
    }

    inline constexpr Type fold(TransformFn<void, Type&, const Type&> fn) const noexcept {
        // Do nullptr checks later
        // We should also require default constructible on Type
        Type acc {};
        for (std::size_t i = 0; i < count; ++i)
            fn(acc, data[i]);
        return acc;
    }
    inline constexpr Type fold(TransformFn<void, Type&, const Type&, std::size_t> fn) const noexcept {
        // Do nullptr checks later
        // We should also require default constructible on Type
        Type acc {};
        for (std::size_t i = 0; i < count; ++i)
            fn(acc, data[i], i);
        return acc;
    }

    // slice+=   change size            grow()              } -----+
    // slice+    new changed size       grow_copy()        } ----+ |---- resize() => universal and signed
    // slice-=   change size            shrink()            } ---|-+
    // slice-    new changed size       shrink_copy()      } ----+------- resize_copy() => universal and signed

    // slice<<=  change ptr             slide_backw()        } -----+
    // slice<<   new changed ptr        slide_backw_copy()  } ----+ |---- slide() => universal and signed
    // slice>>=  change ptr             slide_forw()         } ---|-+
    // slice>>   new changed ptr        slide_forw_copy()   } ----+------- slide_copy() => universal and signed

    // slice/=   move head forw         trim()              } -----+
    // slice/    new forw moved head    trim_copy()        } ----+ |--- nudge() => universal and signed
    // slice|=   move head backw        extend()            } ---|-+
    // slice|    new backw moved head   extend_copy()      } ----+------ nudge_copy() => universal and signed

    // optional:
    // ++slice   move head by 1         consume_front()
    // +slice    new moved head by 1    consume_front_copy()
    // --slice   move tail by -1        consume_back()
    // -slice    new moved tail by -1   consume_back_copy()

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
        assert(x < 0 ? static_cast<std::size_t>(-x) <= count : true);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (x < 0 && static_cast<std::size_t>(-x) > count)
            throw std::underflow_error("MemSlice<Type>.resize(diff_t): Shrinking by too much");
        #endif // VECLIB_ASSERT_NOEXCEPT
        count += x;
        return *this;
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

    /// @brief Move the head of the slice forward while keeping the last element still
    /// @param x The number of elements to move the head by
    /// @return A reference to the modified slice
    /// @throws `std::runtime_error` if the underlying data pointer is `nullptr`,
    ///         if exceptions are not disabled by defining `VECLIB_ASSERT_NOEXCEPT`,
    ///         otherwise an assert will fail, `std::overflow_error` if `x` is greater
    ///         than the size of the slice
    inline constexpr MemSlice<Type>& trim(std::size_t x) {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        assert(x <= count);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr)
            throw std::runtime_error("MemSlice<Type>.trim(std::size_t): Data pointer is nullptr");
        if (x > count) throw std::overflow_error("MemSlice<Type>.trim(std::size_t): Can't trim past slice size");
        #endif // VECLIB_ASSERT_NOEXCEPT
        data += x;
        count -= x;
        return *this;
    }
    /// @brief Move the head of the slice forward while keeping the last element still
    /// @param x The number of elements to move the head by
    /// @return A copy of this slice on which the operation was performed
    inline constexpr MemSlice<Type> trim_copy(std::size_t x) const {
        MemSlice<Type> self = *this;
        self.trim(x);
        return self;
    }
    /// @brief Move the head of the slice backward while keeping the last element still
    /// @param x The number of elements to move the head by
    /// @return A reference to the modified slice
    inline constexpr MemSlice<Type>& extend(std::size_t x) {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr)
            throw std::runtime_error("MemSlice<Type>.extend(std::size_t): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        data -= x;
        count += x;
        return *this;
    }
    /// @brief Move the head of the slice backward while keeping the last element still
    /// @param x The number of elements to move the head by
    /// @return A copy of the slice on which the operation was performed
    inline constexpr MemSlice<Type> extend_copy(std::size_t x) const noexcept {
        MemSlice<Type> self = *this;
        self.extend(x);
        return self;
    }
    /// @brief Move the head of the slice arbitrarily while keeping the last element still
    /// @param x The number of elements to move the head by
    /// @return A reference to the modified slice
    inline constexpr MemSlice<Type>& nudge(diff_t x) {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        assert(x >= 0 ? static_cast<std::size_t>(x) <= count : true);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr)
            throw std::runtime_error("MemSlice<Type>.extend(std::size_t): Data pointer is nullptr");
        if (x >= 0 && static_cast<std::size_t>(x) > count)
            throw std::overflow_error("MemSlice<Type>.nudge(std::size_t): Can't nudge past slice size");
        #endif // VECLIB_ASSERT_NOEXCEPT
        data += x;
        count -= x;
        return *this;
    }
    /// @brief Move the head of the slice arbitrarily while keeping the last element still
    /// @param x The number of elements to move the head by
    /// @return A copy of the slice on which the operation was performed
    inline constexpr MemSlice<Type> nudge_copy(diff_t x) const {
        MemSlice<Type> self = *this;
        self.nudge(x);
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
    inline constexpr MemSlice<Type> consume_front_copy() const {
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
    inline constexpr MemSlice<Type> consume_back_copy() const {
        MemSlice<Type> self = *this;
        self.consume_back();
        return self;
    }

    #ifndef VECLIB_NO_OPERATOR_OVERLOADS // We create extra operator overloads unless it's not requested

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
    inline constexpr MemSlice<Type> operator/(std::size_t x) const { return trim_copy(x); }
    /// @brief Operator overload for calling `extend(x)`
    inline constexpr MemSlice<Type>& operator|=(std::size_t x) noexcept { return extend(x); }
    /// @brief Operator overload for calling `extend_copy(x)`
    inline constexpr MemSlice<Type> operator|(std::size_t x) const noexcept { return extend_copy(x); }

    /// @brief Operator overload for calling `consume_front()`
    inline constexpr MemSlice<Type>& operator++() { return consume_front(); }
    /// @brief Operator overload for calling `consume_front_copy()`
    inline constexpr MemSlice<Type> operator+() const { return consume_front_copy(); }
    /// @brief Operator overload for calling `consume_back()`
    inline constexpr MemSlice<Type>& operator--() { return consume_back(); }
    /// @brief Operator overload for calling `consume_back_copy()`
    inline constexpr MemSlice<Type> operator-() const { return consume_back_copy(); }

    #endif // VECLIB_NO_OPERATOR_OVERLOADS

    /// @brief Receive a forward iterator to the first element
    /// @return A pointer to the first element
    inline constexpr const Type* begin() {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("MemSlice<Type>.begin(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return data;
    }
    /// @brief Receive a const forward iterator to the first element
    /// @return A const pointer to the first element
    inline constexpr const Type* begin() const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("MemSlice<Type>.begin(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return data;
    }

    /// @brief Receive a forward iterator past the last element
    /// @return A pointer pointing past the last element
    inline constexpr const Type* end() {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        //assert(count != 0);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("MemSlice<Type>.end(): Data pointer is nullptr");
        //if (count == 0) throw std::logic_error("MemSlice<Type>.end(): The size of the slice is 0");
        #endif // VECLIB_ASSERT_NOEXCEPT
        // Instead of throwing or asserting, we just short-circuit and make loops never run
        return count != 0 ? data + count : data;
    }
    /// @brief Receive a const forward iterator past the last element
    /// @return A const pointer pointing past the last element
    inline constexpr const Type* end() const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("MemSlice<Type>.end(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return count != 0 ? data + count : data;
    }

    /// @brief Receive a reverse iterator to the first element
    /// @return A reverse iterator object pointing to the first element
    inline constexpr ReverseMemIterator<const Type> rbegin() {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("MemSlice<Type>.rbegin(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return ReverseMemIterator<Type>(count != 0 ? data + count - 1 : data);
    }
    /// @brief Receive a const reverse iterator to the first element
    /// @return A const reverse iterator object pointing to the first element
    inline constexpr const ReverseMemIterator<const Type> rbegin() const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("MemSlice<Type>.rbegin(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return ReverseMemIterator<Type>(count != 0 ? data + count - 1 : data);
    }

    /// @brief Receive a reverse iterator past the last element
    /// @return A reverse iterator object pointing past the last element
    inline constexpr ReverseMemIterator<const Type> rend() {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("MemSlice<Type>.rend(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return ReverseMemIterator<Type>(count != 0 ? data - 1 : data);
    }
    /// @brief Receive a const reverse iterator past the last element
    /// @return A const reverse iterator object pointing past the last element
    inline constexpr const ReverseMemIterator<const Type> rend() const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("MemSlice<Type>.rend(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return ReverseMemIterator<Type>(count != 0 ? data - 1 : data);
    }

    /// @brief Receive a const forward iterator to the first element
    /// @return A const pointer to the first element
    inline constexpr const Type* cbegin() const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("MemSlice<Type>.cbegin(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return data;
    }

    /// @brief Receive a const forward iterator past the last element
    /// @return A const pointer pointing past the last element
    inline constexpr const Type* cend() const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("MemSlice<Type>.cend(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return data + count;
    }

    /// @brief Receive a const reverse iterator to the first element
    /// @return A const reverse iterator object pointing to the first element
    inline constexpr const ReverseMemIterator<const Type> crbegin() const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("MemSlice<Type>.crbegin(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return ReverseMemIterator<Type>(data + count - 1);
    }

    /// @brief Receive a const reverse iterator past the last element
    /// @return A const reverse iterator object pointing past the last element
    inline constexpr const ReverseMemIterator<const Type> crend() const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("MemSlice<Type>.crend(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return ReverseMemIterator<Type>(data - 1);
    }
};

/// @brief Shorter alias for `MemSlice`
template <typename Type>
using Slice = MemSlice<Type>;

/// @brief Static array class
/// @tparam Type The type of each element in the array
/// @tparam Size The size of the array
template <typename Type, std::size_t Size>
class Array {
private:
    Type data[Size] = {};

public:

    /// @brief Make the template parameter accessible to iterators
    using Item = Type;

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
    Array<Type, Size>& operator=(Array<Type, Size>&& other) noexcept {
        for (std::size_t i = 0; i < Size; ++i) {
            data[i] = std::move(other.data[i]);
            //other.data[i].~Type();
        }
        other.data = nullptr;
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
        assert(data);
        assert(end >= start);
        assert(start < Size);
        assert(end <= Size);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error(
            "Array<Type, Size>.slice(std::size_t, std::size_t): Data pointer is nullptr");
        if (end < start) throw std::out_of_range(
            "Array<Type, Size>.slice(std::size_t, std::size_t): End index is smaller than start index");
        if (start >= Size) throw std::out_of_range(
            "Array<Type, Size>.slice(std::size_t, std::size_t): Start index is out of bounds");
        if (end > Size) throw std::out_of_range(
            "Array<Type, Size>.slice(std::size_t, std::size_t): End index is out of bounds");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return MemSlice<Type>(data + start, end - start);
    }
    /// @brief Construct a `MemSlice` object out of this array, optionally
    ///        specifying start and end indeces (end indeces are exclusive)
    /// @return A `MemSlice` object referencing the whole array or part of it
    inline constexpr MemSlice<Type> slice(std::size_t start = 0, std::size_t end = Size)const  {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        assert(end >= start);
        assert(start < Size);
        assert(end <= Size);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error(
            "Array<Type, Size>.slice(std::size_t, std::size_t): Data pointer is nullptr");
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

    #ifndef VECLIB_NO_OPERATOR_OVERLOADS // We create extra operator overloads unless it's not requested

    /// @brief Index the array circularly, preventing any out-of-bounds
    ///        access and wrapping around once the maximum index is reached
    /// @param i The index into the array
    /// @return A reference to the element at the specified index
    inline constexpr Type& operator()(std::size_t i) noexcept { return data[i % Size]; }
    /// @brief Index the array circularly, preventing any out-of-bounds
    ///        access and wrapping around once the maximum index is reached
    /// @param i The index into the array
    /// @return A const reference to the element at the specified index
    inline constexpr const Type& operator()(std::size_t i) const noexcept { return data[i % Size]; }

    #endif // VECLIB_NO_OPERATOR_OVERLOADS

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
    /// @brief Index the array circularly, preventing any out-of-bounds
    ///        access and wrapping around once the maximum index is reached
    /// @param i The index into the array
    /// @return A reference to the element at the specified index
    inline constexpr Type& circular_at(std::size_t i) noexcept {
        return data[i % Size]; // Never goes out of bounds
    }
    /// @brief Index the array circularly, preventing any out-of-bounds
    ///        access and wrapping around once the maximum index is reached
    /// @param i The index into the array
    /// @return A const reference to the element at the specified index
    inline constexpr const Type& circular_at(std::size_t i) const noexcept {
        return data[i % Size]; // Never goes out of bounds
    }

    inline constexpr bool contains(const Type& x) const requires std::equality_comparable<Type> {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data != nullptr);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error(
            "Array<Type, Size>.contains(const Type&): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        for (std::size_t i < 0; i < Size; ++i)
            if (data[i] == x) return true;
        return false;
    }

    inline constexpr Type* map(TransformFn<void, Type&> fn) {
        // No noexcept because we don't know what the lambdas could do
        // and we have this check too
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data != nullptr);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error(
            "Array<Type, Size>.map(TransformFn<void, Type&>): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        for (std::size_t i = 0; i < Size; ++i)
            fn(data[i]);
        return data;
    }

    inline constexpr Type* map(TransformFn<void, Type&, std::size_t> fn) {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data != nullptr);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error(
            "Array<Type, Size>.map(TransformFn<void, Type&, std::size_t>): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        for (std::size_t i = 0; i < Size; ++i)
            fn(data[i], i);
        return data;
    }

    inline constexpr Type fold(TransformFn<void, Type&, const Type&> fn) const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data != nullptr);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error(
            "Array<Type, Size>.fold(TransformFn<void, Type&, const Type&>): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        // We need a default constructor,
        // maybe add a concept constraint to Type for this later if needed
        Type acc {};
        for (std::size_t i < 0; i < Size; ++i)
            fn(acc, data[i]);
        return acc;
    }

    inline constexpr Type fold(TransformFn<void, Type&, const Type&, std::size_t> fn) const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data != nullptr);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error(
            "Array<Type, Size>.fold(TransformFn<void, Type&, const Type&, std::size_t>): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        // We need a default constructor,
        // maybe add a concept constraint to Type for this later if needed
        Type acc {};
        for (std::size_t i < 0; i < Size; ++i)
            fn(acc, data[i], i);
        return acc;
    }

    /// @brief Receive a forward iterator to the first element of the array
    /// @return A pointer to the first element
    inline constexpr Type* begin() {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("Array<Type, Size>.begin(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return data;
    }
    /// @brief Receive a const forward iterator to the first element of the array
    /// @return A const pointer to the first element
    inline constexpr const Type* begin() const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("Array<Type, Size>.begin(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return data;
    }

    /// @brief Receive a forward iterator past the last element of the array
    /// @return A pointer pointing past the last element
    inline constexpr Type* end() {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("Array<Type, Size>.end(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return &data[Size];
    }
    /// @brief Receive a const forward iterator past the last element of the array
    /// @return A const pointer pointing past the last element
    inline constexpr const Type* end() const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("Array<Type, Size>.end(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return &data[Size];
    }

    /// @brief Receive a reverse iterator to the first element of the array
    /// @return A reverse iterator object pointing to the first element
    inline constexpr ReverseMemIterator<Type> rbegin() requires (Size != 0) {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("Array<Type, Size>.rbegin(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return ReverseMemIterator<Type>(&data[Size - 1]);
    }
    /// @brief Receive a const reverse iterator to the first element of the array
    /// @return A const reverse iterator object pointing to the first element
    inline constexpr const ReverseMemIterator<Type> rbegin() const requires (Size != 0) {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("Array<Type, Size>.rbegin(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return ReverseMemIterator<Type>(&data[Size - 1]);
    }

    /// @brief Receive a reverse iterator past the last element of the array
    /// @return A reverse iterator object pointing past the last element
    inline constexpr ReverseMemIterator<Type> rend() requires (Size != 0) {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("Array<Type, Size>.rend(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return ReverseMemIterator<Type>(data - 1);
    }
    /// @brief Receive a const reverse iterator past the last element of the array
    /// @return A const reverse iterator object pointing past the last element
    inline constexpr const ReverseMemIterator<Type> rend() const requires (Size != 0) {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("Array<Type, Size>.rend(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return ReverseMemIterator<Type>(data - 1);
    }

    /// @brief Receive a const forward iterator to the first element of the array
    /// @return A const pointer to the first element
    inline constexpr const Type* cbegin() const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("Array<Type, Size>.cbegin(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return data;
    }

    /// @brief Receive a const forward iterator past the last element of the array
    /// @return A const pointer pointing past the last element
    inline constexpr const Type* cend() const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("Array<Type, Size>.cend(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return &data[Size];
    }

    /// @brief Receive a const reverse iterator to the first element of the array
    /// @return A const reverse iterator object pointing to the first element
    inline constexpr const ReverseMemIterator<Type> crbegin() const requires (Size != 0) {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("Array<Type, Size>.crbegin(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return ReverseMemIterator<Type>(&data[Size - 1]);
    }

    /// @brief Receive a const reverse iterator past the last element of the array
    /// @return A const reverse iterator object pointing past the last element
    inline constexpr const ReverseMemIterator<Type> crend() const requires (Size != 0) {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("Array<Type, Size>.crend(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return ReverseMemIterator<Type>(data - 1);
    }

    #ifndef VECLIB_NO_OPERATOR_OVERLOADS // We create extra operator overloads unless it's not requested

    // Arithmetic operations for integral types

    inline constexpr Array<Type, Size>& operator++() noexcept requires std::is_arithmetic_v<Type> {
        for (std::size_t i = 0; i < Size; ++i)
            ++data[i];
        return *this;
    }
    inline constexpr Array<Type, Size>& operator++(int) noexcept requires std::is_arithmetic_v<Type> {
        Array<Type, Size> self = *this;
        for (std::size_t i = 0; i < Size; ++i)
            ++data[i];
        return self;
    }
    inline constexpr Array<Type, Size>& operator--() noexcept requires std::is_arithmetic_v<Type> {
        for (std::size_t i = 0; i < Size; ++i)
            --data[i];
        return *this;
    }
    inline constexpr Array<Type, Size>& operator--(int) noexcept requires std::is_arithmetic_v<Type> {
        Array<Type, Size> self = *this;
        for (std::size_t i = 0; i < Size; ++i)
            --data[i];
        return self;
    }

    inline constexpr Array<Type, Size>& operator+=(const Array<Type, Size>& other)
            noexcept requires std::is_arithmetic_v<Type> {
        for (std::size_t i = 0; i < Size; ++i)
            data[i] += other.data[i];
        return *this;
    }
    inline constexpr Array<Type, Size>& operator+=(const Type x)
            noexcept requires std::is_arithmetic_v<Type> {
        for (std::size_t i = 0; i < Size; ++i)
            data[i] += x;
        return *this;
    }
    inline constexpr Array<Type, Size>& operator-=(const Array<Type, Size>& other)
            noexcept requires std::is_arithmetic_v<Type> {
        for (std::size_t i = 0; i < Size; ++i)
            data[i] -= other.data[i];
        return *this;
    }
    inline constexpr Array<Type, Size>& operator-=(const Type x)
            noexcept requires std::is_arithmetic_v<Type> {
        for (std::size_t i = 0; i < Size; ++i)
            data[i] -= x;
        return *this;
    }
    inline constexpr Array<Type, Size>& operator*=(const Array<Type, Size>& other)
            noexcept requires std::is_arithmetic_v<Type> {
        for (std::size_t i = 0; i < Size; ++i)
            data[i] *= other.data[i];
        return *this;
    }
    inline constexpr Array<Type, Size>& operator*=(const Type x)
            noexcept requires std::is_arithmetic_v<Type> {
        for (std::size_t i = 0; i < Size; ++i)
            data[i] *= x;
        return *this;
    }
    inline constexpr Array<Type, Size>& operator/=(const Array<Type, Size>& other)
            noexcept requires std::is_arithmetic_v<Type> {
        for (std::size_t i = 0; i < Size; ++i)
            data[i] /= other.data[i];
        return *this;
    }
    inline constexpr Array<Type, Size>& operator/=(const Type x)
            noexcept requires std::is_arithmetic_v<Type> {
        for (std::size_t i = 0; i < Size; ++i)
            data[i] /= x;
        return *this;
    }
    inline constexpr Array<Type, Size>& operator%=(const Array<Type, Size>& other)
            noexcept requires std::is_arithmetic_v<Type> {
        for (std::size_t i = 0; i < Size; ++i)
            data[i] %= other.data[i];
        return *this;
    }
    inline constexpr Array<Type, Size>& operator%=(const Type x)
            noexcept requires std::is_arithmetic_v<Type> {
        for (std::size_t i = 0; i < Size; ++i)
            data[i] %= x;
        return *this;
    }

    inline constexpr Array<Type, Size> operator+(const Array<Type, Size>& other)
            const noexcept requires std::is_arithmetic_v<Type> {
        Array<Type, Size> output = *this;
        for (std::size_t i = 0; i < Size; ++i)
            output[i] += other.data[i];
        return output;
    }
    inline constexpr Array<Type, Size> operator+(const Type x)
            const noexcept requires std::is_arithmetic_v<Type> {
        Array<Type, Size> output = *this;
        for (std::size_t i = 0; i < Size; ++i)
            output[i] += x;
        return output;
    }
    inline constexpr Array<Type, Size> operator-(const Array<Type, Size>& other)
            const noexcept requires std::is_arithmetic_v<Type> {
        Array<Type, Size> output = *this;
        for (std::size_t i = 0; i < Size; ++i)
            output[i] -= other.data[i];
        return output;
    }
    inline constexpr Array<Type, Size> operator-(const Type x)
            const noexcept requires std::is_arithmetic_v<Type> {
        Array<Type, Size> output = *this;
        for (std::size_t i = 0; i < Size; ++i)
            output[i] -= x;
        return output;
    }
    inline constexpr Array<Type, Size> operator*(const Array<Type, Size>& other)
            const noexcept requires std::is_arithmetic_v<Type> {
        Array<Type, Size> output = *this;
        for (std::size_t i = 0; i < Size; ++i)
            output[i] *= other.data[i];
        return output;
    }
    inline constexpr Array<Type, Size> operator*(const Type x)
            const noexcept requires std::is_arithmetic_v<Type> {
        Array<Type, Size> output = *this;
        for (std::size_t i = 0; i < Size; ++i)
            output[i] *= x;
        return output;
    }
    inline constexpr Array<Type, Size> operator/(const Array<Type, Size>& other)
            const noexcept requires std::is_arithmetic_v<Type> {
        Array<Type, Size> output = *this;
        for (std::size_t i = 0; i < Size; ++i)
            output[i] /= other.data[i];
        return output;
    }
    inline constexpr Array<Type, Size> operator/(const Type x)
            const noexcept requires std::is_arithmetic_v<Type> {
        Array<Type, Size> output = *this;
        for (std::size_t i = 0; i < Size; ++i)
            output[i] /= x;
        return output;
    }
    inline constexpr Array<Type, Size> operator%(const Array<Type, Size>& other)
            const noexcept requires std::is_arithmetic_v<Type> {
        Array<Type, Size> output = *this;
        for (std::size_t i = 0; i < Size; ++i)
            output[i] %= other.data[i];
        return output;
    }
    inline constexpr Array<Type, Size> operator%(const Type x)
            const noexcept requires std::is_arithmetic_v<Type> {
        Array<Type, Size> output = *this;
        for (std::size_t i = 0; i < Size; ++i)
            output[i] %= x;
        return output;
    }

    inline constexpr bool operator==(const Array<Type, Size>& other)
            const noexcept requires std::equality_comparable<Type> {
        for (std::size_t i = 0; i < Size; ++i)
            if (data[i] != other.data[i]) return false;
        return true;
    }
    inline constexpr bool operator==(const Type& x)
            const noexcept requires std::equality_comparable<Type> {
        for (std::size_t i = 0; i < Size; ++i)
            if (data[i] != x) return false;
        return true;
    }
    inline constexpr bool operator!=(const Array<Type, Size>& other)
            const noexcept requires std::equality_comparable<Type> {
        return !(*this == other);
    }
    inline constexpr bool operator!=(const Type& x)
            const noexcept requires std::equality_comparable<Type> {
        return !(*this == x);
    }

    #endif // VECLIB_NO_OPERATOR_OVERLOADS
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

    //using IdxForwIterator = IndexedIterator<Vector<Type, Grow>, Type, std::size_t, PreferredIndexing::Operator>;
    //using ConstIdxForwIterator = IndexedIterator<Vector<Type, Grow>, const Type, std::size_t, PreferredIndexing::Operator>;

    /// @brief Reallocate the vector, doesn't update `count`, only `cap`
    /// @param new_count The new `count` value for the vector
    /// @param update_cap `true` by default if `cap` needs to be updated
    ///                   according to `new_count`
    /// @return The new `count` value without setting it
    std::size_t realloc(std::size_t new_count, bool update_cap = true) {
        Type* old = data;
        std::size_t old_cap = cap;
        std::size_t old_count = count;

        if (update_cap) {
            if constexpr (Grow == GrowType::OneAndHalf) // Update capacity accordingly
                cap = new_count + new_count / 2;
            else // Grow == GrowType::DoubleSize
                cap = new_count * 2;
        } else cap = new_count; // We just make them equal

        Type* new_data = VECLIB_NONCONSTRUCTOR_NEW(cap, Type);
        std::size_t to_copy = old ? /*min*/ (old_count < new_count ? old_count : new_count) : 0;
        for (std::size_t i = 0; i < to_copy; ++i)
            new(&new_data[i]) Type(std::move(old[i]));

        if (old) {
            for (std::size_t i = 0; i < old_count; ++i)
                old[i].~Type();
            VECLIB_NONDESTRUCTOR_DELETE(old, old_cap, Type);
        }

        data = new_data;
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

    /// @brief Make the template parameter accessible to iterators
    using Item = Type;

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

    Vector(const std::initializer_list<Type>& args) {
        count = realloc(args.size());
        for (std::size_t i = 0; i < count; ++i)
            new(&data[i]) Type(args.begin()[i]); // Use placement new and `Type`'s copy constructor
    }
    inline constexpr Vector<Type, Grow>& operator=(const std::initializer_list<Type>& args) {
        clear();
        count = realloc(args.size());
        for (std::size_t i = 0; i < count; ++i)
            new(&data[i]) Type(args.begin()[i]);
        return *this;
    }

    Vector(const Vector<Type, Grow>& other) : data(nullptr), count(0), cap(0) {
        if (other.empty()) return;
        cap = other.cap;
        data = VECLIB_NONCONSTRUCTOR_NEW(cap, Type);
        for (std::size_t i = 0; i < other.count; ++i)
            new(&data[i]) Type(other.data[i]);
        count = other.count;
    }
    inline constexpr Vector<Type, Grow>& operator=(const Vector<Type, Grow>& other) {
        if (this == &other) return *this;
        clear();
        if (other.empty()) return *this;
        cap = other.cap;
        data = VECLIB_NONCONSTRUCTOR_NEW(cap, Type);
        for (std::size_t i = 0; i < other.count; ++i)
            new(&data[i]) Type(other.data[i]);
        count = other.count;
        return *this;
    }

    Vector(Vector<Type, Grow>&& other) noexcept
            : data(other.data), count(other.count), cap(other.cap) {
        other.count = 0;
        other.cap = 0;
        other.data = nullptr;
    }
    inline constexpr Vector<Type, Grow>& operator=(Vector<Type, Grow>&& other) noexcept {
        if (this == &other) return *this;
        clear();
        count = other.count;
        cap = other.cap;
        data = other.data;
        other.count = 0;
        other.cap = 0;
        other.data = nullptr;
        return *this;
    }

    inline constexpr const Type* get() const noexcept { return data; }
    inline constexpr std::size_t size() const noexcept { return count; }
    inline constexpr std::size_t capacity() const noexcept { return cap; }
    inline constexpr bool empty() const noexcept { return count == 0; }

    inline constexpr void clear() noexcept {
        if (!data) return;
        for (std::size_t i = 0; i < count; ++i)
            data[i].~Type();
        VECLIB_NONDESTRUCTOR_DELETE(data, cap, Type);
        data = nullptr;
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

    inline constexpr std::size_t shrink_to_fit() {
        std::size_t cut = cap - count;
        if (cut == 0) return 0; // This also covers data being nullptr indirectly
        VECLIB_NONDESTRUCTOR_DELETE(data + count, cut, Type);
        return cut
    }

    inline constexpr void swap(Vector<Type, Grow>& other) {
        Vector<Type, Grow> self = std::move(*this); // That one trick -_-
        *this = std::move(other);
        other = std::move(self);
    }

    inline constexpr std::size_t remove(Type* itr) { // TODO: make an index-based version
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data <= itr && itr <= data + count);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (itr < data || data + count <= itr)
            throw std::out_of_range("Vector<Type, Grow>.remove(Type*): Pointer is out of range");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return pop_at(itr - data); // Reuse code
    }

    /// @brief Construct a `MemSlice` object referencing the contents of this vector,
    ///        optionally specifying the starting index and the ending index (which
    ///        is exclusive)
    /// @param start The starting index
    /// @param end The ending index
    /// @return A `MemSlice` object referencing the specified range
    inline constexpr MemSlice<Type> slice(std::size_t start, std::size_t end) {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        assert(end >= start);
        assert(start < count);
        assert(end <= count);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error(
            "Vector<Type, Grow>.slice(std::size_t, std::size_t): Data pointer is nullptr");
        if (end < start) throw std::out_of_range(
            "Vector<Type, Grow>.slice(std::size_t, std::size_t): End index is smaller than start index");
        if (start >= count) throw std::out_of_range(
            "Vector<Type, Grow>.slice(std::size_t, std::size_t): Start index is out of bounds");
        if (end > count) throw std::out_of_range(
            "Vector<Type, Grow>.slice(std::size_t, std::size_t): End index is out of bounds");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return MemSlice<Type>(data + start, end - start);
    }
    /// @brief Construct a `MemSlice` object referencing the contents of this vector,
    ///        optionally specifying the starting index and the ending index (which
    ///        is exclusive)
    /// @param start The starting index
    /// @param end The ending index
    /// @return A `MemSlice` object referencing the specified range
    inline constexpr MemSlice<Type> slice(std::size_t start = 0) {
        // Use this workaround since we need a compile-time-known number for default arguments
        std::size_t end = count;
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        assert(end >= start);
        assert(start < count);
        assert(end <= count);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error(
            "Vector<Type, Grow>.slice(std::size_t, std::size_t): Data pointer is nullptr");
        if (end < start) throw std::out_of_range(
            "Vector<Type, Grow>.slice(std::size_t, std::size_t): End index is smaller than start index");
        if (start >= count) throw std::out_of_range(
            "Vector<Type, Grow>.slice(std::size_t, std::size_t): Start index is out of bounds");
        if (end > count) throw std::out_of_range(
            "Vector<Type, Grow>.slice(std::size_t, std::size_t): End index is out of bounds");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return MemSlice<Type>(data + start, end - start);
    }
    /// @brief Construct a `MemSlice` object referencing the contents of this vector,
    ///        optionally specifying the starting index and the ending index (which
    ///        is exclusive)
    /// @param start The starting index
    /// @param end The ending index
    /// @return A `MemSlice` object referencing the specified range
    inline constexpr MemSlice<Type> slice(std::size_t start, std::size_t end) const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        assert(end >= start);
        assert(start < count);
        assert(end <= count);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error(
            "Vector<Type, Grow>.slice(std::size_t, std::size_t): Data pointer is nullptr");
        if (end < start) throw std::out_of_range(
            "Vector<Type, Grow>.slice(std::size_t, std::size_t): End index is smaller than start index");
        if (start >= count) throw std::out_of_range(
            "Vector<Type, Grow>.slice(std::size_t, std::size_t): Start index is out of bounds");
        if (end > count) throw std::out_of_range(
            "Vector<Type, Grow>.slice(std::size_t, std::size_t): End index is out of bounds");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return MemSlice<Type>(data + start, end - start);
    }
    /// @brief Construct a `MemSlice` object referencing the contents of this vector,
    ///        optionally specifying the starting index and the ending index (which
    ///        is exclusive)
    /// @param start The starting index
    /// @param end The ending index
    /// @return A `MemSlice` object referencing the specified range
    inline constexpr MemSlice<Type> slice(std::size_t start = 0) const {
        // Use this workaround since we need a compile-time-known number for default arguments
        std::size_t end = count;
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        assert(end >= start);
        assert(start < count);
        assert(end <= count);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error(
            "Vector<Type, Grow>.slice(std::size_t, std::size_t): Data pointer is nullptr");
        if (end < start) throw std::out_of_range(
            "Vector<Type, Grow>.slice(std::size_t, std::size_t): End index is smaller than start index");
        if (start >= count) throw std::out_of_range(
            "Vector<Type, Grow>.slice(std::size_t, std::size_t): Start index is out of bounds");
        if (end > count) throw std::out_of_range(
            "Vector<Type, Grow>.slice(std::size_t, std::size_t): End index is out of bounds");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return MemSlice<Type>(data + start, end - start);
    }

    inline constexpr Type& first() noexcept { return data[0]; }
    inline constexpr const Type& first() const noexcept { return data[0]; }

    inline constexpr Type& last() noexcept { return data[count - 1]; }
    inline constexpr const Type& last() const noexcept { return data[count - 1]; }

    inline constexpr Type& operator[](std::size_t i) noexcept { return data[i]; }
    inline constexpr const Type& operator[](std::size_t i) const noexcept { return data[i]; }

    #ifndef VECLIB_NO_OPERATOR_OVERLOADS // We create extra operator overloads unless it's not requested

    /// @brief Index the array circularly, preventing any out-of-bounds
    ///        access and wrapping around once the maximum index is reached
    /// @param i The index into the array
    /// @return A reference to the element at the specified index
    inline constexpr Type& operator()(std::size_t i) noexcept { return data[i % count]; }
    /// @brief Index the array circularly, preventing any out-of-bounds
    ///        access and wrapping around once the maximum index is reached
    /// @param i The index into the array
    /// @return A const reference to the element at the specified index
    inline constexpr const Type& operator()(std::size_t i) const noexcept { return data[i % count]; }

    #endif // VECLIB_NO_OPERATOR_OVERLOADS

    inline constexpr Type& at(std::size_t i) {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(i < count && data != nullptr);
        #else // VECLIB_ASSERT_NOEXCEPT
        // We also cover the case in which data is nullptr since when it's cleared count is set to 0
        if (i >= count) throw std::out_of_range("Vector<Type, Grow>.at(std::size_t): Index is out of bounds");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return data[i];
    }
    inline constexpr const Type& at(std::size_t i) const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(i < count && data != nullptr);
        #else // VECLIB_ASSERT_NOEXCEPT
        // We also cover the case in which data is nullptr since when it's cleared count is set to 0
        if (i >= count) throw std::out_of_range("Vector<Type, Grow>.at(std::size_t): Index is out of bounds");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return data[i];
    }
    /// @brief Index the array circularly, preventing any out-of-bounds
    ///        access and wrapping around once the maximum index is reached
    /// @param i The index into the array
    /// @return A reference to the element at the specified index
    inline constexpr Type& circular_at(std::size_t i) noexcept {
        return data[i % count]; // Never goes out of bounds
    }
    /// @brief Index the array circularly, preventing any out-of-bounds
    ///        access and wrapping around once the maximum index is reached
    /// @param i The index into the array
    /// @return A const reference to the element at the specified index
    inline constexpr const Type& circular_at(std::size_t i) const noexcept {
        return data[i % count]; // Never goes out of bounds
    }

    inline constexpr bool contains(const Type& x) const requires std::equality_comparable<Type> {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data != nullptr);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error(
            "Vector<Type, Grow>.contains(const Type&): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        for (std::size_t i < 0; i < count; ++i)
            if (data[i] == x) return true;
        return false;
    }

    inline constexpr Type* map(TransformFn<void, Type&> fn) {
        // No noexcept because we don't know what the lambdas could do
        // and we have this check too
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data != nullptr);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error(
            "Vector<Type, Grow>.map(TransformFn<void, Type&>): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        for (std::size_t i = 0; i < Size; ++i)
            fn(data[i]);
        return data;
    }

    inline constexpr Type* map(TransformFn<void, Type&, std::size_t> fn) {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data != nullptr);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error(
            "Vector<Type, Grow>.map(TransformFn<void, Type&, std::size_t>): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        for (std::size_t i = 0; i < Size; ++i)
            fn(data[i], i);
        return data;
    }

    inline constexpr Type fold(TransformFn<void, Type&, const Type&> fn) const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data != nullptr);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error(
            "Vector<Type, Grow>.fold(TransformFn<void, Type&, const Type&>): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        // We need a default constructor,
        // maybe add a concept constraint to Type for this later if needed
        Type acc {};
        for (std::size_t i < 0; i < Size; ++i)
            fn(acc, data[i]);
        return acc;
    }

    inline constexpr Type fold(TransformFn<void, Type&, const Type&, std::size_t> fn) const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data != nullptr);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error(
            "Vector<Type, Grow>.fold(TransformFn<void, Type&, const Type&, std::size_t>): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        // We need a default constructor,
        // maybe add a concept constraint to Type for this later if needed
        Type acc {};
        for (std::size_t i < 0; i < Size; ++i)
            fn(acc, data[i], i);
        return acc;
    }

    template <bool Preallocate = true>
    inline constexpr Vector<Type, Grow> filter(TransformFn<bool, const Type&> fn) const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data != nullptr);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error(
            "Vector<Type, Grow>.filter(TransformFn<bool, const Type&>): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        Vector<Type, Grow> filtered;
        if constexpr (Preallocate) {
            // Use reserve and not resize so that we
            // can use `.push_back()` in all cases
            filtered.reserve(count); // Preallocate elements
        }
        for (std::size_t i = 0; i < count; ++i)
            if (fn(data[i])) filtered.push_back(data[i]);
        filtered.shrink_to_fit();
        return filtered;
    }

    template <bool Preallocate = true>
    inline constexpr Vector<Type, Grow> filter(TransformFn<bool, const Type&, std::size_t> fn) const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data != nullptr);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error(
            "Vector<Type, Grow>.filter(TransformFn<bool, const Type&, std::size_t>): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        Vector<Type, Grow> filtered;
        if constexpr (Preallocate) {
            filtered.reserve(count); // Preallocate elements
        }
        for (std::size_t i = 0; i < count; ++i)
            if (fn(data[i], i)) filtered.push_back(data[i]);
        filtered.shrink_to_fit(); // Be sure to have it of the right size
        return filtered;
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

    inline constexpr std::size_t append(const Vector<Type, Grow>& other) {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data != nullptr);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error(
            "Vector<Type, Grow>.append(const Vector<Type, Grow>&): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        if (other.data == nullptr) return count;
        reserve(count + other.count);
        for (std::size_t i = 0; i < other.count; ++i)
            push_back(other[i]);
        return count;
    }
    inline constexpr std::size_t append(Vector<Type, Grow>&& other) {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data != nullptr);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error(
            "Vector<Type, Grow>.append(Vector<Type, Grow>&&): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        if (other.data == nullptr) return count;
        reserve(count + other.count);
        for (std::size_t i = 0; i < other.count; ++i)
            push_back(std::move(other[i]));
        // Move it
        other.count = 0;
        other.cap = 0;
        other.data = nullptr;
        return count;
    }
    template <std::size_t ArraySize>
    inline constexpr std::size_t append(const Array<Type, ArraySize>& other) {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data != nullptr);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error(
            "Vector<Type, Grow>.append<ArraySize>(const Array<Type, ArraySize>&): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        if (other.get() == nullptr) return count;
        reserve(count + ArraySize);
        for (std::size_t i = 0; i < ArraySize; ++i)
            push_back(other[i]);
        return count;
    }
    template <std::size_t ArraySize>
    inline constexpr std::size_t append(Array<Type, ArraySize>&& other) {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data != nullptr);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error(
            "Vector<Type, Grow>.append<ArraySize>(Array<Type, ArraySize>&&): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        if (other.get() == nullptr) return count;
        reserve(count + ArraySize);
        for (std::size_t i = 0; i < ArraySize; ++i)
            push_back(std::move(other[i]));
        // I absolutely HATE this but it's needed unfortunately
        const_cast<Type*>(other.get()) = nullptr;
        return count;
    }

    // Iterators

    inline constexpr Type* p_begin() {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("Vector<Type, Grow>.p_begin(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return data;
    }
    inline constexpr const Type* p_begin() const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("Vector<Type, Grow>.p_begin(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return data;
    }

    inline constexpr Type* p_end() {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("Vector<Type, Grow>.p_end(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return count != 0 ? data + count : data;
    }
    inline constexpr const Type* p_end() const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("Vector<Type, Grow>.p_end(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return count != 0 ? data + count : data;
    }

    inline constexpr ReverseMemIterator<Type> p_rbegin() {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("Vector<Type, Grow>.p_rbegin(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        // Could this ternary check just be the old data + count - 1?
        return ReverseMemIterator<Type>(count != 0 ? data + count - 1 : data);
    }
    inline constexpr const ReverseMemIterator<const Type> p_rbegin() const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("Vector<Type, Grow>.p_rbegin(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return ReverseMemIterator<const Type>(count != 0 ? data + count - 1 : data);
    }

    inline constexpr ReverseMemIterator<Type> p_rend() {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("Vector<Type, Grow>.p_rend(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return ReverseMemIterator<Type>(count != 0 ? data - 1 : data);
    }
    inline constexpr const ReverseMemIterator<const Type> p_rend() const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("Vector<Type, Grow>.p_rend(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return ReverseMemIterator<const Type>(count != 0 ? data - 1 : data);
    }

    inline constexpr const Type* p_cbegin() const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("Vector<Type, Grow>.p_cbegin(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return data;
    }

    inline constexpr const Type* p_cend() const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("Vector<Type, Grow>.p_cend(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return data + count;
    }

    inline constexpr const ReverseMemIterator<const Type> p_crbegin() const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("Vector<Type, Grow>.p_crbegin(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return ReverseMemIterator<const Type>(data + count - 1);
    }

    inline constexpr const ReverseMemIterator<const Type> p_crend() const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("Vector<Type, Grow>.p_crend(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return ReverseMemIterator<const Type>(data - 1);
    }

    #if 0

    // Indexed iterator versions (reverse is not possible due to rend()/crend() not being able to go below index 0)

    inline constexpr IdxForwIterator i_begin() {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("Vector<Type, Grow>.i_begin(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return IdxForwIterator(this, 0);
    }
    inline constexpr const ConstIdxForwIterator i_begin() const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("Vector<Type, Grow>.i_begin(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return ConstIdxForwIterator(this, 0);
    }

    inline constexpr IdxForwIterator i_end() {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("Vector<Type, Grow>.i_end(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return IdxForwIterator(this, count);
    }
    inline constexpr const ConstIdxForwIterator i_end() const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("Vector<Type, Grow>.i_end(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return ConstIdxForwIterator(this, count);
    }

    inline constexpr const ConstIdxForwIterator i_cbegin() const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("Vector<Type, Grow>.i_cbegin(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return ConstIdxForwIterator(this, 0);
    }

    inline constexpr const ConstIdxForwIterator i_cend() const {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(data);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (data == nullptr) throw std::runtime_error("Vector<Type, Grow>.i_cend(): Data pointer is nullptr");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return ConstIdxForwIterator(this, count);
    }

    #endif // 0

    #ifndef VECLIB_USE_INDEXED_ITERATORS // Default pointer-based implementations

    inline constexpr auto begin() { return p_begin(); }
    inline constexpr const auto begin() const { return p_begin(); }

    inline constexpr auto end() { return p_end(); }
    inline constexpr const auto end() const { return p_end(); }

    inline constexpr auto rbegin() { return p_rbegin(); }
    inline constexpr const auto rbegin() const { return p_rbegin(); }

    inline constexpr auto rend() { return p_rend(); }
    inline constexpr const auto rend() const { return p_rend(); }

    inline constexpr const auto cbegin() const { return p_cbegin(); }
    inline constexpr const auto cend() const { return p_cend(); }

    inline constexpr const auto crbegin() const { return p_crbegin(); }
    inline constexpr const auto crend() const { return p_crend(); }

    #else // VECLIB_USE_INDEXED_ITERATORS

    #if 0

    inline constexpr auto begin() { return i_begin(); }
    inline constexpr const auto begin() const { return i_begin(); }

    inline constexpr auto end() { return i_end(); }
    inline constexpr const auto end() const { return i_end(); }

    inline constexpr const auto cbegin() const { return i_cbegin(); }
    inline constexpr const auto cend() const { return i_cbegin(); }

    #endif // 0

    #endif // VECLIB_USE_INDEXED_ITERATORS

    #ifndef VECLIB_NO_OPERATOR_OVERLOADS // We create extra operator overloads unless it's not requested

    // Arithmetic overloads for integral types

    inline constexpr Vector<Type, Grow>& operator++() noexcept requires std::is_arithmetic_v<Type> {
        for (std::size_t i = 0; i < count; ++i)
            ++data[i];
        return *this;
    }
    inline constexpr Vector<Type, Grow> operator++(int) noexcept requires std::is_arithmetic_v<Type> {
        Vector<Type, Grow> self = *this;
        for (std::size_t i = 0; i < count; ++i)
            ++data[i];
        return self;
    }
    inline constexpr Vector<Type, Grow>& operator--() noexcept requires std::is_arithmetic_v<Type> {
        for (std::size_t i = 0; i < count; ++i)
            --data[i];
        return *this;
    }
    inline constexpr Vector<Type, Grow> operator--(int) noexcept requires std::is_arithmetic_v<Type> {
        Vector<Type, Grow> self = *this;
        for (std::size_t i = 0; i < count; ++i)
            --data[i];
        return self;
    }

    inline constexpr Vector<Type, Grow>& operator+=(const Vector<Type, Grow>& other)
            noexcept requires std::is_arithmetic_v<Type> {
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
    inline constexpr Vector<Type, Grow>& operator+=(const Type x)
            noexcept requires std::is_arithmetic_v<Type> {
        for (std::size_t i = 0; i < count; ++i)
            data[i] += x;
        return *this;
    }
    inline constexpr Vector<Type, Grow>& operator-=(const Vector<Type, Grow>& other)
            noexcept requires std::is_arithmetic_v<Type> {
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
    inline constexpr Vector<Type, Grow>& operator-=(const Type x)
            noexcept requires std::is_arithmetic_v<Type> {
        for (std::size_t i = 0; i < count; ++i)
            data[i] -= x;
        return *this;
    }
    inline constexpr Vector<Type, Grow>& operator*=(const Vector<Type, Grow>& other)
            noexcept requires std::is_arithmetic_v<Type> {
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
    inline constexpr Vector<Type, Grow>& operator*=(const Type x)
            noexcept requires std::is_arithmetic_v<Type> {
        for (std::size_t i = 0; i < count; ++i)
            data[i] *= x;
        return *this;
    }
    inline constexpr Vector<Type, Grow>& operator/=(const Vector<Type, Grow>& other)
            noexcept requires std::is_arithmetic_v<Type> {
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
    inline constexpr Vector<Type, Grow>& operator/=(const Type x)
            noexcept requires std::is_arithmetic_v<Type> {
        for (std::size_t i = 0; i < count; ++i)
            data[i] /= x;
        return *this;
    }
    inline constexpr Vector<Type, Grow>& operator%=(const Vector<Type, Grow>& other)
            noexcept requires std::is_arithmetic_v<Type> {
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
    inline constexpr Vector<Type, Grow>& operator%=(const Type x)
            noexcept requires std::is_arithmetic_v<Type> {
        for (std::size_t i = 0; i < count; ++i)
            data[i] %= x;
        return *this;
    }

    inline constexpr Vector<Type, Grow> operator+(const Vector<Type, Grow>& other)
            const requires std::is_arithmetic_v<Type> {
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
    inline constexpr Vector<Type, Grow> operator+(const Type x)
            const requires std::is_arithmetic_v<Type> {
        Vector<Type, Grow> output = *this;
        for (std::size_t i = 0; i < count; ++i)
            output[i] += x;
        return output;
    }
    inline constexpr Vector<Type, Grow> operator-(const Vector<Type, Grow>& other)
            const requires std::is_arithmetic_v<Type> {
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
    inline constexpr Vector<Type, Grow> operator-(const Type x)
            const requires std::is_arithmetic_v<Type> {
        Vector<Type, Grow> output = *this;
        for (std::size_t i = 0; i < count; ++i)
            output[i] -= x;
        return output;
    }
    inline constexpr Vector<Type, Grow> operator*(const Vector<Type, Grow>& other)
            const requires std::is_arithmetic_v<Type> {
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
    inline constexpr Vector<Type, Grow> operator*(const Type x)
            const requires std::is_arithmetic_v<Type> {
        Vector<Type, Grow> output = *this;
        for (std::size_t i = 0; i < count; ++i)
            output[i] *= x;
        return output;
    }
    inline constexpr Vector<Type, Grow> operator/(const Vector<Type, Grow>& other)
            const requires std::is_arithmetic_v<Type> {
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
    inline constexpr Vector<Type, Grow> operator/(const Type x)
            const requires std::is_arithmetic_v<Type> {
        Vector<Type, Grow> output = *this;
        for (std::size_t i = 0; i < count; ++i)
            output[i] /= x;
        return output;
    }
    inline constexpr Vector<Type, Grow> operator%(const Vector<Type, Grow>& other)
            const requires std::is_arithmetic_v<Type> {
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
    inline constexpr Vector<Type, Grow> operator%(const Type x)
            const requires std::is_arithmetic_v<Type> {
        Vector<Type, Grow> output = *this;
        for (std::size_t i = 0; i < count; ++i)
            output[i] %= x;
        return output;
    }

    inline constexpr bool operator==(const Vector<Type, Grow>& other)
            const requires std::equality_comparable<Type> {
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
    inline constexpr bool operator==(const Type& x)
            const noexcept requires std::equality_comparable<Type> {
        for (std::size_t i = 0; i < count; ++i)
            if (data[i] != x) return false; // Short-circuit
        return true;
    }
    inline constexpr bool operator!=(const Vector<Type, Grow>& other)
            const requires std::equality_comparable<Type> {
        return !(*this == other);
    }
    inline constexpr bool operator!=(const Type& x)
            const noexcept requires std::equality_comparable<Type> {
        return !(*this == x);
    }

    #endif // VECLIB_NO_OPERATOR_OVERLOADS
};


// Sneaky formula to nudge the size into the next byte
#define VECLIB_BITSET_BYTESIZE(Size) (Size + (CHAR_BIT - 1) / 8)

template <std::size_t Size>
class Bitset;

template <std::size_t ProxySize>
class BitsetProxy;

template <std::size_t ItrSize>
class BitsetIterator {
friend Bitset;
private:
    Bitset<Size>& bitset;
    std::size_t index;

public:
    BitsetIterator() = delete;
    ~BitsetIterator() noexcept = delete;

    Bitsetiterator(Bitset<ItrSize>& b, std::size_t i) noexcept
        : bitset(b), index(i) {}

    inline constexpr BitsetProxy<ItrSize> operator*() noexcept {
        return bitset[index];
    }
    inline constexpr const BitsetProxy<ItrSize> operator*() const noexcept {
        return bitset[index];
    }

    inline constexpr BitsetIterator<ItrSize>& operator++() noexcept {
        ++index;
        return *this;
    }
    inline constexpr BitsetIterator<ItrSize> operator++(int) noexcept {
        BitsetIterator<ItrSize> self = *this;
        ++*this;
        return self;
    }
    inline constexpr BitsetIterator<ItrSize>& operator--() noexcept {
        --index;
        return *this;
    }
    inline constexpr BitsetIterator<ItrSize> operator--(int) noexcept {
        BitsetIterator<ItrSize> self = *this;
        --*this;
        return self;
    }

    inline constexpr BitsetIterator<ItrSize>& operator+=(std::size_t x) noexcept {
        index += x;
        return *this;
    }
    inline constexpr BitsetIterator<ItrSize>& operator-=(std::size_t x) noexcept {
        index -= x;
        return *this;
    }

    inline constexpr BitsetIterator<ItrSize> operator+(std::size_t x) const noexcept {
        BitsetIterator<ItrSize> self = *this;
        self += x;
        return self;
    }
    inline constexpr BitsetIterator<ItrSize> operator-(std::size_t x) const noexcept {
        BitsetIterator<ItrSize> self = *this;
        self -= x;
        return self;
    }

    inline constexpr bool operator==(const BitsetIterator<ItrSize>& other) const noexcept {
        return &bitset == &other.bitset && index == other.index;
    }
    inline constexpr bool operator!=(const BitsetIterator<ItrSize>& other) const noexcept {
        return !(*this == other);
    }
    inline consteval operator bool() const noexcept { return true; }
};

template <std::size_t BitsetSize>
using BitPtr = BitsetIterator<BitsetSize>;

template <std::size_t ProxySize>
class BitsetProxy {
friend Bitset;
private:
    Bitset<ProxySize>& bitset;
    std::size_t index;

 public:
    BitsetProxy() = delete;
    ~BitsetProxy() noexcept = default;

    BitsetProxy(Bitset<ProxySize>& b, std::size_t i) noexcept
        : bitset(b), index(i) {}

    inline constexpr operator bool() const noexcept {
        return bitset.get_at(index);
    }
    inline constexpr BitsetProxy<ProxySize>& operator=(bool value) noexcept {
        bitset.set_at(index, value);
        return *this;
    }

    inline constexpr BitPtr<ProxySize> operator&() const noexcept {
        return BitPtr<ProxySize>(bitset, index);
    }
};

template <std::size_t Size>
class Bitset {
private:
    std::uint8_t data[VECLIB_BITSET_BYTESIZE(Size)] = {0}; // Zero-initialization

    inline constexpr bool get_at(std::size_t index) const noexcept {
        return data[index / CHAR_BIT] & (1 << (index % CHAR_BIT));
    }

    inline constexpr void set_at(std::size_t index, bool value) noexcept {
        if (value) data[index / CHAR_BIT] |= 1 << (index % CHAR_BIT);
        else data[index / CHAR_BIT] &= ~(1 << (index % CHAR_BIT));
    }

public:
    Bitset() noexcept = default;
    ~Bitset() noexcept = default;

    Bitset(const std::initializer_list<std::uint8_t>& args) {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(args.size() <= VECLIB_BITSET_BYTESIZE(Size));
        #else // VECLIB_ASSERT_NOEXCEPT
        if (args.size() > VECLIB_BITSET_BYTESIZE(Size)) throw std::invalid_argument(
            "Bitset<Size>::Bitset(const std::initializer_list<std::uint8_t>&): Too many bytes provided");
        #endif // VECLIB_ASSERT_NOEXCEPT
        for (std::size_t i = 0; i < args.size(); ++i)
            data[i] = args.begin()[i];
        // Needed?
        for (std::size_t i = args.size(); i < VECLIB_BITSET_BYTESIZE(Size); ++i)
            data[i] = 0;
    }

    Bitset(const std::initializer_list<bool>& args) {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(args.size() <= Size);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (args.size() > Size) throw std::invalid_argument(
            "Bitset<Size>::Bitset(const std::initializer_list<bool>&): Too many bits provided");
        #endif // VECLIB_ASSERT_NOEXCEPT
        for (std::size_t i = 0; i < args.size(); ++i)
            data[i / CHAR_BIT] |= (args.begin()[i] ? 1 : 0) << (i % CHAR_BIT);
        for (std::size_t i = args.size(); i < Size; ++i)
            data[i / CHAR_BIT] &= ~(1 << i % CHAR_BIT); // Set all others to 0
    }

    Bitset(std::uint8_t x) noexcept requires (Size == 8) {
        std::memcpy(data, &x, sizeof(x));
    }
    Bitset(std::uint16_t x) noexcept requires (Size == 16) {
        std::memcpy(data, &x, sizeof(x));
    }
    Bitset(std::uint32_t x) noexcept requires (Size == 32) {
        std::memcpy(data, &x, sizeof(x));
    }
    Bitset(std::uint64_t x) noexcept requires (Size == 64) {
        std::memcpy(data, &x, sizeof(x));
    }

    inline constexpr const std::uint8_t* get() const noexcept { return data; }
    inline constexpr std::size_t size() const noexcept { return Size; }
    inline constexpr std::size_t bytesize() const noexcept { return VECLIB_BITSET_BYTESIZE(Size); }

    inline constexpr BitsetProxy<Size> operator[](std::size_t index) noexcept {
        return BitsetProxy<Size>(*this, index);
    }
    inline constexpr const BitsetProxy<Size> operator[](std::size_t index) const noexcept {
        return BitsetProxy<Size>(*this, index);
    }

    inline constexpr BitsetProxy<Size> at(std::size_t index) noexcept {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(index < Size);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (index >= Size) throw std::out_of_range("Bitset<Size>.at(std::size_t): Index is out of bounds");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return BitsetProxy<Size>(*this, index);
    }
    inline constexpr const BitsetProxy<Size> at(std::size_t index) const noexcept {
        #ifdef VECLIB_ASSERT_NOEXCEPT
        assert(index < Size);
        #else // VECLIB_ASSERT_NOEXCEPT
        if (index >= Size) throw std::out_of_range("Bitset<Size>.at(std::size_t): Index is out of bounds");
        #endif // VECLIB_ASSERT_NOEXCEPT
        return BitsetProxy<Size>(*this, index);
    }

    inline constexpr std::size_t on() const noexcept {
        std::size_t count = 0;
        for (std::size_t i = 0; i < Size; ++i)
            if (data[i / CHAR_BIT] & (1 << (i % CHAR_BIT)) != 0) ++count;
        return count;
    }
    inline constexpr std::size_t off() const noexcept {
        return Size - on();
    }

    inline constexpr bool any() const noexcept {
        for (std::size_t i = 0; i < VECLIB_BITSET_BYTESIZE(Size); ++i)
            if (data[i] != 0) return true;
        return false;
    }
    inline constexpr bool none() const noexcept {
        return !any();
    }
    inline constexpr operator bool() const noexcept {
        return any();
    }

    inline constexpr Bitset<Size>& operator&=(const Bitset<Size>& other) noexcept {
        for (std::size_t i = 0; i < VECLIB_BITSET_BYTESIZE(Size); ++i)
            data[i] &= other.data[i];
        return *this;
    }
    inline constexpr Bitset<Size>& operator|=(const Bitset<Size>& other) noexcept {
        for (std::size_t i = 0; i < VECLIB_BITSET_BYTESIZE(Size); ++i)
            data[i] |= other.data[i];
        return *this;
    }
    inline constexpr Bitset<Size>& operator^=(const Bitset<Size>& other) noexcept {
        for (std::size_t i = 0; i < VECLIB_BITSET_BYTESIZE(Size); ++i)
            data[i] ^= other.data[i];
        return *this;
    }

    inline constexpr Bitset<Size> operator&(const Bitset<Size>& other) const noexcept {
        Bitset<Size> self = *this;
        self &= other;
        return self;
    }
    inline constexpr Bitset<Size> operator|(const Bitset<Size>& other) const noexcept {
        Bitset<Size> self = *this;
        self |= other;
        return self;
    }
    inline constexpr Bitset<Size> operator^(const Bitset<Size>& other) const noexcept {
        Bitset<Size> self = *this;
        self ^= other;
        return self;
    }

    inline constexpr bool operator==(const Bitset<Size>& other) const noexcept {
        for (std::size_t i = 0; i < VECLIB_BITSET_BYTESIZE(Size); ++i)
            if (data[i] != other.data[i]) return false;
        return true;
    }
    inline constexpr bool operator!=(const Bitset<Size>& other) const noexcept {
        return !(*this == other);
    }
};

#undef VECLIB_BITSET_BYTESIZE

} // namespace veclib

#endif // ARRAY_HPP
