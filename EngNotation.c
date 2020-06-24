//
// EngNotation.c
// EngineeringNotationFormatter
//
// Copyright (C) 2005-2009 by Jukka Korpela
// Copyright (C) 2009-2013 by David Hoerl
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "EngNotation.h"


/* 
 * Smallest power of ten for which there is a prefix defined.
 * If the set of prefixes will be extended, change this constant
 * and update the table "prefix".
 */

#ifndef ENG_FORMAT_MICRO_GLYPH
//# define ENG_FORMAT_MICRO_GLYPH "µ"
#define ENG_FORMAT_MICRO_GLYPH "u"
#endif

#define NUM_PREFIX 17
static char *prefix[NUM_PREFIX] = {
  "y", "z", "a", "f",
  "p", "n", ENG_FORMAT_MICRO_GLYPH, "m",
  "",  "k", "M", "G",
  "T", "P", "E", "Z",
  "Y"
};
static char *reversePrefix[NUM_PREFIX] = {
 "e-24", "e-21", "e-18", "e-15",
 "e-12", "e-9", "e-6", "e-3",
 "e0", "e3", "e6", "e9",
 "e12", "e15", "e18", "e21",
 "e24"
};

const int prefix_count = NUM_PREFIX;
const int prefix_start = -24;
const int prefix_end   = prefix_start + 3 * ( prefix_count - 1 );

char *to_engineering_string(double value, unsigned int digits, bool exponential)
{
	bool is_signed = signbit(value);
	const char *sign = is_signed ? "-" : "";
	
	int classify = fpclassify(value);
	if(classify != FP_NORMAL) {
		switch(classify) {
		case FP_SUBNORMAL:
		case FP_ZERO:
		default:
		{
			char *s;
			if(exponential) {
				asprintf(&s, "%s%.*fe0", sign, digits-1, 0.0);
			} else {
				asprintf(&s, "%s%.*f", sign, digits-1, 0.0);
			}
			return s;
		}
		case FP_INFINITE:
			return strdup("INFINITE");
			break;
		case FP_NAN:
			return strdup("NaN");
			break;
		}
	}
	if(is_signed) {
		value = fabs(value);
	}

	if(digits < 3) {
		digits = 3;
	} else
	if(digits > 9) {
		digits = 9;
	}

	// correctly round to desired precision
	long expof10 = lrint( floor( log10(value) ) );
	if(expof10 > 0) {
		expof10 = (expof10/3)*3;
	} else {
		expof10 = ((-expof10+3)/3)*(-3);
	}

	value *= pow(10.0, -expof10);

	long lintgr, lfract;
	{
		double intgr, fract;
		
		fract = modf(value, &intgr);
		lintgr = lrint(intgr);
	
		if(lintgr >= 1000) {
			digits -= 3;				// fractional digits
		} else
		if(lintgr >= 100) {
			digits -= 2;				// fractional digits
		} else
		if(lintgr >= 10) {
			digits -= 1;				// fractional digits
		} else
		if(lintgr >= 1) {
		} else {
			//assert(!"Impossible to get < 1 unless the fractional part is 1!");
			digits += 1;				// fractional digits
		}
		
		// how much to multiple the fraction to get it to round the one-off value
		double fractMult = pow(10.0, (int)digits - 1);
		long lfractMult = lrint(fractMult);

		// round the fraction to the correct number of places
		fract *= fractMult;
		lfract = lrint(fract);

		// did the rounding the fractional component cause an increase in the integral value?
		if(lfract >= lfractMult) {
			lfract -= lfractMult;			// remove overflow value

			long nlintgr = lintgr + 1;
			if( (lintgr < 1000 && nlintgr >= 1000) || (lintgr < 100 && nlintgr >= 100) || (lintgr < 10 && nlintgr >= 10) || (lintgr < 1 && nlintgr >= 1) ) {
				lfract /= 10;
				fractMult /= 10;
				digits -= 1;
			}
			lintgr = nlintgr;				// rounded up, so increase integral part
		}

		if(lintgr >= 1000) {
			expof10 += 3;
			digits += 3;

			long fullVal = lrint(lintgr*fractMult) + lfract;
			long fullMult = lrintf(1000.0 * fractMult);

			lintgr = fullVal / fullMult;
			lfract = fullVal - (lintgr * fullMult);
		}
	}

	--digits;
	const char *decimal_str = digits ? "." : "";
	
	char *result;
	if(exponential || (expof10 < prefix_start) || (expof10 > prefix_end)) {
		asprintf(&result, "%s%ld%s%0.*lde%ld", sign, lintgr, decimal_str, digits, lfract, expof10);
	} else {
		const char *s = prefix[(expof10-prefix_start)/3];
		asprintf(&result, "%s%ld%s%0.*ld%s%s", sign, lintgr, decimal_str, digits, lfract, *s ? " " : "", s);
	}
	return result;
}