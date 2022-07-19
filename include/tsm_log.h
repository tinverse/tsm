#pragma once
#ifndef USE_EXTERNAL_LOG
enum LogLevel
{
    SM_DEBUG,
    SM_INFO,
    SM_WARNING,
    SM_ERROR,
};

template <typename LogLevelT>
struct NullStream
{
    __attribute__((noinline)) explicit NullStream(const char* /* filename */,
                          int /* lineno */,
                          LogLevelT const& /* unused */) noexcept
    {}
    template<typename T>
    __attribute__((noinline)) NullStream& operator<<(T& /* */) noexcept { return *this; }

    __attribute__((noinline)) NullStream& operator<<(int /**/) noexcept { return *this; }
};
#define LOG(Level) NullStream<LogLevel>(__FILE__, __LINE__, Level)
#endif
