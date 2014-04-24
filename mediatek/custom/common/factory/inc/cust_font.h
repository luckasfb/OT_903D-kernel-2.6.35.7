
#ifndef FTM_CUST_FONT_H
#define FTM_CUST_FONT_H

#include "cust.h"

#if defined(FEATURE_FTM_FONT_10x18)
#define CHAR_WIDTH      10
#define CHAR_HEIGHT     18
#else
#error "font size is not definied"
#endif

#endif /* FTM_CUST_H */
