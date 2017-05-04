#include <stdio.h>
#include <string.h>

/*-- Local inlcude files --*/
#include "utility.h"
#include "../webs.h"

int printerList(int eid, request * wp, int argc, char **argv)
{
	int i;
	size_t len, nr_newline;
	char str[BUFSIZ / 4];

	memset(str, 0, sizeof(str));
	getPrinterList(str, sizeof(str));

	/* convert '\n' in str to "<br>" */
	for (len = 0, nr_newline = 0; str[len]; len++) {
		if (str[len] == '\n')
			nr_newline++;
	}

	for (i = len + 3 * nr_newline; i >= 0; len--, i--) {
		if (str[len] == '\n') {
			i -= 3;
			memcpy(str + i, "<br>", 4);
		} else {
			str[i] = str[len];
		}
	}

	boaWrite(wp, "%s", str);

	return 0;
}

