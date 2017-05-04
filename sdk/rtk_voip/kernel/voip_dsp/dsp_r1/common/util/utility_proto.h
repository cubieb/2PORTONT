#ifndef UTILITY_PROTO_H
#define UTILITY_PROTO_H

//ASM function prototype
void Copys(Word16 x[], Word16 y[], Word16 L);
void Pre_Process2s(uint32 sid, Word16 *signal, Word16 lg);	// in util/pre_procs.S
/* The Len parameter in memset32/64s and memcpy32/64s is counted by 2-bytes unit*/
void memset32s(Word32 *Dst, int c, Word32 Len);
void memset64s(Word32 *Dst, int c, Word32 Len);
void memcpy16s(Word16 *Dst, Word16 *Src, Word32 Len);
void memcpy32s(Word32 *Dst, Word32 *Src, Word32 Len);
void memcpy64s(Word32 *Dst, Word32 *Src, Word32 Len);


//C function prototype
void Set_zeroc(Word16 x[], Word16 L);
void Copyc(Word16 x[], Word16 y[], Word16 L);
void Pre_Process2c(uint32 sid, Word16 *signal, Word16 lg);	// in util/pre_proc.c

#endif	// UTILITY_PROTO_H

