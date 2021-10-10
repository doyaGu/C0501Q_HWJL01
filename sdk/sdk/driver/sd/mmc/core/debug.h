#ifndef _MMC_CORE_DEBUG_H
#define _MMC_CORE_DEBUG_H


#define PR_ERR      1
#define PR_WARN     0
#define PR_INFO     0
#define PR_DBG      0
#define DEV_DBG     0
#define DEV_WARN     0
#ifndef DBG
#define DBG	pr_debug
#endif


#if defined(_WIN32)

#if PR_ERR
#define pr_err(string, ...)   do { printf("[SD][ERR] "); printf(string, __VA_ARGS__); } while (0)
#else
#define pr_err(string, ...)
#endif

#if PR_WARN
#define pr_warn(string, ...)   do { printf("[SD][WARN] "); printf(string, __VA_ARGS__); } while (0)
#define pr_warn_ratelimited(string, ...)   do { printf("[SD][WARN] "); printf(string, __VA_ARGS__); } while (0)
#else
#define pr_warn(string, ...)
#define pr_warn_ratelimited(string, ...)
#endif

#if PR_INFO
#define pr_info(string, ...)   do { printf("[SD][INFO] "); printf(string, __VA_ARGS__); } while (0)
#else
#define pr_info(string, ...)
#endif

#if PR_DBG
#define pr_debug(string, ...)   do { printf("[SD][DBG] "); printf(string, __VA_ARGS__); } while (0)
#else
#define pr_debug(string, ...)
#endif

#if DEV_DBG
#define dev_dbg(ddev, string, ...)    do { printf("[SD][DBG] "); printf(string, __VA_ARGS__); } while (0)
#else
#define dev_dbg(ddev, string, ...)
#endif

#if DEV_WARN
#define dev_warn(ddev, string, ...)    do { printf("[SD][WARN] "); printf(string, __VA_ARGS__); } while (0)
#else
#define dev_warn(ddev, string, ...)
#endif

#else // #if defined(_WIN32)

#if PR_ERR
#define pr_err(string, args...)   do { ithPrintf("[SD][ERR] "); ithPrintf(string, ## args); } while (0)
#else
#define pr_err(string, args...)
#endif

#if PR_WARN
#define pr_warn(string, args...)   do { ithPrintf("[SD][WARN] "); ithPrintf(string, ## args); } while (0)
#define pr_warn_ratelimited(string, args...)   do { ithPrintf("[SD][WARN] "); ithPrintf(string, ## args); } while (0)
#else
#define pr_warn(string, args...)
#define pr_warn_ratelimited(string, args...)
#endif

#if PR_INFO
#define pr_info(string, args...)   do { ithPrintf("[SD][INFO] "); ithPrintf(string, ## args); } while (0)
#else
#define pr_info(string, args...)
#endif

#if PR_DBG
#define pr_debug(string, args...)   do { ithPrintf("[SD][DBG] "); ithPrintf(string, ## args); } while (0)
#else
#define pr_debug(string, args...)
#endif

#if DEV_DBG
#define dev_dbg(ddev, string, args...)    do { ithPrintf("[SD][DBG] "); ithPrintf(string, ## args); } while (0)
#else
#define dev_dbg(ddev, string, args...)
#endif

#if DEV_WARN
#define dev_warn(ddev, string, args...)    do { ithPrintf("[SD][WARN] "); ithPrintf(string, ## args); } while (0)
#else
#define dev_warn(ddev, string, args...)
#endif

#endif // #if defined(_WIN32)

#endif
