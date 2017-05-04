#include <base/base16.h>
#include <sys/types.h>

int B16_ascii_to_nibble(char c)
{
	if ((c >= 'A') && (c <= 'F'))
		return c - 'A' + 10;
	else if ((c >= 'a') && (c <= 'f'))
		return c - 'a' + 10;
	else if ((c >= '0') && (c <= '9'))
		return c - '0';
	else
		return -1;
	return 0;
}

void B16_byte_to_ascii(char *nib0, char *nib1, unsigned char byte)
{
	int first, second;

	first = byte / 16;
	second = byte % 16;

	if (first >= 10) {
		*nib0 = (char)(first - 10 + 'a');
	} else {
		*nib0 = (char)(first + '0');
	}
	if (second >= 10) {
		*nib1 = (char)(second - 10 + 'a');
	} else {
		*nib1 = (char)(second + '0');
	}
}

int B16_encode(char *source, char *destination, int size_source, int size_destination)
{
	int i;
	if (size_destination < (size_source * 2))
		return -1;
	for (i = 0; i < size_source; i++)
		B16_byte_to_ascii(&(destination[(i*2)]), &(destination[(i*2)+1]), source[i]);
	return (size_source * 2);
}

int B16_decode(char *source, char *destination, int size_source, int size_destination)
{
	int i, n1,n2,t;

	if (size_source % 2)
		return -1;
	if (size_destination < (size_source / 2))
		return -1;
	for (i = 0; i < (size_source/2); i++) {
		if ((n1 = B16_ascii_to_nibble(source[(i*2)])) == -1) 
			return 3;
		if ((n2 = B16_ascii_to_nibble(source[(i*2)+1])) == -1)
			return 4;
		t = (n1 * 16) + n2;
		if ((t < 0) || (t > 255))
			return 6;
		destination[i] = (unsigned char)t;
	}
	return (size_source / 2);
}

