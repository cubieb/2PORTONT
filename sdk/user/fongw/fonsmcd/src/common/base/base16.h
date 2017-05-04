#ifndef _BASE16_H_
#define _BASE16_H_
#ifdef __cplusplus
extern "C" {
#endif
int B16_ascii_to_nibble(char c);
void B16_byte_to_ascii(char *nib0, char *nib1, unsigned char byte);
int B16_encode(char *source, char *destination, int size_source, int size_destination);
int B16_decode(char *source, char *destination, int size_source, int size_destination);
#ifdef __cplusplus
}
#endif
#endif
