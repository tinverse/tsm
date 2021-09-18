#pragma once
#include <iostream>
enum LogLevel
{
    DEBUG,
    INFO,
    WARNING,
    ERROR,
};

struct null_ostream : public std::ostream
{
    struct null_buffer : public std::streambuf
    {
        int overflow(int c) override { return c; }
    };

    explicit null_ostream(LogLevel const& /* unused */)
      : std::ostream(&sb_)
    {}

  private:
    null_buffer sb_;
};

#ifndef USE_EXTERNAL_LOG
using LOG = null_ostream;
#endif
