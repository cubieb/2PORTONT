#include "multilang.h"

unsigned char g_language_state = 0;
unsigned char g_language_state_prev = 0;
const char **strtbl;

const char *multilang (int tag)
{
	const char *strudf = "String is undefined!";

	if (tag >= LANG_TAG_MAX) {
		fprintf (stderr, "The tag is out of range!!(%d)\n", tag);
		return strudf;
	}

	if (strtbl[tag] == NULL) {
		fprintf (stderr, "The string indexed is NULL!!(%d)\n", tag);
		return strudf;
	}

	//printf ("Lan_%d => strtbl[%d] = %s\n", g_language_state, tag, strtbl[tag]);

	return strtbl[tag];
}

