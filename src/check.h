#ifndef CHECK_MACRO_GUARD
#define CHECK_MACRO_GUARD

// CHECK is for situations where there is no obvious action to take on an error
// except to crash
#ifndef CHECK
#    ifdef NDEBUG
#        define CHECK(x)                                                                                                                                       \
            do {                                                                                                                                               \
                if (!(x))                                                                                                                                      \
                    abort();                                                                                                                                   \
            } while (0)
#    else
#        define CHECK assert
#    endif
#endif

#ifndef NAPI_CHECK
#endif

#endif // CHECK_MACRO_GUARD
