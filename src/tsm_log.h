#pragma once
#include <iostream>
struct null_ostream : public std::ostream
{
    struct null_buffer : public std::streambuf
    {
        int overflow(int c) { return c; }
    };

    null_ostream()
      : std::ostream(&m_sb)
    {}

  private:
    null_buffer m_sb;
};

#ifndef LOG
#define LOG(level) null_ostream()
#endif

#ifndef DLOG
#define DLOG(level) null_ostream()
#endif
