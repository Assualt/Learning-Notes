#pragma once

#include "Range.h"
#include <bits/move.h>
#include <type_traits>

namespace muduo::base {

template <class T> struct IsSomeString : std::false_type {};

template <typename Alloc>
struct IsSomeString<std::basic_string<char, std::char_traits<char>, Alloc>> : std::true_type {};

struct Ignore {
    Ignore() = default;
    template <class T> constexpr /* implicit */ Ignore(const T &) {}
    template <class T> const Ignore &operator=(T const &) const { return *this; }
};

template <class...> using Ignored = Ignore;

template <typename... Ts> struct LastElementImpl {
    static void call(Ignored<Ts>...) {}
};

template <typename Head, typename... Ts> struct LastElementImpl<Head, Ts...> {
    template <typename Last> static Last call(Ignored<Ts>..., Last &&last) { return std::forward<Last>(last); }
};

template <typename... Ts> auto getLastElement(const Ts &...ts) -> decltype(LastElementImpl<Ts...>::call(ts...)) {
    return LastElementImpl<Ts...>::call(ts...);
}

template <class... Ts>
struct LastElement : std::decay<decltype(LastElementImpl<Ts...>::call(std::declval<Ts>()...))> {};

template <class Target, class Src>
typename std::enable_if<std::is_same<Target, typename std::decay<Src>::type>::value, Target>::type
    to(const Src &value) {
    return std::forward<Target>(value);
}

template <class Target, class... Ts>
typename std::enable_if<IsSomeString<Target>::value &&
                            (sizeof...(Ts) != 1 ||
                             !std::is_same<Target, typename LastElementImpl<const Ts &...>::type>::value),
                        Target>::type
    to(const Ts &...vs) {
    Target tgs;
    return tgs;
}

} // namespace muduo::base