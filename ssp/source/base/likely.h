//
// Created by 侯鑫 on 2024/1/3.
//

#ifndef SSP_TOOLS_LIKELY_H
#define SSP_TOOLS_LIKELY_H

#if __GNUC__
#define DETAIL_BUILTIN_EXPECT(b, t) (__builtin_expect(b, t))
#else
#define DETAIL_BUILTIN_EXPECT(b, t) b
#endif

#define LIKELY(x) DETAIL_BUILTIN_EXPECT((x), 1)
#define UNLIKELY(x) DETAIL_BUILTIN_EXPECT((x), 0)

//  Un-namespaced annotations

#undef LIKELY
#undef UNLIKELY

#if defined(__GNUC__)
#define LIKELY(x) (__builtin_expect((x), 1))
#define UNLIKELY(x) (__builtin_expect((x), 0))
#else
#define LIKELY(x) (x)
#define UNLIKELY(x) (x)
#endif


#endif //SSP_TOOLS_LIKELY_H
