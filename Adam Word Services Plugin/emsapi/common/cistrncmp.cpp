/* cistrncmp.h */

#include "cistrncmp.h"

int32 cistrncmp(const char *str1, const char *str2, int32 max)
{
	char		c1;
	char		c2;
	int32		loop;

	for (loop = 0; loop < max; loop++)
	{
		c1 = *str1++;
		if ((c1 >= 'A') && (c1 <= 'Z'))	c1 += ('a' - 'A');
		c2 = *str2++;

		if ((c2 >= 'A') && (c2 <= 'Z'))	c2 += ('a' - 'A');
		if (c1 == c2) {}
		else if (c1 < c2)			return -1;
		else if ((c1 > c2) || (!c2))	return 1;
	}
	return 0;
}
