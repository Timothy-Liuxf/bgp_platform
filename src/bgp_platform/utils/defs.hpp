#ifndef BGP_PLATFORM_UTILS_DEFS_HPP_
#define BGP_PLATFORM_UTILS_DEFS_HPP_

#define BGP_PLATFORM_NAMESPACE             bgp_platform
#define BGP_PLATFORM_NAMESPACE_BEGIN       namespace BGP_PLATFORM_NAMESPACE {
#define BGP_PLATFORM_NAMESPACE_END         }

#define BGP_PLATFORM_LIKELY(cond)          __builtin_expect(!!(cond), 1)
#define BGP_PLATFORM_UNLIKELY(cond)        __builtin_expect(!!(cond), 0)
#define BGP_PLATFORM_IF_LIKELY(cond)       if (BGP_PLATFORM_LIKELY(cond))
#define BGP_PLATFORM_IF_UNLIKELY(cond)     if (BGP_PLATFORM_UNLIKELY(cond))

#define BGP_PLATFORM_UNUSED_PARAMETER(arg) (void)arg
#define BGP_PLATFORM_UNUSED_VARIABLE(var)  (void)var

#endif  // BGP_PLATFORM_UTILS_DEFS_HPP_
