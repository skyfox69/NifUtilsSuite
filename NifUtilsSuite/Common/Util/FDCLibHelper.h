#pragma once

#include <vector>

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

int strcasecmp(const char *str1, const char *str2);

int strncasecmp(const char *str1, const char *str2, int n);

const char *strcasestr(const char *s, const char *pattern);


unsigned int bgr2argb(const unsigned int bgr);

unsigned int argb2bgr(const unsigned int argb);


unsigned int strexplode(const char* pString, const char* pDelim, vector<string>& vecOut);

#ifdef __cplusplus
}
#endif