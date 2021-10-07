#pragma once

#include <assert.h>
#include <climits>
#include <iostream>
#include <string.h>
#include <type_traits>
namespace muduo {
namespace base {
namespace detail {

template <class T> struct IsCharPointer {};

template <> struct IsCharPointer<char *> { typedef int type; };

template <> struct IsCharPointer<const char *> {
    typedef int const_type;
    typedef int type;
};

template <typename Char> constexpr std::size_t constexpr_strlen(const Char *s, unsigned) noexcept {
    std::size_t ret = 0;
    while (*s++) {
        ++ret;
    }
    return ret;
}
} // namespace detail

template <class Iter> class Range {
private:
    template <class Alloc> using mstring = std::basic_string<Iter, std::char_traits<Iter>, Alloc>;

protected:
    Iter begin_, end_;

public:
    typedef std::size_t                                                                          size_type;
    typedef Iter                                                                                 iterator;
    typedef Iter                                                                                 const_iterator;
    typedef typename std::remove_reference<typename std::iterator_traits<Iter>::reference>::type value_type;
    /* iterator */
    using difference_type = typename std::iterator_traits<Iter>::difference_type;
    typedef typename std::iterator_traits<Iter>::reference reference;

    typedef typename std::conditional<std::is_same<Iter, char *>::value || std::is_same<Iter, unsigned char *>::value, Range<const value_type *>, Range<Iter>>::type const_range_type;

    typedef std::char_traits<typename std::remove_const<value_type>::type> traits_type;

public:
    constexpr Range()
        : begin_()
        , end_() {
    }
    constexpr Range(const Range &)                                                                    = default;
    constexpr Range(Range &&)                                                                         = default;
    Range &                                                                                   operator=(const Range &rhs) & = default;
    Range &                                                                                   operator=(Range &&rhs) & = default;
    template <class Alloc, class T, typename detail::IsCharPointer<T>::const_type = 0> Range &operator=(mstring<Alloc> &&rhs) = delete;
    constexpr Range(Iter start, Iter end)
        : begin_(start)
        , end_(end) {
    }
    constexpr Range(Iter start, size_t size)
        : begin_(start)
        , end_(begin_ + size) {
    }
    Range(std::nullptr_t) = delete;
    constexpr Range(Iter str)
        : begin_(str)
        , end_(str + detail::constexpr_strlen(str, 0)) {
    }

public:
    void clear() {
        begin_ = Iter();
        end_   = Iter();
    }
    void assign(Iter start, Iter end) {
        begin_ = start;
        end_   = end;
    }
    void reset(Iter start, size_type size) {
        begin_ = start;
        end_   = start + size;
    }
    template <typename Alloc> void reset(const Range::mstring<Alloc> &str) {
        reset(str.data(), str.size());
    }
    constexpr size_type size() const {
        return size_type(end_ - begin_);
    }
    constexpr size_type walk_size() const {
        return size_type(std::distance(begin_, end_));
    }
    constexpr bool empty() const {
        return begin_ == end_;
    }
    constexpr Iter data() const {
        return begin_;
    }
    constexpr Iter start() const {
        return begin_;
    }
    constexpr Iter begin() const {
        return begin_;
    }
    constexpr Iter end() const {
        return end_;
    }
    constexpr Iter cbegin() const {
        return begin_;
    }
    constexpr Iter cend() const {
        return end_;
    }
    value_type &front() {
        assert(begin_ < end_);
        return *begin_;
    }
    value_type &back() {
        assert(begin_ < end_);
        return *std::prev(end_);
    }
    const value_type &front() const {
        assert(begin_ < end_);
        return *begin_;
    }
    const value_type &back() const {
        assert(begin_ < end_);
        return *std::prev(end_);
    }
    value_type &operator[](size_type nIndex) {
        assert(nIndex < size());
        return begin_[ nIndex ];
    }
    const value_type &operator[](size_type nIndex) const {
        assert(nIndex < size());
        return begin_[ nIndex ];
    }
    value_type &at(size_type nIndex) {
        if (nIndex >= size())
            throw std::out_of_range("index out of range");
        return begin_[ nIndex ];
    }
    const value_type &at(size_type nIndex) const {
        if (nIndex >= size())
            throw std::out_of_range("index out of range");
        return begin_[ nIndex ];
    }
    Range subPiece(size_type first, size_type length = size_type(-1)) const {
        if (first > size())
            throw std::out_of_range("index out of range");
        return Range(begin_ + first, std::min(length, size() - first));
    }
    void pop_front() {
        assert(begin_ < end_);
        begin_++;
    }
    void pop_back() {
        assert(begin_ < end_);
        end_--;
    }
    int compare(const const_range_type &o) const {
        const size_type tsize = this->size();
        const size_type osize = o.size();
        const size_type msize = std::min(tsize, osize);
        int             r     = traits_type::compare(data(), o.data(), msize);
        if (r == 0 && tsize != osize) {
            // We check the signed bit of the subtraction and bit shift it
            // to produce either 0 or 2. The subtraction yields the
            // comparison values of either -1 or 1.
            r = (static_cast<int>((osize - tsize) >> (CHAR_BIT * sizeof(size_t) - 1)) << 1) - 1;
        }
        return r;
    }

    void swap(Range &rhs) {
        std::swap(begin_, rhs.begin_);
        std::swap(end_, rhs.end_);
    }
    const_range_type castToConst() const {
        return const_range_type(*this);
    }
    bool startsWith(const const_range_type &other) {
        return size() >= other.size() && castToConst().subPiece(0, other.size()) == other;
    }
    bool startsWith(value_type c) const {
        return !empty() && front() == c;
    }
    template <class Comp> bool startsWith(const const_range_type &other, Comp &&eq) const {
        if (size() < other.size()) {
            return false;
        }
        auto const trunc = subPiece(0, other.size());
        return std::equal(trunc.begin(), trunc.end(), other.begin(), std::forward<Comp>(eq));
    }
    bool endsWith(const const_range_type &other) const {
        return size() >= other.size() && castToConst().subPiece(size() - other.size()) == other;
    }
    bool endsWith(value_type c) const {
        return !empty() && back() == c;
    }
    template <class Comp> bool endsWith(const const_range_type &other, Comp &&eq) const {
        if (size() < other.size()) {
            return false;
        }
        auto const trunc = subPiece(size() - other.size());
        return std::equal(trunc.begin(), trunc.end(), other.begin(), std::forward<Comp>(eq));
    }
    bool removePrefix(const const_range_type &prefix) {
        return startsWith(prefix) && (begin_ += prefix.size(), true);
    }
    bool removePrefix(value_type prefix) {
        return startsWith(prefix) && (++begin_, true);
    }
    bool removeSuffix(const const_range_type &suffix) {
        return endsWith(suffix) && (end_ -= suffix.size(), true);
    }
    bool removeSuffix(value_type suffix) {
        return endsWith(suffix) && (--end_, true);
    }
    bool replaceAt(size_t pos, const_range_type replacement) {
        if (size() < pos + replacement.size()) {
            return false;
        }
        std::copy(replacement.begin(), replacement.end(), begin() + pos);
        return true;
    }
    size_t replaceAll(const_range_type source, const_range_type dest) {
        if (source.size() != dest.size()) {
            throw std::invalid_argument("replacement must have the same size as source");
        }
        if (dest.empty()) {
            return 0;
        }
        size_t    pos          = 0;
        size_t    num_replaced = 0;
        size_type found        = std::string::npos;
        while ((found = find(source, pos)) != std::string::npos) {
            replaceAt(found, dest);
            pos += source.size();
            ++num_replaced;
        }
        return num_replaced;
    }
};

typedef Range<const char *>          StringPiece;
typedef Range<char *>                MutableStringPiece;
typedef Range<const unsigned char *> ByteRange;
typedef Range<unsigned char *>       MutableByteRange;

template <class C> inline std::basic_ostream<C> &operator<<(std::basic_ostream<C> &os, Range<C const *> piece) {
    using StreamSize = decltype(os.width());
    os.write(piece.start(), static_cast<StreamSize>(piece.size()));
    return os;
}

template <class C> inline std::basic_ostream<C> &operator<<(std::basic_ostream<C> &os, Range<C *> piece) {
    using StreamSize = decltype(os.width());
    os.write(piece.start(), static_cast<StreamSize>(piece.size()));
    return os;
}

template <class Iter> inline bool operator==(const Range<Iter> &lhs, const Range<Iter> &rhs) {
    return lhs.size() == rhs.size() && lhs.compare(rhs) == 0;
}
template <class Iter> inline bool operator!=(const Range<Iter> &lhs, const Range<Iter> &rhs) {
    return !(operator==(lhs, rhs));
}

template <class Iter> inline bool operator<(const Range<Iter> &lhs, const Range<Iter> &rhs) {
    return lhs.compare(rhs) < 0;
}

template <class Iter> inline bool operator<=(const Range<Iter> &lhs, const Range<Iter> &rhs) {
    return lhs.compare(rhs) <= 0;
}

template <class Iter> inline bool operator>(const Range<Iter> &lhs, const Range<Iter> &rhs) {
    return lhs.compare(rhs) > 0;
}

template <class Iter> inline bool operator>=(const Range<Iter> &lhs, const Range<Iter> &rhs) {
    return lhs.compare(rhs) >= 0;
}
} // namespace base
}