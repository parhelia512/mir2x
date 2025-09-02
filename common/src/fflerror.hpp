#pragma once
#include <cstring>
#include <string>
#include <stdexcept>
#include <execinfo.h>
#include "strf.hpp"

#define fflerror(...) std::runtime_error(str_ffl() + ": " + str_printf(__VA_ARGS__))

inline std::string _fflerror_helper(size_t index)
{
    return str_printf("[%zu]: NA", index);
}

template<typename T> std::string _fflerror_helper(size_t index, const T & t)
{
    return str_printf("[%zu]: %s", index, str_any(t).c_str());
}

template<typename T, typename ... Args> std::string _fflerror_helper(size_t index, const T & t, Args && ... args)
{
    return _fflerror_helper(index, t) + ", " + _fflerror_helper(index + 1, std::forward<Args>(args)...);
}

template<typename ... Args> constexpr size_t _fflerror_count_helper(Args && ...)
{
    return sizeof...(Args);
}

#define fflreach() fflerror("bad_reach")
#define fflvalue(...) fflerror("%s", _fflerror_helper(0, __VA_ARGS__).c_str())

#define fflassert(cond, ...) \
        do{ \
            if(cond){}else{ \
                throw fflerror("assertion failed: %s%s", #cond, \
                        (_fflerror_count_helper(__VA_ARGS__) == 0) ? "" : (std::string(", ") + _fflerror_helper(0, ##__VA_ARGS__)).c_str()); \
            } \
        } \
        while(0)

#define argcheck(x, ...) (\
        []<typename T, typename... F>(T && argx, F && ... f) -> decltype(auto) \
        { \
            if constexpr (sizeof...(F) == 0){ \
                fflassert(argx); \
            } \
            else if constexpr (sizeof...(F) == 1){ \
                static_assert(std::is_invocable_v<F...>, "argcheck's second argument must be an invocable"); \
                fflassert(std::invoke(std::forward<F>(f)...)); \
            } \
            else{ \
                static_assert(sizeof...(F) < 2, "argcheck must have 1 or 2 arguments"); \
            } \
            return std::forward<T>(argx); \
        }) \
        (std::forward<decltype((x))>(x) __VA_OPT__(,) __VA_ARGS__)
