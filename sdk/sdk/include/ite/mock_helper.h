#ifndef ITE_MOCK_HELPER_H
#define ITE_MOCK_HELPER_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CFG_DEV_TEST
    #define DECLARE_COULD_BE_MOCKED_FUNC1(ret_type, func, para1_type) \
        extern ret_type ## (*_ ## func)( ## para1_type);              \
        extern ret_type func ## _default( ## para1_type)
    #define DECLARE_COULD_BE_MOCKED_FUNC2(ret_type, func, para1_type, para2_type) \
        extern ret_type ## (*_ ## func)( ## para1_type, ## para2_type);           \
        extern ret_type func ## _default( ## para1_type, ## para2_type)
    #define DECLARE_COULD_BE_MOCKED_FUNC5(ret_type, func, para1_type, para2_type, para3_type, para4_type, para5_type)   \
        extern ret_type ## (*_ ## func)( ## para1_type, ## para2_type, ## para3_type, ## para4_type, ## para5_type);    \
        extern ret_type func ## _default( ## para1_type, ## para2_type, ## para3_type, ## para4_type, ## para5_type)
    #define DECLARE_COULD_BE_MOCKED_FUNC6(ret_type, func, para1_type, para2_type, para3_type, para4_type, para5_type, para6_type)   \
        extern ret_type ## (*_ ## func)( ## para1_type, ## para2_type, ## para3_type, ## para4_type, ## para5_type, ## para6_type);    \
        extern ret_type func ## _default(## para1_type, ## para2_type, ## para3_type, ## para4_type, ## para5_type, ## para6_type)
    #define DECLARE_COULD_BE_MOCKED_FUNC7(ret_type, func, para1_type, para2_type, para3_type, para4_type, para5_type, para6_type, para7_type)   \
        extern ret_type ## (*_ ## func)( ## para1_type, ## para2_type, ## para3_type, ## para4_type, ## para5_type, ## para6_type, ## para7_type);    \
        extern ret_type func ## _default(## para1_type, ## para2_type, ## para3_type, ## para4_type, ## para5_type, ## para6_type, ## para7_type)
    #define DEFINE_COULD_BE_MOCKED_NOT_VOID_FUNC1(ret_type, func, para1_type, para1_name)                               \
        ret_type ## (*_ ## func)(para1_type) = func ## _default;                                                        \
        ret_type        func ## ( ## para1_type para1_name ## ) { return (_ ## func) ? _ ## func( ## para1_name) : 0; } \
        ret_type        func ## _default( ## para1_type para1_name ## )
    #define DEFINE_COULD_BE_MOCKED_NOT_VOID_FUNC5(ret_type, func, para1_type, para1_name, para2_type, para2_name, para3_type, para3_name, para4_type, para4_name, para5_type, para5_name)                               \
        ret_type ## (*_ ## func)(para1_type, para2_type, para3_type, para4_type, para5_type) = func ## _default;                                                        \
        ret_type        func ## ( ## para1_type para1_name ## , ## para2_type para2_name ## , ## para3_type para3_name ## , ## para4_type para4_name ## , ## para5_type para5_name ## ) { return (_ ## func) ? _ ## func( ## para1_name, ## para2_name, ## para3_name, ## para4_name, ## para5_name) : 0; } \
        ret_type        func ## _default( ## para1_type para1_name ## , ## para2_type para2_name ## , ## para3_type para3_name ## , ## para4_type para4_name ## , ## para5_type para5_name ## )
    #define DEFINE_COULD_BE_MOCKED_NOT_VOID_FUNC6(ret_type, func, para1_type, para1_name, para2_type, para2_name, para3_type, para3_name, para4_type, para4_name, para5_type, para5_name, para6_type, para6_name)                               \
        ret_type ## (*_ ## func)(para1_type, para2_type, para3_type, para4_type, para5_type, para6_type) = func ## _default;                                                        \
        ret_type        func ## ( ## para1_type para1_name ## , ## para2_type para2_name ## , ## para3_type para3_name ## , ## para4_type para4_name ## , ## para5_type para5_name ##, ## para6_type para6_name ## ) { return (_ ## func) ? _ ## func( ## para1_name, ## para2_name, ## para3_name, ## para4_name, ## para5_name, ## para6_name) : 0; } \
        ret_type        func ## _default( ## para1_type para1_name ## , ## para2_type para2_name ## , ## para3_type para3_name ## , ## para4_type para4_name ## , ## para5_type para5_name ## , ## para6_type para6_name ## )
    #define DEFINE_COULD_BE_MOCKED_NOT_VOID_FUNC7(ret_type, func, para1_type, para1_name, para2_type, para2_name, para3_type, para3_name, para4_type, para4_name, para5_type, para5_name, para6_type, para6_name, para7_type, para7_name)                               \
        ret_type ## (*_ ## func)(para1_type, para2_type, para3_type, para4_type, para5_type, para6_type, para7_type) = func ## _default;                                                        \
        ret_type        func ## ( ## para1_type para1_name ## , ## para2_type para2_name ## , ## para3_type para3_name ## , ## para4_type para4_name ## , ## para5_type para5_name ##, ## para6_type para6_name ##, ## para7_type para7_name ## ) { return (_ ## func) ? _ ## func( ## para1_name, ## para2_name, ## para3_name, ## para4_name, ## para5_name, ## para6_name, ## para7_name) : 0; } \
        ret_type        func ## _default( ## para1_type para1_name ## , ## para2_type para2_name ## , ## para3_type para3_name ## , ## para4_type para4_name ## , ## para5_type para5_name ## , ## para6_type para6_name ## , ## para7_type para7_name ## )
    #define DEFINE_COULD_BE_MOCKED_VOID_FUNC2(func, para1_type, para1_name, para2_type, para2_name) \
        void ## (*_ ## func)(para1_type, para2_type) = func ## _default;                            \
        void            func ## ( ## para1_type para1_name ## , ## para2_type para2_name ## ) { if (_ ## func) { _ ## func( ## para1_name, ## para2_name); } } \
        void            func ## _default( ## para1_type para1_name ## , ## para2_type para2_name ## )
    #define MOCK(original_func, mock_func)         { _ ## original_func = mock_func; }
    #define MOCK_RESTORE_TO_DEFAULT(original_func) { _ ## original_func = original_func ## _default; }
#else
    #define DECLARE_COULD_BE_MOCKED_FUNC1(func, ...)
    #define DECLARE_COULD_BE_MOCKED_FUNC2(func, ...)
    #define DECLARE_COULD_BE_MOCKED_FUNC5(func, ...)
    #define DECLARE_COULD_BE_MOCKED_FUNC6(func, ...)
    #define DECLARE_COULD_BE_MOCKED_FUNC7(func, ...)
    #define MOCK(original_func, mock_func)
    #define MOCK_RESTORE_TO_DEFAULT(original_func)
#endif

#ifdef __cplusplus
}
#endif

#endif