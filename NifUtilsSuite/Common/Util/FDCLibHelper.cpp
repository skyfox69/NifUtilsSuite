#include "FDCLibHelper.h"
#include <string.h>
#include <ctype.h>

//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999, 2000, 2001 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//

//*****************************************************************************
//
const char *
strcasestr(const char *s, const char *pattern)
{
    int		length = strlen(pattern);

    while (*s)
    {
	if (strncasecmp(s, pattern, length) == 0)
	    return s;
	s++;
    }
    return 0;
}

//*****************************************************************************
//
int strncasecmp(const char *str1, const char *str2, int n)
{
    if (!str1 && !str2)
	return 0;
    if (!str1)
	return 1;
    if (!str2)
	return -1;
    if (n < 0)
	return 0;
    while (n &&
	   *str1 &&
	   *str2 &&
	   tolower((unsigned char)*str1) == tolower((unsigned char)*str2))
    {
	str1++;
	str2++;
	n--;
    }

    return n == 0 ? 0 :
	tolower((unsigned char)*str1) - tolower((unsigned char)*str2);
}

//*****************************************************************************
//
int strcasecmp(const char *str1, const char *str2)
{
    if (!str1 && !str2)
	return 0;
    if (!str1)
	return 1;
    if (!str2)
	return -1;
    while (*str1 &&
	   *str2 &&
	   tolower((unsigned char)*str1) == tolower((unsigned char)*str2))
    {
	str1++;
	str2++;
    }

    return tolower((unsigned char)*str1) - tolower((unsigned char)*str2);
}

//*****************************************************************************
//
unsigned int bgr2argb(const unsigned int bgr)
{
	return (0xFF000000 | ((bgr & 0x000000FF) << 16) | (bgr & 0x0000FF00) | ((bgr & 0x00FF0000) >> 16));
}

//*****************************************************************************
//
unsigned int argb2bgr(const unsigned int argb)
{
	return (((argb & 0x000000FF) << 16) | (argb & 0x0000FF00) | ((argb & 0x00FF0000) >> 16));
}
