#include <string>

#include "format.h"

#define HOUR 3600
#define MIN 60

using std::string;

// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS

// Reference: https://www.csestack.org/online-tool-to-convert-seconds-to-hours-minutes-hhmmss/
// Reference: https://stackoverflow.com/a/14011098/975592
string Format::ElapsedTime(long seconds) {
    char s[25];
    int hr=(int)(seconds/3600);
    int min=((int)(seconds/60))%60;
    int sec=(int)(seconds%60);

    sprintf(s, "%02d:%02d:%02d", hr, min, sec);

    return s;
}