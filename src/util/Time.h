#ifndef TIME_H
#define TIME_H

#include "std.h"

namespace jian {

class Time {
public:
    static string time();
    static int year();
    static int mon();
    static int date();
    static int hour();
    static int min();
    static int sec();
};

}

#endif
