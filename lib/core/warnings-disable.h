#ifndef WARNINGS_DISABLE_H
#define WARNINGS_DISABLE_H
// clang-format off

/**
 * Disable warnings; mainly for Qt headers.
 *
 * clang's -Weverything was added in 3.9.  However, Apple invented
 * a whole new set of version numbers for clang, so it's hard to
 * figure out which features are supported on OSX.
 */
#if defined(__clang__) && !defined(__APPLE__) && (                             \
    (__clang_major__ >= 4) ||                                                  \
    ((__clang_major__ == 3) && (__clang_major__ >= 9)))

    /* Disable compiler warnings (for including Qt headers). */
    #define WARNINGS_DISABLE                                                   \
        _Pragma("clang diagnostic push")                                       \
        _Pragma("clang diagnostic ignored \"-Weverything\"")

    #define WARNINGS_ENABLE						       \
        _Pragma("clang diagnostic pop")

/* No special handling for other compilers. */
#else
    #define WARNINGS_DISABLE /* NOTHING */
    #define WARNINGS_ENABLE /* NOTHING */
#endif /* !__clang__ */

// clang-format on
#endif
