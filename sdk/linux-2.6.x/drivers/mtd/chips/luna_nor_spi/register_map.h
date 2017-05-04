#ifndef _REGISTER_MAP_H_
#define _REGISTER_MAP_H_
/*-----------------------------------------------------
 Extraced from ./chip_index.xml: v0.0.13|2014-06-05
 RP Last modified at Thu Jun  5 14:19:18 2014
-----------------------------------------------------*/
typedef volatile unsigned int * regval;

#define RLOAD(_var, _reg)  _reg##_T _var = { .v = _reg##rv }
#define RTYPE(_var, _reg)  _reg##_T _var = { .v = 0 }
#define RSTORE(_var, _reg) _reg##rv = _var.v

/* for __VA_NARG__ */
#define PP_NARG(...)  PP_NARG_(__VA_ARGS__,PP_RSEQ_N())
#define PP_NARG_(...) PP_ARG_N(__VA_ARGS__)
#define PP_ARG_N(_01,_02,_03,_04,_05,_06,_07,_08,_09,_10, \
                 _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
                 _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
                 _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
                 _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
                 _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
                 _61,_62,_63,  N,...) N
#define PP_RSEQ_N()                      63, _rset62, 61, _rset60, \
  59, _rset58, 57, _rset56, 55, _rset54, 53, _rset52, 51, _rset50, \
  49, _rset48, 47, _rset46, 45, _rset44, 43, _rset42, 41, _rset40, \
  39, _rset38, 37, _rset36, 35, _rset34, 33, _rset32, 31, _rset30, \
  29, _rset28, 27, _rset26, 25, _rset24, 23, _rset22, 21, _rset20, \
  19, _rset18, 17, _rset16, 15, _rset14, 13, _rset12, 11, _rset10, \
  09,  _rset8, 07,  _rset6, 05,  _rset4, 03,  _rset2, 01,      00
#define rset(rtype, ival, ...) do {            \
		rtype##_T dummyr = { .v = ival };          \
		PP_NARG(__VA_ARGS__)(dummyr, __VA_ARGS__); \
		rtype##rv = dummyr.v;                      \
	} while(0)
#define _rset2(var, \
               f00, v00) do { \
		var.f.f00 = v00; \
	} while(0)

#define _rset4(var, \
               f00, v00, f01, v01) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
	} while(0)

#define _rset6(var, \
               f00, v00, f01, v01, f02, v02) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
	} while(0)

#define _rset8(var, \
               f00, v00, f01, v01, f02, v02, f03, v03) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
	} while(0)

#define _rset10(var, \
                f00, v00, f01, v01, f02, v02, f03, v03, \
                f04, v04) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
	} while(0)

#define _rset12(var, \
                f00, v00, f01, v01, f02, v02, f03, v03, \
                f04, v04, f05, v05) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
	} while(0)

#define _rset14(var, \
                f00, v00, f01, v01, f02, v02, f03, v03, \
                f04, v04, f05, v05, f06, v06) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
	} while(0)

#define _rset16(var, \
                f00, v00, f01, v01, f02, v02, f03, v03, \
                f04, v04, f05, v05, f06, v06, f07, v07) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
	} while(0)

#define _rset18(var, \
                f00, v00, f01, v01, f02, v02, f03, v03, \
                f04, v04, f05, v05, f06, v06, f07, v07, \
                f08, v08) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
	} while(0)

#define _rset20(var, \
                f00, v00, f01, v01, f02, v02, f03, v03, \
                f04, v04, f05, v05, f06, v06, f07, v07, \
                f08, v08, f09, v09) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
	} while(0)

#define _rset22(var, \
                f00, v00, f01, v01, f02, v02, f03, v03, \
                f04, v04, f05, v05, f06, v06, f07, v07, \
                f08, v08, f09, v09, f10, v10) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
	} while(0)

#define _rset24(var, \
                f00, v00, f01, v01, f02, v02, f03, v03, \
                f04, v04, f05, v05, f06, v06, f07, v07, \
                f08, v08, f09, v09, f10, v10, f11, v11) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
		var.f.f11 = v11; \
	} while(0)

#define _rset26(var, \
                f00, v00, f01, v01, f02, v02, f03, v03, \
                f04, v04, f05, v05, f06, v06, f07, v07, \
                f08, v08, f09, v09, f10, v10, f11, v11, \
                f12, v12) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
		var.f.f11 = v11; \
		var.f.f12 = v12; \
	} while(0)

#define _rset28(var, \
                f00, v00, f01, v01, f02, v02, f03, v03, \
                f04, v04, f05, v05, f06, v06, f07, v07, \
                f08, v08, f09, v09, f10, v10, f11, v11, \
                f12, v12, f13, v13) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
		var.f.f11 = v11; \
		var.f.f12 = v12; \
		var.f.f13 = v13; \
	} while(0)

#define _rset30(var, \
                f00, v00, f01, v01, f02, v02, f03, v03, \
                f04, v04, f05, v05, f06, v06, f07, v07, \
                f08, v08, f09, v09, f10, v10, f11, v11, \
                f12, v12, f13, v13, f14, v14) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
		var.f.f11 = v11; \
		var.f.f12 = v12; \
		var.f.f13 = v13; \
		var.f.f14 = v14; \
	} while(0)

#define _rset32(var, \
                f00, v00, f01, v01, f02, v02, f03, v03, \
                f04, v04, f05, v05, f06, v06, f07, v07, \
                f08, v08, f09, v09, f10, v10, f11, v11, \
                f12, v12, f13, v13, f14, v14, f15, v15) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
		var.f.f11 = v11; \
		var.f.f12 = v12; \
		var.f.f13 = v13; \
		var.f.f14 = v14; \
		var.f.f15 = v15; \
	} while(0)

#define _rset34(var, \
                f00, v00, f01, v01, f02, v02, f03, v03, \
                f04, v04, f05, v05, f06, v06, f07, v07, \
                f08, v08, f09, v09, f10, v10, f11, v11, \
                f12, v12, f13, v13, f14, v14, f15, v15, \
                f16, v16) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
		var.f.f11 = v11; \
		var.f.f12 = v12; \
		var.f.f13 = v13; \
		var.f.f14 = v14; \
		var.f.f15 = v15; \
		var.f.f16 = v16; \
	} while(0)

#define _rset36(var, \
                f00, v00, f01, v01, f02, v02, f03, v03, \
                f04, v04, f05, v05, f06, v06, f07, v07, \
                f08, v08, f09, v09, f10, v10, f11, v11, \
                f12, v12, f13, v13, f14, v14, f15, v15, \
                f16, v16, f17, v17) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
		var.f.f11 = v11; \
		var.f.f12 = v12; \
		var.f.f13 = v13; \
		var.f.f14 = v14; \
		var.f.f15 = v15; \
		var.f.f16 = v16; \
		var.f.f17 = v17; \
	} while(0)

#define _rset38(var, \
                f00, v00, f01, v01, f02, v02, f03, v03, \
                f04, v04, f05, v05, f06, v06, f07, v07, \
                f08, v08, f09, v09, f10, v10, f11, v11, \
                f12, v12, f13, v13, f14, v14, f15, v15, \
                f16, v16, f17, v17, f18, v18) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
		var.f.f11 = v11; \
		var.f.f12 = v12; \
		var.f.f13 = v13; \
		var.f.f14 = v14; \
		var.f.f15 = v15; \
		var.f.f16 = v16; \
		var.f.f17 = v17; \
		var.f.f18 = v18; \
	} while(0)

#define _rset40(var, \
                f00, v00, f01, v01, f02, v02, f03, v03, \
                f04, v04, f05, v05, f06, v06, f07, v07, \
                f08, v08, f09, v09, f10, v10, f11, v11, \
                f12, v12, f13, v13, f14, v14, f15, v15, \
                f16, v16, f17, v17, f18, v18, f19, v19) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
		var.f.f11 = v11; \
		var.f.f12 = v12; \
		var.f.f13 = v13; \
		var.f.f14 = v14; \
		var.f.f15 = v15; \
		var.f.f16 = v16; \
		var.f.f17 = v17; \
		var.f.f18 = v18; \
		var.f.f19 = v19; \
	} while(0)

#define _rset42(var, \
                f00, v00, f01, v01, f02, v02, f03, v03, \
                f04, v04, f05, v05, f06, v06, f07, v07, \
                f08, v08, f09, v09, f10, v10, f11, v11, \
                f12, v12, f13, v13, f14, v14, f15, v15, \
                f16, v16, f17, v17, f18, v18, f19, v19, \
                f20, v20) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
		var.f.f11 = v11; \
		var.f.f12 = v12; \
		var.f.f13 = v13; \
		var.f.f14 = v14; \
		var.f.f15 = v15; \
		var.f.f16 = v16; \
		var.f.f17 = v17; \
		var.f.f18 = v18; \
		var.f.f19 = v19; \
		var.f.f20 = v20; \
	} while(0)

#define _rset44(var, \
                f00, v00, f01, v01, f02, v02, f03, v03, \
                f04, v04, f05, v05, f06, v06, f07, v07, \
                f08, v08, f09, v09, f10, v10, f11, v11, \
                f12, v12, f13, v13, f14, v14, f15, v15, \
                f16, v16, f17, v17, f18, v18, f19, v19, \
                f20, v20, f21, v21) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
		var.f.f11 = v11; \
		var.f.f12 = v12; \
		var.f.f13 = v13; \
		var.f.f14 = v14; \
		var.f.f15 = v15; \
		var.f.f16 = v16; \
		var.f.f17 = v17; \
		var.f.f18 = v18; \
		var.f.f19 = v19; \
		var.f.f20 = v20; \
		var.f.f21 = v21; \
	} while(0)

#define _rset46(var, \
                f00, v00, f01, v01, f02, v02, f03, v03, \
                f04, v04, f05, v05, f06, v06, f07, v07, \
                f08, v08, f09, v09, f10, v10, f11, v11, \
                f12, v12, f13, v13, f14, v14, f15, v15, \
                f16, v16, f17, v17, f18, v18, f19, v19, \
                f20, v20, f21, v21, f22, v22) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
		var.f.f11 = v11; \
		var.f.f12 = v12; \
		var.f.f13 = v13; \
		var.f.f14 = v14; \
		var.f.f15 = v15; \
		var.f.f16 = v16; \
		var.f.f17 = v17; \
		var.f.f18 = v18; \
		var.f.f19 = v19; \
		var.f.f20 = v20; \
		var.f.f21 = v21; \
		var.f.f22 = v22; \
	} while(0)

#define _rset48(var, \
                f00, v00, f01, v01, f02, v02, f03, v03, \
                f04, v04, f05, v05, f06, v06, f07, v07, \
                f08, v08, f09, v09, f10, v10, f11, v11, \
                f12, v12, f13, v13, f14, v14, f15, v15, \
                f16, v16, f17, v17, f18, v18, f19, v19, \
                f20, v20, f21, v21, f22, v22, f23, v23) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
		var.f.f11 = v11; \
		var.f.f12 = v12; \
		var.f.f13 = v13; \
		var.f.f14 = v14; \
		var.f.f15 = v15; \
		var.f.f16 = v16; \
		var.f.f17 = v17; \
		var.f.f18 = v18; \
		var.f.f19 = v19; \
		var.f.f20 = v20; \
		var.f.f21 = v21; \
		var.f.f22 = v22; \
		var.f.f23 = v23; \
	} while(0)

#define _rset50(var, \
                f00, v00, f01, v01, f02, v02, f03, v03, \
                f04, v04, f05, v05, f06, v06, f07, v07, \
                f08, v08, f09, v09, f10, v10, f11, v11, \
                f12, v12, f13, v13, f14, v14, f15, v15, \
                f16, v16, f17, v17, f18, v18, f19, v19, \
                f20, v20, f21, v21, f22, v22, f23, v23, \
                f24, v24) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
		var.f.f11 = v11; \
		var.f.f12 = v12; \
		var.f.f13 = v13; \
		var.f.f14 = v14; \
		var.f.f15 = v15; \
		var.f.f16 = v16; \
		var.f.f17 = v17; \
		var.f.f18 = v18; \
		var.f.f19 = v19; \
		var.f.f20 = v20; \
		var.f.f21 = v21; \
		var.f.f22 = v22; \
		var.f.f23 = v23; \
		var.f.f24 = v24; \
	} while(0)

#define _rset52(var, \
                f00, v00, f01, v01, f02, v02, f03, v03, \
                f04, v04, f05, v05, f06, v06, f07, v07, \
                f08, v08, f09, v09, f10, v10, f11, v11, \
                f12, v12, f13, v13, f14, v14, f15, v15, \
                f16, v16, f17, v17, f18, v18, f19, v19, \
                f20, v20, f21, v21, f22, v22, f23, v23, \
                f24, v24, f25, v25) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
		var.f.f11 = v11; \
		var.f.f12 = v12; \
		var.f.f13 = v13; \
		var.f.f14 = v14; \
		var.f.f15 = v15; \
		var.f.f16 = v16; \
		var.f.f17 = v17; \
		var.f.f18 = v18; \
		var.f.f19 = v19; \
		var.f.f20 = v20; \
		var.f.f21 = v21; \
		var.f.f22 = v22; \
		var.f.f23 = v23; \
		var.f.f24 = v24; \
		var.f.f25 = v25; \
	} while(0)

#define _rset54(var, \
                f00, v00, f01, v01, f02, v02, f03, v03, \
                f04, v04, f05, v05, f06, v06, f07, v07, \
                f08, v08, f09, v09, f10, v10, f11, v11, \
                f12, v12, f13, v13, f14, v14, f15, v15, \
                f16, v16, f17, v17, f18, v18, f19, v19, \
                f20, v20, f21, v21, f22, v22, f23, v23, \
                f24, v24, f25, v25, f26, v26) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
		var.f.f11 = v11; \
		var.f.f12 = v12; \
		var.f.f13 = v13; \
		var.f.f14 = v14; \
		var.f.f15 = v15; \
		var.f.f16 = v16; \
		var.f.f17 = v17; \
		var.f.f18 = v18; \
		var.f.f19 = v19; \
		var.f.f20 = v20; \
		var.f.f21 = v21; \
		var.f.f22 = v22; \
		var.f.f23 = v23; \
		var.f.f24 = v24; \
		var.f.f25 = v25; \
		var.f.f26 = v26; \
	} while(0)

#define _rset56(var, \
                f00, v00, f01, v01, f02, v02, f03, v03, \
                f04, v04, f05, v05, f06, v06, f07, v07, \
                f08, v08, f09, v09, f10, v10, f11, v11, \
                f12, v12, f13, v13, f14, v14, f15, v15, \
                f16, v16, f17, v17, f18, v18, f19, v19, \
                f20, v20, f21, v21, f22, v22, f23, v23, \
                f24, v24, f25, v25, f26, v26, f27, v27) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
		var.f.f11 = v11; \
		var.f.f12 = v12; \
		var.f.f13 = v13; \
		var.f.f14 = v14; \
		var.f.f15 = v15; \
		var.f.f16 = v16; \
		var.f.f17 = v17; \
		var.f.f18 = v18; \
		var.f.f19 = v19; \
		var.f.f20 = v20; \
		var.f.f21 = v21; \
		var.f.f22 = v22; \
		var.f.f23 = v23; \
		var.f.f24 = v24; \
		var.f.f25 = v25; \
		var.f.f26 = v26; \
		var.f.f27 = v27; \
	} while(0)

#define _rset58(var, \
                f00, v00, f01, v01, f02, v02, f03, v03, \
                f04, v04, f05, v05, f06, v06, f07, v07, \
                f08, v08, f09, v09, f10, v10, f11, v11, \
                f12, v12, f13, v13, f14, v14, f15, v15, \
                f16, v16, f17, v17, f18, v18, f19, v19, \
                f20, v20, f21, v21, f22, v22, f23, v23, \
                f24, v24, f25, v25, f26, v26, f27, v27, \
                f28, v28) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
		var.f.f11 = v11; \
		var.f.f12 = v12; \
		var.f.f13 = v13; \
		var.f.f14 = v14; \
		var.f.f15 = v15; \
		var.f.f16 = v16; \
		var.f.f17 = v17; \
		var.f.f18 = v18; \
		var.f.f19 = v19; \
		var.f.f20 = v20; \
		var.f.f21 = v21; \
		var.f.f22 = v22; \
		var.f.f23 = v23; \
		var.f.f24 = v24; \
		var.f.f25 = v25; \
		var.f.f26 = v26; \
		var.f.f27 = v27; \
		var.f.f28 = v28; \
	} while(0)

#define _rset60(var, \
                f00, v00, f01, v01, f02, v02, f03, v03, \
                f04, v04, f05, v05, f06, v06, f07, v07, \
                f08, v08, f09, v09, f10, v10, f11, v11, \
                f12, v12, f13, v13, f14, v14, f15, v15, \
                f16, v16, f17, v17, f18, v18, f19, v19, \
                f20, v20, f21, v21, f22, v22, f23, v23, \
                f24, v24, f25, v25, f26, v26, f27, v27, \
                f28, v28, f29, v29) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
		var.f.f11 = v11; \
		var.f.f12 = v12; \
		var.f.f13 = v13; \
		var.f.f14 = v14; \
		var.f.f15 = v15; \
		var.f.f16 = v16; \
		var.f.f17 = v17; \
		var.f.f18 = v18; \
		var.f.f19 = v19; \
		var.f.f20 = v20; \
		var.f.f21 = v21; \
		var.f.f22 = v22; \
		var.f.f23 = v23; \
		var.f.f24 = v24; \
		var.f.f25 = v25; \
		var.f.f26 = v26; \
		var.f.f27 = v27; \
		var.f.f28 = v28; \
		var.f.f29 = v29; \
	} while(0)

#define _rset62(var, \
                f00, v00, f01, v01, f02, v02, f03, v03, \
                f04, v04, f05, v05, f06, v06, f07, v07, \
                f08, v08, f09, v09, f10, v10, f11, v11, \
                f12, v12, f13, v13, f14, v14, f15, v15, \
                f16, v16, f17, v17, f18, v18, f19, v19, \
                f20, v20, f21, v21, f22, v22, f23, v23, \
                f24, v24, f25, v25, f26, v26, f27, v27, \
                f28, v28, f29, v29, f30, v30) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
		var.f.f11 = v11; \
		var.f.f12 = v12; \
		var.f.f13 = v13; \
		var.f.f14 = v14; \
		var.f.f15 = v15; \
		var.f.f16 = v16; \
		var.f.f17 = v17; \
		var.f.f18 = v18; \
		var.f.f19 = v19; \
		var.f.f20 = v20; \
		var.f.f21 = v21; \
		var.f.f22 = v22; \
		var.f.f23 = v23; \
		var.f.f24 = v24; \
		var.f.f25 = v25; \
		var.f.f26 = v26; \
		var.f.f27 = v27; \
		var.f.f28 = v28; \
		var.f.f29 = v29; \
		var.f.f30 = v30; \
	} while(0)

/*-----------------------------------------------------
 Extraced from file_LXB_CTRL.xml
-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int no_use31:32; //0
	} f;
	unsigned int v;
} LXB_CTRL_NO_USE_0000_T;
#define LXB_CTRL_NO_USE_0000rv (*((regval)0xb8000000))
#define LXB_CTRL_NO_USE_0000dv (0x00000000)
#define RMOD_LXB_CTRL_NO_USE_0000(...) rset(LXB_CTRL_NO_USE_0000, LXB_CTRL_NO_USE_0000rv, __VA_ARGS__)
#define RIZS_LXB_CTRL_NO_USE_0000(...) rset(LXB_CTRL_NO_USE_0000, 0, __VA_ARGS__)
#define RFLD_LXB_CTRL_NO_USE_0000(fld) (*((const volatile LXB_CTRL_NO_USE_0000_T *)0xb8000000)).f.fld

typedef union {
	struct {
		unsigned int no_use31:24; //0
		unsigned int mbz_0:8; //0
	} f;
	unsigned int v;
} LXB_CTRL_NO_USE_0004_T;
#define LXB_CTRL_NO_USE_0004rv (*((regval)0xb8000004))
#define LXB_CTRL_NO_USE_0004dv (0x00000000)
#define RMOD_LXB_CTRL_NO_USE_0004(...) rset(LXB_CTRL_NO_USE_0004, LXB_CTRL_NO_USE_0004rv, __VA_ARGS__)
#define RIZS_LXB_CTRL_NO_USE_0004(...) rset(LXB_CTRL_NO_USE_0004, 0, __VA_ARGS__)
#define RFLD_LXB_CTRL_NO_USE_0004(fld) (*((const volatile LXB_CTRL_NO_USE_0004_T *)0xb8000004)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:28; //0
		unsigned int to_en:1; //1
		unsigned int to_lmt:3; //7
	} f;
	unsigned int v;
} BUS_TO_CTRL_T;
#define BUS_TO_CTRLrv (*((regval)0xb8000008))
#define BUS_TO_CTRLdv (0x0000000f)
#define RMOD_BUS_TO_CTRL(...) rset(BUS_TO_CTRL, BUS_TO_CTRLrv, __VA_ARGS__)
#define RIZS_BUS_TO_CTRL(...) rset(BUS_TO_CTRL, 0, __VA_ARGS__)
#define RFLD_BUS_TO_CTRL(fld) (*((const volatile BUS_TO_CTRL_T *)0xb8000008)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:21; //0
		unsigned int no_use10:3; //0
		unsigned int mbz_1:2; //0
		unsigned int lx2_arb_mode:2; //0
		unsigned int lx1_arb_mode:2; //0
		unsigned int lx0_arb_mode:2; //0
	} f;
	unsigned int v;
} LBPRICR_T;
#define LBPRICRrv (*((regval)0xb800000c))
#define LBPRICRdv (0x00000000)
#define RMOD_LBPRICR(...) rset(LBPRICR, LBPRICRrv, __VA_ARGS__)
#define RIZS_LBPRICR(...) rset(LBPRICR, 0, __VA_ARGS__)
#define RFLD_LBPRICR(fld) (*((const volatile LBPRICR_T *)0xb800000c)).f.fld

typedef union {
	struct {
		unsigned int slot7_own:4; //1
		unsigned int slot6_own:4; //0
		unsigned int slot5_own:4; //1
		unsigned int slot4_own:4; //0
		unsigned int slot3_own:4; //1
		unsigned int slot2_own:4; //0
		unsigned int slot1_own:4; //1
		unsigned int slot0_own:4; //0
	} f;
	unsigned int v;
} LX0_ARB_CFG0_T;
#define LX0_ARB_CFG0rv (*((regval)0xb8000010))
#define LX0_ARB_CFG0dv (0x10101010)
#define RMOD_LX0_ARB_CFG0(...) rset(LX0_ARB_CFG0, LX0_ARB_CFG0rv, __VA_ARGS__)
#define RIZS_LX0_ARB_CFG0(...) rset(LX0_ARB_CFG0, 0, __VA_ARGS__)
#define RFLD_LX0_ARB_CFG0(fld) (*((const volatile LX0_ARB_CFG0_T *)0xb8000010)).f.fld

typedef union {
	struct {
		unsigned int slot15_own:4; //1
		unsigned int slot14_own:4; //0
		unsigned int slot13_own:4; //1
		unsigned int slot12_own:4; //0
		unsigned int slot11_own:4; //1
		unsigned int slot10_own:4; //0
		unsigned int slot9_own:4; //1
		unsigned int slot8_own:4; //0
	} f;
	unsigned int v;
} LX0_ARB_CFG1_T;
#define LX0_ARB_CFG1rv (*((regval)0xb8000014))
#define LX0_ARB_CFG1dv (0x10101010)
#define RMOD_LX0_ARB_CFG1(...) rset(LX0_ARB_CFG1, LX0_ARB_CFG1rv, __VA_ARGS__)
#define RIZS_LX0_ARB_CFG1(...) rset(LX0_ARB_CFG1, 0, __VA_ARGS__)
#define RFLD_LX0_ARB_CFG1(fld) (*((const volatile LX0_ARB_CFG1_T *)0xb8000014)).f.fld

typedef union {
	struct {
		unsigned int slot23_own:4; //1
		unsigned int slot22_own:4; //0
		unsigned int slot21_own:4; //1
		unsigned int slot20_own:4; //0
		unsigned int slot19_own:4; //1
		unsigned int slot18_own:4; //0
		unsigned int slot17_own:4; //1
		unsigned int slot16_own:4; //0
	} f;
	unsigned int v;
} LX0_ARB_CFG2_T;
#define LX0_ARB_CFG2rv (*((regval)0xb8000018))
#define LX0_ARB_CFG2dv (0x10101010)
#define RMOD_LX0_ARB_CFG2(...) rset(LX0_ARB_CFG2, LX0_ARB_CFG2rv, __VA_ARGS__)
#define RIZS_LX0_ARB_CFG2(...) rset(LX0_ARB_CFG2, 0, __VA_ARGS__)
#define RFLD_LX0_ARB_CFG2(fld) (*((const volatile LX0_ARB_CFG2_T *)0xb8000018)).f.fld

typedef union {
	struct {
		unsigned int slot31_own:4; //1
		unsigned int slot30_own:4; //0
		unsigned int slot29_own:4; //1
		unsigned int slot28_own:4; //0
		unsigned int slot27_own:4; //1
		unsigned int slot26_own:4; //0
		unsigned int slot25_own:4; //1
		unsigned int slot24_own:4; //0
	} f;
	unsigned int v;
} LX0_ARB_CFG3_T;
#define LX0_ARB_CFG3rv (*((regval)0xb800001c))
#define LX0_ARB_CFG3dv (0x10101010)
#define RMOD_LX0_ARB_CFG3(...) rset(LX0_ARB_CFG3, LX0_ARB_CFG3rv, __VA_ARGS__)
#define RIZS_LX0_ARB_CFG3(...) rset(LX0_ARB_CFG3, 0, __VA_ARGS__)
#define RFLD_LX0_ARB_CFG3(fld) (*((const volatile LX0_ARB_CFG3_T *)0xb800001c)).f.fld

typedef union {
	struct {
		unsigned int slot7_own:4; //1
		unsigned int slot6_own:4; //0
		unsigned int slot5_own:4; //1
		unsigned int slot4_own:4; //0
		unsigned int slot3_own:4; //1
		unsigned int slot2_own:4; //0
		unsigned int slot1_own:4; //1
		unsigned int slot0_own:4; //0
	} f;
	unsigned int v;
} LX1_ARB_CFG0_T;
#define LX1_ARB_CFG0rv (*((regval)0xb8000020))
#define LX1_ARB_CFG0dv (0x10101010)
#define RMOD_LX1_ARB_CFG0(...) rset(LX1_ARB_CFG0, LX1_ARB_CFG0rv, __VA_ARGS__)
#define RIZS_LX1_ARB_CFG0(...) rset(LX1_ARB_CFG0, 0, __VA_ARGS__)
#define RFLD_LX1_ARB_CFG0(fld) (*((const volatile LX1_ARB_CFG0_T *)0xb8000020)).f.fld

typedef union {
	struct {
		unsigned int slot15_own:4; //1
		unsigned int slot14_own:4; //0
		unsigned int slot13_own:4; //1
		unsigned int slot12_own:4; //0
		unsigned int slot11_own:4; //1
		unsigned int slot10_own:4; //0
		unsigned int slot9_own:4; //1
		unsigned int slot8_own:4; //0
	} f;
	unsigned int v;
} LX1_ARB_CFG1_T;
#define LX1_ARB_CFG1rv (*((regval)0xb8000024))
#define LX1_ARB_CFG1dv (0x10101010)
#define RMOD_LX1_ARB_CFG1(...) rset(LX1_ARB_CFG1, LX1_ARB_CFG1rv, __VA_ARGS__)
#define RIZS_LX1_ARB_CFG1(...) rset(LX1_ARB_CFG1, 0, __VA_ARGS__)
#define RFLD_LX1_ARB_CFG1(fld) (*((const volatile LX1_ARB_CFG1_T *)0xb8000024)).f.fld

typedef union {
	struct {
		unsigned int slot23_own:4; //1
		unsigned int slot22_own:4; //0
		unsigned int slot21_own:4; //1
		unsigned int slot20_own:4; //0
		unsigned int slot19_own:4; //1
		unsigned int slot18_own:4; //0
		unsigned int slot17_own:4; //1
		unsigned int slot16_own:4; //0
	} f;
	unsigned int v;
} LX1_ARB_CFG2_T;
#define LX1_ARB_CFG2rv (*((regval)0xb8000028))
#define LX1_ARB_CFG2dv (0x10101010)
#define RMOD_LX1_ARB_CFG2(...) rset(LX1_ARB_CFG2, LX1_ARB_CFG2rv, __VA_ARGS__)
#define RIZS_LX1_ARB_CFG2(...) rset(LX1_ARB_CFG2, 0, __VA_ARGS__)
#define RFLD_LX1_ARB_CFG2(fld) (*((const volatile LX1_ARB_CFG2_T *)0xb8000028)).f.fld

typedef union {
	struct {
		unsigned int slot31_own:4; //1
		unsigned int slot30_own:4; //0
		unsigned int slot29_own:4; //1
		unsigned int slot28_own:4; //0
		unsigned int slot27_own:4; //1
		unsigned int slot26_own:4; //0
		unsigned int slot25_own:4; //1
		unsigned int slot24_own:4; //0
	} f;
	unsigned int v;
} LX1_ARB_CFG3_T;
#define LX1_ARB_CFG3rv (*((regval)0xb800002c))
#define LX1_ARB_CFG3dv (0x10101010)
#define RMOD_LX1_ARB_CFG3(...) rset(LX1_ARB_CFG3, LX1_ARB_CFG3rv, __VA_ARGS__)
#define RIZS_LX1_ARB_CFG3(...) rset(LX1_ARB_CFG3, 0, __VA_ARGS__)
#define RFLD_LX1_ARB_CFG3(fld) (*((const volatile LX1_ARB_CFG3_T *)0xb800002c)).f.fld

typedef union {
	struct {
		unsigned int slot7_own:4; //1
		unsigned int slot6_own:4; //0
		unsigned int slot5_own:4; //2
		unsigned int slot4_own:4; //1
		unsigned int slot3_own:4; //0
		unsigned int slot2_own:4; //2
		unsigned int slot1_own:4; //1
		unsigned int slot0_own:4; //0
	} f;
	unsigned int v;
} LX2_ARB_CFG0_T;
#define LX2_ARB_CFG0rv (*((regval)0xb8000030))
#define LX2_ARB_CFG0dv (0x10210210)
#define RMOD_LX2_ARB_CFG0(...) rset(LX2_ARB_CFG0, LX2_ARB_CFG0rv, __VA_ARGS__)
#define RIZS_LX2_ARB_CFG0(...) rset(LX2_ARB_CFG0, 0, __VA_ARGS__)
#define RFLD_LX2_ARB_CFG0(fld) (*((const volatile LX2_ARB_CFG0_T *)0xb8000030)).f.fld

typedef union {
	struct {
		unsigned int slot15_own:4; //1
		unsigned int slot14_own:4; //0
		unsigned int slot13_own:4; //2
		unsigned int slot12_own:4; //1
		unsigned int slot11_own:4; //0
		unsigned int slot10_own:4; //2
		unsigned int slot9_own:4; //1
		unsigned int slot8_own:4; //0
	} f;
	unsigned int v;
} LX2_ARB_CFG1_T;
#define LX2_ARB_CFG1rv (*((regval)0xb8000034))
#define LX2_ARB_CFG1dv (0x10210210)
#define RMOD_LX2_ARB_CFG1(...) rset(LX2_ARB_CFG1, LX2_ARB_CFG1rv, __VA_ARGS__)
#define RIZS_LX2_ARB_CFG1(...) rset(LX2_ARB_CFG1, 0, __VA_ARGS__)
#define RFLD_LX2_ARB_CFG1(fld) (*((const volatile LX2_ARB_CFG1_T *)0xb8000034)).f.fld

typedef union {
	struct {
		unsigned int slot23_own:4; //1
		unsigned int slot22_own:4; //0
		unsigned int slot21_own:4; //2
		unsigned int slot20_own:4; //1
		unsigned int slot19_own:4; //0
		unsigned int slot18_own:4; //2
		unsigned int slot17_own:4; //1
		unsigned int slot16_own:4; //0
	} f;
	unsigned int v;
} LX2_ARB_CFG2_T;
#define LX2_ARB_CFG2rv (*((regval)0xb8000038))
#define LX2_ARB_CFG2dv (0x10210210)
#define RMOD_LX2_ARB_CFG2(...) rset(LX2_ARB_CFG2, LX2_ARB_CFG2rv, __VA_ARGS__)
#define RIZS_LX2_ARB_CFG2(...) rset(LX2_ARB_CFG2, 0, __VA_ARGS__)
#define RFLD_LX2_ARB_CFG2(fld) (*((const volatile LX2_ARB_CFG2_T *)0xb8000038)).f.fld

typedef union {
	struct {
		unsigned int slot31_own:4; //1
		unsigned int slot30_own:4; //0
		unsigned int slot29_own:4; //2
		unsigned int slot28_own:4; //1
		unsigned int slot27_own:4; //0
		unsigned int slot26_own:4; //2
		unsigned int slot25_own:4; //1
		unsigned int slot24_own:4; //0
	} f;
	unsigned int v;
} LXBUS2_ARBIT_CFG_3_T;
#define LXBUS2_ARBIT_CFG_3rv (*((regval)0xb800003c))
#define LXBUS2_ARBIT_CFG_3dv (0x10210210)
#define RMOD_LXBUS2_ARBIT_CFG_3(...) rset(LXBUS2_ARBIT_CFG_3, LXBUS2_ARBIT_CFG_3rv, __VA_ARGS__)
#define RIZS_LXBUS2_ARBIT_CFG_3(...) rset(LXBUS2_ARBIT_CFG_3, 0, __VA_ARGS__)
#define RFLD_LXBUS2_ARBIT_CFG_3(fld) (*((const volatile LXBUS2_ARBIT_CFG_3_T *)0xb800003c)).f.fld

typedef union {
	struct {
		unsigned int bonding_magic_no:28; //0
		unsigned int mbz_0:2; //0
		unsigned int sw_bonding_mode:2; //0
	} f;
	unsigned int v;
} VOIP_BONDING_CTRL_T;
#define VOIP_BONDING_CTRLrv (*((regval)0xb8000040))
#define VOIP_BONDING_CTRLdv (0x00000000)
#define RMOD_VOIP_BONDING_CTRL(...) rset(VOIP_BONDING_CTRL, VOIP_BONDING_CTRLrv, __VA_ARGS__)
#define RIZS_VOIP_BONDING_CTRL(...) rset(VOIP_BONDING_CTRL, 0, __VA_ARGS__)
#define RFLD_VOIP_BONDING_CTRL(fld) (*((const volatile VOIP_BONDING_CTRL_T *)0xb8000040)).f.fld

typedef union {
	struct {
		unsigned int no_use31:21; //0
		unsigned int no_use10:1; //0
		unsigned int no_use09:1; //0
		unsigned int no_use08:1; //0
		unsigned int mbz_0:1; //0
		unsigned int no_use06:1; //0
		unsigned int no_use05:1; //0
		unsigned int cf_cksel_lx:1; //1
		unsigned int no_use03:1; //0
		unsigned int cf_ckse_ocp0:1; //1
		unsigned int rdy_for_pathch:1; //0
		unsigned int soc_init_rdy:1; //0
	} f;
	unsigned int v;
} SYS_STATUS_T;
#define SYS_STATUSrv (*((regval)0xb8000044))
#define SYS_STATUSdv (0x00000014)
#define RMOD_SYS_STATUS(...) rset(SYS_STATUS, SYS_STATUSrv, __VA_ARGS__)
#define RIZS_SYS_STATUS(...) rset(SYS_STATUS, 0, __VA_ARGS__)
#define RFLD_SYS_STATUS(fld) (*((const volatile SYS_STATUS_T *)0xb8000044)).f.fld

typedef union {
	struct {
		unsigned int ltoc:32; //0
	} f;
	unsigned int v;
} LXBUS_TO_CTRL_T;
#define LXBUS_TO_CTRLrv (*((regval)0xb8000048))
#define LXBUS_TO_CTRLdv (0x00000000)
#define RMOD_LXBUS_TO_CTRL(...) rset(LXBUS_TO_CTRL, LXBUS_TO_CTRLrv, __VA_ARGS__)
#define RIZS_LXBUS_TO_CTRL(...) rset(LXBUS_TO_CTRL, 0, __VA_ARGS__)
#define RFLD_LXBUS_TO_CTRL(fld) (*((const volatile LXBUS_TO_CTRL_T *)0xb8000048)).f.fld

/*-----------------------------------------------------
 Extraced from file_PAD_CTRL.xml
-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int mbz_0:12; //0
		unsigned int dram_type:2; //3
		unsigned int boot_up_type:2; //0
		unsigned int no_use15:1; //0
		unsigned int mbz_1:3; //0
		unsigned int no_use11:4; //0
		unsigned int mbz_2:1; //0
		unsigned int spi_flash_4b_en:1; //0
		unsigned int mbz_3:1; //0
		unsigned int scan_mode:1; //0
		unsigned int speed_up:1; //0
		unsigned int mbz_4:3; //0
	} f;
	unsigned int v;
} PIN_STS_T;
#define PIN_STSrv (*((regval)0xb8000100))
#define PIN_STSdv (0x000c0000)
#define RMOD_PIN_STS(...) rset(PIN_STS, PIN_STSrv, __VA_ARGS__)
#define RIZS_PIN_STS(...) rset(PIN_STS, 0, __VA_ARGS__)
#define RFLD_PIN_STS(fld) (*((const volatile PIN_STS_T *)0xb8000100)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:22; //0
		unsigned int mbz_1:4; //0
		unsigned int no_use05:6; //0
	} f;
	unsigned int v;
} PAD_CTRL_NO_USE_0104_T;
#define PAD_CTRL_NO_USE_0104rv (*((regval)0xb8000104))
#define PAD_CTRL_NO_USE_0104dv (0x00000000)
#define RMOD_PAD_CTRL_NO_USE_0104(...) rset(PAD_CTRL_NO_USE_0104, PAD_CTRL_NO_USE_0104rv, __VA_ARGS__)
#define RIZS_PAD_CTRL_NO_USE_0104(...) rset(PAD_CTRL_NO_USE_0104, 0, __VA_ARGS__)
#define RFLD_PAD_CTRL_NO_USE_0104(fld) (*((const volatile PAD_CTRL_NO_USE_0104_T *)0xb8000104)).f.fld

typedef union {
	struct {
		unsigned int no_use31:31; //0
		unsigned int mbz_0:1; //0
	} f;
	unsigned int v;
} PAD_CTRL_NO_USE_0108_T;
#define PAD_CTRL_NO_USE_0108rv (*((regval)0xb8000108))
#define PAD_CTRL_NO_USE_0108dv (0x00000000)
#define RMOD_PAD_CTRL_NO_USE_0108(...) rset(PAD_CTRL_NO_USE_0108, PAD_CTRL_NO_USE_0108rv, __VA_ARGS__)
#define RIZS_PAD_CTRL_NO_USE_0108(...) rset(PAD_CTRL_NO_USE_0108, 0, __VA_ARGS__)
#define RFLD_PAD_CTRL_NO_USE_0108(fld) (*((const volatile PAD_CTRL_NO_USE_0108_T *)0xb8000108)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:2; //0
		unsigned int no_use29:30; //0
	} f;
	unsigned int v;
} PAD_CTRL_NO_USE_010C_T;
#define PAD_CTRL_NO_USE_010Crv (*((regval)0xb800010c))
#define PAD_CTRL_NO_USE_010Cdv (0x00000000)
#define RMOD_PAD_CTRL_NO_USE_010C(...) rset(PAD_CTRL_NO_USE_010C, PAD_CTRL_NO_USE_010Crv, __VA_ARGS__)
#define RIZS_PAD_CTRL_NO_USE_010C(...) rset(PAD_CTRL_NO_USE_010C, 0, __VA_ARGS__)
#define RFLD_PAD_CTRL_NO_USE_010C(fld) (*((const volatile PAD_CTRL_NO_USE_010C_T *)0xb800010c)).f.fld

typedef union {
	struct {
		unsigned int no_use31:5; //0
		unsigned int mbz_0:27; //0
	} f;
	unsigned int v;
} PAD_CTRL_NO_USE_0110_T;
#define PAD_CTRL_NO_USE_0110rv (*((regval)0xb8000110))
#define PAD_CTRL_NO_USE_0110dv (0x00000000)
#define RMOD_PAD_CTRL_NO_USE_0110(...) rset(PAD_CTRL_NO_USE_0110, PAD_CTRL_NO_USE_0110rv, __VA_ARGS__)
#define RIZS_PAD_CTRL_NO_USE_0110(...) rset(PAD_CTRL_NO_USE_0110, 0, __VA_ARGS__)
#define RFLD_PAD_CTRL_NO_USE_0110(fld) (*((const volatile PAD_CTRL_NO_USE_0110_T *)0xb8000110)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:1; //0
		unsigned int no_use30:15; //0
		unsigned int mbz_1:3; //0
		unsigned int no_use12:13; //0
	} f;
	unsigned int v;
} PAD_CTRL_NO_USE_0114_T;
#define PAD_CTRL_NO_USE_0114rv (*((regval)0xb8000114))
#define PAD_CTRL_NO_USE_0114dv (0x00000000)
#define RMOD_PAD_CTRL_NO_USE_0114(...) rset(PAD_CTRL_NO_USE_0114, PAD_CTRL_NO_USE_0114rv, __VA_ARGS__)
#define RIZS_PAD_CTRL_NO_USE_0114(...) rset(PAD_CTRL_NO_USE_0114, 0, __VA_ARGS__)
#define RFLD_PAD_CTRL_NO_USE_0114(fld) (*((const volatile PAD_CTRL_NO_USE_0114_T *)0xb8000114)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:6; //0
		unsigned int dtn:2; //0
		unsigned int dtp:2; //0
		unsigned int force_mode:1; //0
		unsigned int drv:1; //0
		unsigned int nt:3; //0
		unsigned int pt:3; //0
		unsigned int plsb:2; //0
		unsigned int ttcn:3; //0
		unsigned int ttcp:3; //0
		unsigned int ttfn:3; //0
		unsigned int ttfp:3; //0
	} f;
	unsigned int v;
} DDR_MD_IOPDCR0_T;
#define DDR_MD_IOPDCR0rv (*((regval)0xb8000118))
#define DDR_MD_IOPDCR0dv (0x00000000)
#define RMOD_DDR_MD_IOPDCR0(...) rset(DDR_MD_IOPDCR0, DDR_MD_IOPDCR0rv, __VA_ARGS__)
#define RIZS_DDR_MD_IOPDCR0(...) rset(DDR_MD_IOPDCR0, 0, __VA_ARGS__)
#define RFLD_DDR_MD_IOPDCR0(fld) (*((const volatile DDR_MD_IOPDCR0_T *)0xb8000118)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:6; //0
		unsigned int dtn:2; //0
		unsigned int dtp:2; //0
		unsigned int force_mode:1; //0
		unsigned int drv:1; //0
		unsigned int nt:3; //0
		unsigned int pt:3; //0
		unsigned int plsb:2; //0
		unsigned int ttcn:3; //0
		unsigned int ttcp:3; //0
		unsigned int ttfn:3; //0
		unsigned int ttfp:3; //0
	} f;
	unsigned int v;
} DDR_MD_IOPDCR1_T;
#define DDR_MD_IOPDCR1rv (*((regval)0xb800011c))
#define DDR_MD_IOPDCR1dv (0x00000000)
#define RMOD_DDR_MD_IOPDCR1(...) rset(DDR_MD_IOPDCR1, DDR_MD_IOPDCR1rv, __VA_ARGS__)
#define RIZS_DDR_MD_IOPDCR1(...) rset(DDR_MD_IOPDCR1, 0, __VA_ARGS__)
#define RFLD_DDR_MD_IOPDCR1(fld) (*((const volatile DDR_MD_IOPDCR1_T *)0xb800011c)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:6; //0
		unsigned int dtn:2; //0
		unsigned int dtp:2; //0
		unsigned int force_mode:1; //0
		unsigned int drv:1; //0
		unsigned int nt:3; //0
		unsigned int pt:3; //0
		unsigned int plsb:2; //0
		unsigned int ttcn:3; //0
		unsigned int ttcp:3; //0
		unsigned int ttfn:3; //0
		unsigned int ttfp:3; //0
	} f;
	unsigned int v;
} DDR_MD_IOPDCR2_T;
#define DDR_MD_IOPDCR2rv (*((regval)0xb8000120))
#define DDR_MD_IOPDCR2dv (0x00000000)
#define RMOD_DDR_MD_IOPDCR2(...) rset(DDR_MD_IOPDCR2, DDR_MD_IOPDCR2rv, __VA_ARGS__)
#define RIZS_DDR_MD_IOPDCR2(...) rset(DDR_MD_IOPDCR2, 0, __VA_ARGS__)
#define RFLD_DDR_MD_IOPDCR2(fld) (*((const volatile DDR_MD_IOPDCR2_T *)0xb8000120)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:6; //0
		unsigned int dtn:2; //0
		unsigned int dtp:2; //0
		unsigned int force_mode:1; //0
		unsigned int drv:1; //0
		unsigned int nt:3; //0
		unsigned int pt:3; //0
		unsigned int plsb:2; //0
		unsigned int ttcn:3; //0
		unsigned int ttcp:3; //0
		unsigned int ttfn:3; //0
		unsigned int ttfp:3; //0
	} f;
	unsigned int v;
} DDR_MD_IOPDCR3_T;
#define DDR_MD_IOPDCR3rv (*((regval)0xb8000124))
#define DDR_MD_IOPDCR3dv (0x00000000)
#define RMOD_DDR_MD_IOPDCR3(...) rset(DDR_MD_IOPDCR3, DDR_MD_IOPDCR3rv, __VA_ARGS__)
#define RIZS_DDR_MD_IOPDCR3(...) rset(DDR_MD_IOPDCR3, 0, __VA_ARGS__)
#define RFLD_DDR_MD_IOPDCR3(fld) (*((const volatile DDR_MD_IOPDCR3_T *)0xb8000124)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:6; //0
		unsigned int hdqm_drv:1; //0
		unsigned int hdqm_nt:3; //0
		unsigned int hdqm_pt:3; //0
		unsigned int hdqm_plsb:2; //0
		unsigned int hdqm_force_mode:1; //0
		unsigned int mbz_1:6; //0
		unsigned int ldqm_drv:1; //0
		unsigned int ldqm_nt:3; //0
		unsigned int ldqm_pt:3; //0
		unsigned int ldqm_plsb:2; //0
		unsigned int ldqm_force_mode:1; //0
	} f;
	unsigned int v;
} DDR_DM_IOPDCR_T;
#define DDR_DM_IOPDCRrv (*((regval)0xb8000128))
#define DDR_DM_IOPDCRdv (0x00000000)
#define RMOD_DDR_DM_IOPDCR(...) rset(DDR_DM_IOPDCR, DDR_DM_IOPDCRrv, __VA_ARGS__)
#define RIZS_DDR_DM_IOPDCR(...) rset(DDR_DM_IOPDCR, 0, __VA_ARGS__)
#define RFLD_DDR_DM_IOPDCR(fld) (*((const volatile DDR_DM_IOPDCR_T *)0xb8000128)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:6; //0
		unsigned int ckn_drv:1; //0
		unsigned int ckn_nt:3; //0
		unsigned int ckn_pt:3; //0
		unsigned int ckn_plsb:2; //0
		unsigned int ckn_force_mode:1; //0
		unsigned int mbz_1:6; //0
		unsigned int ck_drv:1; //0
		unsigned int ck_nt:3; //0
		unsigned int ck_pt:3; //0
		unsigned int ck_plsb:2; //0
		unsigned int ck_force_mode:1; //0
	} f;
	unsigned int v;
} DDR_CK_IOPDCR_T;
#define DDR_CK_IOPDCRrv (*((regval)0xb800012c))
#define DDR_CK_IOPDCRdv (0x00000000)
#define RMOD_DDR_CK_IOPDCR(...) rset(DDR_CK_IOPDCR, DDR_CK_IOPDCRrv, __VA_ARGS__)
#define RIZS_DDR_CK_IOPDCR(...) rset(DDR_CK_IOPDCR, 0, __VA_ARGS__)
#define RFLD_DDR_CK_IOPDCR(fld) (*((const volatile DDR_CK_IOPDCR_T *)0xb800012c)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:6; //0
		unsigned int dtn:2; //0
		unsigned int dtp:2; //0
		unsigned int force_mode:1; //0
		unsigned int drv:1; //0
		unsigned int nt:3; //0
		unsigned int pt:3; //0
		unsigned int plsb:2; //0
		unsigned int ttcn:3; //0
		unsigned int ttcp:3; //0
		unsigned int ttfn:3; //0
		unsigned int ttfp:3; //0
	} f;
	unsigned int v;
} DDR_DQS_IOPDCR0_T;
#define DDR_DQS_IOPDCR0rv (*((regval)0xb8000130))
#define DDR_DQS_IOPDCR0dv (0x00000000)
#define RMOD_DDR_DQS_IOPDCR0(...) rset(DDR_DQS_IOPDCR0, DDR_DQS_IOPDCR0rv, __VA_ARGS__)
#define RIZS_DDR_DQS_IOPDCR0(...) rset(DDR_DQS_IOPDCR0, 0, __VA_ARGS__)
#define RFLD_DDR_DQS_IOPDCR0(fld) (*((const volatile DDR_DQS_IOPDCR0_T *)0xb8000130)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:6; //0
		unsigned int dtn:2; //0
		unsigned int dtp:2; //0
		unsigned int force_mode:1; //0
		unsigned int drv:1; //0
		unsigned int nt:3; //0
		unsigned int pt:3; //0
		unsigned int plsb:2; //0
		unsigned int ttcn:3; //0
		unsigned int ttcp:3; //0
		unsigned int ttfn:3; //0
		unsigned int ttfp:3; //0
	} f;
	unsigned int v;
} DDR_DQS_IOPDCR1_T;
#define DDR_DQS_IOPDCR1rv (*((regval)0xb8000134))
#define DDR_DQS_IOPDCR1dv (0x00000000)
#define RMOD_DDR_DQS_IOPDCR1(...) rset(DDR_DQS_IOPDCR1, DDR_DQS_IOPDCR1rv, __VA_ARGS__)
#define RIZS_DDR_DQS_IOPDCR1(...) rset(DDR_DQS_IOPDCR1, 0, __VA_ARGS__)
#define RFLD_DDR_DQS_IOPDCR1(fld) (*((const volatile DDR_DQS_IOPDCR1_T *)0xb8000134)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:6; //0
		unsigned int dtn:2; //0
		unsigned int dtp:2; //0
		unsigned int force_mode:1; //0
		unsigned int drv:1; //0
		unsigned int nt:3; //0
		unsigned int pt:3; //0
		unsigned int plsb:2; //0
		unsigned int ttcn:3; //0
		unsigned int ttcp:3; //0
		unsigned int ttfn:3; //0
		unsigned int ttfp:3; //0
	} f;
	unsigned int v;
} DDR_DQS_IOPDCR2_T;
#define DDR_DQS_IOPDCR2rv (*((regval)0xb8000138))
#define DDR_DQS_IOPDCR2dv (0x00000000)
#define RMOD_DDR_DQS_IOPDCR2(...) rset(DDR_DQS_IOPDCR2, DDR_DQS_IOPDCR2rv, __VA_ARGS__)
#define RIZS_DDR_DQS_IOPDCR2(...) rset(DDR_DQS_IOPDCR2, 0, __VA_ARGS__)
#define RFLD_DDR_DQS_IOPDCR2(fld) (*((const volatile DDR_DQS_IOPDCR2_T *)0xb8000138)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:6; //0
		unsigned int dtn:2; //0
		unsigned int dtp:2; //0
		unsigned int force_mode:1; //0
		unsigned int drv:1; //0
		unsigned int nt:3; //0
		unsigned int pt:3; //0
		unsigned int plsb:2; //0
		unsigned int ttcn:3; //0
		unsigned int ttcp:3; //0
		unsigned int ttfn:3; //0
		unsigned int ttfp:3; //0
	} f;
	unsigned int v;
} DDR_DQS_IOPDCR3_T;
#define DDR_DQS_IOPDCR3rv (*((regval)0xb800013c))
#define DDR_DQS_IOPDCR3dv (0x00000000)
#define RMOD_DDR_DQS_IOPDCR3(...) rset(DDR_DQS_IOPDCR3, DDR_DQS_IOPDCR3rv, __VA_ARGS__)
#define RIZS_DDR_DQS_IOPDCR3(...) rset(DDR_DQS_IOPDCR3, 0, __VA_ARGS__)
#define RFLD_DDR_DQS_IOPDCR3(fld) (*((const volatile DDR_DQS_IOPDCR3_T *)0xb800013c)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:6; //0
		unsigned int misc_drv:1; //0
		unsigned int misc_nt:3; //0
		unsigned int misc_pt:3; //0
		unsigned int misc_plsb:2; //0
		unsigned int misc_force_mode:1; //0
		unsigned int mbz_1:6; //0
		unsigned int mba_drv:1; //0
		unsigned int mba_nt:3; //0
		unsigned int mba_pt:3; //0
		unsigned int mba_plsb:2; //0
		unsigned int mba_force_mode:1; //0
	} f;
	unsigned int v;
} DDR_MA_IOPDCR_T;
#define DDR_MA_IOPDCRrv (*((regval)0xb8000140))
#define DDR_MA_IOPDCRdv (0x00000000)
#define RMOD_DDR_MA_IOPDCR(...) rset(DDR_MA_IOPDCR, DDR_MA_IOPDCRrv, __VA_ARGS__)
#define RIZS_DDR_MA_IOPDCR(...) rset(DDR_MA_IOPDCR, 0, __VA_ARGS__)
#define RFLD_DDR_MA_IOPDCR(fld) (*((const volatile DDR_MA_IOPDCR_T *)0xb8000140)).f.fld

typedef union {
	struct {
		unsigned int ddr3_md:1; //0
		unsigned int ddr_cal_srst:1; //1
		unsigned int ddr_zq_pwrdn:1; //0
		unsigned int force_mode:1; //0
		unsigned int plsb:1; //0
		unsigned int vpcalen:1; //0
		unsigned int podt:3; //0
		unsigned int pocd:5; //0
		unsigned int nodt:3; //0
		unsigned int nocd:5; //0
		unsigned int cal:2; //0
		unsigned int mbz_0:3; //0
		unsigned int ddr2_dqs_se:1; //0
		unsigned int ddr_io_speed:1; //1
		unsigned int ddr_io_pwdpad3:1; //0
		unsigned int ddr3_mode:1; //0
		unsigned int ddr2_mode:1; //1
	} f;
	unsigned int v;
} DDR123_MCR_T;
#define DDR123_MCRrv (*((regval)0xb8000144))
#define DDR123_MCRdv (0x40000009)
#define RMOD_DDR123_MCR(...) rset(DDR123_MCR, DDR123_MCRrv, __VA_ARGS__)
#define RIZS_DDR123_MCR(...) rset(DDR123_MCR, 0, __VA_ARGS__)
#define RFLD_DDR123_MCR(fld) (*((const volatile DDR123_MCR_T *)0xb8000144)).f.fld

/*-----------------------------------------------------
 Extraced from file_CLK_CTRL.xml
-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int mbz_0:10; //0
		unsigned int ocp0_pll_div:6; //14
		unsigned int mbz_1:10; //0
		unsigned int sdpll_div:6; //14
	} f;
	unsigned int v;
} SYSPLLCTR_T;
#define SYSPLLCTRrv (*((regval)0xb8000200))
#define SYSPLLCTRdv (0x000e000e)
#define RMOD_SYSPLLCTR(...) rset(SYSPLLCTR, SYSPLLCTRrv, __VA_ARGS__)
#define RIZS_SYSPLLCTR(...) rset(SYSPLLCTR, 0, __VA_ARGS__)
#define RFLD_SYSPLLCTR(fld) (*((const volatile SYSPLLCTR_T *)0xb8000200)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:1; //0
		unsigned int ssc_rst_n:1; //1
		unsigned int ssc_test_mode:2; //0
		unsigned int ssc_offset:8; //0
		unsigned int ssc_step:6; //0
		unsigned int ssc_period:7; //0
		unsigned int ssc_en:1; //0
		unsigned int mbz_1:1; //0
		unsigned int pwrdn_pll_ddr:1; //0
		unsigned int frac_en:1; //0
		unsigned int pllddr_fupdn:1; //0
		unsigned int pllddr_psen:1; //1
		unsigned int pllddr_oeb:1; //0
	} f;
	unsigned int v;
} PLLCCR_T;
#define PLLCCRrv (*((regval)0xb8000204))
#define PLLCCRdv (0x40000002)
#define RMOD_PLLCCR(...) rset(PLLCCR, PLLCCRrv, __VA_ARGS__)
#define RIZS_PLLCCR(...) rset(PLLCCR, 0, __VA_ARGS__)
#define RFLD_PLLCCR(fld) (*((const volatile PLLCCR_T *)0xb8000204)).f.fld

typedef union {
	struct {
		unsigned int pll_ctrl:32; //118185
	} f;
	unsigned int v;
} PLL0_T;
#define PLL0rv (*((regval)0xb8000208))
#define PLL0dv (0x0001cda9)
#define RMOD_PLL0(...) rset(PLL0, PLL0rv, __VA_ARGS__)
#define RIZS_PLL0(...) rset(PLL0, 0, __VA_ARGS__)
#define RFLD_PLL0(fld) (*((const volatile PLL0_T *)0xb8000208)).f.fld

typedef union {
	struct {
		unsigned int pll_ctrl:32; //2123736121
	} f;
	unsigned int v;
} PLL1_T;
#define PLL1rv (*((regval)0xb800020c))
#define PLL1dv (0x7e95a439)
#define RMOD_PLL1(...) rset(PLL1, PLL1rv, __VA_ARGS__)
#define RIZS_PLL1(...) rset(PLL1, 0, __VA_ARGS__)
#define RFLD_PLL1(fld) (*((const volatile PLL1_T *)0xb800020c)).f.fld

typedef union {
	struct {
		unsigned int pll_ctrl:32; //15794152
	} f;
	unsigned int v;
} PLL2_T;
#define PLL2rv (*((regval)0xb8000210))
#define PLL2dv (0x00f0ffe8)
#define RMOD_PLL2(...) rset(PLL2, PLL2rv, __VA_ARGS__)
#define RIZS_PLL2(...) rset(PLL2, 0, __VA_ARGS__)
#define RFLD_PLL2(fld) (*((const volatile PLL2_T *)0xb8000210)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:14; //0
		unsigned int no_use15:18; //0
	} f;
	unsigned int v;
} CLK_CTRL_NO_USE_0218_T;
#define CLK_CTRL_NO_USE_0218rv (*((regval)0xb8000218))
#define CLK_CTRL_NO_USE_0218dv (0x00000000)
#define RMOD_CLK_CTRL_NO_USE_0218(...) rset(CLK_CTRL_NO_USE_0218, CLK_CTRL_NO_USE_0218rv, __VA_ARGS__)
#define RIZS_CLK_CTRL_NO_USE_0218(...) rset(CLK_CTRL_NO_USE_0218, 0, __VA_ARGS__)
#define RFLD_CLK_CTRL_NO_USE_0218(fld) (*((const volatile CLK_CTRL_NO_USE_0218_T *)0xb8000218)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:11; //0
		unsigned int ddrckm90_tap:5; //0
		unsigned int mbz_1:3; //0
		unsigned int ddrckm_tap:5; //0
		unsigned int mbz_2:2; //0
		unsigned int no_use05:1; //0
		unsigned int ddrck_phs_sel:5; //0
	} f;
	unsigned int v;
} DRCKO_T;
#define DRCKOrv (*((regval)0xb800021c))
#define DRCKOdv (0x00000000)
#define RMOD_DRCKO(...) rset(DRCKO, DRCKOrv, __VA_ARGS__)
#define RIZS_DRCKO(...) rset(DRCKO, 0, __VA_ARGS__)
#define RFLD_DRCKO(fld) (*((const volatile DRCKO_T *)0xb800021c)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:29; //0
		unsigned int mckg_phase_sel:3; //2
	} f;
	unsigned int v;
} MCKG_PHS_SEL_T;
#define MCKG_PHS_SELrv (*((regval)0xb8000220))
#define MCKG_PHS_SELdv (0x00000002)
#define RMOD_MCKG_PHS_SEL(...) rset(MCKG_PHS_SEL, MCKG_PHS_SELrv, __VA_ARGS__)
#define RIZS_MCKG_PHS_SEL(...) rset(MCKG_PHS_SEL, 0, __VA_ARGS__)
#define RFLD_MCKG_PHS_SEL(fld) (*((const volatile MCKG_PHS_SEL_T *)0xb8000220)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:30; //0
		unsigned int mckg_divl:2; //0
	} f;
	unsigned int v;
} MCKG_DIV_SEL_T;
#define MCKG_DIV_SELrv (*((regval)0xb8000224))
#define MCKG_DIV_SELdv (0x00000000)
#define RMOD_MCKG_DIV_SEL(...) rset(MCKG_DIV_SEL, MCKG_DIV_SELrv, __VA_ARGS__)
#define RIZS_MCKG_DIV_SEL(...) rset(MCKG_DIV_SEL, 0, __VA_ARGS__)
#define RFLD_MCKG_DIV_SEL(fld) (*((const volatile MCKG_DIV_SEL_T *)0xb8000224)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:20; //0
		unsigned int spi_flash_clk_div:4; //3
		unsigned int mbz_1:4; //0
		unsigned int lxpll_div:4; //3
	} f;
	unsigned int v;
} LX_CLK_PLL_T;
#define LX_CLK_PLLrv (*((regval)0xb8000228))
#define LX_CLK_PLLdv (0x00000303)
#define RMOD_LX_CLK_PLL(...) rset(LX_CLK_PLL, LX_CLK_PLLrv, __VA_ARGS__)
#define RIZS_LX_CLK_PLL(...) rset(LX_CLK_PLL, 0, __VA_ARGS__)
#define RFLD_LX_CLK_PLL(fld) (*((const volatile LX_CLK_PLL_T *)0xb8000228)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:31; //0
		unsigned int start_dram_clk_out:1; //1
	} f;
	unsigned int v;
} DRAM_CLK_EN_T;
#define DRAM_CLK_ENrv (*((regval)0xb800022c))
#define DRAM_CLK_ENdv (0x00000001)
#define RMOD_DRAM_CLK_EN(...) rset(DRAM_CLK_EN, DRAM_CLK_ENrv, __VA_ARGS__)
#define RIZS_DRAM_CLK_EN(...) rset(DRAM_CLK_EN, 0, __VA_ARGS__)
#define RFLD_DRAM_CLK_EN(fld) (*((const volatile DRAM_CLK_EN_T *)0xb800022c)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:31; //0
		unsigned int dram_clk_dg_en:1; //0
	} f;
	unsigned int v;
} DRAM_CLK_CHG_T;
#define DRAM_CLK_CHGrv (*((regval)0xb8000230))
#define DRAM_CLK_CHGdv (0x00000000)
#define RMOD_DRAM_CLK_CHG(...) rset(DRAM_CLK_CHG, DRAM_CLK_CHGrv, __VA_ARGS__)
#define RIZS_DRAM_CLK_CHG(...) rset(DRAM_CLK_CHG, 0, __VA_ARGS__)
#define RFLD_DRAM_CLK_CHG(fld) (*((const volatile DRAM_CLK_CHG_T *)0xb8000230)).f.fld

/*-----------------------------------------------------
 Extraced from file_CMU.xml
-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int oc0_busy:1; //1
		unsigned int mbz_0:5; //0
		unsigned int oc0_freq_div:3; //7
		unsigned int oc1_freq_div:3; //7
		unsigned int lx0_freq_div:3; //7
		unsigned int lx1_freq_div:3; //7
		unsigned int lx2_freq_div:3; //7
		unsigned int lxp_freq_div:3; //7
		unsigned int cmu_mode:2; //0
		unsigned int mbz_1:1; //0
		unsigned int oc0_se_dram:1; //0
		unsigned int oc1_se_dram:1; //0
		unsigned int lx0_se_dram:1; //0
		unsigned int lx1_se_dram:1; //0
		unsigned int lx2_se_dram:1; //0
	} f;
	unsigned int v;
} CMUGCR_T;
#define CMUGCRrv (*((regval)0xb8000308))
#define CMUGCRdv (0x83ffff00)
#define RMOD_CMUGCR(...) rset(CMUGCR, CMUGCRrv, __VA_ARGS__)
#define RIZS_CMUGCR(...) rset(CMUGCR, 0, __VA_ARGS__)
#define RFLD_CMUGCR(fld) (*((const volatile CMUGCR_T *)0xb8000308)).f.fld

typedef union {
	struct {
		unsigned int oc0_dly_base:3; //0
		unsigned int oc1_dly_base:3; //0
		unsigned int lx0_dly_base:3; //0
		unsigned int lx1_dly_base:3; //0
		unsigned int lx2_dly_base:3; //0
		unsigned int lxp_dly_base:3; //0
		unsigned int mbz_0:14; //0
	} f;
	unsigned int v;
} CMUSDCR_T;
#define CMUSDCRrv (*((regval)0xb800030c))
#define CMUSDCRdv (0x00000000)
#define RMOD_CMUSDCR(...) rset(CMUSDCR, CMUSDCRrv, __VA_ARGS__)
#define RIZS_CMUSDCR(...) rset(CMUSDCR, 0, __VA_ARGS__)
#define RFLD_CMUSDCR(fld) (*((const volatile CMUSDCR_T *)0xb800030c)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:20; //0
		unsigned int oc0_auto_bz:1; //0
		unsigned int oc0_spif_hs:1; //0
		unsigned int oc0_se_spif:1; //0
		unsigned int oc0_dram_hs:1; //0
		unsigned int oc0_se_spif_wk:1; //0
		unsigned int oc0_se_spif_slp:1; //0
		unsigned int oc0_se_dram_wk:1; //0
		unsigned int oc0_se_dram_slp:1; //0
		unsigned int oc0_dly_mul:4; //0
	} f;
	unsigned int v;
} CMUO0CR_T;
#define CMUO0CRrv (*((regval)0xb8000310))
#define CMUO0CRdv (0x00000000)
#define RMOD_CMUO0CR(...) rset(CMUO0CR, CMUO0CRrv, __VA_ARGS__)
#define RIZS_CMUO0CR(...) rset(CMUO0CR, 0, __VA_ARGS__)
#define RFLD_CMUO0CR(fld) (*((const volatile CMUO0CR_T *)0xb8000310)).f.fld

typedef union {
	struct {
		unsigned int oc0slp_cnt:32; //0
	} f;
	unsigned int v;
} CMUO0SC_T;
#define CMUO0SCrv (*((regval)0xb8000328))
#define CMUO0SCdv (0x00000000)
#define RMOD_CMUO0SC(...) rset(CMUO0SC, CMUO0SCrv, __VA_ARGS__)
#define RIZS_CMUO0SC(...) rset(CMUO0SC, 0, __VA_ARGS__)
#define RFLD_CMUO0SC(fld) (*((const volatile CMUO0SC_T *)0xb8000328)).f.fld

/*-----------------------------------------------------
 Extraced from file_MEM_BIST.xml
-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int cpu_dvs_7:4; //0
		unsigned int cpu_dvs_6:4; //0
		unsigned int cpu_dvs_5:4; //0
		unsigned int cpu_dvs_4:4; //0
		unsigned int cpu_dvs_3:4; //0
		unsigned int cpu_dvs_2:4; //0
		unsigned int cpu_dvs_1:4; //0
		unsigned int cpu_dvs_0:4; //0
	} f;
	unsigned int v;
} BIST0_T;
#define BIST0rv (*((regval)0xb8000400))
#define BIST0dv (0x00000000)
#define RMOD_BIST0(...) rset(BIST0, BIST0rv, __VA_ARGS__)
#define RIZS_BIST0(...) rset(BIST0, 0, __VA_ARGS__)
#define RFLD_BIST0(fld) (*((const volatile BIST0_T *)0xb8000400)).f.fld

typedef union {
	struct {
		unsigned int cpu_dvs_15:4; //0
		unsigned int cpu_dvs_14:4; //0
		unsigned int cpu_dvs_13:4; //0
		unsigned int cpu_dvs_12:4; //0
		unsigned int cpu_dvs_11:4; //0
		unsigned int cpu_dvs_10:4; //0
		unsigned int cpu_dvs_9:4; //0
		unsigned int cpu_dvs_8:4; //0
	} f;
	unsigned int v;
} BIST1_T;
#define BIST1rv (*((regval)0xb8000404))
#define BIST1dv (0x00000000)
#define RMOD_BIST1(...) rset(BIST1, BIST1rv, __VA_ARGS__)
#define RIZS_BIST1(...) rset(BIST1, 0, __VA_ARGS__)
#define RFLD_BIST1(fld) (*((const volatile BIST1_T *)0xb8000404)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:1; //0
		unsigned int voipacc_dvs:5; //0
		unsigned int cpu_dvse:1; //0
		unsigned int sram_ctrl_dvse:1; //0
		unsigned int gmac_dvs:4; //0
		unsigned int sram_ctrl_dvs:4; //0
		unsigned int cpu_dvs_19:4; //0
		unsigned int cpu_dvs_18:4; //0
		unsigned int cpu_dvs_17:4; //0
		unsigned int cpu_dvs_16:4; //0
	} f;
	unsigned int v;
} BIST2_T;
#define BIST2rv (*((regval)0xb8000408))
#define BIST2dv (0x00000000)
#define RMOD_BIST2(...) rset(BIST2, BIST2rv, __VA_ARGS__)
#define RIZS_BIST2(...) rset(BIST2, 0, __VA_ARGS__)
#define RFLD_BIST2(fld) (*((const volatile BIST2_T *)0xb8000408)).f.fld

typedef union {
	struct {
		unsigned int fftacc_dvs:5; //0
		unsigned int mbz_0:14; //0
		unsigned int no_use12:2; //0
		unsigned int drf_test_resume_mode_1:1; //0
		unsigned int drf_test_resume_mode_0:1; //0
		unsigned int no_use08:2; //0
		unsigned int drf_bist_mode_1:1; //0
		unsigned int drf_bist_mode_0:1; //0
		unsigned int bist_rst_n:1; //1
		unsigned int no_use03:2; //0
		unsigned int bist_mode_1:1; //0
		unsigned int bist_mode_0:1; //0
	} f;
	unsigned int v;
} BIST3_T;
#define BIST3rv (*((regval)0xb800040c))
#define BIST3dv (0x00000010)
#define RMOD_BIST3(...) rset(BIST3, BIST3rv, __VA_ARGS__)
#define RIZS_BIST3(...) rset(BIST3, 0, __VA_ARGS__)
#define RFLD_BIST3(fld) (*((const volatile BIST3_T *)0xb800040c)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:2; //0
		unsigned int drf_pon_outq_ram_bist_fail:1; //0
		unsigned int drf_pon_pt_ram_bist_fail:1; //0
		unsigned int drf_pon_qcnt_ram_bist_fail:1; //0
		unsigned int drf_pon_cir_ram_bist_fail:1; //0
		unsigned int drf_pon_pir_ram_bist_fail:1; //0
		unsigned int drf_pon_lb_ram_bist_fail:1; //0
		unsigned int drf_ponnic_bist_fail:1; //0
		unsigned int drf_pon_fp_ram_bist_fail:1; //0
		unsigned int drf_pon_desc_ram_bist_fail:1; //0
		unsigned int drf_pon_data_ram_bist_fail:1; //0
		unsigned int drf_gmac_bist_fail:1; //0
		unsigned int drf_voipacc_bist_fail:2; //0
		unsigned int drf_fftacc_bist_fail:2; //0
		unsigned int pon_outq_ram_bist_fail:1; //0
		unsigned int pon_pt_ram_bist_fail:1; //0
		unsigned int pon_qcnt_ram_bist_fail:1; //0
		unsigned int pon_cir_ram_bist_fail:1; //0
		unsigned int pon_pir_ram_bist_fail:1; //0
		unsigned int pon_lb_ram_bist_fail:1; //0
		unsigned int ponnic_bist_fail:1; //0
		unsigned int pon_fp_ram_bist_fail:1; //0
		unsigned int pon_desc_ram_bist_fail:1; //0
		unsigned int pon_data_ram_bist_fail:1; //0
		unsigned int gmac_bist_fail:1; //0
		unsigned int voipacc_bist_fail:2; //0
		unsigned int fftacc_bist_fail:2; //0
	} f;
	unsigned int v;
} BIST4_T;
#define BIST4rv (*((regval)0xb8000410))
#define BIST4dv (0x00000000)
#define RMOD_BIST4(...) rset(BIST4, BIST4rv, __VA_ARGS__)
#define RIZS_BIST4(...) rset(BIST4, 0, __VA_ARGS__)
#define RFLD_BIST4(fld) (*((const volatile BIST4_T *)0xb8000410)).f.fld

typedef union {
	struct {
		unsigned int drf_sram_bist_done:1; //0
		unsigned int drf_cpu_bist0_done:1; //0
		unsigned int drf_cpu_bist1_done:1; //0
		unsigned int cpu_bist0_done:1; //0
		unsigned int cpu_bist1_done:1; //0
		unsigned int sram_bist_done:1; //0
		unsigned int drf_pon_outq_ram_bist_done:1; //0
		unsigned int drf_pon_pt_ram_bist_done:1; //0
		unsigned int drf_pon_qcnt_ram_bist_done:1; //0
		unsigned int drf_pon_cir_ram_bist_done:1; //0
		unsigned int drf_pon_pir_ram_bist_done:1; //0
		unsigned int drf_pon_lb_ram_bist_done:1; //0
		unsigned int drf_ponnic_bist_done:1; //0
		unsigned int drf_pon_fp_ram_bist_done:1; //0
		unsigned int drf_pon_desc_ram_bist_done:1; //0
		unsigned int drf_pon_data_ram_bist_done:1; //0
		unsigned int drf_gmac_bist_done:1; //0
		unsigned int drf_voipacc_bist_done:1; //0
		unsigned int drf_fftacc_bist_done:1; //0
		unsigned int pon_outq_ram_bist_done:1; //0
		unsigned int pon_pt_ram_bist_done:1; //0
		unsigned int pon_qcnt_ram_bist_done:1; //0
		unsigned int pon_cir_ram_bist_done:1; //0
		unsigned int pon_pir_ram_bist_done:1; //0
		unsigned int pon_lb_ram_bist_done:1; //0
		unsigned int ponnic_bist_done:1; //0
		unsigned int pon_fp_ram_bist_done:1; //0
		unsigned int pon_desc_ram_bist_done:1; //0
		unsigned int pon_data_ram_bist_done:1; //0
		unsigned int gmac_bist_done:1; //0
		unsigned int voipacc_bist_done:1; //0
		unsigned int fftacc_bist_done:1; //0
	} f;
	unsigned int v;
} BIST5_T;
#define BIST5rv (*((regval)0xb8000414))
#define BIST5dv (0x00000000)
#define RMOD_BIST5(...) rset(BIST5, BIST5rv, __VA_ARGS__)
#define RIZS_BIST5(...) rset(BIST5, 0, __VA_ARGS__)
#define RFLD_BIST5(fld) (*((const volatile BIST5_T *)0xb8000414)).f.fld

typedef union {
	struct {
		unsigned int drf_start_pause:1; //1
		unsigned int mbz_0:9; //0
		unsigned int cpu_bist0_fail10:1; //0
		unsigned int cpu_bist0_fail9:1; //0
		unsigned int cpu_bist0_fail8:1; //0
		unsigned int cpu_bist0_fail7:1; //0
		unsigned int cpu_bist0_fail6:1; //0
		unsigned int cpu_bist0_fail5:1; //0
		unsigned int cpu_bist0_fail4:1; //0
		unsigned int cpu_bist0_fail3:1; //0
		unsigned int cpu_bist0_fail2:1; //0
		unsigned int cpu_bist0_fail1:1; //0
		unsigned int cpu_bist0_fail0:1; //0
		unsigned int cpu_bist1_fail8:1; //0
		unsigned int cpu_bist1_fail7:1; //0
		unsigned int cpu_bist1_fail6:1; //0
		unsigned int cpu_bist1_fail5:1; //0
		unsigned int cpu_bist1_fail4:1; //0
		unsigned int cpu_bist1_fail3:1; //0
		unsigned int cpu_bist1_fail2:1; //0
		unsigned int cpu_bist1_fail1:1; //0
		unsigned int cpu_bist1_fail0:1; //0
		unsigned int sramcl_bist_fail1:1; //0
		unsigned int sramcl_bist_fail0:1; //0
	} f;
	unsigned int v;
} BIST6_T;
#define BIST6rv (*((regval)0xb8000418))
#define BIST6dv (0x80000000)
#define RMOD_BIST6(...) rset(BIST6, BIST6rv, __VA_ARGS__)
#define RIZS_BIST6(...) rset(BIST6, 0, __VA_ARGS__)
#define RFLD_BIST6(fld) (*((const volatile BIST6_T *)0xb8000418)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:10; //0
		unsigned int drf_cpu_bist0_fail10:1; //0
		unsigned int drf_cpu_bist0_fail9:1; //0
		unsigned int drf_cpu_bist0_fail8:1; //0
		unsigned int drf_cpu_bist0_fail7:1; //0
		unsigned int drf_cpu_bist0_fail6:1; //0
		unsigned int drf_cpu_bist0_fail5:1; //0
		unsigned int drf_cpu_bist0_fail4:1; //0
		unsigned int drf_cpu_bist0_fail3:1; //0
		unsigned int drf_cpu_bist0_fail2:1; //0
		unsigned int drf_cpu_bist0_fail1:1; //0
		unsigned int drf_cpu_bist0_fail0:1; //0
		unsigned int drf_cpu_bist1_fail8:1; //0
		unsigned int drf_cpu_bist1_fail7:1; //0
		unsigned int drf_cpu_bist1_fail6:1; //0
		unsigned int drf_cpu_bist1_fail5:1; //0
		unsigned int drf_cpu_bist1_fail4:1; //0
		unsigned int drf_cpu_bist1_fail3:1; //0
		unsigned int drf_cpu_bist1_fail2:1; //0
		unsigned int drf_cpu_bist1_fail1:1; //0
		unsigned int drf_cpu_bist1_fail0:1; //0
		unsigned int drf_sramcl_bist_fail1:1; //0
		unsigned int drf_sramcl_bist_fail0:1; //0
	} f;
	unsigned int v;
} BIST7_T;
#define BIST7rv (*((regval)0xb800041c))
#define BIST7dv (0x00000000)
#define RMOD_BIST7(...) rset(BIST7, BIST7rv, __VA_ARGS__)
#define RIZS_BIST7(...) rset(BIST7, 0, __VA_ARGS__)
#define RFLD_BIST7(fld) (*((const volatile BIST7_T *)0xb800041c)).f.fld

/*-----------------------------------------------------
 Extraced from file_IP_ENABLE.xml
-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int mbz_0:4; //0
		unsigned int ponnic_en:1; //1
		unsigned int voip_int_sel:2; //0
		unsigned int dbg_grp_sel:5; //0
		unsigned int dbg_sub_grp_sel:4; //0
		unsigned int no_use15:3; //0
		unsigned int voipfft_en:1; //1
		unsigned int voipacc_en:1; //1
		unsigned int no_use10:1; //0
		unsigned int gdma1_en:1; //1
		unsigned int gdma0_en:1; //1
		unsigned int no_use07:5; //0
		unsigned int pcm_en:1; //1
		unsigned int gmac_en:1; //1
		unsigned int prei_voip_en:1; //1
	} f;
	unsigned int v;
} IP_EN_CTRL_T;
#define IP_EN_CTRLrv (*((regval)0xb8000600))
#define IP_EN_CTRLdv (0x08001b07)
#define RMOD_IP_EN_CTRL(...) rset(IP_EN_CTRL, IP_EN_CTRLrv, __VA_ARGS__)
#define RIZS_IP_EN_CTRL(...) rset(IP_EN_CTRL, 0, __VA_ARGS__)
#define RFLD_IP_EN_CTRL(fld) (*((const volatile IP_EN_CTRL_T *)0xb8000600)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:2; //0
		unsigned int no_use29:7; //0
		unsigned int dll_dis:1; //0
		unsigned int dll_pd:1; //1
		unsigned int dll_dl:2; //2
		unsigned int dll_start_b:1; //1
		unsigned int dll_pfd:2; //0
		unsigned int dll_vr:4; //10
		unsigned int dll_md:3; //0
		unsigned int dll_lf:3; //4
		unsigned int dll_cp:6; //4
	} f;
	unsigned int v;
} DLL_CTRL_T;
#define DLL_CTRLrv (*((regval)0xb8000604))
#define DLL_CTRLdv (0x0034a104)
#define RMOD_DLL_CTRL(...) rset(DLL_CTRL, DLL_CTRLrv, __VA_ARGS__)
#define RIZS_DLL_CTRL(...) rset(DLL_CTRL, 0, __VA_ARGS__)
#define RFLD_DLL_CTRL(fld) (*((const volatile DLL_CTRL_T *)0xb8000604)).f.fld

typedef union {
	struct {
		unsigned int dll_mode:1; //0
		unsigned int cf_mvref0_pd_ref:1; //0
		unsigned int cf_mvref0_por_sel:2; //2
		unsigned int cf_mvref1_pd_ref:1; //0
		unsigned int cf_mvref1_por_sel:2; //2
		unsigned int cf_clk_oc1_slower_spif:1; //0
		unsigned int mbz_0:3; //0
		unsigned int ana_dll_ctrl0:21; //1346142
	} f;
	unsigned int v;
} ANA_DLL0_T;
#define ANA_DLL0rv (*((regval)0xb8000608))
#define ANA_DLL0dv (0x24148a5e)
#define RMOD_ANA_DLL0(...) rset(ANA_DLL0, ANA_DLL0rv, __VA_ARGS__)
#define RIZS_ANA_DLL0(...) rset(ANA_DLL0, 0, __VA_ARGS__)
#define RFLD_ANA_DLL0(fld) (*((const volatile ANA_DLL0_T *)0xb8000608)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:11; //0
		unsigned int ana_dll_ctrl1:21; //1346142
	} f;
	unsigned int v;
} ANA_DLL1_T;
#define ANA_DLL1rv (*((regval)0xb800060c))
#define ANA_DLL1dv (0x00148a5e)
#define RMOD_ANA_DLL1(...) rset(ANA_DLL1, ANA_DLL1rv, __VA_ARGS__)
#define RIZS_ANA_DLL1(...) rset(ANA_DLL1, 0, __VA_ARGS__)
#define RFLD_ANA_DLL1(fld) (*((const volatile ANA_DLL1_T *)0xb800060c)).f.fld

typedef union {
	struct {
		unsigned int no_use_hw_rst_out_n:1; //1
		unsigned int no_use_pmd_done:1; //0
		unsigned int voip_cnt_start:1; //0
		unsigned int no_use28:2; //0
		unsigned int pcm_spd_mode:1; //0
		unsigned int mbz_0:3; //0
		unsigned int ocp_ext_dbg_sel:7; //0
		unsigned int voip_cnt:16; //0
	} f;
	unsigned int v;
} SMCR_T;
#define SMCRrv (*((regval)0xb8000610))
#define SMCRdv (0x80000000)
#define RMOD_SMCR(...) rset(SMCR, SMCRrv, __VA_ARGS__)
#define RIZS_SMCR(...) rset(SMCR, 0, __VA_ARGS__)
#define RFLD_SMCR(fld) (*((const volatile SMCR_T *)0xb8000610)).f.fld

typedef union {
	struct {
		unsigned int genr0_rst_n:1; //1
		unsigned int mini0_rst_n:1; //1
		unsigned int mini1_rst_n:1; //1
		unsigned int mini2_rst_n:1; //1
		unsigned int mbz_0:12; //0
		unsigned int genr0_wire_sel:1; //0
		unsigned int genr0_ro_sel:3; //0
		unsigned int mini0_wire_sel:1; //0
		unsigned int mini0_ro_sel:3; //0
		unsigned int mini1_wire_sel:1; //0
		unsigned int mini1_ro_sel:3; //0
		unsigned int mini2_wire_sel:1; //0
		unsigned int mini2_ro_sel:3; //0
	} f;
	unsigned int v;
} SPD_SENSOR0_T;
#define SPD_SENSOR0rv (*((regval)0xb8000618))
#define SPD_SENSOR0dv (0xf0000000)
#define RMOD_SPD_SENSOR0(...) rset(SPD_SENSOR0, SPD_SENSOR0rv, __VA_ARGS__)
#define RIZS_SPD_SENSOR0(...) rset(SPD_SENSOR0, 0, __VA_ARGS__)
#define RFLD_SPD_SENSOR0(fld) (*((const volatile SPD_SENSOR0_T *)0xb8000618)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:1; //0
		unsigned int genr0_dss_ready:1; //0
		unsigned int genr0_wrost_go:1; //0
		unsigned int mbz_1:9; //0
		unsigned int genr0_dss_cnt_out:20; //0
	} f;
	unsigned int v;
} SPD_SENSOR1_T;
#define SPD_SENSOR1rv (*((regval)0xb800061c))
#define SPD_SENSOR1dv (0x00000000)
#define RMOD_SPD_SENSOR1(...) rset(SPD_SENSOR1, SPD_SENSOR1rv, __VA_ARGS__)
#define RIZS_SPD_SENSOR1(...) rset(SPD_SENSOR1, 0, __VA_ARGS__)
#define RFLD_SPD_SENSOR1(fld) (*((const volatile SPD_SENSOR1_T *)0xb800061c)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:1; //0
		unsigned int mini0_dss_ready:1; //0
		unsigned int mini0_wrost_go:1; //0
		unsigned int mbz_1:9; //0
		unsigned int mini0_dss_cnt_out:20; //0
	} f;
	unsigned int v;
} SPD_SENSOR2_T;
#define SPD_SENSOR2rv (*((regval)0xb8000620))
#define SPD_SENSOR2dv (0x00000000)
#define RMOD_SPD_SENSOR2(...) rset(SPD_SENSOR2, SPD_SENSOR2rv, __VA_ARGS__)
#define RIZS_SPD_SENSOR2(...) rset(SPD_SENSOR2, 0, __VA_ARGS__)
#define RFLD_SPD_SENSOR2(fld) (*((const volatile SPD_SENSOR2_T *)0xb8000620)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:1; //0
		unsigned int mini1_dss_ready:1; //0
		unsigned int mini1_wrost_go:1; //0
		unsigned int mbz_1:9; //0
		unsigned int mini1_dss_cnt_out:20; //0
	} f;
	unsigned int v;
} SPD_SENSOR3_T;
#define SPD_SENSOR3rv (*((regval)0xb8000624))
#define SPD_SENSOR3dv (0x00000000)
#define RMOD_SPD_SENSOR3(...) rset(SPD_SENSOR3, SPD_SENSOR3rv, __VA_ARGS__)
#define RIZS_SPD_SENSOR3(...) rset(SPD_SENSOR3, 0, __VA_ARGS__)
#define RFLD_SPD_SENSOR3(fld) (*((const volatile SPD_SENSOR3_T *)0xb8000624)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:1; //0
		unsigned int mini2_dss_ready:1; //0
		unsigned int mini2_wrost_go:1; //0
		unsigned int mbz_1:9; //0
		unsigned int mini2_dss_cnt_out:20; //0
	} f;
	unsigned int v;
} SPD_SENSOR4_T;
#define SPD_SENSOR4rv (*((regval)0xb8000628))
#define SPD_SENSOR4dv (0x00000000)
#define RMOD_SPD_SENSOR4(...) rset(SPD_SENSOR4, SPD_SENSOR4rv, __VA_ARGS__)
#define RIZS_SPD_SENSOR4(...) rset(SPD_SENSOR4, 0, __VA_ARGS__)
#define RFLD_SPD_SENSOR4(fld) (*((const volatile SPD_SENSOR4_T *)0xb8000628)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:11; //0
		unsigned int genr_speed_en:1; //0
		unsigned int genr_dss_data:20; //0
	} f;
	unsigned int v;
} SPD_SENSOR5_T;
#define SPD_SENSOR5rv (*((regval)0xb800062c))
#define SPD_SENSOR5dv (0x00000000)
#define RMOD_SPD_SENSOR5(...) rset(SPD_SENSOR5, SPD_SENSOR5rv, __VA_ARGS__)
#define RIZS_SPD_SENSOR5(...) rset(SPD_SENSOR5, 0, __VA_ARGS__)
#define RFLD_SPD_SENSOR5(fld) (*((const volatile SPD_SENSOR5_T *)0xb800062c)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:11; //0
		unsigned int mini0_speed_en:1; //0
		unsigned int mini0_dss_data:20; //0
	} f;
	unsigned int v;
} SPD_SENSOR6_T;
#define SPD_SENSOR6rv (*((regval)0xb8000630))
#define SPD_SENSOR6dv (0x00000000)
#define RMOD_SPD_SENSOR6(...) rset(SPD_SENSOR6, SPD_SENSOR6rv, __VA_ARGS__)
#define RIZS_SPD_SENSOR6(...) rset(SPD_SENSOR6, 0, __VA_ARGS__)
#define RFLD_SPD_SENSOR6(fld) (*((const volatile SPD_SENSOR6_T *)0xb8000630)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:11; //0
		unsigned int mini1_speed_en:1; //0
		unsigned int mini1_dss_data:20; //0
	} f;
	unsigned int v;
} SPD_SENSOR7_T;
#define SPD_SENSOR7rv (*((regval)0xb8000634))
#define SPD_SENSOR7dv (0x00000000)
#define RMOD_SPD_SENSOR7(...) rset(SPD_SENSOR7, SPD_SENSOR7rv, __VA_ARGS__)
#define RIZS_SPD_SENSOR7(...) rset(SPD_SENSOR7, 0, __VA_ARGS__)
#define RFLD_SPD_SENSOR7(fld) (*((const volatile SPD_SENSOR7_T *)0xb8000634)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:11; //0
		unsigned int mini2_speed_en:1; //0
		unsigned int mini2_dss_data:20; //0
	} f;
	unsigned int v;
} SPD_SENSOR8_T;
#define SPD_SENSOR8rv (*((regval)0xb8000638))
#define SPD_SENSOR8dv (0x00000000)
#define RMOD_SPD_SENSOR8(...) rset(SPD_SENSOR8, SPD_SENSOR8rv, __VA_ARGS__)
#define RIZS_SPD_SENSOR8(...) rset(SPD_SENSOR8, 0, __VA_ARGS__)
#define RFLD_SPD_SENSOR8(fld) (*((const volatile SPD_SENSOR8_T *)0xb8000638)).f.fld

/*-----------------------------------------------------
 Extraced from file_MEM_GLB.xml
-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int dram_type:4; //1
		unsigned int boot_sel:4; //0
		unsigned int ip_ref:1; //0
		unsigned int dp_ref:1; //0
		unsigned int eeprom_type:1; //1
		unsigned int d_signal:1; //0
		unsigned int flash_map0_dis:1; //0
		unsigned int flash_map1_dis:1; //1
		unsigned int mbz_0:3; //0
		unsigned int d_init_trig:1; //0
		unsigned int ocp1_frq_slower:1; //0
		unsigned int lx1_frq_slower:1; //0
		unsigned int lx2_frq_slower:1; //0
		unsigned int lx3_frq_slower:1; //0
		unsigned int ocp0_frq_slower:1; //0
		unsigned int ocp1_rbf_mask_en:1; //0
		unsigned int ocp0_rbf_mask_en:1; //0
		unsigned int ocp1_rbf_f_dis:1; //1
		unsigned int ocp0_rbf_f_dis:1; //1
		unsigned int sync_lx0_dram:1; //0
		unsigned int sync_lx1_dram:1; //0
		unsigned int sync_lx2_dram:1; //0
		unsigned int sync_lx3_dram:1; //0
		unsigned int sync_ocp0_dram:1; //0
	} f;
	unsigned int v;
} MCR_T;
#define MCRrv (*((regval)0xb8001000))
#define MCRdv (0x10200060)
#define RMOD_MCR(...) rset(MCR, MCRrv, __VA_ARGS__)
#define RIZS_MCR(...) rset(MCR, 0, __VA_ARGS__)
#define RFLD_MCR(fld) (*((const volatile MCR_T *)0xb8001000)).f.fld

typedef union {
	struct {
		unsigned int no_use31:2; //0
		unsigned int bankcnt:2; //1
		unsigned int no_use27:2; //0
		unsigned int dbuswid:2; //1
		unsigned int rowcnt:4; //0
		unsigned int colcnt:4; //0
		unsigned int dchipsel:1; //1
		unsigned int fast_rx:1; //0
		unsigned int bstref:1; //0
		unsigned int no_use12:13; //0
	} f;
	unsigned int v;
} DCR_T;
#define DCRrv (*((regval)0xb8001004))
#define DCRdv (0x11008000)
#define RMOD_DCR(...) rset(DCR, DCRrv, __VA_ARGS__)
#define RIZS_DCR(...) rset(DCR, 0, __VA_ARGS__)
#define RFLD_DCR(fld) (*((const volatile DCR_T *)0xb8001004)).f.fld

typedef union {
	struct {
		unsigned int t_cas:4; //2
		unsigned int t_wr:4; //3
		unsigned int t_cwl:4; //0
		unsigned int t_rtp:4; //15
		unsigned int t_wtr:4; //15
		unsigned int t_refi:4; //0
		unsigned int t_refi_unit:4; //4
		unsigned int t_cas_phy:4; //2
	} f;
	unsigned int v;
} DTR0_T;
#define DTR0rv (*((regval)0xb8001008))
#define DTR0dv (0x230ff042)
#define RMOD_DTR0(...) rset(DTR0, DTR0rv, __VA_ARGS__)
#define RIZS_DTR0(...) rset(DTR0, 0, __VA_ARGS__)
#define RFLD_DTR0(fld) (*((const volatile DTR0_T *)0xb8001008)).f.fld

typedef union {
	struct {
		unsigned int no_use31:3; //0
		unsigned int t_rp:5; //31
		unsigned int no_use23:3; //0
		unsigned int t_rcd:5; //31
		unsigned int no_use15:3; //0
		unsigned int t_rrd:5; //31
		unsigned int no_use07:3; //0
		unsigned int t_fawg:5; //31
	} f;
	unsigned int v;
} DTR1_T;
#define DTR1rv (*((regval)0xb800100c))
#define DTR1dv (0x1f1f1f1f)
#define RMOD_DTR1(...) rset(DTR1, DTR1rv, __VA_ARGS__)
#define RIZS_DTR1(...) rset(DTR1, 0, __VA_ARGS__)
#define RFLD_DTR1(fld) (*((const volatile DTR1_T *)0xb800100c)).f.fld

typedef union {
	struct {
		unsigned int no_use31:4; //0
		unsigned int t_rfc:8; //255
		unsigned int no_use19:2; //0
		unsigned int t_ras:6; //63
		unsigned int no_use11:12; //0
	} f;
	unsigned int v;
} DTR2_T;
#define DTR2rv (*((regval)0xb8001010))
#define DTR2dv (0x0ff3f000)
#define RMOD_DTR2(...) rset(DTR2, DTR2rv, __VA_ARGS__)
#define RIZS_DTR2(...) rset(DTR2, 0, __VA_ARGS__)
#define RFLD_DTR2(fld) (*((const volatile DTR2_T *)0xb8001010)).f.fld

typedef union {
	struct {
		unsigned int dtr_up_busy_mrs_busy:1; //0
		unsigned int mbz_0:5; //0
		unsigned int en_wr_leveling:1; //0
		unsigned int dis_dram_ref:1; //0
		unsigned int no_use23:3; //0
		unsigned int mr_mode_en:1; //0
		unsigned int no_use19:2; //0
		unsigned int mr_mode:2; //0
		unsigned int no_use15:2; //0
		unsigned int mr_data:14; //0
	} f;
	unsigned int v;
} DMCR_T;
#define DMCRrv (*((regval)0xb800101c))
#define DMCRdv (0x00000000)
#define RMOD_DMCR(...) rset(DMCR, DMCRrv, __VA_ARGS__)
#define RIZS_DMCR(...) rset(DMCR, 0, __VA_ARGS__)
#define RFLD_DMCR(fld) (*((const volatile DMCR_T *)0xb800101c)).f.fld

typedef union {
	struct {
		unsigned int ocp_req:32; //2863311530
	} f;
	unsigned int v;
} GIAR0_T;
#define GIAR0rv (*((regval)0xb8001020))
#define GIAR0dv (0xaaaaaaaa)
#define RMOD_GIAR0(...) rset(GIAR0, GIAR0rv, __VA_ARGS__)
#define RIZS_GIAR0(...) rset(GIAR0, 0, __VA_ARGS__)
#define RFLD_GIAR0(fld) (*((const volatile GIAR0_T *)0xb8001020)).f.fld

typedef union {
	struct {
		unsigned int lx_req:32; //1431655765
	} f;
	unsigned int v;
} GIAR1_T;
#define GIAR1rv (*((regval)0xb8001024))
#define GIAR1dv (0x55555555)
#define RMOD_GIAR1(...) rset(GIAR1, GIAR1rv, __VA_ARGS__)
#define RIZS_GIAR1(...) rset(GIAR1, 0, __VA_ARGS__)
#define RFLD_GIAR1(fld) (*((const volatile GIAR1_T *)0xb8001024)).f.fld

typedef union {
	struct {
		unsigned int lx0_req:32; //3991793133
	} f;
	unsigned int v;
} LXIAR0_T;
#define LXIAR0rv (*((regval)0xb8001028))
#define LXIAR0dv (0xedededed)
#define RMOD_LXIAR0(...) rset(LXIAR0, LXIAR0rv, __VA_ARGS__)
#define RIZS_LXIAR0(...) rset(LXIAR0, 0, __VA_ARGS__)
#define RFLD_LXIAR0(fld) (*((const volatile LXIAR0_T *)0xb8001028)).f.fld

typedef union {
	struct {
		unsigned int lx1_req:32; //3082270647
	} f;
	unsigned int v;
} LXIAR1_T;
#define LXIAR1rv (*((regval)0xb800102c))
#define LXIAR1dv (0xb7b7b7b7)
#define RMOD_LXIAR1(...) rset(LXIAR1, LXIAR1rv, __VA_ARGS__)
#define RIZS_LXIAR1(...) rset(LXIAR1, 0, __VA_ARGS__)
#define RFLD_LXIAR1(fld) (*((const volatile LXIAR1_T *)0xb800102c)).f.fld

typedef union {
	struct {
		unsigned int lx2_req:32; //3739147998
	} f;
	unsigned int v;
} LXIAR2_T;
#define LXIAR2rv (*((regval)0xb8001030))
#define LXIAR2dv (0xdededede)
#define RMOD_LXIAR2(...) rset(LXIAR2, LXIAR2rv, __VA_ARGS__)
#define RIZS_LXIAR2(...) rset(LXIAR2, 0, __VA_ARGS__)
#define RFLD_LXIAR2(fld) (*((const volatile LXIAR2_T *)0xb8001030)).f.fld

typedef union {
	struct {
		unsigned int lx3_req:32; //2071690107
	} f;
	unsigned int v;
} LXIAR3_T;
#define LXIAR3rv (*((regval)0xb8001034))
#define LXIAR3dv (0x7b7b7b7b)
#define RMOD_LXIAR3(...) rset(LXIAR3, LXIAR3rv, __VA_ARGS__)
#define RIZS_LXIAR3(...) rset(LXIAR3, 0, __VA_ARGS__)
#define RFLD_LXIAR3(fld) (*((const volatile LXIAR3_T *)0xb8001034)).f.fld

typedef union {
	struct {
		unsigned int flush_ocp_cmd:1; //0
		unsigned int dram_cmd_going:1; //1
		unsigned int mbz_0:30; //0
	} f;
	unsigned int v;
} MSRR_T;
#define MSRRrv (*((regval)0xb8001038))
#define MSRRdv (0x40000000)
#define RMOD_MSRR(...) rset(MSRR, MSRRrv, __VA_ARGS__)
#define RIZS_MSRR(...) rset(MSRR, 0, __VA_ARGS__)
#define RFLD_MSRR(fld) (*((const volatile MSRR_T *)0xb8001038)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:28; //0
		unsigned int dbg_mem_ctrl_sel:4; //0
	} f;
	unsigned int v;
} MCDSR_T;
#define MCDSRrv (*((regval)0xb800103c))
#define MCDSRdv (0x00000000)
#define RMOD_MCDSR(...) rset(MCDSR, MCDSRrv, __VA_ARGS__)
#define RIZS_MCDSR(...) rset(MCDSR, 0, __VA_ARGS__)
#define RFLD_MCDSR(fld) (*((const volatile MCDSR_T *)0xb800103c)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:2; //0
		unsigned int pm_mode:2; //0
		unsigned int t_cke:4; //15
		unsigned int mbz_1:2; //0
		unsigned int trsd:10; //1023
		unsigned int mbz_2:2; //0
		unsigned int t_xsref:10; //1023
	} f;
	unsigned int v;
} MPMR0_T;
#define MPMR0rv (*((regval)0xb8001040))
#define MPMR0dv (0x0f3ff3ff)
#define RMOD_MPMR0(...) rset(MPMR0, MPMR0rv, __VA_ARGS__)
#define RIZS_MPMR0(...) rset(MPMR0, 0, __VA_ARGS__)
#define RFLD_MPMR0(fld) (*((const volatile MPMR0_T *)0xb8001040)).f.fld

typedef union {
	struct {
		unsigned int t_xard:4; //15
		unsigned int t_axpd:4; //15
		unsigned int mbz_0:24; //0
	} f;
	unsigned int v;
} MPMR1_T;
#define MPMR1rv (*((regval)0xb8001044))
#define MPMR1dv (0xff000000)
#define RMOD_MPMR1(...) rset(MPMR1, MPMR1rv, __VA_ARGS__)
#define RIZS_MPMR1(...) rset(MPMR1, 0, __VA_ARGS__)
#define RFLD_MPMR1(fld) (*((const volatile MPMR1_T *)0xb8001044)).f.fld

typedef union {
	struct {
		unsigned int dqs0_en_hclk:1; //0
		unsigned int mbz_0:2; //0
		unsigned int dqs0_en_tap:5; //0
		unsigned int dqs1_en_hclk:1; //0
		unsigned int mbz_1:2; //0
		unsigned int dqs1_en_tap:5; //0
		unsigned int mbz_2:16; //0
	} f;
	unsigned int v;
} DIDER_T;
#define DIDERrv (*((regval)0xb8001050))
#define DIDERdv (0x00000000)
#define RMOD_DIDER(...) rset(DIDER, DIDERrv, __VA_ARGS__)
#define RIZS_DIDER(...) rset(DIDER, 0, __VA_ARGS__)
#define RFLD_DIDER(fld) (*((const volatile DIDER_T *)0xb8001050)).f.fld

typedef union {
	struct {
		unsigned int ocd_dt1:16; //0
		unsigned int ocd_dt0:16; //0
	} f;
	unsigned int v;
} D2ODR0_T;
#define D2ODR0rv (*((regval)0xb8001074))
#define D2ODR0dv (0x00000000)
#define RMOD_D2ODR0(...) rset(D2ODR0, D2ODR0rv, __VA_ARGS__)
#define RIZS_D2ODR0(...) rset(D2ODR0, 0, __VA_ARGS__)
#define RFLD_D2ODR0(fld) (*((const volatile D2ODR0_T *)0xb8001074)).f.fld

typedef union {
	struct {
		unsigned int ocd_dt3:16; //0
		unsigned int ocd_dt2:16; //0
	} f;
	unsigned int v;
} D2ODR1_T;
#define D2ODR1rv (*((regval)0xb8001078))
#define D2ODR1dv (0x00000000)
#define RMOD_D2ODR1(...) rset(D2ODR1, D2ODR1rv, __VA_ARGS__)
#define RIZS_D2ODR1(...) rset(D2ODR1, 0, __VA_ARGS__)
#define RFLD_D2ODR1(fld) (*((const volatile D2ODR1_T *)0xb8001078)).f.fld

typedef union {
	struct {
		unsigned int odt_always_on:1; //0
		unsigned int te_alwyas_on:1; //0
		unsigned int mbz_0:30; //0
	} f;
	unsigned int v;
} D23OSCR_T;
#define D23OSCRrv (*((regval)0xb800107c))
#define D23OSCRdv (0x00000000)
#define RMOD_D23OSCR(...) rset(D23OSCR, D23OSCRrv, __VA_ARGS__)
#define RIZS_D23OSCR(...) rset(D23OSCR, 0, __VA_ARGS__)
#define RFLD_D23OSCR(fld) (*((const volatile D23OSCR_T *)0xb800107c)).f.fld

typedef union {
	struct {
		unsigned int zq_long_tri:1; //0
		unsigned int zq_short_en:1; //0
		unsigned int mbz_0:7; //0
		unsigned int t_zqcs:7; //127
		unsigned int mbz_1:16; //0
	} f;
	unsigned int v;
} D3ZQCCR_T;
#define D3ZQCCRrv (*((regval)0xb8001080))
#define D3ZQCCRdv (0x007f0000)
#define RMOD_D3ZQCCR(...) rset(D3ZQCCR, D3ZQCCRrv, __VA_ARGS__)
#define RIZS_D3ZQCCR(...) rset(D3ZQCCR, 0, __VA_ARGS__)
#define RFLD_D3ZQCCR(fld) (*((const volatile D3ZQCCR_T *)0xb8001080)).f.fld

typedef union {
	struct {
		unsigned int zq_pad_force_on:1; //0
		unsigned int mbz_0:2; //0
		unsigned int zctrl_clk_sel:1; //1
		unsigned int trim_mode:4; //15
		unsigned int mbz_1:24; //0
	} f;
	unsigned int v;
} DDZQPR_T;
#define DDZQPRrv (*((regval)0xb8001090))
#define DDZQPRdv (0x1f000000)
#define RMOD_DDZQPR(...) rset(DDZQPR, DDZQPRrv, __VA_ARGS__)
#define RIZS_DDZQPR(...) rset(DDZQPR, 0, __VA_ARGS__)
#define RFLD_DDZQPR(fld) (*((const volatile DDZQPR_T *)0xb8001090)).f.fld

typedef union {
	struct {
		unsigned int zctrl_start:1; //0
		unsigned int zq_pad_period_cali_en:1; //0
		unsigned int mbz_0:16; //0
		unsigned int zprog:14; //0
	} f;
	unsigned int v;
} DDZQPCR_T;
#define DDZQPCRrv (*((regval)0xb8001094))
#define DDZQPCRdv (0x00000000)
#define RMOD_DDZQPCR(...) rset(DDZQPCR, DDZQPCRrv, __VA_ARGS__)
#define RIZS_DDZQPCR(...) rset(DDZQPCR, 0, __VA_ARGS__)
#define RFLD_DDZQPCR(fld) (*((const volatile DDZQPCR_T *)0xb8001094)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:1; //0
		unsigned int zctrl_status:31; //0
	} f;
	unsigned int v;
} DDZQPSR_T;
#define DDZQPSRrv (*((regval)0xb8001098))
#define DDZQPSRdv (0x00000000)
#define RMOD_DDZQPSR(...) rset(DDZQPSR, DDZQPSRrv, __VA_ARGS__)
#define RIZS_DDZQPSR(...) rset(DDZQPSR, 0, __VA_ARGS__)
#define RFLD_DDZQPSR(fld) (*((const volatile DDZQPSR_T *)0xb8001098)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:28; //0
		unsigned int err_flag:4; //0
	} f;
	unsigned int v;
} MCESR0_T;
#define MCESR0rv (*((regval)0xb80010a0))
#define MCESR0dv (0x00000000)
#define RMOD_MCESR0(...) rset(MCESR0, MCESR0rv, __VA_ARGS__)
#define RIZS_MCESR0(...) rset(MCESR0, 0, __VA_ARGS__)
#define RFLD_MCESR0(fld) (*((const volatile MCESR0_T *)0xb80010a0)).f.fld

typedef union {
	struct {
		unsigned int err1_dq15r_flag:1; //0
		unsigned int err1_dq14r_flag:1; //0
		unsigned int err1_dq13r_flag:1; //0
		unsigned int err1_dq12r_flag:1; //0
		unsigned int err1_dq11r_flag:1; //0
		unsigned int err1_dq10r_flag:1; //0
		unsigned int err1_dq9r_flag:1; //0
		unsigned int err1_dq8r_flag:1; //0
		unsigned int err1_dq7r_flag:1; //0
		unsigned int err1_dq6r_flag:1; //0
		unsigned int err1_dq5r_flag:1; //0
		unsigned int err1_dq4r_flag:1; //0
		unsigned int err1_dq3r_flag:1; //0
		unsigned int err1_dq2r_flag:1; //0
		unsigned int err1_dq1r_flag:1; //0
		unsigned int err1_dq0r_flag:1; //0
		unsigned int err1_dq15f_flag:1; //0
		unsigned int err1_dq14f_flag:1; //0
		unsigned int err1_dq13f_flag:1; //0
		unsigned int err1_dq12f_flag:1; //0
		unsigned int err1_dq11f_flag:1; //0
		unsigned int err1_dq10f_flag:1; //0
		unsigned int err1_dq9f_flag:1; //0
		unsigned int err1_dq8f_flag:1; //0
		unsigned int err1_dq7f_flag:1; //0
		unsigned int err1_dq6f_flag:1; //0
		unsigned int err1_dq5f_flag:1; //0
		unsigned int err1_dq4f_flag:1; //0
		unsigned int err1_dq3f_flag:1; //0
		unsigned int err1_dq2f_flag:1; //0
		unsigned int err1_dq1f_flag:1; //0
		unsigned int err1_dq0f_flag:1; //0
	} f;
	unsigned int v;
} MCESR1_T;
#define MCESR1rv (*((regval)0xb80010a4))
#define MCESR1dv (0x00000000)
#define RMOD_MCESR1(...) rset(MCESR1, MCESR1rv, __VA_ARGS__)
#define RIZS_MCESR1(...) rset(MCESR1, 0, __VA_ARGS__)
#define RFLD_MCESR1(fld) (*((const volatile MCESR1_T *)0xb80010a4)).f.fld

typedef union {
	struct {
		unsigned int err2_dq15r_flag:1; //0
		unsigned int err2_dq14r_flag:1; //0
		unsigned int err2_dq13r_flag:1; //0
		unsigned int err2_dq12r_flag:1; //0
		unsigned int err2_dq11r_flag:1; //0
		unsigned int err2_dq10r_flag:1; //0
		unsigned int err2_dq9r_flag:1; //0
		unsigned int err2_dq8r_flag:1; //0
		unsigned int err2_dq7r_flag:1; //0
		unsigned int err2_dq6r_flag:1; //0
		unsigned int err2_dq5r_flag:1; //0
		unsigned int err2_dq4r_flag:1; //0
		unsigned int err2_dq3r_flag:1; //0
		unsigned int err2_dq2r_flag:1; //0
		unsigned int err2_dq1r_flag:1; //0
		unsigned int err2_dq0r_flag:1; //0
		unsigned int err2_dq15f_flag:1; //0
		unsigned int err2_dq14f_flag:1; //0
		unsigned int err2_dq13f_flag:1; //0
		unsigned int err2_dq12f_flag:1; //0
		unsigned int err2_dq11f_flag:1; //0
		unsigned int err2_dq10f_flag:1; //0
		unsigned int err2_dq9f_flag:1; //0
		unsigned int err2_dq8f_flag:1; //0
		unsigned int err2_dq7f_flag:1; //0
		unsigned int err2_dq6f_flag:1; //0
		unsigned int err2_dq5f_flag:1; //0
		unsigned int err2_dq4f_flag:1; //0
		unsigned int err2_dq3f_flag:1; //0
		unsigned int err2_dq2f_flag:1; //0
		unsigned int err2_dq1f_flag:1; //0
		unsigned int err2_dq0f_flag:1; //0
	} f;
	unsigned int v;
} MCESR2_T;
#define MCESR2rv (*((regval)0xb80010a8))
#define MCESR2dv (0x00000000)
#define RMOD_MCESR2(...) rset(MCESR2, MCESR2rv, __VA_ARGS__)
#define RIZS_MCESR2(...) rset(MCESR2, 0, __VA_ARGS__)
#define RFLD_MCESR2(fld) (*((const volatile MCESR2_T *)0xb80010a8)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:16; //0
		unsigned int mbz_1:2; //0
		unsigned int lx3_jt_num:2; //0
		unsigned int mbz_2:2; //0
		unsigned int lx2_jt_num:2; //0
		unsigned int mbz_3:2; //0
		unsigned int lx1_jt_num:2; //0
		unsigned int mbz_4:2; //0
		unsigned int lx0_jt_num:2; //0
	} f;
	unsigned int v;
} MCLJTCR_T;
#define MCLJTCRrv (*((regval)0xb80010b0))
#define MCLJTCRdv (0x00000000)
#define RMOD_MCLJTCR(...) rset(MCLJTCR, MCLJTCRrv, __VA_ARGS__)
#define RIZS_MCLJTCR(...) rset(MCLJTCR, 0, __VA_ARGS__)
#define RFLD_MCLJTCR(fld) (*((const volatile MCLJTCR_T *)0xb80010b0)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:19; //0
		unsigned int lx3_bpre_dis:1; //0
		unsigned int mbz_1:3; //0
		unsigned int lx2_bpre_dis:1; //0
		unsigned int mbz_2:3; //0
		unsigned int lx1_bpre_dis:1; //0
		unsigned int mbz_3:3; //0
		unsigned int lx0_bpre_dis:1; //0
	} f;
	unsigned int v;
} MCLPBRCR_T;
#define MCLPBRCRrv (*((regval)0xb80010b4))
#define MCLPBRCRdv (0x00000000)
#define RMOD_MCLPBRCR(...) rset(MCLPBRCR, MCLPBRCRrv, __VA_ARGS__)
#define RIZS_MCLPBRCR(...) rset(MCLPBRCR, 0, __VA_ARGS__)
#define RFLD_MCLPBRCR(fld) (*((const volatile MCLPBRCR_T *)0xb80010b4)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:3; //0
		unsigned int ocp0_raw_dis:1; //0
		unsigned int mbz_1:3; //0
		unsigned int ocp1_raw_dis:1; //0
		unsigned int mbz_2:3; //0
		unsigned int lx0_raw_dis:1; //0
		unsigned int mbz_3:3; //0
		unsigned int lx1_raw_dis:1; //0
		unsigned int mbz_4:3; //0
		unsigned int lx2_raw_dis:1; //0
		unsigned int mbz_5:3; //0
		unsigned int lx3_raw_dis:1; //0
		unsigned int mbz_6:3; //0
		unsigned int ocp0_ocp1_raw_en:1; //0
		unsigned int mbz_7:4; //0
	} f;
	unsigned int v;
} MCERAWCR_T;
#define MCERAWCRrv (*((regval)0xb80010c0))
#define MCERAWCRdv (0x00000000)
#define RMOD_MCERAWCR(...) rset(MCERAWCR, MCERAWCRrv, __VA_ARGS__)
#define RIZS_MCERAWCR(...) rset(MCERAWCR, 0, __VA_ARGS__)
#define RFLD_MCERAWCR(fld) (*((const volatile MCERAWCR_T *)0xb80010c0)).f.fld

/*-----------------------------------------------------
 Extraced from file_MEM_VER.xml
-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int mcv_year:8; //20
		unsigned int mcv_date:16; //532
		unsigned int mcv_hour:8; //0
	} f;
	unsigned int v;
} MCVR_T;
#define MCVRrv (*((regval)0xb80010f8))
#define MCVRdv (0x14021400)
#define RMOD_MCVR(...) rset(MCVR, MCVRrv, __VA_ARGS__)
#define RIZS_MCVR(...) rset(MCVR, 0, __VA_ARGS__)
#define RFLD_MCVR(fld) (*((const volatile MCVR_T *)0xb80010f8)).f.fld

typedef union {
	struct {
		unsigned int soc_plat_num:16; //1593
		unsigned int soc_plat_cut_num:4; //1
		unsigned int mbz_0:12; //0
	} f;
	unsigned int v;
} SOCPNR_T;
#define SOCPNRrv (*((regval)0xb80010fc))
#define SOCPNRdv (0x06391000)
#define RMOD_SOCPNR(...) rset(SOCPNR, SOCPNRrv, __VA_ARGS__)
#define RIZS_SOCPNR(...) rset(SOCPNR, 0, __VA_ARGS__)
#define RFLD_SOCPNR(fld) (*((const volatile SOCPNR_T *)0xb80010fc)).f.fld

/*-----------------------------------------------------
 Extraced from file_MEM_SPI_FLASH.xml
-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int spi_clk_div:3; //7
		unsigned int rbo:1; //1
		unsigned int wbo:1; //1
		unsigned int spi_tcs:5; //31
		unsigned int no_use21:1; //0
		unsigned int mbz_0:8; //0
		unsigned int ocp0_frq_slower:1; //0
		unsigned int mbz_1:2; //0
		unsigned int hw_reset_en:1; //1
		unsigned int rst_cmd_dis:1; //1
		unsigned int spi_tchsh:4; //4
		unsigned int spi_tslch:4; //4
	} f;
	unsigned int v;
} SFCR_T;
#define SFCRrv (*((regval)0xb8001200))
#define SFCRdv (0xffc00344)
#define RMOD_SFCR(...) rset(SFCR, SFCRrv, __VA_ARGS__)
#define RIZS_SFCR(...) rset(SFCR, 0, __VA_ARGS__)
#define RFLD_SFCR(fld) (*((const volatile SFCR_T *)0xb8001200)).f.fld

typedef union {
	struct {
		unsigned int sfcmd:8; //3
		unsigned int sfsize:3; //7
		unsigned int rd_opt:1; //0
		unsigned int cmd_io:2; //0
		unsigned int addr_io:2; //0
		unsigned int mbz_0:3; //0
		unsigned int data_io:2; //0
		unsigned int hold_till_sfdr2:1; //0
		unsigned int mmio_4b_addr_en:1; //0
		unsigned int mbz_1:8; //0
		unsigned int pio_en:1; //0
	} f;
	unsigned int v;
} SFCR2_T;
#define SFCR2rv (*((regval)0xb8001204))
#define SFCR2dv (0x03e00000)
#define RMOD_SFCR2(...) rset(SFCR2, SFCR2rv, __VA_ARGS__)
#define RIZS_SFCR2(...) rset(SFCR2, 0, __VA_ARGS__)
#define RFLD_SFCR2(fld) (*((const volatile SFCR2_T *)0xb8001204)).f.fld

typedef union {
	struct {
		unsigned int spi_csb0:1; //1
		unsigned int spi_csb1:1; //1
		unsigned int len:2; //3
		unsigned int spi_rdy:1; //1
		unsigned int io_width:2; //0
		unsigned int chip_sel:1; //0
		unsigned int cmd_byte:8; //0
		unsigned int no_use15:4; //0
		unsigned int spi_csb0_sts:1; //1
		unsigned int spi_csb1_sts:1; //1
		unsigned int no_use09:5; //0
		unsigned int spi_idle:1; //0
		unsigned int mbz_0:3; //0
		unsigned int mio_pio_err:1; //0
	} f;
	unsigned int v;
} SFCSR_T;
#define SFCSRrv (*((regval)0xb8001208))
#define SFCSRdv (0xf8000c00)
#define RMOD_SFCSR(...) rset(SFCSR, SFCSRrv, __VA_ARGS__)
#define RIZS_SFCSR(...) rset(SFCSR, 0, __VA_ARGS__)
#define RFLD_SFCSR(fld) (*((const volatile SFCSR_T *)0xb8001208)).f.fld

typedef union {
	struct {
		unsigned int data3:8; //0
		unsigned int data2:8; //0
		unsigned int data1:8; //0
		unsigned int data0:8; //0
	} f;
	unsigned int v;
} SFDR_T;
#define SFDRrv (*((regval)0xb800120c))
#define SFDRdv (0x00000000)
#define RMOD_SFDR(...) rset(SFDR, SFDRrv, __VA_ARGS__)
#define RIZS_SFDR(...) rset(SFDR, 0, __VA_ARGS__)
#define RFLD_SFDR(fld) (*((const volatile SFDR_T *)0xb800120c)).f.fld

typedef union {
	struct {
		unsigned int data3:8; //0
		unsigned int data2:8; //0
		unsigned int data1:8; //0
		unsigned int data0:8; //0
	} f;
	unsigned int v;
} SFDR2_T;
#define SFDR2rv (*((regval)0xb8001210))
#define SFDR2dv (0x00000000)
#define RMOD_SFDR2(...) rset(SFDR2, SFDR2rv, __VA_ARGS__)
#define RIZS_SFDR2(...) rset(SFDR2, 0, __VA_ARGS__)
#define RFLD_SFDR2(fld) (*((const volatile SFDR2_T *)0xb8001210)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:3; //0
		unsigned int io3_delay:5; //0
		unsigned int mbz_1:3; //0
		unsigned int io2_delay:5; //0
		unsigned int mbz_2:3; //0
		unsigned int io1_delay:5; //0
		unsigned int mbz_3:3; //0
		unsigned int io0_delay:5; //0
	} f;
	unsigned int v;
} SFRDR_T;
#define SFRDRrv (*((regval)0xb8001218))
#define SFRDRdv (0x00000000)
#define RMOD_SFRDR(...) rset(SFRDR, SFRDRrv, __VA_ARGS__)
#define RIZS_SFRDR(...) rset(SFRDR, 0, __VA_ARGS__)
#define RFLD_SFRDR(fld) (*((const volatile SFRDR_T *)0xb8001218)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:28; //0
		unsigned int rst_rcv_time:4; //11
	} f;
	unsigned int v;
} SFPPR_T;
#define SFPPRrv (*((regval)0xb800121c))
#define SFPPRdv (0x0000000b)
#define RMOD_SFPPR(...) rset(SFPPR, SFPPRrv, __VA_ARGS__)
#define RIZS_SFPPR(...) rset(SFPPR, 0, __VA_ARGS__)
#define RFLD_SFPPR(fld) (*((const volatile SFPPR_T *)0xb800121c)).f.fld

/*-----------------------------------------------------
 Extraced from file_MEM_UNMAP.xml
-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int no_use31:1; //0
		unsigned int addr:23; //0
		unsigned int no_use07:7; //0
		unsigned int unmap_en:1; //0
	} f;
	unsigned int v;
} UMSAR0_T;
#define UMSAR0rv (*((regval)0xb8001300))
#define UMSAR0dv (0x00000000)
#define RMOD_UMSAR0(...) rset(UMSAR0, UMSAR0rv, __VA_ARGS__)
#define RIZS_UMSAR0(...) rset(UMSAR0, 0, __VA_ARGS__)
#define RFLD_UMSAR0(fld) (*((const volatile UMSAR0_T *)0xb8001300)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:28; //0
		unsigned int size:4; //8
	} f;
	unsigned int v;
} UMSSR0_T;
#define UMSSR0rv (*((regval)0xb8001304))
#define UMSSR0dv (0x00000008)
#define RMOD_UMSSR0(...) rset(UMSSR0, UMSSR0rv, __VA_ARGS__)
#define RIZS_UMSSR0(...) rset(UMSSR0, 0, __VA_ARGS__)
#define RFLD_UMSSR0(fld) (*((const volatile UMSSR0_T *)0xb8001304)).f.fld

typedef union {
	struct {
		unsigned int no_use31:1; //0
		unsigned int addr:23; //0
		unsigned int no_use07:7; //0
		unsigned int unmap_en:1; //0
	} f;
	unsigned int v;
} UMSAR1_T;
#define UMSAR1rv (*((regval)0xb8001310))
#define UMSAR1dv (0x00000000)
#define RMOD_UMSAR1(...) rset(UMSAR1, UMSAR1rv, __VA_ARGS__)
#define RIZS_UMSAR1(...) rset(UMSAR1, 0, __VA_ARGS__)
#define RFLD_UMSAR1(fld) (*((const volatile UMSAR1_T *)0xb8001310)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:28; //0
		unsigned int size:4; //8
	} f;
	unsigned int v;
} UMSSR1_T;
#define UMSSR1rv (*((regval)0xb8001314))
#define UMSSR1dv (0x00000008)
#define RMOD_UMSSR1(...) rset(UMSSR1, UMSSR1rv, __VA_ARGS__)
#define RIZS_UMSSR1(...) rset(UMSSR1, 0, __VA_ARGS__)
#define RFLD_UMSSR1(fld) (*((const volatile UMSSR1_T *)0xb8001314)).f.fld

typedef union {
	struct {
		unsigned int no_use31:1; //0
		unsigned int addr:23; //0
		unsigned int no_use07:7; //0
		unsigned int unmap_en:1; //0
	} f;
	unsigned int v;
} UMSAR2_T;
#define UMSAR2rv (*((regval)0xb8001320))
#define UMSAR2dv (0x00000000)
#define RMOD_UMSAR2(...) rset(UMSAR2, UMSAR2rv, __VA_ARGS__)
#define RIZS_UMSAR2(...) rset(UMSAR2, 0, __VA_ARGS__)
#define RFLD_UMSAR2(fld) (*((const volatile UMSAR2_T *)0xb8001320)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:28; //0
		unsigned int size:4; //8
	} f;
	unsigned int v;
} UMSSR2_T;
#define UMSSR2rv (*((regval)0xb8001324))
#define UMSSR2dv (0x00000008)
#define RMOD_UMSSR2(...) rset(UMSSR2, UMSSR2rv, __VA_ARGS__)
#define RIZS_UMSSR2(...) rset(UMSSR2, 0, __VA_ARGS__)
#define RFLD_UMSSR2(fld) (*((const volatile UMSSR2_T *)0xb8001324)).f.fld

typedef union {
	struct {
		unsigned int no_use31:1; //0
		unsigned int addr:23; //0
		unsigned int no_use07:7; //0
		unsigned int enunmap:1; //0
	} f;
	unsigned int v;
} UMSAR3_T;
#define UMSAR3rv (*((regval)0xb8001330))
#define UMSAR3dv (0x00000000)
#define RMOD_UMSAR3(...) rset(UMSAR3, UMSAR3rv, __VA_ARGS__)
#define RIZS_UMSAR3(...) rset(UMSAR3, 0, __VA_ARGS__)
#define RFLD_UMSAR3(fld) (*((const volatile UMSAR3_T *)0xb8001330)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:28; //0
		unsigned int size:4; //8
	} f;
	unsigned int v;
} UMSSR3_T;
#define UMSSR3rv (*((regval)0xb8001334))
#define UMSSR3dv (0x00000008)
#define RMOD_UMSSR3(...) rset(UMSSR3, UMSSR3rv, __VA_ARGS__)
#define RIZS_UMSSR3(...) rset(UMSSR3, 0, __VA_ARGS__)
#define RFLD_UMSSR3(fld) (*((const volatile UMSSR3_T *)0xb8001334)).f.fld

/*-----------------------------------------------------
 Extraced from file_MEM_DDR_PHY.xml
-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int ac_mode:1; //1
		unsigned int dqs_se:1; //0
		unsigned int mbz_0:9; //0
		unsigned int dqs0_group_tap:5; //0
		unsigned int mbz_1:3; //0
		unsigned int dqs1_group_tap:5; //0
		unsigned int mbz_2:2; //0
		unsigned int ac_dyn_bptr_clr_en:1; //0
		unsigned int ac_bptr_clear:1; //0
		unsigned int ac_debug_sel:4; //0
	} f;
	unsigned int v;
} DACCR_T;
#define DACCRrv (*((regval)0xb8001500))
#define DACCRdv (0x80000000)
#define RMOD_DACCR(...) rset(DACCR, DACCRrv, __VA_ARGS__)
#define RIZS_DACCR(...) rset(DACCR, 0, __VA_ARGS__)
#define RFLD_DACCR(fld) (*((const volatile DACCR_T *)0xb8001500)).f.fld

typedef union {
	struct {
		unsigned int ac_silen_period_en:1; //0
		unsigned int ac_silen_gen_en:1; //0
		unsigned int mbz_0:9; //0
		unsigned int ac_silen_trig:1; //0
		unsigned int ac_silen_period_unit:4; //0
		unsigned int ac_silen_period:8; //0
		unsigned int ac_silen_len:8; //0
	} f;
	unsigned int v;
} DACSPCR_T;
#define DACSPCRrv (*((regval)0xb8001504))
#define DACSPCRdv (0x00000000)
#define RMOD_DACSPCR(...) rset(DACSPCR, DACSPCRrv, __VA_ARGS__)
#define RIZS_DACSPCR(...) rset(DACSPCR, 0, __VA_ARGS__)
#define RFLD_DACSPCR(fld) (*((const volatile DACSPCR_T *)0xb8001504)).f.fld

typedef union {
	struct {
		unsigned int ac_silen_addr:32; //0
	} f;
	unsigned int v;
} DACSPAR_T;
#define DACSPARrv (*((regval)0xb8001508))
#define DACSPARdv (0x00000000)
#define RMOD_DACSPAR(...) rset(DACSPAR, DACSPARrv, __VA_ARGS__)
#define RIZS_DACSPAR(...) rset(DACSPAR, 0, __VA_ARGS__)
#define RFLD_DACSPAR(fld) (*((const volatile DACSPAR_T *)0xb8001508)).f.fld

typedef union {
	struct {
		unsigned int ac_sps_dq15r:1; //0
		unsigned int ac_sps_dq14r:1; //0
		unsigned int ac_sps_dq13r:1; //0
		unsigned int ac_sps_dq12r:1; //0
		unsigned int ac_sps_dq11r:1; //0
		unsigned int ac_sps_dq10r:1; //0
		unsigned int ac_sps_dq9r:1; //0
		unsigned int ac_sps_dq8r:1; //0
		unsigned int ac_sps_dq7r:1; //0
		unsigned int ac_sps_dq6r:1; //0
		unsigned int ac_sps_dq5r:1; //0
		unsigned int ac_sps_dq4r:1; //0
		unsigned int ac_sps_dq3r:1; //0
		unsigned int ac_sps_dq2r:1; //0
		unsigned int ac_sps_dq1r:1; //0
		unsigned int ac_sps_dq0r:1; //0
		unsigned int ac_sps_dq15f:1; //0
		unsigned int ac_sps_dq14f:1; //0
		unsigned int ac_sps_dq13f:1; //0
		unsigned int ac_sps_dq12f:1; //0
		unsigned int ac_sps_dq11f:1; //0
		unsigned int ac_sps_dq10f:1; //0
		unsigned int ac_sps_dq9f:1; //0
		unsigned int ac_sps_dq8f:1; //0
		unsigned int ac_sps_dq7f:1; //0
		unsigned int ac_sps_dq6f:1; //0
		unsigned int ac_sps_dq5f:1; //0
		unsigned int ac_sps_dq4f:1; //0
		unsigned int ac_sps_dq3f:1; //0
		unsigned int ac_sps_dq2f:1; //0
		unsigned int ac_sps_dq1f:1; //0
		unsigned int ac_sps_dq0f:1; //0
	} f;
	unsigned int v;
} DACSPSR_T;
#define DACSPSRrv (*((regval)0xb800150c))
#define DACSPSRdv (0x00000000)
#define RMOD_DACSPSR(...) rset(DACSPSR, DACSPSRrv, __VA_ARGS__)
#define RIZS_DACSPSR(...) rset(DACSPSR, 0, __VA_ARGS__)
#define RFLD_DACSPSR(fld) (*((const volatile DACSPSR_T *)0xb800150c)).f.fld

typedef union {
	struct {
		unsigned int dqn_r_ac_en:1; //0
		unsigned int mbz_0:2; //0
		unsigned int dqn_phase_shift_90:5; //0
		unsigned int mbz_1:3; //0
		unsigned int dqn_r_ac_max_tap:5; //31
		unsigned int mbz_2:3; //0
		unsigned int dqn_r_ac_cur_tap:5; //15
		unsigned int mbz_3:3; //0
		unsigned int dqn_r_ac_min_tap:5; //0
	} f;
	unsigned int v;
} DACDQ_IDX_RR_T;
#define DACDQ_IDX_RRrv (*((regval)0xb8001510))
#define DACDQ_IDX_RRdv (0x001f0f00)
#define RMOD_DACDQ_IDX_RR(...) rset(DACDQ_IDX_RR, DACDQ_IDX_RRrv, __VA_ARGS__)
#define RIZS_DACDQ_IDX_RR(...) rset(DACDQ_IDX_RR, 0, __VA_ARGS__)
#define RFLD_DACDQ_IDX_RR(fld) (*((const volatile DACDQ_IDX_RR_T *)0xb8001510)).f.fld

typedef union {
	struct {
		unsigned int dqn_f_ac_en:1; //0
		unsigned int mbz_0:2; //0
		unsigned int mbz_1:5; //0
		unsigned int mbz_2:3; //0
		unsigned int dqn_f_ac_max_tap:5; //31
		unsigned int mbz_3:3; //0
		unsigned int dqn_f_ac_cur_tap:5; //15
		unsigned int mbz_4:3; //0
		unsigned int dqn_f_ac_min_tap:5; //0
	} f;
	unsigned int v;
} DACDQ_IDX_FR_T;
#define DACDQ_IDX_FRrv (*((regval)0xb8001550))
#define DACDQ_IDX_FRdv (0x001f0f00)
#define RMOD_DACDQ_IDX_FR(...) rset(DACDQ_IDX_FR, DACDQ_IDX_FRrv, __VA_ARGS__)
#define RIZS_DACDQ_IDX_FR(...) rset(DACDQ_IDX_FR, 0, __VA_ARGS__)
#define RFLD_DACDQ_IDX_FR(fld) (*((const volatile DACDQ_IDX_FR_T *)0xb8001514)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:3; //0
		unsigned int dqm0_phase_shift_90:5; //0
		unsigned int mbz_1:3; //0
		unsigned int dqm1_phase_shift_90:5; //0
		unsigned int mbz_2:16; //0
	} f;
	unsigned int v;
} DCDQMR_T;
#define DCDQMRrv (*((regval)0xb8001590))
#define DCDQMRdv (0x00000000)
#define RMOD_DCDQMR(...) rset(DCDQMR, DCDQMRrv, __VA_ARGS__)
#define RIZS_DCDQMR(...) rset(DCDQMR, 0, __VA_ARGS__)
#define RFLD_DCDQMR(fld) (*((const volatile DCDQMR_T *)0xb8001518)).f.fld

typedef union {
	struct {
		unsigned int dq7r_fifo_d:4; //0
		unsigned int dq6r_fifo_d:4; //0
		unsigned int dq5r_fifo_d:4; //0
		unsigned int dq4r_fifo_d:4; //0
		unsigned int dq3r_fifo_d:4; //0
		unsigned int dq2r_fifo_d:3; //0
		unsigned int dq1r_fifo_d:5; //0
		unsigned int dq0r_fifo_d:4; //0
	} f;
	unsigned int v;
} DCFDRR0_T;
#define DCFDRR0rv (*((regval)0xb8001594))
#define DCFDRR0dv (0x00000000)
#define RMOD_DCFDRR0(...) rset(DCFDRR0, DCFDRR0rv, __VA_ARGS__)
#define RIZS_DCFDRR0(...) rset(DCFDRR0, 0, __VA_ARGS__)
#define RFLD_DCFDRR0(fld) (*((const volatile DCFDRR0_T *)0xb800151c)).f.fld

typedef union {
	struct {
		unsigned int dq15r_fifo_d:4; //0
		unsigned int dq14r_fifo_d:4; //0
		unsigned int dq13r_fifo_d:4; //0
		unsigned int dq12r_fifo_d:4; //0
		unsigned int dq11r_fifo_d:4; //0
		unsigned int dq10r_fifo_d:3; //0
		unsigned int dq9r_fifo_d:5; //0
		unsigned int dq8r_fifo_d:4; //0
	} f;
	unsigned int v;
} DCFDRR1_T;
#define DCFDRR1rv (*((regval)0xb8001598))
#define DCFDRR1dv (0x00000000)
#define RMOD_DCFDRR1(...) rset(DCFDRR1, DCFDRR1rv, __VA_ARGS__)
#define RIZS_DCFDRR1(...) rset(DCFDRR1, 0, __VA_ARGS__)
#define RFLD_DCFDRR1(fld) (*((const volatile DCFDRR1_T *)0xb8001520)).f.fld

typedef union {
	struct {
		unsigned int dq7f_fifo_d:4; //0
		unsigned int dq6f_fifo_d:4; //0
		unsigned int dq5f_fifo_d:4; //0
		unsigned int dq4f_fifo_d:4; //0
		unsigned int dq3f_fifo_d:4; //0
		unsigned int dq2f_fifo_d:3; //0
		unsigned int dq1f_fifo_d:5; //0
		unsigned int dq0f_fifo_d:4; //0
	} f;
	unsigned int v;
} DCFDFR0_T;
#define DCFDFR0rv (*((regval)0xb800159c))
#define DCFDFR0dv (0x00000000)
#define RMOD_DCFDFR0(...) rset(DCFDFR0, DCFDFR0rv, __VA_ARGS__)
#define RIZS_DCFDFR0(...) rset(DCFDFR0, 0, __VA_ARGS__)
#define RFLD_DCFDFR0(fld) (*((const volatile DCFDFR0_T *)0xb8001524)).f.fld

typedef union {
	struct {
		unsigned int dq15f_fifo_d:4; //0
		unsigned int dq14f_fifo_d:4; //0
		unsigned int dq13f_fifo_d:4; //0
		unsigned int dq12f_fifo_d:4; //0
		unsigned int dq11f_fifo_d:4; //0
		unsigned int dq10f_fifo_d:3; //0
		unsigned int dq9f_fifo_d:5; //0
		unsigned int dq8f_fifo_d:4; //0
	} f;
	unsigned int v;
} DCFDFR1_T;
#define DCFDFR1rv (*((regval)0xb80015a0))
#define DCFDFR1dv (0x00000000)
#define RMOD_DCFDFR1(...) rset(DCFDFR1, DCFDFR1rv, __VA_ARGS__)
#define RIZS_DCFDFR1(...) rset(DCFDFR1, 0, __VA_ARGS__)
#define RFLD_DCFDFR1(fld) (*((const volatile DCFDFR1_T *)0xb8001528)).f.fld

typedef union {
	struct {
		unsigned int fifo_d_dbg_en:1; //0
		unsigned int mbz_0:30; //0
		unsigned int fifo_d_sel:1; //0
	} f;
	unsigned int v;
} DCFDDCR_T;
#define DCFDDCRrv (*((regval)0xb80015a4))
#define DCFDDCRdv (0x00000000)
#define RMOD_DCFDDCR(...) rset(DCFDDCR, DCFDDCRrv, __VA_ARGS__)
#define RIZS_DCFDDCR(...) rset(DCFDDCR, 0, __VA_ARGS__)
#define RFLD_DCFDDCR(fld) (*((const volatile DCFDDCR_T *)0xb800152c)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:4; //0
		unsigned int cke_dly_tap:4; //0
		unsigned int cs1_dly_tap:4; //0
		unsigned int cs0_dly_tap:4; //0
		unsigned int odt_dly_tap:4; //0
		unsigned int ras_dly_tap:4; //0
		unsigned int cas_dly_tap:4; //0
		unsigned int we_dly_tap:4; //0
	} f;
	unsigned int v;
} DCSDCR0_T;
#define DCSDCR0rv (*((regval)0xb80015b0))
#define DCSDCR0dv (0x00000000)
#define RMOD_DCSDCR0(...) rset(DCSDCR0, DCSDCR0rv, __VA_ARGS__)
#define RIZS_DCSDCR0(...) rset(DCSDCR0, 0, __VA_ARGS__)
#define RFLD_DCSDCR0(fld) (*((const volatile DCSDCR0_T *)0xb8001538)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:3; //0
		unsigned int dqs1_dly_tap:5; //0
		unsigned int mbz_1:3; //0
		unsigned int dqs0_dly_tap:5; //0
		unsigned int mbz_2:4; //0
		unsigned int ba2_dly_tap:4; //0
		unsigned int ba1_dly_tap:4; //0
		unsigned int ba0_dly_tap:4; //0
	} f;
	unsigned int v;
} DCSDCR1_T;
#define DCSDCR1rv (*((regval)0xb80015b4))
#define DCSDCR1dv (0x00000000)
#define RMOD_DCSDCR1(...) rset(DCSDCR1, DCSDCR1rv, __VA_ARGS__)
#define RIZS_DCSDCR1(...) rset(DCSDCR1, 0, __VA_ARGS__)
#define RFLD_DCSDCR1(fld) (*((const volatile DCSDCR1_T *)0xb800153c)).f.fld

typedef union {
	struct {
		unsigned int a7_dly_tap:4; //0
		unsigned int a6_dly_tap:4; //0
		unsigned int a5_dly_tap:4; //0
		unsigned int a4_dly_tap:4; //0
		unsigned int a3_dly_tap:4; //0
		unsigned int a2_dly_tap:4; //0
		unsigned int a1_dly_tap:4; //0
		unsigned int a0_dly_tap:4; //0
	} f;
	unsigned int v;
} DCSDCR2_T;
#define DCSDCR2rv (*((regval)0xb80015b8))
#define DCSDCR2dv (0x00000000)
#define RMOD_DCSDCR2(...) rset(DCSDCR2, DCSDCR2rv, __VA_ARGS__)
#define RIZS_DCSDCR2(...) rset(DCSDCR2, 0, __VA_ARGS__)
#define RFLD_DCSDCR2(fld) (*((const volatile DCSDCR2_T *)0xb8001540)).f.fld

typedef union {
	struct {
		unsigned int a15_dly_tap:4; //0
		unsigned int a14_dly_tap:4; //0
		unsigned int a13_dly_tap:4; //0
		unsigned int a12_dly_tap:4; //0
		unsigned int a11_dly_tap:4; //0
		unsigned int a10_dly_tap:4; //0
		unsigned int a9_dly_tap:4; //0
		unsigned int a8_dly_tap:4; //0
	} f;
	unsigned int v;
} DCSDCR3_T;
#define DCSDCR3rv (*((regval)0xb80015bc))
#define DCSDCR3dv (0x00000000)
#define RMOD_DCSDCR3(...) rset(DCSDCR3, DCSDCR3rv, __VA_ARGS__)
#define RIZS_DCSDCR3(...) rset(DCSDCR3, 0, __VA_ARGS__)
#define RFLD_DCSDCR3(fld) (*((const volatile DCSDCR3_T *)0xb8001544)).f.fld

/*-----------------------------------------------------
 Extraced from file_MEM_CACHE.xml
-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int enable:1; //0
		unsigned int cache_src:3; //0
		unsigned int mbz_0:27; //0
		unsigned int invalidate:1; //0
	} f;
	unsigned int v;
} MCCCR_T;
#define MCCCRrv (*((regval)0xb8001600))
#define MCCCRdv (0x00000000)
#define RMOD_MCCCR(...) rset(MCCCR, MCCCRrv, __VA_ARGS__)
#define RIZS_MCCCR(...) rset(MCCCR, 0, __VA_ARGS__)
#define RFLD_MCCCR(fld) (*((const volatile MCCCR_T *)0xb8001600)).f.fld

typedef union {
	struct {
		unsigned int mc_badr:32; //532676608
	} f;
	unsigned int v;
} MCBAR_T;
#define MCBARrv (*((regval)0xb8001604))
#define MCBARdv (0x1fc00000)
#define RMOD_MCBAR(...) rset(MCBAR, MCBARrv, __VA_ARGS__)
#define RIZS_MCBAR(...) rset(MCBAR, 0, __VA_ARGS__)
#define RFLD_MCBAR(fld) (*((const volatile MCBAR_T *)0xb8001604)).f.fld

typedef union {
	struct {
		unsigned int mc_madr:32; //4294836224
	} f;
	unsigned int v;
} MCMR_T;
#define MCMRrv (*((regval)0xb8001608))
#define MCMRdv (0xfffe0000)
#define RMOD_MCMR(...) rset(MCMR, MCMRrv, __VA_ARGS__)
#define RIZS_MCMR(...) rset(MCMR, 0, __VA_ARGS__)
#define RFLD_MCMR(fld) (*((const volatile MCMR_T *)0xb8001608)).f.fld

typedef union {
	struct {
		unsigned int trig:1; //0
		unsigned int tag_idx:6; //0
		unsigned int dw_idx:6; //0
		unsigned int mode:3; //0
		unsigned int mbz_0:16; //0
	} f;
	unsigned int v;
} MCDCSR_T;
#define MCDCSRrv (*((regval)0xb800160c))
#define MCDCSRdv (0x00000000)
#define RMOD_MCDCSR(...) rset(MCDCSR, MCDCSRrv, __VA_ARGS__)
#define RIZS_MCDCSR(...) rset(MCDCSR, 0, __VA_ARGS__)
#define RFLD_MCDCSR(fld) (*((const volatile MCDCSR_T *)0xb800160c)).f.fld

typedef union {
	struct {
		unsigned int tag:31; //0
		unsigned int dw_valid:1; //0
	} f;
	unsigned int v;
} MCDDR0_T;
#define MCDDR0rv (*((regval)0xb8001610))
#define MCDDR0dv (0x00000000)
#define RMOD_MCDDR0(...) rset(MCDDR0, MCDDR0rv, __VA_ARGS__)
#define RIZS_MCDDR0(...) rset(MCDDR0, 0, __VA_ARGS__)
#define RFLD_MCDDR0(fld) (*((const volatile MCDDR0_T *)0xb8001610)).f.fld

typedef union {
	struct {
		unsigned int dw_data:32; //0
	} f;
	unsigned int v;
} MCDDR1_T;
#define MCDDR1rv (*((regval)0xb8001614))
#define MCDDR1dv (0x00000000)
#define RMOD_MCDDR1(...) rset(MCDDR1, MCDDR1rv, __VA_ARGS__)
#define RIZS_MCDDR1(...) rset(MCDDR1, 0, __VA_ARGS__)
#define RFLD_MCDDR1(fld) (*((const volatile MCDDR1_T *)0xb8001614)).f.fld

/*-----------------------------------------------------
 Extraced from file_MEM_ARB.xml
-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int arb_clear:1; //0
		unsigned int arb_new_arb_en:1; //0
		unsigned int mbz_0:11; //0
		unsigned int arb_slot_sel_fast:1; //0
		unsigned int arb_dec_borrow_sel:2; //0
		unsigned int arb_idle_cnt:16; //256
	} f;
	unsigned int v;
} HASR_T;
#define HASRrv (*((regval)0xb8001800))
#define HASRdv (0x00000100)
#define RMOD_HASR(...) rset(HASR, HASRrv, __VA_ARGS__)
#define RIZS_HASR(...) rset(HASR, 0, __VA_ARGS__)
#define RFLD_HASR(fld) (*((const volatile HASR_T *)0xb8001800)).f.fld

typedef union {
	struct {
		unsigned int arb_dec_issue:1; //0
		unsigned int arb_dec_dir:1; //0
		unsigned int mbz_0:6; //0
		unsigned int idle_cnt_en:1; //0
		unsigned int lxo_cnt_en:1; //0
		unsigned int lxp_cnt_en:1; //0
		unsigned int ocp_cnt_en:1; //0
		unsigned int idle_cnt_rst:1; //0
		unsigned int lxo_cnt_rst:1; //0
		unsigned int lxp_cnt_rst:1; //0
		unsigned int ocp_cnt_rst:1; //0
		unsigned int mbz_1:13; //0
		unsigned int arb_dec_idx:3; //0
	} f;
	unsigned int v;
} ARB_CTRL_T;
#define ARB_CTRLrv (*((regval)0xb8001804))
#define ARB_CTRLdv (0x00000000)
#define RMOD_ARB_CTRL(...) rset(ARB_CTRL, ARB_CTRLrv, __VA_ARGS__)
#define RIZS_ARB_CTRL(...) rset(ARB_CTRL, 0, __VA_ARGS__)
#define RFLD_ARB_CTRL(fld) (*((const volatile ARB_CTRL_T *)0xb8001804)).f.fld

typedef union {
	struct {
		unsigned int arb_dec_write_msk:32; //0
	} f;
	unsigned int v;
} ARB_WRITE_MSK_T;
#define ARB_WRITE_MSKrv (*((regval)0xb8001808))
#define ARB_WRITE_MSKdv (0x00000000)
#define RMOD_ARB_WRITE_MSK(...) rset(ARB_WRITE_MSK, ARB_WRITE_MSKrv, __VA_ARGS__)
#define RIZS_ARB_WRITE_MSK(...) rset(ARB_WRITE_MSK, 0, __VA_ARGS__)
#define RFLD_ARB_WRITE_MSK(fld) (*((const volatile ARB_WRITE_MSK_T *)0xb8001808)).f.fld

typedef union {
	struct {
		unsigned int arb_dec_data:32; //0
	} f;
	unsigned int v;
} ARB_DATA_T;
#define ARB_DATArv (*((regval)0xb800180c))
#define ARB_DATAdv (0x00000000)
#define RMOD_ARB_DATA(...) rset(ARB_DATA, ARB_DATArv, __VA_ARGS__)
#define RIZS_ARB_DATA(...) rset(ARB_DATA, 0, __VA_ARGS__)
#define RFLD_ARB_DATA(fld) (*((const volatile ARB_DATA_T *)0xb800180c)).f.fld

typedef union {
	struct {
		unsigned int iso2_msk:32; //2863311530
	} f;
	unsigned int v;
} ARB_MSK0_T;
#define ARB_MSK0rv (*((regval)0xb8001810))
#define ARB_MSK0dv (0xaaaaaaaa)
#define RMOD_ARB_MSK0(...) rset(ARB_MSK0, ARB_MSK0rv, __VA_ARGS__)
#define RIZS_ARB_MSK0(...) rset(ARB_MSK0, 0, __VA_ARGS__)
#define RFLD_ARB_MSK0(fld) (*((const volatile ARB_MSK0_T *)0xb8001810)).f.fld

typedef union {
	struct {
		unsigned int ac4_msk:32; //1431655765
	} f;
	unsigned int v;
} ARB_MSK1_T;
#define ARB_MSK1rv (*((regval)0xb8001814))
#define ARB_MSK1dv (0x55555555)
#define RMOD_ARB_MSK1(...) rset(ARB_MSK1, ARB_MSK1rv, __VA_ARGS__)
#define RIZS_ARB_MSK1(...) rset(ARB_MSK1, 0, __VA_ARGS__)
#define RFLD_ARB_MSK1(fld) (*((const volatile ARB_MSK1_T *)0xb8001814)).f.fld

typedef union {
	struct {
		unsigned int mem_arb_f_msk:32; //0
	} f;
	unsigned int v;
} ARB_MSK2_T;
#define ARB_MSK2rv (*((regval)0xb8001818))
#define ARB_MSK2dv (0x00000000)
#define RMOD_ARB_MSK2(...) rset(ARB_MSK2, ARB_MSK2rv, __VA_ARGS__)
#define RIZS_ARB_MSK2(...) rset(ARB_MSK2, 0, __VA_ARGS__)
#define RFLD_ARB_MSK2(fld) (*((const volatile ARB_MSK2_T *)0xb8001818)).f.fld

typedef union {
	struct {
		unsigned int nen_arb_oc_msk:32; //0
	} f;
	unsigned int v;
} ARB_MSK3_T;
#define ARB_MSK3rv (*((regval)0xb800181c))
#define ARB_MSK3dv (0x00000000)
#define RMOD_ARB_MSK3(...) rset(ARB_MSK3, ARB_MSK3rv, __VA_ARGS__)
#define RIZS_ARB_MSK3(...) rset(ARB_MSK3, 0, __VA_ARGS__)
#define RFLD_ARB_MSK3(fld) (*((const volatile ARB_MSK3_T *)0xb800181c)).f.fld

typedef union {
	struct {
		unsigned int arb_idle_cnt:32; //0
	} f;
	unsigned int v;
} ARB_DBG_COUNTER0_T;
#define ARB_DBG_COUNTER0rv (*((regval)0xb8001820))
#define ARB_DBG_COUNTER0dv (0x00000000)
#define RMOD_ARB_DBG_COUNTER0(...) rset(ARB_DBG_COUNTER0, ARB_DBG_COUNTER0rv, __VA_ARGS__)
#define RIZS_ARB_DBG_COUNTER0(...) rset(ARB_DBG_COUNTER0, 0, __VA_ARGS__)
#define RFLD_ARB_DBG_COUNTER0(fld) (*((const volatile ARB_DBG_COUNTER0_T *)0xb8001820)).f.fld

typedef union {
	struct {
		unsigned int arb_ocp_cnt:32; //0
	} f;
	unsigned int v;
} ARB_DBG_COUNTER1_T;
#define ARB_DBG_COUNTER1rv (*((regval)0xb8001824))
#define ARB_DBG_COUNTER1dv (0x00000000)
#define RMOD_ARB_DBG_COUNTER1(...) rset(ARB_DBG_COUNTER1, ARB_DBG_COUNTER1rv, __VA_ARGS__)
#define RIZS_ARB_DBG_COUNTER1(...) rset(ARB_DBG_COUNTER1, 0, __VA_ARGS__)
#define RFLD_ARB_DBG_COUNTER1(fld) (*((const volatile ARB_DBG_COUNTER1_T *)0xb8001824)).f.fld

typedef union {
	struct {
		unsigned int arb_lxp_cnt:32; //0
	} f;
	unsigned int v;
} ARB_DBG_COUNTER2_T;
#define ARB_DBG_COUNTER2rv (*((regval)0xb8001828))
#define ARB_DBG_COUNTER2dv (0x00000000)
#define RMOD_ARB_DBG_COUNTER2(...) rset(ARB_DBG_COUNTER2, ARB_DBG_COUNTER2rv, __VA_ARGS__)
#define RIZS_ARB_DBG_COUNTER2(...) rset(ARB_DBG_COUNTER2, 0, __VA_ARGS__)
#define RFLD_ARB_DBG_COUNTER2(fld) (*((const volatile ARB_DBG_COUNTER2_T *)0xb8001828)).f.fld

typedef union {
	struct {
		unsigned int arb_lxo_cnt:32; //0
	} f;
	unsigned int v;
} ARB_DBG_COUNTER3_T;
#define ARB_DBG_COUNTER3rv (*((regval)0xb800182c))
#define ARB_DBG_COUNTER3dv (0x00000000)
#define RMOD_ARB_DBG_COUNTER3(...) rset(ARB_DBG_COUNTER3, ARB_DBG_COUNTER3rv, __VA_ARGS__)
#define RIZS_ARB_DBG_COUNTER3(...) rset(ARB_DBG_COUNTER3, 0, __VA_ARGS__)
#define RFLD_ARB_DBG_COUNTER3(fld) (*((const volatile ARB_DBG_COUNTER3_T *)0xb800182c)).f.fld

/*-----------------------------------------------------
 Extraced from file_UART.xml
-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int rbr_thr_dll:8; //0
		unsigned int mbz_0:24; //0
	} f;
	unsigned int v;
} UART_PBR_THR_DLL_T;
#define UART_PBR_THR_DLLrv (*((regval)0xb8002000))
#define UART_PBR_THR_DLLdv (0x00000000)
#define RMOD_UART_PBR_THR_DLL(...) rset(UART_PBR_THR_DLL, UART_PBR_THR_DLLrv, __VA_ARGS__)
#define RIZS_UART_PBR_THR_DLL(...) rset(UART_PBR_THR_DLL, 0, __VA_ARGS__)
#define RFLD_UART_PBR_THR_DLL(fld) (*((const volatile UART_PBR_THR_DLL_T *)0xb8002000)).f.fld

typedef union {
	struct {
		unsigned int ier_dlm:8; //0
		unsigned int mbz_0:24; //0
	} f;
	unsigned int v;
} UART_IER_DLM_T;
#define UART_IER_DLMrv (*((regval)0xb8002004))
#define UART_IER_DLMdv (0x00000000)
#define RMOD_UART_IER_DLM(...) rset(UART_IER_DLM, UART_IER_DLMrv, __VA_ARGS__)
#define RIZS_UART_IER_DLM(...) rset(UART_IER_DLM, 0, __VA_ARGS__)
#define RFLD_UART_IER_DLM(fld) (*((const volatile UART_IER_DLM_T *)0xb8002004)).f.fld

typedef union {
	struct {
		unsigned int iir_fcr_1:2; //3
		unsigned int mbz_0:2; //0
		unsigned int iir_fcr_0:4; //1
		unsigned int mbz_1:24; //0
	} f;
	unsigned int v;
} UART_IIR_FCR_T;
#define UART_IIR_FCRrv (*((regval)0xb8002008))
#define UART_IIR_FCRdv (0xc1000000)
#define RMOD_UART_IIR_FCR(...) rset(UART_IIR_FCR, UART_IIR_FCRrv, __VA_ARGS__)
#define RIZS_UART_IIR_FCR(...) rset(UART_IIR_FCR, 0, __VA_ARGS__)
#define RFLD_UART_IIR_FCR(fld) (*((const volatile UART_IIR_FCR_T *)0xb8002008)).f.fld

typedef union {
	struct {
		unsigned int dlab:1; //0
		unsigned int brk:1; //0
		unsigned int eps:2; //0
		unsigned int pen:1; //0
		unsigned int stb:1; //0
		unsigned int wls_1:1; //1
		unsigned int wls_0:1; //1
		unsigned int mbz_0:24; //0
	} f;
	unsigned int v;
} UART_LCR_T;
#define UART_LCRrv (*((regval)0xb800200c))
#define UART_LCRdv (0x03000000)
#define RMOD_UART_LCR(...) rset(UART_LCR, UART_LCRrv, __VA_ARGS__)
#define RIZS_UART_LCR(...) rset(UART_LCR, 0, __VA_ARGS__)
#define RFLD_UART_LCR(fld) (*((const volatile UART_LCR_T *)0xb800200c)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:1; //0
		unsigned int lxclk_sel:1; //0
		unsigned int afe:1; //0
		unsigned int loop:1; //0
		unsigned int out2:1; //0
		unsigned int out1:1; //0
		unsigned int rts:1; //0
		unsigned int dtr:1; //0
		unsigned int mbz_1:24; //0
	} f;
	unsigned int v;
} UART_MCR_T;
#define UART_MCRrv (*((regval)0xb8002010))
#define UART_MCRdv (0x00000000)
#define RMOD_UART_MCR(...) rset(UART_MCR, UART_MCRrv, __VA_ARGS__)
#define RIZS_UART_MCR(...) rset(UART_MCR, 0, __VA_ARGS__)
#define RFLD_UART_MCR(fld) (*((const volatile UART_MCR_T *)0xb8002010)).f.fld

typedef union {
	struct {
		unsigned int rfe:1; //0
		unsigned int temt:1; //1
		unsigned int thre:1; //1
		unsigned int bi:1; //0
		unsigned int fe:1; //0
		unsigned int pe:1; //0
		unsigned int oe:1; //0
		unsigned int dr:1; //0
		unsigned int mbz_0:24; //0
	} f;
	unsigned int v;
} UART_LSR_T;
#define UART_LSRrv (*((regval)0xb8002014))
#define UART_LSRdv (0x60000000)
#define RMOD_UART_LSR(...) rset(UART_LSR, UART_LSRrv, __VA_ARGS__)
#define RIZS_UART_LSR(...) rset(UART_LSR, 0, __VA_ARGS__)
#define RFLD_UART_LSR(fld) (*((const volatile UART_LSR_T *)0xb8002014)).f.fld

typedef union {
	struct {
		unsigned int dcts:1; //0
		unsigned int ddsr:1; //0
		unsigned int teri:1; //0
		unsigned int ddcd:1; //1
		unsigned int cts:1; //0
		unsigned int dsr:1; //0
		unsigned int ri:1; //0
		unsigned int dcd:1; //0
		unsigned int mbz_0:24; //0
	} f;
	unsigned int v;
} UART_MSR_T;
#define UART_MSRrv (*((regval)0xb8002018))
#define UART_MSRdv (0x10000000)
#define RMOD_UART_MSR(...) rset(UART_MSR, UART_MSRrv, __VA_ARGS__)
#define RIZS_UART_MSR(...) rset(UART_MSR, 0, __VA_ARGS__)
#define RFLD_UART_MSR(fld) (*((const volatile UART_MSR_T *)0xb8002018)).f.fld

/*-----------------------------------------------------
 Extraced from file_INTR_CTRL.xml
-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int tmo_ie:1; //0
		unsigned int voipacc_ie:1; //0
		unsigned int spi_ie:1; //0
		unsigned int ptm_ie:1; //0
		unsigned int gmac1_ie:1; //0
		unsigned int gmac0_ie:1; //0
		unsigned int no_use25:3; //0
		unsigned int pon_ip:1; //0
		unsigned int gdma1_ie:1; //0
		unsigned int gdma0_ie:1; //0
		unsigned int no_use19:2; //0
		unsigned int pcm_ie:1; //0
		unsigned int no_use16:4; //0
		unsigned int peripheral_ie:1; //0
		unsigned int no_use11:3; //0
		unsigned int switch_ie:1; //0
		unsigned int no_use07:4; //0
		unsigned int fftacc_ie:1; //0
		unsigned int no_use02:3; //0
	} f;
	unsigned int v;
} GIMR0_T;
#define GIMR0rv (*((regval)0xb8003000))
#define GIMR0dv (0x00000000)
#define RMOD_GIMR0(...) rset(GIMR0, GIMR0rv, __VA_ARGS__)
#define RIZS_GIMR0(...) rset(GIMR0, 0, __VA_ARGS__)
#define RFLD_GIMR0(fld) (*((const volatile GIMR0_T *)0xb8003000)).f.fld

typedef union {
	struct {
		unsigned int lbctmom2_ie:1; //0
		unsigned int lbctmom1_ie:1; //0
		unsigned int lbctmom0_ie:1; //0
		unsigned int lbctmos3_ie:1; //0
		unsigned int lbctmos2_ie:1; //0
		unsigned int lbctmos1_ie:1; //0
		unsigned int lbctmos0_ie:1; //0
		unsigned int no_use24:1; //0
		unsigned int ocpto0_ie:1; //0
		unsigned int no_use22:1; //0
		unsigned int ocp0_cpu1_ila_ip:1; //0
		unsigned int uart3_ie:1; //0
		unsigned int uart2_ie:1; //0
		unsigned int uart1_ie:1; //0
		unsigned int uart0_ie:1; //0
		unsigned int tc5_ie:1; //0
		unsigned int tc4_ie:1; //0
		unsigned int tc3_ie:1; //0
		unsigned int tc2_ie:1; //0
		unsigned int tc1_ie:1; //0
		unsigned int tc0_ie:1; //0
		unsigned int gpio1_ie:1; //0
		unsigned int gpio0_ie:1; //0
		unsigned int wdt_ph2to_ie:1; //0
		unsigned int wdt_ph1to_ie:1; //0
		unsigned int tc5_dly_int_ie:1; //0
		unsigned int tc4_dly_int_ie:1; //0
		unsigned int tc3_dly_int_ie:1; //0
		unsigned int tc2_dly_int_ie:1; //0
		unsigned int tc1_dly_int_ie:1; //0
		unsigned int tc0_dly_int_ie:1; //0
		unsigned int no_use00:1; //0
	} f;
	unsigned int v;
} GIMR1_T;
#define GIMR1rv (*((regval)0xb8003004))
#define GIMR1dv (0x00000000)
#define RMOD_GIMR1(...) rset(GIMR1, GIMR1rv, __VA_ARGS__)
#define RIZS_GIMR1(...) rset(GIMR1, 0, __VA_ARGS__)
#define RFLD_GIMR1(fld) (*((const volatile GIMR1_T *)0xb8003004)).f.fld

typedef union {
	struct {
		unsigned int tmo_ip:1; //0
		unsigned int voipacc_ip:1; //0
		unsigned int spi_ip:1; //0
		unsigned int xsi_ip:1; //0
		unsigned int gmac1_ip:1; //0
		unsigned int gmac0_ip:1; //0
		unsigned int mbz_0:3; //0
		unsigned int pon_ip:1; //0
		unsigned int gdma1_ip:1; //0
		unsigned int gdma0_ip:1; //0
		unsigned int mbz_1:2; //0
		unsigned int pcm1_ip:1; //0
		unsigned int mbz_2:4; //0
		unsigned int peripheral_ip:1; //0
		unsigned int mbz_3:3; //0
		unsigned int switch_ip:1; //0
		unsigned int mbz_4:4; //0
		unsigned int fftacc_ip:1; //0
		unsigned int mbz_5:3; //0
	} f;
	unsigned int v;
} GISR0_T;
#define GISR0rv (*((regval)0xb8003008))
#define GISR0dv (0x00000000)
#define RMOD_GISR0(...) rset(GISR0, GISR0rv, __VA_ARGS__)
#define RIZS_GISR0(...) rset(GISR0, 0, __VA_ARGS__)
#define RFLD_GISR0(fld) (*((const volatile GISR0_T *)0xb8003008)).f.fld

typedef union {
	struct {
		unsigned int lbctmom2_ip:1; //0
		unsigned int lbctmom1_ip:1; //0
		unsigned int lbctmom0_ip:1; //0
		unsigned int lbctmos3_ip:1; //0
		unsigned int lbctmos2_ip:1; //0
		unsigned int lbctmos1_ip:1; //0
		unsigned int lbctmos0_ip:1; //0
		unsigned int ocpto1_ip:1; //0
		unsigned int ocpto0_ip:1; //0
		unsigned int ocp1_cpu0_ila_ip:1; //0
		unsigned int ocp0_cpu1_ila_ip:1; //0
		unsigned int uart3_ip:1; //0
		unsigned int uart2_ip:1; //0
		unsigned int uart1_ip:1; //0
		unsigned int uart0_ip:1; //0
		unsigned int tc5_ip:1; //0
		unsigned int tc4_ip:1; //0
		unsigned int tc3_ip:1; //0
		unsigned int tc2_ip:1; //0
		unsigned int tc1_ip:1; //0
		unsigned int tc0_ip:1; //0
		unsigned int gpio1_ip:1; //0
		unsigned int gpio0_ip:1; //0
		unsigned int wdt_ph2to_ip:1; //0
		unsigned int wdt_ph1to_ip:1; //0
		unsigned int tc5_dly_int_ip:1; //0
		unsigned int tc4_dly_int_ip:1; //0
		unsigned int tc3_dly_int_ip:1; //0
		unsigned int tc2_dly_int_ip:1; //0
		unsigned int tc1_dly_int_ip:1; //0
		unsigned int tc0_dly_int_ip:1; //0
		unsigned int mbz_0:1; //0
	} f;
	unsigned int v;
} GISR1_T;
#define GISR1rv (*((regval)0xb800300c))
#define GISR1dv (0x00000000)
#define RMOD_GISR1(...) rset(GISR1, GISR1rv, __VA_ARGS__)
#define RIZS_GISR1(...) rset(GISR1, 0, __VA_ARGS__)
#define RFLD_GISR1(fld) (*((const volatile GISR1_T *)0xb800300c)).f.fld

typedef union {
	struct {
		unsigned int lbctmom2_rs:4; //0
		unsigned int lbctmom1_rs:4; //0
		unsigned int lbctmom0_rs:4; //0
		unsigned int lbctmos3_rs:4; //0
		unsigned int lbctmos2_rs:4; //0
		unsigned int lbctmos1_rs:4; //0
		unsigned int lbctmos0_rs:4; //0
		unsigned int no_use03:4; //0
	} f;
	unsigned int v;
} IRR0_T;
#define IRR0rv (*((regval)0xb8003010))
#define IRR0dv (0x00000000)
#define RMOD_IRR0(...) rset(IRR0, IRR0rv, __VA_ARGS__)
#define RIZS_IRR0(...) rset(IRR0, 0, __VA_ARGS__)
#define RFLD_IRR0(fld) (*((const volatile IRR0_T *)0xb8003010)).f.fld

typedef union {
	struct {
		unsigned int ocpto0_rs:4; //0
		unsigned int no_use27:4; //0
		unsigned int ocp0_cpu1_ila_rs:4; //0
		unsigned int voipacc_rs:4; //0
		unsigned int spi_rs:4; //0
		unsigned int xsi_rs:4; //0
		unsigned int gmac1_rs:4; //0
		unsigned int gmac0_rs:4; //0
	} f;
	unsigned int v;
} IRR1_T;
#define IRR1rv (*((regval)0xb8003014))
#define IRR1dv (0x00000000)
#define RMOD_IRR1(...) rset(IRR1, IRR1rv, __VA_ARGS__)
#define RIZS_IRR1(...) rset(IRR1, 0, __VA_ARGS__)
#define RFLD_IRR1(fld) (*((const volatile IRR1_T *)0xb8003014)).f.fld

typedef union {
	struct {
		unsigned int no_use31:12; //0
		unsigned int pon_rs:4; //0
		unsigned int gdma1_rs:4; //0
		unsigned int gdma0_rs:4; //0
		unsigned int no_use07:8; //0
	} f;
	unsigned int v;
} IRR2_T;
#define IRR2rv (*((regval)0xb8003018))
#define IRR2dv (0x00000000)
#define RMOD_IRR2(...) rset(IRR2, IRR2rv, __VA_ARGS__)
#define RIZS_IRR2(...) rset(IRR2, 0, __VA_ARGS__)
#define RFLD_IRR2(fld) (*((const volatile IRR2_T *)0xb8003018)).f.fld

typedef union {
	struct {
		unsigned int pcm_rs:4; //0
		unsigned int no_use27:16; //0
		unsigned int uart3_rs:4; //0
		unsigned int uart2_rs:4; //0
		unsigned int uart1_rs:4; //0
	} f;
	unsigned int v;
} IRR3_T;
#define IRR3rv (*((regval)0xb800301c))
#define IRR3dv (0x00000000)
#define RMOD_IRR3(...) rset(IRR3, IRR3rv, __VA_ARGS__)
#define RIZS_IRR3(...) rset(IRR3, 0, __VA_ARGS__)
#define RFLD_IRR3(fld) (*((const volatile IRR3_T *)0xb800301c)).f.fld

typedef union {
	struct {
		unsigned int uart0_rs:4; //0
		unsigned int tc5_rs:4; //0
		unsigned int tc4_rs:4; //0
		unsigned int tc3_rs:4; //0
		unsigned int tc2_rs:4; //0
		unsigned int tc1_rs:4; //0
		unsigned int tc0_rs:4; //0
		unsigned int gpio1_rs:4; //0
	} f;
	unsigned int v;
} IRR4_T;
#define IRR4rv (*((regval)0xb8003020))
#define IRR4dv (0x00000000)
#define RMOD_IRR4(...) rset(IRR4, IRR4rv, __VA_ARGS__)
#define RIZS_IRR4(...) rset(IRR4, 0, __VA_ARGS__)
#define RFLD_IRR4(fld) (*((const volatile IRR4_T *)0xb8003020)).f.fld

typedef union {
	struct {
		unsigned int gpio0_rs:4; //0
		unsigned int switch_rs:4; //0
		unsigned int no_use23:4; //0
		unsigned int wdog_ph2to_rs:4; //0
		unsigned int wdog_ph1to_rs:4; //0
		unsigned int no_use11:4; //0
		unsigned int fftacc_rs:4; //0
		unsigned int no_use03:4; //0
	} f;
	unsigned int v;
} IRR5_T;
#define IRR5rv (*((regval)0xb8003024))
#define IRR5dv (0x00000000)
#define RMOD_IRR5(...) rset(IRR5, IRR5rv, __VA_ARGS__)
#define RIZS_IRR5(...) rset(IRR5, 0, __VA_ARGS__)
#define RFLD_IRR5(fld) (*((const volatile IRR5_T *)0xb8003024)).f.fld

/*-----------------------------------------------------
 Extraced from file_DLY_INTC.xml
-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int iti_trig:1; //0
		unsigned int no_use30:27; //0
		unsigned int delayed_ip_sel:4; //0
	} f;
	unsigned int v;
} TC0_DLY_INTR_T;
#define TC0_DLY_INTRrv (*((regval)0xb8003080))
#define TC0_DLY_INTRdv (0x00000000)
#define RMOD_TC0_DLY_INTR(...) rset(TC0_DLY_INTR, TC0_DLY_INTRrv, __VA_ARGS__)
#define RIZS_TC0_DLY_INTR(...) rset(TC0_DLY_INTR, 0, __VA_ARGS__)
#define RFLD_TC0_DLY_INTR(fld) (*((const volatile TC0_DLY_INTR_T *)0xb8003080)).f.fld

typedef union {
	struct {
		unsigned int iti_trig:1; //0
		unsigned int no_use30:27; //0
		unsigned int delayed_ip_sel:4; //0
	} f;
	unsigned int v;
} TC1_DLY_INTR_T;
#define TC1_DLY_INTRrv (*((regval)0xb8003084))
#define TC1_DLY_INTRdv (0x00000000)
#define RMOD_TC1_DLY_INTR(...) rset(TC1_DLY_INTR, TC1_DLY_INTRrv, __VA_ARGS__)
#define RIZS_TC1_DLY_INTR(...) rset(TC1_DLY_INTR, 0, __VA_ARGS__)
#define RFLD_TC1_DLY_INTR(fld) (*((const volatile TC1_DLY_INTR_T *)0xb8003084)).f.fld

typedef union {
	struct {
		unsigned int iti_trig:1; //0
		unsigned int no_use30:27; //0
		unsigned int delayed_ip_sel:4; //0
	} f;
	unsigned int v;
} TC2_DLY_INTR_T;
#define TC2_DLY_INTRrv (*((regval)0xb8003088))
#define TC2_DLY_INTRdv (0x00000000)
#define RMOD_TC2_DLY_INTR(...) rset(TC2_DLY_INTR, TC2_DLY_INTRrv, __VA_ARGS__)
#define RIZS_TC2_DLY_INTR(...) rset(TC2_DLY_INTR, 0, __VA_ARGS__)
#define RFLD_TC2_DLY_INTR(fld) (*((const volatile TC2_DLY_INTR_T *)0xb8003088)).f.fld

typedef union {
	struct {
		unsigned int iti_trig:1; //0
		unsigned int no_use30:27; //0
		unsigned int delayed_ip_sel:4; //0
	} f;
	unsigned int v;
} TC3_DLY_INTR_T;
#define TC3_DLY_INTRrv (*((regval)0xb800308c))
#define TC3_DLY_INTRdv (0x00000000)
#define RMOD_TC3_DLY_INTR(...) rset(TC3_DLY_INTR, TC3_DLY_INTRrv, __VA_ARGS__)
#define RIZS_TC3_DLY_INTR(...) rset(TC3_DLY_INTR, 0, __VA_ARGS__)
#define RFLD_TC3_DLY_INTR(fld) (*((const volatile TC3_DLY_INTR_T *)0xb800308c)).f.fld

typedef union {
	struct {
		unsigned int iti_trig:1; //0
		unsigned int no_use30:27; //0
		unsigned int delayed_ip_sel:4; //0
	} f;
	unsigned int v;
} TC4_DLY_INTR_T;
#define TC4_DLY_INTRrv (*((regval)0xb8003090))
#define TC4_DLY_INTRdv (0x00000000)
#define RMOD_TC4_DLY_INTR(...) rset(TC4_DLY_INTR, TC4_DLY_INTRrv, __VA_ARGS__)
#define RIZS_TC4_DLY_INTR(...) rset(TC4_DLY_INTR, 0, __VA_ARGS__)
#define RFLD_TC4_DLY_INTR(fld) (*((const volatile TC4_DLY_INTR_T *)0xb8003090)).f.fld

typedef union {
	struct {
		unsigned int iti_trig:1; //0
		unsigned int no_use30:27; //0
		unsigned int delayed_ip_sel:4; //0
	} f;
	unsigned int v;
} TC5_DLY_INTR_T;
#define TC5_DLY_INTRrv (*((regval)0xb8003094))
#define TC5_DLY_INTRdv (0x00000000)
#define RMOD_TC5_DLY_INTR(...) rset(TC5_DLY_INTR, TC5_DLY_INTRrv, __VA_ARGS__)
#define RIZS_TC5_DLY_INTR(...) rset(TC5_DLY_INTR, 0, __VA_ARGS__)
#define RFLD_TC5_DLY_INTR(fld) (*((const volatile TC5_DLY_INTR_T *)0xb8003094)).f.fld

/*-----------------------------------------------------
 Extraced from file_TIMER.xml
-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int mbz_0:4; //0
		unsigned int tc0data:28; //0
	} f;
	unsigned int v;
} TC0DATA_T;
#define TC0DATArv (*((regval)0xb8003200))
#define TC0DATAdv (0x00000000)
#define RMOD_TC0DATA(...) rset(TC0DATA, TC0DATArv, __VA_ARGS__)
#define RIZS_TC0DATA(...) rset(TC0DATA, 0, __VA_ARGS__)
#define RFLD_TC0DATA(fld) (*((const volatile TC0DATA_T *)0xb8003200)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:4; //0
		unsigned int tc0value:28; //0
	} f;
	unsigned int v;
} TC0CNT_T;
#define TC0CNTrv (*((regval)0xb8003204))
#define TC0CNTdv (0x00000000)
#define RMOD_TC0CNT(...) rset(TC0CNT, TC0CNTrv, __VA_ARGS__)
#define RIZS_TC0CNT(...) rset(TC0CNT, 0, __VA_ARGS__)
#define RFLD_TC0CNT(fld) (*((const volatile TC0CNT_T *)0xb8003204)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:3; //0
		unsigned int tc0en:1; //0
		unsigned int mbz_1:3; //0
		unsigned int tc0mode:1; //0
		unsigned int mbz_2:8; //0
		unsigned int tc0divfactor:16; //0
	} f;
	unsigned int v;
} TC0CTRL_T;
#define TC0CTRLrv (*((regval)0xb8003208))
#define TC0CTRLdv (0x00000000)
#define RMOD_TC0CTRL(...) rset(TC0CTRL, TC0CTRLrv, __VA_ARGS__)
#define RIZS_TC0CTRL(...) rset(TC0CTRL, 0, __VA_ARGS__)
#define RFLD_TC0CTRL(fld) (*((const volatile TC0CTRL_T *)0xb8003208)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:11; //0
		unsigned int tc0ie:1; //0
		unsigned int mbz_1:3; //0
		unsigned int tc0ip:1; //0
		unsigned int mbz_2:16; //0
	} f;
	unsigned int v;
} TC0INTR_T;
#define TC0INTRrv (*((regval)0xb800320c))
#define TC0INTRdv (0x00000000)
#define RMOD_TC0INTR(...) rset(TC0INTR, TC0INTRrv, __VA_ARGS__)
#define RIZS_TC0INTR(...) rset(TC0INTR, 0, __VA_ARGS__)
#define RFLD_TC0INTR(fld) (*((const volatile TC0INTR_T *)0xb800320c)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:4; //0
		unsigned int tc1data:28; //0
	} f;
	unsigned int v;
} TC1DATA_T;
#define TC1DATArv (*((regval)0xb8003210))
#define TC1DATAdv (0x00000000)
#define RMOD_TC1DATA(...) rset(TC1DATA, TC1DATArv, __VA_ARGS__)
#define RIZS_TC1DATA(...) rset(TC1DATA, 0, __VA_ARGS__)
#define RFLD_TC1DATA(fld) (*((const volatile TC1DATA_T *)0xb8003210)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:4; //0
		unsigned int tc1value:28; //0
	} f;
	unsigned int v;
} TC1CNTR_T;
#define TC1CNTRrv (*((regval)0xb8003214))
#define TC1CNTRdv (0x00000000)
#define RMOD_TC1CNTR(...) rset(TC1CNTR, TC1CNTRrv, __VA_ARGS__)
#define RIZS_TC1CNTR(...) rset(TC1CNTR, 0, __VA_ARGS__)
#define RFLD_TC1CNTR(fld) (*((const volatile TC1CNTR_T *)0xb8003214)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:3; //0
		unsigned int tc1en:1; //0
		unsigned int mbz_1:3; //0
		unsigned int tc1mode:1; //0
		unsigned int mbz_2:8; //0
		unsigned int tc1divfactor:16; //0
	} f;
	unsigned int v;
} TC1CTRL_T;
#define TC1CTRLrv (*((regval)0xb8003218))
#define TC1CTRLdv (0x00000000)
#define RMOD_TC1CTRL(...) rset(TC1CTRL, TC1CTRLrv, __VA_ARGS__)
#define RIZS_TC1CTRL(...) rset(TC1CTRL, 0, __VA_ARGS__)
#define RFLD_TC1CTRL(fld) (*((const volatile TC1CTRL_T *)0xb8003218)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:11; //0
		unsigned int tc1ie:1; //0
		unsigned int mbz_1:3; //0
		unsigned int tc1ip:1; //0
		unsigned int mbz_2:16; //0
	} f;
	unsigned int v;
} TC1INTR_T;
#define TC1INTRrv (*((regval)0xb800321c))
#define TC1INTRdv (0x00000000)
#define RMOD_TC1INTR(...) rset(TC1INTR, TC1INTRrv, __VA_ARGS__)
#define RIZS_TC1INTR(...) rset(TC1INTR, 0, __VA_ARGS__)
#define RFLD_TC1INTR(fld) (*((const volatile TC1INTR_T *)0xb800321c)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:4; //0
		unsigned int tc1data:28; //0
	} f;
	unsigned int v;
} TC2DATA_T;
#define TC2DATArv (*((regval)0xb8003220))
#define TC2DATAdv (0x00000000)
#define RMOD_TC2DATA(...) rset(TC2DATA, TC2DATArv, __VA_ARGS__)
#define RIZS_TC2DATA(...) rset(TC2DATA, 0, __VA_ARGS__)
#define RFLD_TC2DATA(fld) (*((const volatile TC2DATA_T *)0xb8003220)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:4; //0
		unsigned int tc2value:28; //0
	} f;
	unsigned int v;
} TC2CNTR_T;
#define TC2CNTRrv (*((regval)0xb8003224))
#define TC2CNTRdv (0x00000000)
#define RMOD_TC2CNTR(...) rset(TC2CNTR, TC2CNTRrv, __VA_ARGS__)
#define RIZS_TC2CNTR(...) rset(TC2CNTR, 0, __VA_ARGS__)
#define RFLD_TC2CNTR(fld) (*((const volatile TC2CNTR_T *)0xb8003224)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:3; //0
		unsigned int tc2en:1; //0
		unsigned int mbz_1:3; //0
		unsigned int tc2mode:1; //0
		unsigned int mbz_2:8; //0
		unsigned int tc2divfactor:16; //0
	} f;
	unsigned int v;
} TC2CTRL_T;
#define TC2CTRLrv (*((regval)0xb8003228))
#define TC2CTRLdv (0x00000000)
#define RMOD_TC2CTRL(...) rset(TC2CTRL, TC2CTRLrv, __VA_ARGS__)
#define RIZS_TC2CTRL(...) rset(TC2CTRL, 0, __VA_ARGS__)
#define RFLD_TC2CTRL(fld) (*((const volatile TC2CTRL_T *)0xb8003228)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:11; //0
		unsigned int tc2ie:1; //0
		unsigned int mbz_1:3; //0
		unsigned int tc2ip:1; //0
		unsigned int mbz_2:16; //0
	} f;
	unsigned int v;
} TC2INTR_T;
#define TC2INTRrv (*((regval)0xb800322c))
#define TC2INTRdv (0x00000000)
#define RMOD_TC2INTR(...) rset(TC2INTR, TC2INTRrv, __VA_ARGS__)
#define RIZS_TC2INTR(...) rset(TC2INTR, 0, __VA_ARGS__)
#define RFLD_TC2INTR(fld) (*((const volatile TC2INTR_T *)0xb800322c)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:4; //0
		unsigned int tc1data:28; //0
	} f;
	unsigned int v;
} TC3DATA_T;
#define TC3DATArv (*((regval)0xb8003230))
#define TC3DATAdv (0x00000000)
#define RMOD_TC3DATA(...) rset(TC3DATA, TC3DATArv, __VA_ARGS__)
#define RIZS_TC3DATA(...) rset(TC3DATA, 0, __VA_ARGS__)
#define RFLD_TC3DATA(fld) (*((const volatile TC3DATA_T *)0xb8003230)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:4; //0
		unsigned int tc3value:28; //0
	} f;
	unsigned int v;
} TC3CNTR_T;
#define TC3CNTRrv (*((regval)0xb8003234))
#define TC3CNTRdv (0x00000000)
#define RMOD_TC3CNTR(...) rset(TC3CNTR, TC3CNTRrv, __VA_ARGS__)
#define RIZS_TC3CNTR(...) rset(TC3CNTR, 0, __VA_ARGS__)
#define RFLD_TC3CNTR(fld) (*((const volatile TC3CNTR_T *)0xb8003234)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:3; //0
		unsigned int tc3en:1; //0
		unsigned int mbz_1:3; //0
		unsigned int tc3mode:1; //0
		unsigned int mbz_2:8; //0
		unsigned int tc3divfactor:16; //0
	} f;
	unsigned int v;
} TC3CTRL_T;
#define TC3CTRLrv (*((regval)0xb8003238))
#define TC3CTRLdv (0x00000000)
#define RMOD_TC3CTRL(...) rset(TC3CTRL, TC3CTRLrv, __VA_ARGS__)
#define RIZS_TC3CTRL(...) rset(TC3CTRL, 0, __VA_ARGS__)
#define RFLD_TC3CTRL(fld) (*((const volatile TC3CTRL_T *)0xb8003238)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:11; //0
		unsigned int tc3ie:1; //0
		unsigned int mbz_1:3; //0
		unsigned int tc3ip:1; //0
		unsigned int mbz_2:16; //0
	} f;
	unsigned int v;
} TC3INTR_T;
#define TC3INTRrv (*((regval)0xb800323c))
#define TC3INTRdv (0x00000000)
#define RMOD_TC3INTR(...) rset(TC3INTR, TC3INTRrv, __VA_ARGS__)
#define RIZS_TC3INTR(...) rset(TC3INTR, 0, __VA_ARGS__)
#define RFLD_TC3INTR(fld) (*((const volatile TC3INTR_T *)0xb800323c)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:4; //0
		unsigned int tc1data:28; //0
	} f;
	unsigned int v;
} TC4DATA_T;
#define TC4DATArv (*((regval)0xb8003240))
#define TC4DATAdv (0x00000000)
#define RMOD_TC4DATA(...) rset(TC4DATA, TC4DATArv, __VA_ARGS__)
#define RIZS_TC4DATA(...) rset(TC4DATA, 0, __VA_ARGS__)
#define RFLD_TC4DATA(fld) (*((const volatile TC4DATA_T *)0xb8003240)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:4; //0
		unsigned int tc4value:28; //0
	} f;
	unsigned int v;
} TC4CNTR_T;
#define TC4CNTRrv (*((regval)0xb8003244))
#define TC4CNTRdv (0x00000000)
#define RMOD_TC4CNTR(...) rset(TC4CNTR, TC4CNTRrv, __VA_ARGS__)
#define RIZS_TC4CNTR(...) rset(TC4CNTR, 0, __VA_ARGS__)
#define RFLD_TC4CNTR(fld) (*((const volatile TC4CNTR_T *)0xb8003244)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:3; //0
		unsigned int tc4en:1; //0
		unsigned int mbz_1:3; //0
		unsigned int tc4mode:1; //0
		unsigned int mbz_2:8; //0
		unsigned int tc4divfactor:16; //0
	} f;
	unsigned int v;
} TC4CTRL_T;
#define TC4CTRLrv (*((regval)0xb8003248))
#define TC4CTRLdv (0x00000000)
#define RMOD_TC4CTRL(...) rset(TC4CTRL, TC4CTRLrv, __VA_ARGS__)
#define RIZS_TC4CTRL(...) rset(TC4CTRL, 0, __VA_ARGS__)
#define RFLD_TC4CTRL(fld) (*((const volatile TC4CTRL_T *)0xb8003248)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:11; //0
		unsigned int tc4ie:1; //0
		unsigned int mbz_1:3; //0
		unsigned int tc4ip:1; //0
		unsigned int mbz_2:16; //0
	} f;
	unsigned int v;
} TC4INTR_T;
#define TC4INTRrv (*((regval)0xb800324c))
#define TC4INTRdv (0x00000000)
#define RMOD_TC4INTR(...) rset(TC4INTR, TC4INTRrv, __VA_ARGS__)
#define RIZS_TC4INTR(...) rset(TC4INTR, 0, __VA_ARGS__)
#define RFLD_TC4INTR(fld) (*((const volatile TC4INTR_T *)0xb800324c)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:4; //0
		unsigned int tc1data:28; //0
	} f;
	unsigned int v;
} TC5DATA_T;
#define TC5DATArv (*((regval)0xb8003250))
#define TC5DATAdv (0x00000000)
#define RMOD_TC5DATA(...) rset(TC5DATA, TC5DATArv, __VA_ARGS__)
#define RIZS_TC5DATA(...) rset(TC5DATA, 0, __VA_ARGS__)
#define RFLD_TC5DATA(fld) (*((const volatile TC5DATA_T *)0xb8003250)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:4; //0
		unsigned int tc5value:28; //0
	} f;
	unsigned int v;
} TC5CNTR_T;
#define TC5CNTRrv (*((regval)0xb8003254))
#define TC5CNTRdv (0x00000000)
#define RMOD_TC5CNTR(...) rset(TC5CNTR, TC5CNTRrv, __VA_ARGS__)
#define RIZS_TC5CNTR(...) rset(TC5CNTR, 0, __VA_ARGS__)
#define RFLD_TC5CNTR(fld) (*((const volatile TC5CNTR_T *)0xb8003254)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:3; //0
		unsigned int tc5en:1; //0
		unsigned int mbz_1:3; //0
		unsigned int tc5mode:1; //0
		unsigned int mbz_2:8; //0
		unsigned int tc5divfactor:16; //0
	} f;
	unsigned int v;
} TC5CTRL_T;
#define TC5CTRLrv (*((regval)0xb8003258))
#define TC5CTRLdv (0x00000000)
#define RMOD_TC5CTRL(...) rset(TC5CTRL, TC5CTRLrv, __VA_ARGS__)
#define RIZS_TC5CTRL(...) rset(TC5CTRL, 0, __VA_ARGS__)
#define RFLD_TC5CTRL(fld) (*((const volatile TC5CTRL_T *)0xb8003258)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:11; //0
		unsigned int tc5ie:1; //0
		unsigned int mbz_1:3; //0
		unsigned int tc5ip:1; //0
		unsigned int mbz_2:16; //0
	} f;
	unsigned int v;
} TC5INTR_T;
#define TC5INTRrv (*((regval)0xb800325c))
#define TC5INTRdv (0x00000000)
#define RMOD_TC5INTR(...) rset(TC5INTR, TC5INTRrv, __VA_ARGS__)
#define RIZS_TC5INTR(...) rset(TC5INTR, 0, __VA_ARGS__)
#define RFLD_TC5INTR(fld) (*((const volatile TC5INTR_T *)0xb800325c)).f.fld

typedef union {
	struct {
		unsigned int wdt_kick:1; //0
		unsigned int mbz_0:31; //0
	} f;
	unsigned int v;
} WDTCNTRR_T;
#define WDTCNTRRrv (*((regval)0xb8003260))
#define WDTCNTRRdv (0x00000000)
#define RMOD_WDTCNTRR(...) rset(WDTCNTRR, WDTCNTRRrv, __VA_ARGS__)
#define RIZS_WDTCNTRR(...) rset(WDTCNTRR, 0, __VA_ARGS__)
#define RFLD_WDTCNTRR(fld) (*((const volatile WDTCNTRR_T *)0xb8003260)).f.fld

typedef union {
	struct {
		unsigned int ph1_ip:1; //0
		unsigned int ph2_ip:1; //0
		unsigned int mbz_0:30; //0
	} f;
	unsigned int v;
} WDTINTRR_T;
#define WDTINTRRrv (*((regval)0xb8003264))
#define WDTINTRRdv (0x00000000)
#define RMOD_WDTINTRR(...) rset(WDTINTRR, WDTINTRRrv, __VA_ARGS__)
#define RIZS_WDTINTRR(...) rset(WDTINTRR, 0, __VA_ARGS__)
#define RFLD_WDTINTRR(fld) (*((const volatile WDTINTRR_T *)0xb8003264)).f.fld

typedef union {
	struct {
		unsigned int wdt_e:1; //0
		unsigned int wdt_clk_sc:2; //0
		unsigned int mbz_0:2; //0
		unsigned int ph1_to:5; //0
		unsigned int mbz_1:2; //0
		unsigned int ph2_to:5; //0
		unsigned int mbz_2:13; //0
		unsigned int wdt_reset_mode:2; //0
	} f;
	unsigned int v;
} WDT_CTRL_T;
#define WDT_CTRLrv (*((regval)0xb8003268))
#define WDT_CTRLdv (0x00000000)
#define RMOD_WDT_CTRL(...) rset(WDT_CTRL, WDT_CTRLrv, __VA_ARGS__)
#define RIZS_WDT_CTRL(...) rset(WDT_CTRL, 0, __VA_ARGS__)
#define RFLD_WDT_CTRL(fld) (*((const volatile WDT_CTRL_T *)0xb8003268)).f.fld

/*-----------------------------------------------------
 Extraced from file_GPIO.xml
-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int no_use31:32; //0
	} f;
	unsigned int v;
} GPIO_NO_USE_0004_T;
#define GPIO_NO_USE_0004rv (*((regval)0xb8003304))
#define GPIO_NO_USE_0004dv (0x00000000)
#define RMOD_GPIO_NO_USE_0004(...) rset(GPIO_NO_USE_0004, GPIO_NO_USE_0004rv, __VA_ARGS__)
#define RIZS_GPIO_NO_USE_0004(...) rset(GPIO_NO_USE_0004, 0, __VA_ARGS__)
#define RFLD_GPIO_NO_USE_0004(fld) (*((const volatile GPIO_NO_USE_0004_T *)0xb8003304)).f.fld

typedef union {
	struct {
		unsigned int drc_d:8; //0
		unsigned int drc_c:8; //0
		unsigned int drc_b:8; //0
		unsigned int drc_a:8; //0
	} f;
	unsigned int v;
} PABCD_DIR_T;
#define PABCD_DIRrv (*((regval)0xb8003308))
#define PABCD_DIRdv (0x00000000)
#define RMOD_PABCD_DIR(...) rset(PABCD_DIR, PABCD_DIRrv, __VA_ARGS__)
#define RIZS_PABCD_DIR(...) rset(PABCD_DIR, 0, __VA_ARGS__)
#define RFLD_PABCD_DIR(fld) (*((const volatile PABCD_DIR_T *)0xb8003308)).f.fld

typedef union {
	struct {
		unsigned int pd_d:8; //0
		unsigned int pd_c:8; //0
		unsigned int pd_b:8; //0
		unsigned int pd_a:8; //0
	} f;
	unsigned int v;
} PABCD_DAT_T;
#define PABCD_DATrv (*((regval)0xb800330c))
#define PABCD_DATdv (0x00000000)
#define RMOD_PABCD_DAT(...) rset(PABCD_DAT, PABCD_DATrv, __VA_ARGS__)
#define RIZS_PABCD_DAT(...) rset(PABCD_DAT, 0, __VA_ARGS__)
#define RFLD_PABCD_DAT(fld) (*((const volatile PABCD_DAT_T *)0xb800330c)).f.fld

typedef union {
	struct {
		unsigned int ips_d:8; //0
		unsigned int ips_c:8; //0
		unsigned int ips_b:8; //0
		unsigned int ips_a:8; //0
	} f;
	unsigned int v;
} PABCD_ISR_T;
#define PABCD_ISRrv (*((regval)0xb8003310))
#define PABCD_ISRdv (0x00000000)
#define RMOD_PABCD_ISR(...) rset(PABCD_ISR, PABCD_ISRrv, __VA_ARGS__)
#define RIZS_PABCD_ISR(...) rset(PABCD_ISR, 0, __VA_ARGS__)
#define RFLD_PABCD_ISR(fld) (*((const volatile PABCD_ISR_T *)0xb8003310)).f.fld

typedef union {
	struct {
		unsigned int pb7_im:2; //0
		unsigned int pb6_im:2; //0
		unsigned int pb5_im:2; //0
		unsigned int pb4_im:2; //0
		unsigned int pb3_im:2; //0
		unsigned int pb2_im:2; //0
		unsigned int pb1_im:2; //0
		unsigned int pb0_im:2; //0
		unsigned int pa7_im:2; //0
		unsigned int pa6_im:2; //0
		unsigned int pa5_im:2; //0
		unsigned int pa4_im:2; //0
		unsigned int pa3_im:2; //0
		unsigned int pa2_im:2; //0
		unsigned int pa1_im:2; //0
		unsigned int pa0_im:2; //0
	} f;
	unsigned int v;
} PAB_IMR_T;
#define PAB_IMRrv (*((regval)0xb8003314))
#define PAB_IMRdv (0x00000000)
#define RMOD_PAB_IMR(...) rset(PAB_IMR, PAB_IMRrv, __VA_ARGS__)
#define RIZS_PAB_IMR(...) rset(PAB_IMR, 0, __VA_ARGS__)
#define RFLD_PAB_IMR(fld) (*((const volatile PAB_IMR_T *)0xb8003314)).f.fld

typedef union {
	struct {
		unsigned int pd7_im:2; //0
		unsigned int pd6_im:2; //0
		unsigned int pd5_im:2; //0
		unsigned int pd4_im:2; //0
		unsigned int pd3_im:2; //0
		unsigned int pd2_im:2; //0
		unsigned int pd1_im:2; //0
		unsigned int pd0_im:2; //0
		unsigned int pc7_im:2; //0
		unsigned int pc6_im:2; //0
		unsigned int pc5_im:2; //0
		unsigned int pc4_im:2; //0
		unsigned int pc3_im:2; //0
		unsigned int pc2_im:2; //0
		unsigned int pc1_im:2; //0
		unsigned int pc0_im:2; //0
	} f;
	unsigned int v;
} PCD_IMR_T;
#define PCD_IMRrv (*((regval)0xb8003318))
#define PCD_IMRdv (0x00000000)
#define RMOD_PCD_IMR(...) rset(PCD_IMR, PCD_IMRrv, __VA_ARGS__)
#define RIZS_PCD_IMR(...) rset(PCD_IMR, 0, __VA_ARGS__)
#define RFLD_PCD_IMR(fld) (*((const volatile PCD_IMR_T *)0xb8003318)).f.fld

typedef union {
	struct {
		unsigned int no_use31:32; //0
	} f;
	unsigned int v;
} GPIO_NO_USE_0020_T;
#define GPIO_NO_USE_0020rv (*((regval)0xb8003320))
#define GPIO_NO_USE_0020dv (0x00000000)
#define RMOD_GPIO_NO_USE_0020(...) rset(GPIO_NO_USE_0020, GPIO_NO_USE_0020rv, __VA_ARGS__)
#define RIZS_GPIO_NO_USE_0020(...) rset(GPIO_NO_USE_0020, 0, __VA_ARGS__)
#define RFLD_GPIO_NO_USE_0020(fld) (*((const volatile GPIO_NO_USE_0020_T *)0xb8003320)).f.fld

typedef union {
	struct {
		unsigned int drc_h:8; //0
		unsigned int drc_g:8; //0
		unsigned int drc_f:8; //0
		unsigned int drc_e:8; //0
	} f;
	unsigned int v;
} PEFGH_DIR_T;
#define PEFGH_DIRrv (*((regval)0xb8003324))
#define PEFGH_DIRdv (0x00000000)
#define RMOD_PEFGH_DIR(...) rset(PEFGH_DIR, PEFGH_DIRrv, __VA_ARGS__)
#define RIZS_PEFGH_DIR(...) rset(PEFGH_DIR, 0, __VA_ARGS__)
#define RFLD_PEFGH_DIR(fld) (*((const volatile PEFGH_DIR_T *)0xb8003324)).f.fld

typedef union {
	struct {
		unsigned int ph_h:8; //0
		unsigned int ph_g:8; //0
		unsigned int ph_f:8; //0
		unsigned int ph_e:8; //0
	} f;
	unsigned int v;
} PEFGH_DAT_T;
#define PEFGH_DATrv (*((regval)0xb8003328))
#define PEFGH_DATdv (0x00000000)
#define RMOD_PEFGH_DAT(...) rset(PEFGH_DAT, PEFGH_DATrv, __VA_ARGS__)
#define RIZS_PEFGH_DAT(...) rset(PEFGH_DAT, 0, __VA_ARGS__)
#define RFLD_PEFGH_DAT(fld) (*((const volatile PEFGH_DAT_T *)0xb8003328)).f.fld

typedef union {
	struct {
		unsigned int ips_h:8; //0
		unsigned int ips_g:8; //0
		unsigned int ips_f:8; //0
		unsigned int ips_e:8; //0
	} f;
	unsigned int v;
} PEFGH_ISR_T;
#define PEFGH_ISRrv (*((regval)0xb800332c))
#define PEFGH_ISRdv (0x00000000)
#define RMOD_PEFGH_ISR(...) rset(PEFGH_ISR, PEFGH_ISRrv, __VA_ARGS__)
#define RIZS_PEFGH_ISR(...) rset(PEFGH_ISR, 0, __VA_ARGS__)
#define RFLD_PEFGH_ISR(fld) (*((const volatile PEFGH_ISR_T *)0xb800332c)).f.fld

typedef union {
	struct {
		unsigned int pf7_im:2; //0
		unsigned int pf6_im:2; //0
		unsigned int pf5_im:2; //0
		unsigned int pf4_im:2; //0
		unsigned int pf3_im:2; //0
		unsigned int pf2_im:2; //0
		unsigned int pf1_im:2; //0
		unsigned int pf0_im:2; //0
		unsigned int pe7_im:2; //0
		unsigned int pe6_im:2; //0
		unsigned int pe5_im:2; //0
		unsigned int pe4_im:2; //0
		unsigned int pe3_im:2; //0
		unsigned int pe2_im:2; //0
		unsigned int pe1_im:2; //0
		unsigned int pe0_im:2; //0
	} f;
	unsigned int v;
} PEF_IMR_T;
#define PEF_IMRrv (*((regval)0xb8003330))
#define PEF_IMRdv (0x00000000)
#define RMOD_PEF_IMR(...) rset(PEF_IMR, PEF_IMRrv, __VA_ARGS__)
#define RIZS_PEF_IMR(...) rset(PEF_IMR, 0, __VA_ARGS__)
#define RFLD_PEF_IMR(fld) (*((const volatile PEF_IMR_T *)0xb8003330)).f.fld

typedef union {
	struct {
		unsigned int ph7_im:2; //0
		unsigned int ph6_im:2; //0
		unsigned int ph5_im:2; //0
		unsigned int ph4_im:2; //0
		unsigned int ph3_im:2; //0
		unsigned int ph2_im:2; //0
		unsigned int ph1_im:2; //0
		unsigned int ph0_im:2; //0
		unsigned int pg7_im:2; //0
		unsigned int pg6_im:2; //0
		unsigned int pg5_im:2; //0
		unsigned int pg4_im:2; //0
		unsigned int pg3_im:2; //0
		unsigned int pg2_im:2; //0
		unsigned int pg1_im:2; //0
		unsigned int pg0_im:2; //0
	} f;
	unsigned int v;
} PGH_IMR_T;
#define PGH_IMRrv (*((regval)0xb8003334))
#define PGH_IMRdv (0x00000000)
#define RMOD_PGH_IMR(...) rset(PGH_IMR, PGH_IMRrv, __VA_ARGS__)
#define RIZS_PGH_IMR(...) rset(PGH_IMR, 0, __VA_ARGS__)
#define RFLD_PGH_IMR(fld) (*((const volatile PGH_IMR_T *)0xb8003334)).f.fld

/*-----------------------------------------------------
 Extraced from file_OB_TO_MONT.xml
-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int to_ctrl_en:1; //1
		unsigned int err_indcat:1; //0
		unsigned int to_ctrl_thr:4; //7
		unsigned int mbz_0:26; //0
	} f;
	unsigned int v;
} TO_CTRL_T;
#define TO_CTRLrv (*((regval)0xb8005100))
#define TO_CTRLdv (0x9c000000)
#define RMOD_TO_CTRL(...) rset(TO_CTRL, TO_CTRLrv, __VA_ARGS__)
#define RIZS_TO_CTRL(...) rset(TO_CTRL, 0, __VA_ARGS__)
#define RFLD_TO_CTRL(fld) (*((const volatile TO_CTRL_T *)0xb8005100)).f.fld

typedef union {
	struct {
		unsigned int to_ip:2; //0
		unsigned int mbz_0:30; //0
	} f;
	unsigned int v;
} TO_INTR_T;
#define TO_INTRrv (*((regval)0xb8005104))
#define TO_INTRdv (0x00000000)
#define RMOD_TO_INTR(...) rset(TO_INTR, TO_INTRrv, __VA_ARGS__)
#define RIZS_TO_INTR(...) rset(TO_INTR, 0, __VA_ARGS__)
#define RFLD_TO_INTR(fld) (*((const volatile TO_INTR_T *)0xb8005104)).f.fld

typedef union {
	struct {
		unsigned int to_addr:32; //0
	} f;
	unsigned int v;
} TO_MONT_ADDR_T;
#define TO_MONT_ADDRrv (*((regval)0xb8005108))
#define TO_MONT_ADDRdv (0x00000000)
#define RMOD_TO_MONT_ADDR(...) rset(TO_MONT_ADDR, TO_MONT_ADDRrv, __VA_ARGS__)
#define RIZS_TO_MONT_ADDR(...) rset(TO_MONT_ADDR, 0, __VA_ARGS__)
#define RFLD_TO_MONT_ADDR(fld) (*((const volatile TO_MONT_ADDR_T *)0xb8005108)).f.fld

/*-----------------------------------------------------
 Extraced from file_BUS_TRAF_GEN.xml
-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int enable:1; //0
		unsigned int poll:1; //0
		unsigned int bus_traf_gen:2; //0
		unsigned int func:4; //0
		unsigned int mbz_0:1; //0
		unsigned int swap_type:1; //0
		unsigned int ent_size:2; //0
		unsigned int mbz_1:10; //0
		unsigned int dis_check_ddr:1; //0
		unsigned int debug_sel:1; //0
		unsigned int burst_size:2; //0
		unsigned int enough_thr:2; //0
		unsigned int dst_low_thr:2; //0
		unsigned int src_low_th:2; //0
	} f;
	unsigned int v;
} GDMA_CTRL_T;
#define GDMA_CTRLrv (*((regval)0xb800a000))
#define GDMA_CTRLdv (0x00000000)
#define RMOD_GDMA_CTRL(...) rset(GDMA_CTRL, GDMA_CTRLrv, __VA_ARGS__)
#define RIZS_GDMA_CTRL(...) rset(GDMA_CTRL, 0, __VA_ARGS__)
#define RFLD_GDMA_CTRL(fld) (*((const volatile GDMA_CTRL_T *)0xb800a000)).f.fld

typedef union {
	struct {
		unsigned int comp_ie:1; //0
		unsigned int mbz_0:2; //0
		unsigned int need_cpu_ie:1; //0
		unsigned int rd_pkt_ie:1; //0
		unsigned int err_rchk_ie:1; //0
		unsigned int err_rprd_ie:1; //0
		unsigned int err_rrp_ie:1; //0
		unsigned int wr_pkt_ie:1; //0
		unsigned int mbz_1:1; //0
		unsigned int err_wprd_ie:1; //0
		unsigned int err_wrp_ie:1; //0
		unsigned int mbz_2:20; //0
	} f;
	unsigned int v;
} GDMA_IM_T;
#define GDMA_IMrv (*((regval)0xb800a004))
#define GDMA_IMdv (0x00000000)
#define RMOD_GDMA_IM(...) rset(GDMA_IM, GDMA_IMrv, __VA_ARGS__)
#define RIZS_GDMA_IM(...) rset(GDMA_IM, 0, __VA_ARGS__)
#define RFLD_GDMA_IM(fld) (*((const volatile GDMA_IM_T *)0xb800a004)).f.fld

typedef union {
	struct {
		unsigned int comp_ip:1; //0
		unsigned int mbz_0:2; //0
		unsigned int need_cpu_ip:1; //0
		unsigned int rd_pkt_ip:1; //0
		unsigned int err_rchk_ip:1; //0
		unsigned int err_rprd_ip:1; //0
		unsigned int err_rrp_ip:1; //0
		unsigned int wr_pkt_ip:1; //0
		unsigned int mbz_1:1; //0
		unsigned int err_wprd_ip:1; //0
		unsigned int err_wrp_ip:1; //0
		unsigned int mbz_2:20; //0
	} f;
	unsigned int v;
} GDMA_IS_T;
#define GDMA_ISrv (*((regval)0xb800a008))
#define GDMA_ISdv (0x00000000)
#define RMOD_GDMA_IS(...) rset(GDMA_IS, GDMA_ISrv, __VA_ARGS__)
#define RIZS_GDMA_IS(...) rset(GDMA_IS, 0, __VA_ARGS__)
#define RFLD_GDMA_IS(fld) (*((const volatile GDMA_IS_T *)0xb800a008)).f.fld

typedef union {
	struct {
		unsigned int icvl:32; //0
	} f;
	unsigned int v;
} GDMA_ICVL_T;
#define GDMA_ICVLrv (*((regval)0xb800a00c))
#define GDMA_ICVLdv (0x00000000)
#define RMOD_GDMA_ICVL(...) rset(GDMA_ICVL, GDMA_ICVLrv, __VA_ARGS__)
#define RIZS_GDMA_ICVL(...) rset(GDMA_ICVL, 0, __VA_ARGS__)
#define RFLD_GDMA_ICVL(fld) (*((const volatile GDMA_ICVL_T *)0xb800a00c)).f.fld

typedef union {
	struct {
		unsigned int enable:1; //0
		unsigned int no_use30:8; //0
		unsigned int rec_err:1; //0
		unsigned int prec:1; //0
		unsigned int infinite:1; //0
		unsigned int no_use19:2; //0
		unsigned int burst_len:2; //0
		unsigned int no_use15:3; //0
		unsigned int pkt_len:8; //0
		unsigned int no_use04:5; //0
	} f;
	unsigned int v;
} BTG_WRITE_CTRL_T;
#define BTG_WRITE_CTRLrv (*((regval)0xb800a100))
#define BTG_WRITE_CTRLdv (0x00000000)
#define RMOD_BTG_WRITE_CTRL(...) rset(BTG_WRITE_CTRL, BTG_WRITE_CTRLrv, __VA_ARGS__)
#define RIZS_BTG_WRITE_CTRL(...) rset(BTG_WRITE_CTRL, 0, __VA_ARGS__)
#define RFLD_BTG_WRITE_CTRL(fld) (*((const volatile BTG_WRITE_CTRL_T *)0xb800a100)).f.fld

typedef union {
	struct {
		unsigned int iter:32; //0
	} f;
	unsigned int v;
} BTG_WRITE_ITER_T;
#define BTG_WRITE_ITERrv (*((regval)0xb800a104))
#define BTG_WRITE_ITERdv (0x00000000)
#define RMOD_BTG_WRITE_ITER(...) rset(BTG_WRITE_ITER, BTG_WRITE_ITERrv, __VA_ARGS__)
#define RIZS_BTG_WRITE_ITER(...) rset(BTG_WRITE_ITER, 0, __VA_ARGS__)
#define RFLD_BTG_WRITE_ITER(fld) (*((const volatile BTG_WRITE_ITER_T *)0xb800a104)).f.fld

typedef union {
	struct {
		unsigned int resp_time:32; //0
	} f;
	unsigned int v;
} BTG_WRITE_RESP_TIME_T;
#define BTG_WRITE_RESP_TIMErv (*((regval)0xb800a108))
#define BTG_WRITE_RESP_TIMEdv (0x00000000)
#define RMOD_BTG_WRITE_RESP_TIME(...) rset(BTG_WRITE_RESP_TIME, BTG_WRITE_RESP_TIMErv, __VA_ARGS__)
#define RIZS_BTG_WRITE_RESP_TIME(...) rset(BTG_WRITE_RESP_TIME, 0, __VA_ARGS__)
#define RFLD_BTG_WRITE_RESP_TIME(fld) (*((const volatile BTG_WRITE_RESP_TIME_T *)0xb800a108)).f.fld

typedef union {
	struct {
		unsigned int period:32; //0
	} f;
	unsigned int v;
} BTG_WRITE_PERI_T;
#define BTG_WRITE_PERIrv (*((regval)0xb800a10c))
#define BTG_WRITE_PERIdv (0x00000000)
#define RMOD_BTG_WRITE_PERI(...) rset(BTG_WRITE_PERI, BTG_WRITE_PERIrv, __VA_ARGS__)
#define RIZS_BTG_WRITE_PERI(...) rset(BTG_WRITE_PERI, 0, __VA_ARGS__)
#define RFLD_BTG_WRITE_PERI(fld) (*((const volatile BTG_WRITE_PERI_T *)0xb800a10c)).f.fld

typedef union {
	struct {
		unsigned int base_addr:32; //0
	} f;
	unsigned int v;
} BTG_WRITE_ADDR_T;
#define BTG_WRITE_ADDRrv (*((regval)0xb800a110))
#define BTG_WRITE_ADDRdv (0x00000000)
#define RMOD_BTG_WRITE_ADDR(...) rset(BTG_WRITE_ADDR, BTG_WRITE_ADDRrv, __VA_ARGS__)
#define RIZS_BTG_WRITE_ADDR(...) rset(BTG_WRITE_ADDR, 0, __VA_ARGS__)
#define RFLD_BTG_WRITE_ADDR(fld) (*((const volatile BTG_WRITE_ADDR_T *)0xb800a110)).f.fld

typedef union {
	struct {
		unsigned int addr_msk:32; //0
	} f;
	unsigned int v;
} BTG_WRITE_ADDR_MASK_T;
#define BTG_WRITE_ADDR_MASKrv (*((regval)0xb800a114))
#define BTG_WRITE_ADDR_MASKdv (0x00000000)
#define RMOD_BTG_WRITE_ADDR_MASK(...) rset(BTG_WRITE_ADDR_MASK, BTG_WRITE_ADDR_MASKrv, __VA_ARGS__)
#define RIZS_BTG_WRITE_ADDR_MASK(...) rset(BTG_WRITE_ADDR_MASK, 0, __VA_ARGS__)
#define RFLD_BTG_WRITE_ADDR_MASK(fld) (*((const volatile BTG_WRITE_ADDR_MASK_T *)0xb800a114)).f.fld

typedef union {
	struct {
		unsigned int dec_gap:1; //0
		unsigned int no_use30:15; //0
		unsigned int addr_gap:16; //0
	} f;
	unsigned int v;
} BTG_WRITE_GAP_T;
#define BTG_WRITE_GAPrv (*((regval)0xb800a118))
#define BTG_WRITE_GAPdv (0x00000000)
#define RMOD_BTG_WRITE_GAP(...) rset(BTG_WRITE_GAP, BTG_WRITE_GAPrv, __VA_ARGS__)
#define RIZS_BTG_WRITE_GAP(...) rset(BTG_WRITE_GAP, 0, __VA_ARGS__)
#define RFLD_BTG_WRITE_GAP(fld) (*((const volatile BTG_WRITE_GAP_T *)0xb800a118)).f.fld

typedef union {
	struct {
		unsigned int cur_addr:32; //0
	} f;
	unsigned int v;
} BTG_WRITE_CURR_ADDR_T;
#define BTG_WRITE_CURR_ADDRrv (*((regval)0xb800a11c))
#define BTG_WRITE_CURR_ADDRdv (0x00000000)
#define RMOD_BTG_WRITE_CURR_ADDR(...) rset(BTG_WRITE_CURR_ADDR, BTG_WRITE_CURR_ADDRrv, __VA_ARGS__)
#define RIZS_BTG_WRITE_CURR_ADDR(...) rset(BTG_WRITE_CURR_ADDR, 0, __VA_ARGS__)
#define RFLD_BTG_WRITE_CURR_ADDR(fld) (*((const volatile BTG_WRITE_CURR_ADDR_T *)0xb800a11c)).f.fld

typedef union {
	struct {
		unsigned int resp_addr:32; //0
	} f;
	unsigned int v;
} BTG_WRITE_RESP_FAIL_ADDR_T;
#define BTG_WRITE_RESP_FAIL_ADDRrv (*((regval)0xb800a120))
#define BTG_WRITE_RESP_FAIL_ADDRdv (0x00000000)
#define RMOD_BTG_WRITE_RESP_FAIL_ADDR(...) rset(BTG_WRITE_RESP_FAIL_ADDR, BTG_WRITE_RESP_FAIL_ADDRrv, __VA_ARGS__)
#define RIZS_BTG_WRITE_RESP_FAIL_ADDR(...) rset(BTG_WRITE_RESP_FAIL_ADDR, 0, __VA_ARGS__)
#define RFLD_BTG_WRITE_RESP_FAIL_ADDR(fld) (*((const volatile BTG_WRITE_RESP_FAIL_ADDR_T *)0xb800a120)).f.fld

typedef union {
	struct {
		unsigned int peri_addr:32; //0
	} f;
	unsigned int v;
} BTG_WRITE_PERI_FAIL_ADDR_T;
#define BTG_WRITE_PERI_FAIL_ADDRrv (*((regval)0xb800a124))
#define BTG_WRITE_PERI_FAIL_ADDRdv (0x00000000)
#define RMOD_BTG_WRITE_PERI_FAIL_ADDR(...) rset(BTG_WRITE_PERI_FAIL_ADDR, BTG_WRITE_PERI_FAIL_ADDRrv, __VA_ARGS__)
#define RIZS_BTG_WRITE_PERI_FAIL_ADDR(...) rset(BTG_WRITE_PERI_FAIL_ADDR, 0, __VA_ARGS__)
#define RFLD_BTG_WRITE_PERI_FAIL_ADDR(fld) (*((const volatile BTG_WRITE_PERI_FAIL_ADDR_T *)0xb800a124)).f.fld

typedef union {
	struct {
		unsigned int max_resp_time:32; //0
	} f;
	unsigned int v;
} BTG_WRITE_MAX_RESP_TIME_T;
#define BTG_WRITE_MAX_RESP_TIMErv (*((regval)0xb800a12c))
#define BTG_WRITE_MAX_RESP_TIMEdv (0x00000000)
#define RMOD_BTG_WRITE_MAX_RESP_TIME(...) rset(BTG_WRITE_MAX_RESP_TIME, BTG_WRITE_MAX_RESP_TIMErv, __VA_ARGS__)
#define RIZS_BTG_WRITE_MAX_RESP_TIME(...) rset(BTG_WRITE_MAX_RESP_TIME, 0, __VA_ARGS__)
#define RFLD_BTG_WRITE_MAX_RESP_TIME(fld) (*((const volatile BTG_WRITE_MAX_RESP_TIME_T *)0xb800a12c)).f.fld

typedef union {
	struct {
		unsigned int in_vec:32; //0
	} f;
	unsigned int v;
} BTC_WRITE_IN_VEC_T;
#define BTC_WRITE_IN_VECrv (*((regval)0xb800a130))
#define BTC_WRITE_IN_VECdv (0x00000000)
#define RMOD_BTC_WRITE_IN_VEC(...) rset(BTC_WRITE_IN_VEC, BTC_WRITE_IN_VECrv, __VA_ARGS__)
#define RIZS_BTC_WRITE_IN_VEC(...) rset(BTC_WRITE_IN_VEC, 0, __VA_ARGS__)
#define RFLD_BTC_WRITE_IN_VEC(fld) (*((const volatile BTC_WRITE_IN_VEC_T *)0xb800a130)).f.fld

typedef union {
	struct {
		unsigned int enable:1; //0
		unsigned int no_use30:5; //0
		unsigned int chk_mode:2; //0
		unsigned int no_use23:1; //0
		unsigned int rec_err:1; //0
		unsigned int prec:1; //0
		unsigned int infinite:1; //0
		unsigned int no_use19:2; //0
		unsigned int burst_len:2; //0
		unsigned int no_use15:3; //0
		unsigned int pkt_len:8; //0
		unsigned int no_use04:5; //0
	} f;
	unsigned int v;
} BTG_READ_CTRL_T;
#define BTG_READ_CTRLrv (*((regval)0xb800a1c4))
#define BTG_READ_CTRLdv (0x00000000)
#define RMOD_BTG_READ_CTRL(...) rset(BTG_READ_CTRL, BTG_READ_CTRLrv, __VA_ARGS__)
#define RIZS_BTG_READ_CTRL(...) rset(BTG_READ_CTRL, 0, __VA_ARGS__)
#define RFLD_BTG_READ_CTRL(fld) (*((const volatile BTG_READ_CTRL_T *)0xb800a1c4)).f.fld

typedef union {
	struct {
		unsigned int iter:32; //0
	} f;
	unsigned int v;
} BTG_READ_ITER_T;
#define BTG_READ_ITERrv (*((regval)0xb800a1c8))
#define BTG_READ_ITERdv (0x00000000)
#define RMOD_BTG_READ_ITER(...) rset(BTG_READ_ITER, BTG_READ_ITERrv, __VA_ARGS__)
#define RIZS_BTG_READ_ITER(...) rset(BTG_READ_ITER, 0, __VA_ARGS__)
#define RFLD_BTG_READ_ITER(fld) (*((const volatile BTG_READ_ITER_T *)0xb800a1c8)).f.fld

typedef union {
	struct {
		unsigned int resp_time:32; //0
	} f;
	unsigned int v;
} BTG_READ_RESP_TIME_T;
#define BTG_READ_RESP_TIMErv (*((regval)0xb800a1cc))
#define BTG_READ_RESP_TIMEdv (0x00000000)
#define RMOD_BTG_READ_RESP_TIME(...) rset(BTG_READ_RESP_TIME, BTG_READ_RESP_TIMErv, __VA_ARGS__)
#define RIZS_BTG_READ_RESP_TIME(...) rset(BTG_READ_RESP_TIME, 0, __VA_ARGS__)
#define RFLD_BTG_READ_RESP_TIME(fld) (*((const volatile BTG_READ_RESP_TIME_T *)0xb800a1cc)).f.fld

typedef union {
	struct {
		unsigned int period:32; //0
	} f;
	unsigned int v;
} BTG_READ_PERI_T;
#define BTG_READ_PERIrv (*((regval)0xb800a1d0))
#define BTG_READ_PERIdv (0x00000000)
#define RMOD_BTG_READ_PERI(...) rset(BTG_READ_PERI, BTG_READ_PERIrv, __VA_ARGS__)
#define RIZS_BTG_READ_PERI(...) rset(BTG_READ_PERI, 0, __VA_ARGS__)
#define RFLD_BTG_READ_PERI(fld) (*((const volatile BTG_READ_PERI_T *)0xb800a1d0)).f.fld

typedef union {
	struct {
		unsigned int base_addr:32; //0
	} f;
	unsigned int v;
} BTG_READ_ADDR_T;
#define BTG_READ_ADDRrv (*((regval)0xb800a1d4))
#define BTG_READ_ADDRdv (0x00000000)
#define RMOD_BTG_READ_ADDR(...) rset(BTG_READ_ADDR, BTG_READ_ADDRrv, __VA_ARGS__)
#define RIZS_BTG_READ_ADDR(...) rset(BTG_READ_ADDR, 0, __VA_ARGS__)
#define RFLD_BTG_READ_ADDR(fld) (*((const volatile BTG_READ_ADDR_T *)0xb800a1d4)).f.fld

typedef union {
	struct {
		unsigned int addr_msk:32; //0
	} f;
	unsigned int v;
} BTG_READ_ADDR_MASK_T;
#define BTG_READ_ADDR_MASKrv (*((regval)0xb800a1d8))
#define BTG_READ_ADDR_MASKdv (0x00000000)
#define RMOD_BTG_READ_ADDR_MASK(...) rset(BTG_READ_ADDR_MASK, BTG_READ_ADDR_MASKrv, __VA_ARGS__)
#define RIZS_BTG_READ_ADDR_MASK(...) rset(BTG_READ_ADDR_MASK, 0, __VA_ARGS__)
#define RFLD_BTG_READ_ADDR_MASK(fld) (*((const volatile BTG_READ_ADDR_MASK_T *)0xb800a1d8)).f.fld

typedef union {
	struct {
		unsigned int dec_gap:1; //0
		unsigned int no_use30:15; //0
		unsigned int addr_gap:16; //0
	} f;
	unsigned int v;
} BTG_READ_GAP_T;
#define BTG_READ_GAPrv (*((regval)0xb800a1dc))
#define BTG_READ_GAPdv (0x00000000)
#define RMOD_BTG_READ_GAP(...) rset(BTG_READ_GAP, BTG_READ_GAPrv, __VA_ARGS__)
#define RIZS_BTG_READ_GAP(...) rset(BTG_READ_GAP, 0, __VA_ARGS__)
#define RFLD_BTG_READ_GAP(fld) (*((const volatile BTG_READ_GAP_T *)0xb800a1dc)).f.fld

typedef union {
	struct {
		unsigned int cur_addr:32; //0
	} f;
	unsigned int v;
} BTG_READ_CURR_ADDR_T;
#define BTG_READ_CURR_ADDRrv (*((regval)0xb800a1e0))
#define BTG_READ_CURR_ADDRdv (0x00000000)
#define RMOD_BTG_READ_CURR_ADDR(...) rset(BTG_READ_CURR_ADDR, BTG_READ_CURR_ADDRrv, __VA_ARGS__)
#define RIZS_BTG_READ_CURR_ADDR(...) rset(BTG_READ_CURR_ADDR, 0, __VA_ARGS__)
#define RFLD_BTG_READ_CURR_ADDR(fld) (*((const volatile BTG_READ_CURR_ADDR_T *)0xb800a1e0)).f.fld

typedef union {
	struct {
		unsigned int resp_addr:32; //0
	} f;
	unsigned int v;
} BTG_READ_RESP_FAIL_ADDR_T;
#define BTG_READ_RESP_FAIL_ADDRrv (*((regval)0xb800a1e4))
#define BTG_READ_RESP_FAIL_ADDRdv (0x00000000)
#define RMOD_BTG_READ_RESP_FAIL_ADDR(...) rset(BTG_READ_RESP_FAIL_ADDR, BTG_READ_RESP_FAIL_ADDRrv, __VA_ARGS__)
#define RIZS_BTG_READ_RESP_FAIL_ADDR(...) rset(BTG_READ_RESP_FAIL_ADDR, 0, __VA_ARGS__)
#define RFLD_BTG_READ_RESP_FAIL_ADDR(fld) (*((const volatile BTG_READ_RESP_FAIL_ADDR_T *)0xb800a1e4)).f.fld

typedef union {
	struct {
		unsigned int peri_addr:32; //0
	} f;
	unsigned int v;
} BTG_READ_PERI_FAIL_ADDR_T;
#define BTG_READ_PERI_FAIL_ADDRrv (*((regval)0xb800a1e8))
#define BTG_READ_PERI_FAIL_ADDRdv (0x00000000)
#define RMOD_BTG_READ_PERI_FAIL_ADDR(...) rset(BTG_READ_PERI_FAIL_ADDR, BTG_READ_PERI_FAIL_ADDRrv, __VA_ARGS__)
#define RIZS_BTG_READ_PERI_FAIL_ADDR(...) rset(BTG_READ_PERI_FAIL_ADDR, 0, __VA_ARGS__)
#define RFLD_BTG_READ_PERI_FAIL_ADDR(fld) (*((const volatile BTG_READ_PERI_FAIL_ADDR_T *)0xb800a1e8)).f.fld

typedef union {
	struct {
		unsigned int chk_addr:32; //0
	} f;
	unsigned int v;
} BTG_READ_VERI_FAIL_ADDR_T;
#define BTG_READ_VERI_FAIL_ADDRrv (*((regval)0xb800a1ec))
#define BTG_READ_VERI_FAIL_ADDRdv (0x00000000)
#define RMOD_BTG_READ_VERI_FAIL_ADDR(...) rset(BTG_READ_VERI_FAIL_ADDR, BTG_READ_VERI_FAIL_ADDRrv, __VA_ARGS__)
#define RIZS_BTG_READ_VERI_FAIL_ADDR(...) rset(BTG_READ_VERI_FAIL_ADDR, 0, __VA_ARGS__)
#define RFLD_BTG_READ_VERI_FAIL_ADDR(fld) (*((const volatile BTG_READ_VERI_FAIL_ADDR_T *)0xb800a1ec)).f.fld

typedef union {
	struct {
		unsigned int max_resp_time:32; //0
	} f;
	unsigned int v;
} BTG_READ_MAX_RESP_TIME_T;
#define BTG_READ_MAX_RESP_TIMErv (*((regval)0xb800a1f0))
#define BTG_READ_MAX_RESP_TIMEdv (0x00000000)
#define RMOD_BTG_READ_MAX_RESP_TIME(...) rset(BTG_READ_MAX_RESP_TIME, BTG_READ_MAX_RESP_TIMErv, __VA_ARGS__)
#define RIZS_BTG_READ_MAX_RESP_TIME(...) rset(BTG_READ_MAX_RESP_TIME, 0, __VA_ARGS__)
#define RFLD_BTG_READ_MAX_RESP_TIME(fld) (*((const volatile BTG_READ_MAX_RESP_TIME_T *)0xb800a1f0)).f.fld

typedef union {
	struct {
		unsigned int in_vec:32; //0
	} f;
	unsigned int v;
} BTC_READ_IN_VEC0_T;
#define BTC_READ_IN_VEC0rv (*((regval)0xb800a1f4))
#define BTC_READ_IN_VEC0dv (0x00000000)
#define RMOD_BTC_READ_IN_VEC0(...) rset(BTC_READ_IN_VEC0, BTC_READ_IN_VEC0rv, __VA_ARGS__)
#define RIZS_BTC_READ_IN_VEC0(...) rset(BTC_READ_IN_VEC0, 0, __VA_ARGS__)
#define RFLD_BTC_READ_IN_VEC0(fld) (*((const volatile BTC_READ_IN_VEC0_T *)0xb800a1f4)).f.fld

typedef union {
	struct {
		unsigned int in_vec:32; //0
	} f;
	unsigned int v;
} BTC_READ_IN_VEC1_T;
#define BTC_READ_IN_VEC1rv (*((regval)0xb800a1f8))
#define BTC_READ_IN_VEC1dv (0x00000000)
#define RMOD_BTC_READ_IN_VEC1(...) rset(BTC_READ_IN_VEC1, BTC_READ_IN_VEC1rv, __VA_ARGS__)
#define RIZS_BTC_READ_IN_VEC1(...) rset(BTC_READ_IN_VEC1, 0, __VA_ARGS__)
#define RFLD_BTC_READ_IN_VEC1(fld) (*((const volatile BTC_READ_IN_VEC1_T *)0xb800a1f8)).f.fld

typedef union {
	struct {
		unsigned int in_vec:32; //0
	} f;
	unsigned int v;
} BTC_READ_IN_VEC_IDX_T;
#define BTC_READ_IN_VEC_IDXrv (*((regval)0xb800a1fc))
#define BTC_READ_IN_VEC_IDXdv (0x00000000)
#define RMOD_BTC_READ_IN_VEC_IDX(...) rset(BTC_READ_IN_VEC_IDX, BTC_READ_IN_VEC_IDXrv, __VA_ARGS__)
#define RIZS_BTC_READ_IN_VEC_IDX(...) rset(BTC_READ_IN_VEC_IDX, 0, __VA_ARGS__)
#define RFLD_BTC_READ_IN_VEC_IDX(fld) (*((const volatile BTC_READ_IN_VEC_IDX_T *)0xb800a1fc)).f.fld

/*-----------------------------------------------------
 Extraced from file_MEM_SRAM.xml
-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int lx_sa:24; //0
		unsigned int mbz_0:7; //0
		unsigned int ensram:1; //0
	} f;
	unsigned int v;
} CPU_SRAM_SEG0_ADDR_T;
#define CPU_SRAM_SEG0_ADDRrv (*((regval)0xb8004000))
#define CPU_SRAM_SEG0_ADDRdv (0x00000000)
#define RMOD_CPU_SRAM_SEG0_ADDR(...) rset(CPU_SRAM_SEG0_ADDR, CPU_SRAM_SEG0_ADDRrv, __VA_ARGS__)
#define RIZS_CPU_SRAM_SEG0_ADDR(...) rset(CPU_SRAM_SEG0_ADDR, 0, __VA_ARGS__)
#define RFLD_CPU_SRAM_SEG0_ADDR(fld) (*((const volatile CPU_SRAM_SEG0_ADDR_T *)0xb8004000)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:28; //0
		unsigned int size:4; //8
	} f;
	unsigned int v;
} CPU_SRAM_SEG0_SIZE_T;
#define CPU_SRAM_SEG0_SIZErv (*((regval)0xb8004004))
#define CPU_SRAM_SEG0_SIZEdv (0x00000008)
#define RMOD_CPU_SRAM_SEG0_SIZE(...) rset(CPU_SRAM_SEG0_SIZE, CPU_SRAM_SEG0_SIZErv, __VA_ARGS__)
#define RIZS_CPU_SRAM_SEG0_SIZE(...) rset(CPU_SRAM_SEG0_SIZE, 0, __VA_ARGS__)
#define RFLD_CPU_SRAM_SEG0_SIZE(fld) (*((const volatile CPU_SRAM_SEG0_SIZE_T *)0xb8004004)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:16; //0
		unsigned int base:8; //0
		unsigned int mbz_1:8; //0
	} f;
	unsigned int v;
} CPU_SRAM_SEG0_BASE_T;
#define CPU_SRAM_SEG0_BASErv (*((regval)0xb8004008))
#define CPU_SRAM_SEG0_BASEdv (0x00000000)
#define RMOD_CPU_SRAM_SEG0_BASE(...) rset(CPU_SRAM_SEG0_BASE, CPU_SRAM_SEG0_BASErv, __VA_ARGS__)
#define RIZS_CPU_SRAM_SEG0_BASE(...) rset(CPU_SRAM_SEG0_BASE, 0, __VA_ARGS__)
#define RFLD_CPU_SRAM_SEG0_BASE(fld) (*((const volatile CPU_SRAM_SEG0_BASE_T *)0xb8004008)).f.fld

typedef union {
	struct {
		unsigned int lx_sa:24; //0
		unsigned int mbz_0:7; //0
		unsigned int ensram:1; //0
	} f;
	unsigned int v;
} CPU_SRAM_SEG1_ADDR_T;
#define CPU_SRAM_SEG1_ADDRrv (*((regval)0xb8004010))
#define CPU_SRAM_SEG1_ADDRdv (0x00000000)
#define RMOD_CPU_SRAM_SEG1_ADDR(...) rset(CPU_SRAM_SEG1_ADDR, CPU_SRAM_SEG1_ADDRrv, __VA_ARGS__)
#define RIZS_CPU_SRAM_SEG1_ADDR(...) rset(CPU_SRAM_SEG1_ADDR, 0, __VA_ARGS__)
#define RFLD_CPU_SRAM_SEG1_ADDR(fld) (*((const volatile CPU_SRAM_SEG1_ADDR_T *)0xb8004010)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:28; //0
		unsigned int size:4; //0
	} f;
	unsigned int v;
} CPU_SRAM_SEG1_SIZE_T;
#define CPU_SRAM_SEG1_SIZErv (*((regval)0xb8004014))
#define CPU_SRAM_SEG1_SIZEdv (0x00000000)
#define RMOD_CPU_SRAM_SEG1_SIZE(...) rset(CPU_SRAM_SEG1_SIZE, CPU_SRAM_SEG1_SIZErv, __VA_ARGS__)
#define RIZS_CPU_SRAM_SEG1_SIZE(...) rset(CPU_SRAM_SEG1_SIZE, 0, __VA_ARGS__)
#define RFLD_CPU_SRAM_SEG1_SIZE(fld) (*((const volatile CPU_SRAM_SEG1_SIZE_T *)0xb8004014)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:16; //0
		unsigned int base:8; //0
		unsigned int mbz_1:8; //0
	} f;
	unsigned int v;
} CPU_SRAM_SEG1_BASE_T;
#define CPU_SRAM_SEG1_BASErv (*((regval)0xb8004018))
#define CPU_SRAM_SEG1_BASEdv (0x00000000)
#define RMOD_CPU_SRAM_SEG1_BASE(...) rset(CPU_SRAM_SEG1_BASE, CPU_SRAM_SEG1_BASErv, __VA_ARGS__)
#define RIZS_CPU_SRAM_SEG1_BASE(...) rset(CPU_SRAM_SEG1_BASE, 0, __VA_ARGS__)
#define RFLD_CPU_SRAM_SEG1_BASE(fld) (*((const volatile CPU_SRAM_SEG1_BASE_T *)0xb8004018)).f.fld

typedef union {
	struct {
		unsigned int lx_sa:24; //0
		unsigned int mbz_0:7; //0
		unsigned int ensram:1; //0
	} f;
	unsigned int v;
} CPU_SRAM_SEG2_ADDR_T;
#define CPU_SRAM_SEG2_ADDRrv (*((regval)0xb8004020))
#define CPU_SRAM_SEG2_ADDRdv (0x00000000)
#define RMOD_CPU_SRAM_SEG2_ADDR(...) rset(CPU_SRAM_SEG2_ADDR, CPU_SRAM_SEG2_ADDRrv, __VA_ARGS__)
#define RIZS_CPU_SRAM_SEG2_ADDR(...) rset(CPU_SRAM_SEG2_ADDR, 0, __VA_ARGS__)
#define RFLD_CPU_SRAM_SEG2_ADDR(fld) (*((const volatile CPU_SRAM_SEG2_ADDR_T *)0xb8004020)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:28; //0
		unsigned int size:4; //0
	} f;
	unsigned int v;
} CPU_SRAM_SEG2_SIZE_T;
#define CPU_SRAM_SEG2_SIZErv (*((regval)0xb8004024))
#define CPU_SRAM_SEG2_SIZEdv (0x00000000)
#define RMOD_CPU_SRAM_SEG2_SIZE(...) rset(CPU_SRAM_SEG2_SIZE, CPU_SRAM_SEG2_SIZErv, __VA_ARGS__)
#define RIZS_CPU_SRAM_SEG2_SIZE(...) rset(CPU_SRAM_SEG2_SIZE, 0, __VA_ARGS__)
#define RFLD_CPU_SRAM_SEG2_SIZE(fld) (*((const volatile CPU_SRAM_SEG2_SIZE_T *)0xb8004024)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:16; //0
		unsigned int base:8; //0
		unsigned int mbz_1:8; //0
	} f;
	unsigned int v;
} CPU_SRAM_SEG2_BASE_T;
#define CPU_SRAM_SEG2_BASErv (*((regval)0xb8004028))
#define CPU_SRAM_SEG2_BASEdv (0x00000000)
#define RMOD_CPU_SRAM_SEG2_BASE(...) rset(CPU_SRAM_SEG2_BASE, CPU_SRAM_SEG2_BASErv, __VA_ARGS__)
#define RIZS_CPU_SRAM_SEG2_BASE(...) rset(CPU_SRAM_SEG2_BASE, 0, __VA_ARGS__)
#define RFLD_CPU_SRAM_SEG2_BASE(fld) (*((const volatile CPU_SRAM_SEG2_BASE_T *)0xb8004028)).f.fld

typedef union {
	struct {
		unsigned int lx_sa:24; //0
		unsigned int mbz_0:7; //0
		unsigned int ensram:1; //0
	} f;
	unsigned int v;
} CPU_SRAM_SEG3_ADDR_T;
#define CPU_SRAM_SEG3_ADDRrv (*((regval)0xb8004030))
#define CPU_SRAM_SEG3_ADDRdv (0x00000000)
#define RMOD_CPU_SRAM_SEG3_ADDR(...) rset(CPU_SRAM_SEG3_ADDR, CPU_SRAM_SEG3_ADDRrv, __VA_ARGS__)
#define RIZS_CPU_SRAM_SEG3_ADDR(...) rset(CPU_SRAM_SEG3_ADDR, 0, __VA_ARGS__)
#define RFLD_CPU_SRAM_SEG3_ADDR(fld) (*((const volatile CPU_SRAM_SEG3_ADDR_T *)0xb8004030)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:28; //0
		unsigned int size:4; //0
	} f;
	unsigned int v;
} CPU_SRAM_SEG3_SIZE_T;
#define CPU_SRAM_SEG3_SIZErv (*((regval)0xb8004034))
#define CPU_SRAM_SEG3_SIZEdv (0x00000000)
#define RMOD_CPU_SRAM_SEG3_SIZE(...) rset(CPU_SRAM_SEG3_SIZE, CPU_SRAM_SEG3_SIZErv, __VA_ARGS__)
#define RIZS_CPU_SRAM_SEG3_SIZE(...) rset(CPU_SRAM_SEG3_SIZE, 0, __VA_ARGS__)
#define RFLD_CPU_SRAM_SEG3_SIZE(fld) (*((const volatile CPU_SRAM_SEG3_SIZE_T *)0xb8004034)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:16; //0
		unsigned int base:8; //0
		unsigned int mbz_1:8; //0
	} f;
	unsigned int v;
} CPU_SRAM_SEG3_BASE_T;
#define CPU_SRAM_SEG3_BASErv (*((regval)0xb8004038))
#define CPU_SRAM_SEG3_BASEdv (0x00000000)
#define RMOD_CPU_SRAM_SEG3_BASE(...) rset(CPU_SRAM_SEG3_BASE, CPU_SRAM_SEG3_BASErv, __VA_ARGS__)
#define RIZS_CPU_SRAM_SEG3_BASE(...) rset(CPU_SRAM_SEG3_BASE, 0, __VA_ARGS__)
#define RFLD_CPU_SRAM_SEG3_BASE(fld) (*((const volatile CPU_SRAM_SEG3_BASE_T *)0xb8004038)).f.fld

typedef union {
	struct {
		unsigned int lx_sa:24; //0
		unsigned int mbz_0:7; //0
		unsigned int ensram:1; //0
	} f;
	unsigned int v;
} DSP_SRAM_SEG0_ADDR_T;
#define DSP_SRAM_SEG0_ADDRrv (*((regval)0xb8004040))
#define DSP_SRAM_SEG0_ADDRdv (0x00000000)
#define RMOD_DSP_SRAM_SEG0_ADDR(...) rset(DSP_SRAM_SEG0_ADDR, DSP_SRAM_SEG0_ADDRrv, __VA_ARGS__)
#define RIZS_DSP_SRAM_SEG0_ADDR(...) rset(DSP_SRAM_SEG0_ADDR, 0, __VA_ARGS__)
#define RFLD_DSP_SRAM_SEG0_ADDR(fld) (*((const volatile DSP_SRAM_SEG0_ADDR_T *)0xb8004040)).f.fld

typedef union {
	struct {
		unsigned int lx_sa:24; //0
		unsigned int mbz_0:7; //0
		unsigned int ensram:1; //0
	} f;
	unsigned int v;
} DSP_SRAM_SEG1_ADDR_T;
#define DSP_SRAM_SEG1_ADDRrv (*((regval)0xb8004050))
#define DSP_SRAM_SEG1_ADDRdv (0x00000000)
#define RMOD_DSP_SRAM_SEG1_ADDR(...) rset(DSP_SRAM_SEG1_ADDR, DSP_SRAM_SEG1_ADDRrv, __VA_ARGS__)
#define RIZS_DSP_SRAM_SEG1_ADDR(...) rset(DSP_SRAM_SEG1_ADDR, 0, __VA_ARGS__)
#define RFLD_DSP_SRAM_SEG1_ADDR(fld) (*((const volatile DSP_SRAM_SEG1_ADDR_T *)0xb8004050)).f.fld

typedef union {
	struct {
		unsigned int lx_sa:24; //0
		unsigned int mbz_0:7; //0
		unsigned int ensram:1; //0
	} f;
	unsigned int v;
} DSP_SRAM_SEG2_ADDR_T;
#define DSP_SRAM_SEG2_ADDRrv (*((regval)0xb8004060))
#define DSP_SRAM_SEG2_ADDRdv (0x00000000)
#define RMOD_DSP_SRAM_SEG2_ADDR(...) rset(DSP_SRAM_SEG2_ADDR, DSP_SRAM_SEG2_ADDRrv, __VA_ARGS__)
#define RIZS_DSP_SRAM_SEG2_ADDR(...) rset(DSP_SRAM_SEG2_ADDR, 0, __VA_ARGS__)
#define RFLD_DSP_SRAM_SEG2_ADDR(fld) (*((const volatile DSP_SRAM_SEG2_ADDR_T *)0xb8004060)).f.fld

typedef union {
	struct {
		unsigned int lx_sa:24; //0
		unsigned int mbz_0:7; //0
		unsigned int ensram:1; //0
	} f;
	unsigned int v;
} DSP_SRAM_SEG3_ADDR_T;
#define DSP_SRAM_SEG3_ADDRrv (*((regval)0xb8004070))
#define DSP_SRAM_SEG3_ADDRdv (0x00000000)
#define RMOD_DSP_SRAM_SEG3_ADDR(...) rset(DSP_SRAM_SEG3_ADDR, DSP_SRAM_SEG3_ADDRrv, __VA_ARGS__)
#define RIZS_DSP_SRAM_SEG3_ADDR(...) rset(DSP_SRAM_SEG3_ADDR, 0, __VA_ARGS__)
#define RFLD_DSP_SRAM_SEG3_ADDR(fld) (*((const volatile DSP_SRAM_SEG3_ADDR_T *)0xb8004070)).f.fld

/*-----------------------------------------------------
 Extraced from file_PCM.xml
-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int mbz_0:15; //0
		unsigned int isilbe:1; //0
		unsigned int zsilbe:1; //0
		unsigned int c0ilbe:1; //0
		unsigned int linear_mode:1; //0
		unsigned int pcmae:1; //0
		unsigned int mbz_1:1; //0
		unsigned int precise:1; //0
		unsigned int fsinv:1; //0
		unsigned int mbz_2:1; //0
		unsigned int fcnt:8; //0
	} f;
	unsigned int v;
} PCMCR_T;
#define PCMCRrv (*((regval)0xb8008000))
#define PCMCRdv (0x00000000)
#define RMOD_PCMCR(...) rset(PCMCR, PCMCRrv, __VA_ARGS__)
#define RIZS_PCMCR(...) rset(PCMCR, 0, __VA_ARGS__)
#define RFLD_PCMCR(fld) (*((const volatile PCMCR_T *)0xb8008000)).f.fld

typedef union {
	struct {
		unsigned int ch0_slic_sel:4; //0
		unsigned int ch0_band:1; //0
		unsigned int ch0_a:1; //0
		unsigned int ch0_te:1; //0
		unsigned int ch0_re:1; //0
		unsigned int ch1_slic_sel:4; //0
		unsigned int ch1_band:1; //0
		unsigned int ch1_a:1; //0
		unsigned int ch1_te:1; //0
		unsigned int ch1_re:1; //0
		unsigned int ch2_slic_sel:4; //0
		unsigned int ch2_band:1; //0
		unsigned int ch2_a:1; //0
		unsigned int ch2_te:1; //0
		unsigned int ch2_re:1; //0
		unsigned int ch3_slic_sel:4; //0
		unsigned int ch3_band:1; //0
		unsigned int ch3_a:1; //0
		unsigned int ch3_te:1; //0
		unsigned int ch3_re:1; //0
	} f;
	unsigned int v;
} PACHCNR03_T;
#define PACHCNR03rv (*((regval)0xb8008004))
#define PACHCNR03dv (0x00000000)
#define RMOD_PACHCNR03(...) rset(PACHCNR03, PACHCNR03rv, __VA_ARGS__)
#define RIZS_PACHCNR03(...) rset(PACHCNR03, 0, __VA_ARGS__)
#define RFLD_PACHCNR03(fld) (*((const volatile PACHCNR03_T *)0xb8008004)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:3; //0
		unsigned int ch0tsa:5; //0
		unsigned int mbz_1:3; //0
		unsigned int ch1tsa:5; //0
		unsigned int mbz_2:3; //0
		unsigned int ch2tsa:5; //0
		unsigned int mbz_3:3; //0
		unsigned int ch3tsa:5; //0
	} f;
	unsigned int v;
} PATSR03_T;
#define PATSR03rv (*((regval)0xb8008008))
#define PATSR03dv (0x00000000)
#define RMOD_PATSR03(...) rset(PATSR03, PATSR03rv, __VA_ARGS__)
#define RIZS_PATSR03(...) rset(PATSR03, 0, __VA_ARGS__)
#define RFLD_PATSR03(fld) (*((const volatile PATSR03_T *)0xb8008008)).f.fld

typedef union {
	struct {
		unsigned int ch0bsize:8; //0
		unsigned int ch1bsize:8; //0
		unsigned int ch2bsize:8; //0
		unsigned int ch3bsize:8; //0
	} f;
	unsigned int v;
} PABSIZE03_T;
#define PABSIZE03rv (*((regval)0xb800800c))
#define PABSIZE03dv (0x00000000)
#define RMOD_PABSIZE03(...) rset(PABSIZE03, PABSIZE03rv, __VA_ARGS__)
#define RIZS_PABSIZE03(...) rset(PABSIZE03, 0, __VA_ARGS__)
#define RFLD_PABSIZE03(fld) (*((const volatile PABSIZE03_T *)0xb800800c)).f.fld

typedef union {
	struct {
		unsigned int ch03_tx_buf_ptr:30; //0
		unsigned int ch03_tx_p1own:1; //0
		unsigned int ch03_tx_p0own:1; //0
	} f;
	unsigned int v;
} CH03_ATXBSA_T;
#define CH03_ATXBSArv (*((regval)0xb8008010))
#define CH03_ATXBSAdv (0x00000000)
#define RMOD_CH03_ATXBSA(...) rset(CH03_ATXBSA, CH03_ATXBSArv, __VA_ARGS__)
#define RIZS_CH03_ATXBSA(...) rset(CH03_ATXBSA, 0, __VA_ARGS__)
#define RFLD_CH03_ATXBSA(fld) (*((const volatile CH03_ATXBSA_T *)0xb8008010)).f.fld

typedef union {
	struct {
		unsigned int ch03_rx_buf_ptr:30; //0
		unsigned int ch03_rx_p1own:1; //0
		unsigned int ch03_rx_p0own:1; //0
	} f;
	unsigned int v;
} CH03_ARXBSA_T;
#define CH03_ARXBSArv (*((regval)0xb8008014))
#define CH03_ARXBSAdv (0x00000000)
#define RMOD_CH03_ARXBSA(...) rset(CH03_ARXBSA, CH03_ARXBSArv, __VA_ARGS__)
#define RIZS_CH03_ARXBSA(...) rset(CH03_ARXBSA, 0, __VA_ARGS__)
#define RFLD_CH03_ARXBSA(fld) (*((const volatile CH03_ARXBSA_T *)0xb8008014)).f.fld

typedef union {
	struct {
		unsigned int ch0_tx_p0ie:1; //0
		unsigned int ch0_tx_p1ie:1; //0
		unsigned int ch0_rx_p0ie:1; //0
		unsigned int ch0_rx_p1ie:1; //0
		unsigned int ch0_tx_p0uaie:1; //0
		unsigned int ch0_tx_p1uaie:1; //0
		unsigned int ch0_rx_p0uaie:1; //0
		unsigned int ch0_rx_p1uaie:1; //0
		unsigned int ch1_tx_p0ie:1; //0
		unsigned int ch1_tx_p1ie:1; //0
		unsigned int ch1_rx_p0ie:1; //0
		unsigned int ch1_rx_p1ie:1; //0
		unsigned int ch1_tx_p0uaie:1; //0
		unsigned int ch1_tx_p1uaie:1; //0
		unsigned int ch1_rx_p0uaie:1; //0
		unsigned int ch1_rx_p1uaie:1; //0
		unsigned int ch2_tx_p0ie:1; //0
		unsigned int ch2_tx_p1ie:1; //0
		unsigned int ch2_rx_p0ie:1; //0
		unsigned int ch2_rx_p1ie:1; //0
		unsigned int ch2_tx_p0uaie:1; //0
		unsigned int ch2_tx_p1uaie:1; //0
		unsigned int ch2_rx_p0uaie:1; //0
		unsigned int ch2_rx_p1uaie:1; //0
		unsigned int ch3_tx_p0ie:1; //0
		unsigned int ch3_tx_p1ie:1; //0
		unsigned int ch3_rx_p0ie:1; //0
		unsigned int ch3_rx_p1ie:1; //0
		unsigned int ch3_tx_p0uaie:1; //0
		unsigned int ch3_tx_p1uaie:1; //0
		unsigned int ch3_rx_p0uaie:1; //0
		unsigned int ch3_rx_p1uaie:1; //0
	} f;
	unsigned int v;
} PAIMR03_T;
#define PAIMR03rv (*((regval)0xb8008018))
#define PAIMR03dv (0x00000000)
#define RMOD_PAIMR03(...) rset(PAIMR03, PAIMR03rv, __VA_ARGS__)
#define RIZS_PAIMR03(...) rset(PAIMR03, 0, __VA_ARGS__)
#define RFLD_PAIMR03(fld) (*((const volatile PAIMR03_T *)0xb8008018)).f.fld

typedef union {
	struct {
		unsigned int ch0_tx_p0ip:1; //0
		unsigned int ch0_tx_p1ip:1; //0
		unsigned int ch0_rx_p0ip:1; //0
		unsigned int ch0_rx_p1ip:1; //0
		unsigned int ch0_tx_p0ua:1; //0
		unsigned int ch0_tx_p1ua:1; //0
		unsigned int ch0_rx_p0ua:1; //0
		unsigned int ch0_rx_p1ua:1; //0
		unsigned int ch1_tx_p0ip:1; //0
		unsigned int ch1_tx_p1ip:1; //0
		unsigned int ch1_rx_p0ip:1; //0
		unsigned int ch1_rx_p1ip:1; //0
		unsigned int ch1_tx_p0ua:1; //0
		unsigned int ch1_tx_p1ua:1; //0
		unsigned int ch1_rx_p0ua:1; //0
		unsigned int ch1_rx_p1ua:1; //0
		unsigned int ch2_tx_p0ip:1; //0
		unsigned int ch2_tx_p1ip:1; //0
		unsigned int ch2_rx_p0ip:1; //0
		unsigned int ch2_rx_p1ip:1; //0
		unsigned int ch2_tx_p0ua:1; //0
		unsigned int ch2_tx_p1ua:1; //0
		unsigned int ch2_rx_p0ua:1; //0
		unsigned int ch2_rx_p1ua:1; //0
		unsigned int ch3_tx_p0ip:1; //0
		unsigned int ch3_tx_p1ip:1; //0
		unsigned int ch3_rx_p0ip:1; //0
		unsigned int ch3_rx_p1ip:1; //0
		unsigned int ch3_tx_p0ua:1; //0
		unsigned int ch3_tx_p1ua:1; //0
		unsigned int ch3_rx_p0ua:1; //0
		unsigned int ch3_rx_p1ua:1; //0
	} f;
	unsigned int v;
} PAISR03_T;
#define PAISR03rv (*((regval)0xb800801c))
#define PAISR03dv (0x00000000)
#define RMOD_PAISR03(...) rset(PAISR03, PAISR03rv, __VA_ARGS__)
#define RIZS_PAISR03(...) rset(PAISR03, 0, __VA_ARGS__)
#define RFLD_PAISR03(fld) (*((const volatile PAISR03_T *)0xb800801c)).f.fld

typedef union {
	struct {
		unsigned int ch4_slic_sel:4; //0
		unsigned int ch4_band:1; //0
		unsigned int ch4_a:1; //0
		unsigned int ch4_te:1; //0
		unsigned int ch4_re:1; //0
		unsigned int ch5_slic_sel:4; //0
		unsigned int ch5_band:1; //0
		unsigned int ch5_a:1; //0
		unsigned int ch5_te:1; //0
		unsigned int ch5_re:1; //0
		unsigned int ch6_slic_sel:4; //0
		unsigned int ch6_band:1; //0
		unsigned int ch6_a:1; //0
		unsigned int ch6_te:1; //0
		unsigned int ch6_re:1; //0
		unsigned int ch7_slic_sel:4; //0
		unsigned int ch7_band:1; //0
		unsigned int ch7_a:1; //0
		unsigned int ch7_te:1; //0
		unsigned int ch7_re:1; //0
	} f;
	unsigned int v;
} PACHCNR47_T;
#define PACHCNR47rv (*((regval)0xb8008020))
#define PACHCNR47dv (0x00000000)
#define RMOD_PACHCNR47(...) rset(PACHCNR47, PACHCNR47rv, __VA_ARGS__)
#define RIZS_PACHCNR47(...) rset(PACHCNR47, 0, __VA_ARGS__)
#define RFLD_PACHCNR47(fld) (*((const volatile PACHCNR47_T *)0xb8008020)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:3; //0
		unsigned int ch4tsa:5; //0
		unsigned int mbz_1:3; //0
		unsigned int ch5tsa:5; //0
		unsigned int mbz_2:3; //0
		unsigned int ch6tsa:5; //0
		unsigned int mbz_3:3; //0
		unsigned int ch7tsa:5; //0
	} f;
	unsigned int v;
} PATSR47_T;
#define PATSR47rv (*((regval)0xb8008024))
#define PATSR47dv (0x00000000)
#define RMOD_PATSR47(...) rset(PATSR47, PATSR47rv, __VA_ARGS__)
#define RIZS_PATSR47(...) rset(PATSR47, 0, __VA_ARGS__)
#define RFLD_PATSR47(fld) (*((const volatile PATSR47_T *)0xb8008024)).f.fld

typedef union {
	struct {
		unsigned int ch4bsize:8; //0
		unsigned int ch5bsize:8; //0
		unsigned int ch6bsize:8; //0
		unsigned int ch7bsize:8; //0
	} f;
	unsigned int v;
} PABSIZE47_T;
#define PABSIZE47rv (*((regval)0xb8008028))
#define PABSIZE47dv (0x00000000)
#define RMOD_PABSIZE47(...) rset(PABSIZE47, PABSIZE47rv, __VA_ARGS__)
#define RIZS_PABSIZE47(...) rset(PABSIZE47, 0, __VA_ARGS__)
#define RFLD_PABSIZE47(fld) (*((const volatile PABSIZE47_T *)0xb8008028)).f.fld

typedef union {
	struct {
		unsigned int ch47_tx_buf_ptr:30; //0
		unsigned int ch47_tx_p1own:1; //0
		unsigned int ch47_tx_p0own:1; //0
	} f;
	unsigned int v;
} CH47_ATXBSA_T;
#define CH47_ATXBSArv (*((regval)0xb800802c))
#define CH47_ATXBSAdv (0x00000000)
#define RMOD_CH47_ATXBSA(...) rset(CH47_ATXBSA, CH47_ATXBSArv, __VA_ARGS__)
#define RIZS_CH47_ATXBSA(...) rset(CH47_ATXBSA, 0, __VA_ARGS__)
#define RFLD_CH47_ATXBSA(fld) (*((const volatile CH47_ATXBSA_T *)0xb800802c)).f.fld

typedef union {
	struct {
		unsigned int ch47_rx_buf_ptr:30; //0
		unsigned int ch47_rx_p1own:1; //0
		unsigned int ch47_rx_p0own:1; //0
	} f;
	unsigned int v;
} CH47_ARXBSA_T;
#define CH47_ARXBSArv (*((regval)0xb8008030))
#define CH47_ARXBSAdv (0x00000000)
#define RMOD_CH47_ARXBSA(...) rset(CH47_ARXBSA, CH47_ARXBSArv, __VA_ARGS__)
#define RIZS_CH47_ARXBSA(...) rset(CH47_ARXBSA, 0, __VA_ARGS__)
#define RFLD_CH47_ARXBSA(fld) (*((const volatile CH47_ARXBSA_T *)0xb8008030)).f.fld

typedef union {
	struct {
		unsigned int ch4_tx_p0ie:1; //0
		unsigned int ch4_tx_p1ie:1; //0
		unsigned int ch4_rx_p0ie:1; //0
		unsigned int ch4_rx_p1ie:1; //0
		unsigned int ch4_tx_p0uaie:1; //0
		unsigned int ch4_tx_p1uaie:1; //0
		unsigned int ch4_rx_p0uaie:1; //0
		unsigned int ch4_rx_p1uaie:1; //0
		unsigned int ch5_tx_p0ie:1; //0
		unsigned int ch5_tx_p1ie:1; //0
		unsigned int ch5_rx_p0ie:1; //0
		unsigned int ch5_rx_p1ie:1; //0
		unsigned int ch5_tx_p0uaie:1; //0
		unsigned int ch5_tx_p1uaie:1; //0
		unsigned int ch5_rx_p0uaie:1; //0
		unsigned int ch5_rx_p1uaie:1; //0
		unsigned int ch6_tx_p0ie:1; //0
		unsigned int ch6_tx_p1ie:1; //0
		unsigned int ch6_rx_p0ie:1; //0
		unsigned int ch6_rx_p1ie:1; //0
		unsigned int ch6_tx_p0uaie:1; //0
		unsigned int ch6_tx_p1uaie:1; //0
		unsigned int ch6_rx_p0uaie:1; //0
		unsigned int ch6_rx_p1uaie:1; //0
		unsigned int ch7_tx_p0ie:1; //0
		unsigned int ch7_tx_p1ie:1; //0
		unsigned int ch7_rx_p0ie:1; //0
		unsigned int ch7_rx_p1ie:1; //0
		unsigned int ch7_tx_p0uaie:1; //0
		unsigned int ch7_tx_p1uaie:1; //0
		unsigned int ch7_rx_p0uaie:1; //0
		unsigned int ch7_rx_p1uaie:1; //0
	} f;
	unsigned int v;
} PAIMR47_T;
#define PAIMR47rv (*((regval)0xb8008034))
#define PAIMR47dv (0x00000000)
#define RMOD_PAIMR47(...) rset(PAIMR47, PAIMR47rv, __VA_ARGS__)
#define RIZS_PAIMR47(...) rset(PAIMR47, 0, __VA_ARGS__)
#define RFLD_PAIMR47(fld) (*((const volatile PAIMR47_T *)0xb8008034)).f.fld

typedef union {
	struct {
		unsigned int ch4_tx_p0ip:1; //0
		unsigned int ch4_tx_p1ip:1; //0
		unsigned int ch4_rx_p0ip:1; //0
		unsigned int ch4_rx_p1ip:1; //0
		unsigned int ch4_tx_p0ua:1; //0
		unsigned int ch4_tx_p1ua:1; //0
		unsigned int ch4_rx_p0ua:1; //0
		unsigned int ch4_rx_p1ua:1; //0
		unsigned int ch5_tx_p0ip:1; //0
		unsigned int ch5_tx_p1ip:1; //0
		unsigned int ch5_rx_p0ip:1; //0
		unsigned int ch5_rx_p1ip:1; //0
		unsigned int ch5_tx_p0ua:1; //0
		unsigned int ch5_tx_p1ua:1; //0
		unsigned int ch5_rx_p0ua:1; //0
		unsigned int ch5_rx_p1ua:1; //0
		unsigned int ch6_tx_p0ip:1; //0
		unsigned int ch6_tx_p1ip:1; //0
		unsigned int ch6_rx_p0ip:1; //0
		unsigned int ch6_rx_p1ip:1; //0
		unsigned int ch6_tx_p0ua:1; //0
		unsigned int ch6_tx_p1ua:1; //0
		unsigned int ch6_rx_p0ua:1; //0
		unsigned int ch6_rx_p1ua:1; //0
		unsigned int ch7_tx_p0ip:1; //0
		unsigned int ch7_tx_p1ip:1; //0
		unsigned int ch7_rx_p0ip:1; //0
		unsigned int ch7_rx_p1ip:1; //0
		unsigned int ch7_tx_p0ua:1; //0
		unsigned int ch7_tx_p1ua:1; //0
		unsigned int ch7_rx_p0ua:1; //0
		unsigned int ch7_rx_p1ua:1; //0
	} f;
	unsigned int v;
} PAISR47_T;
#define PAISR47rv (*((regval)0xb8008038))
#define PAISR47dv (0x00000000)
#define RMOD_PAISR47(...) rset(PAISR47, PAISR47rv, __VA_ARGS__)
#define RIZS_PAISR47(...) rset(PAISR47, 0, __VA_ARGS__)
#define RFLD_PAISR47(fld) (*((const volatile PAISR47_T *)0xb8008038)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:3; //0
		unsigned int ch0wtsa:5; //0
		unsigned int mbz_1:3; //0
		unsigned int ch1wtsa:5; //0
		unsigned int mbz_2:3; //0
		unsigned int ch2wtsa:5; //0
		unsigned int mbz_3:3; //0
		unsigned int ch3wtsa:5; //0
	} f;
	unsigned int v;
} PAWTSR03_T;
#define PAWTSR03rv (*((regval)0xb80080a8))
#define PAWTSR03dv (0x00000000)
#define RMOD_PAWTSR03(...) rset(PAWTSR03, PAWTSR03rv, __VA_ARGS__)
#define RIZS_PAWTSR03(...) rset(PAWTSR03, 0, __VA_ARGS__)
#define RFLD_PAWTSR03(fld) (*((const volatile PAWTSR03_T *)0xb80080a8)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:3; //0
		unsigned int ch4wtsa:5; //0
		unsigned int mbz_1:3; //0
		unsigned int ch5wtsa:5; //0
		unsigned int mbz_2:3; //0
		unsigned int ch6wtsa:5; //0
		unsigned int mbz_3:3; //0
		unsigned int ch7wtsa:5; //0
	} f;
	unsigned int v;
} PAWTSR47_T;
#define PAWTSR47rv (*((regval)0xb80080ac))
#define PAWTSR47dv (0x00000000)
#define RMOD_PAWTSR47(...) rset(PAWTSR47, PAWTSR47rv, __VA_ARGS__)
#define RIZS_PAWTSR47(...) rset(PAWTSR47, 0, __VA_ARGS__)
#define RFLD_PAWTSR47(fld) (*((const volatile PAWTSR47_T *)0xb80080ac)).f.fld

/*-----------------------------------------------------
 Extraced from file_VOIP_SPI.xml
-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int mbz_0:5; //0
		unsigned int cstime:1; //0
		unsigned int wrie:1; //0
		unsigned int rdie:1; //0
		unsigned int add:8; //0
		unsigned int ctrl:8; //0
		unsigned int lsb:1; //0
		unsigned int mbz_1:1; //0
		unsigned int cmd:1; //0
		unsigned int start:1; //0
		unsigned int sclk_type:1; //0
		unsigned int mbz_2:1; //0
		unsigned int csp:1; //0
		unsigned int mbz_3:1; //0
	} f;
	unsigned int v;
} SPICNR_T;
#define SPICNRrv (*((regval)0xb8009000))
#define SPICNRdv (0x00000000)
#define RMOD_SPICNR(...) rset(SPICNR, SPICNRrv, __VA_ARGS__)
#define RIZS_SPICNR(...) rset(SPICNR, 0, __VA_ARGS__)
#define RFLD_SPICNR(fld) (*((const volatile SPICNR_T *)0xb8009000)).f.fld

typedef union {
	struct {
		unsigned int rdip:1; //0
		unsigned int wdip:1; //0
		unsigned int msif_lock:6; //0
		unsigned int mbz_0:24; //0
	} f;
	unsigned int v;
} SPISTR_T;
#define SPISTRrv (*((regval)0xb8009004))
#define SPISTRdv (0x00000000)
#define RMOD_SPISTR(...) rset(SPISTR, SPISTRrv, __VA_ARGS__)
#define RIZS_SPISTR(...) rset(SPISTR, 0, __VA_ARGS__)
#define RFLD_SPISTR(fld) (*((const volatile SPISTR_T *)0xb8009004)).f.fld

typedef union {
	struct {
		unsigned int div:8; //255
		unsigned int mbz_0:24; //0
	} f;
	unsigned int v;
} SPICKDIV_T;
#define SPICKDIVrv (*((regval)0xb8009008))
#define SPICKDIVdv (0xff000000)
#define RMOD_SPICKDIV(...) rset(SPICKDIV, SPICKDIVrv, __VA_ARGS__)
#define RIZS_SPICKDIV(...) rset(SPICKDIV, 0, __VA_ARGS__)
#define RFLD_SPICKDIV(fld) (*((const volatile SPICKDIV_T *)0xb8009008)).f.fld

typedef union {
	struct {
		unsigned int spi_rtdr0:8; //0
		unsigned int spi_rtdr1:8; //0
		unsigned int mbz_0:16; //0
	} f;
	unsigned int v;
} SPIRTDR_T;
#define SPIRTDRrv (*((regval)0xb800900c))
#define SPIRTDRdv (0x00000000)
#define RMOD_SPIRTDR(...) rset(SPIRTDR, SPIRTDRrv, __VA_ARGS__)
#define RIZS_SPIRTDR(...) rset(SPIRTDR, 0, __VA_ARGS__)
#define RFLD_SPIRTDR(fld) (*((const volatile SPIRTDR_T *)0xb800900c)).f.fld

typedef union {
	struct {
		unsigned int spi_sel_cs0:1; //0
		unsigned int spi_sel_cs1:1; //0
		unsigned int spi_sel_cs2:1; //0
		unsigned int spi_sel_cs3:1; //0
		unsigned int spi_sel_cs4:1; //0
		unsigned int spi_sel_cs5:1; //0
		unsigned int mbz_0:2; //0
		unsigned int spi_ctl_en:1; //1
		unsigned int spi_add_en:1; //1
		unsigned int spi_d0_en:1; //1
		unsigned int spi_d1_en:1; //0
		unsigned int mbz_1:3; //0
		unsigned int spi_csfl:1; //0
		unsigned int spi_dcs_cnt_unit:8; //4
		unsigned int spi_dcs_def_cnt:8; //255
	} f;
	unsigned int v;
} SPITCR_T;
#define SPITCRrv (*((regval)0xb8009010))
#define SPITCRdv (0x00e004ff)
#define RMOD_SPITCR(...) rset(SPITCR, SPITCRrv, __VA_ARGS__)
#define RIZS_SPITCR(...) rset(SPITCR, 0, __VA_ARGS__)
#define RFLD_SPITCR(fld) (*((const volatile SPITCR_T *)0xb8009010)).f.fld

typedef union {
	struct {
		unsigned int cs0_des_cnt:8; //255
		unsigned int cs1_des_cnt:8; //255
		unsigned int cs2_des_cnt:8; //255
		unsigned int cs3_des_cnt:8; //255
	} f;
	unsigned int v;
} SPICDTCR0_T;
#define SPICDTCR0rv (*((regval)0xb8009014))
#define SPICDTCR0dv (0xffffffff)
#define RMOD_SPICDTCR0(...) rset(SPICDTCR0, SPICDTCR0rv, __VA_ARGS__)
#define RIZS_SPICDTCR0(...) rset(SPICDTCR0, 0, __VA_ARGS__)
#define RFLD_SPICDTCR0(fld) (*((const volatile SPICDTCR0_T *)0xb8009014)).f.fld

typedef union {
	struct {
		unsigned int cs4_des_cnt:8; //255
		unsigned int cs5_des_cnt:8; //255
		unsigned int mbz_0:16; //0
	} f;
	unsigned int v;
} SPICDTCR1_T;
#define SPICDTCR1rv (*((regval)0xb8009018))
#define SPICDTCR1dv (0xffff0000)
#define RMOD_SPICDTCR1(...) rset(SPICDTCR1, SPICDTCR1rv, __VA_ARGS__)
#define RIZS_SPICDTCR1(...) rset(SPICDTCR1, 0, __VA_ARGS__)
#define RFLD_SPICDTCR1(fld) (*((const volatile SPICDTCR1_T *)0xb8009018)).f.fld

typedef union {
	struct {
		unsigned int cs_setup_cal:8; //0
		unsigned int cs_hold_cal:8; //0
		unsigned int sdo_setup_cal:8; //0
		unsigned int mbz_0:8; //0
	} f;
	unsigned int v;
} SPICALR_T;
#define SPICALRrv (*((regval)0xb800901c))
#define SPICALRdv (0x00000000)
#define RMOD_SPICALR(...) rset(SPICALR, SPICALRrv, __VA_ARGS__)
#define RIZS_SPICALR(...) rset(SPICALR, 0, __VA_ARGS__)
#define RFLD_SPICALR(fld) (*((const volatile SPICALR_T *)0xb800901c)).f.fld

/*-----------------------------------------------------
 Extraced from file_VOIP_ACC.xml
-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int enable:1; //0
		unsigned int poll:1; //0
		unsigned int no_use29:2; //0
		unsigned int func_en:4; //0
		unsigned int no_use23:15; //0
		unsigned int dbgsel:1; //0
		unsigned int burstsiz:2; //0
		unsigned int enough:2; //0
		unsigned int no_use00:4; //0
	} f;
	unsigned int v;
} VOIPTOPCNR_T;
#define VOIPTOPCNRrv (*((regval)0xb800b000))
#define VOIPTOPCNRdv (0x00000000)
#define RMOD_VOIPTOPCNR(...) rset(VOIPTOPCNR, VOIPTOPCNRrv, __VA_ARGS__)
#define RIZS_VOIPTOPCNR(...) rset(VOIPTOPCNR, 0, __VA_ARGS__)
#define RFLD_VOIPTOPCNR(fld) (*((const volatile VOIPTOPCNR_T *)0xb800b000)).f.fld

typedef union {
	struct {
		unsigned int comp_ie:1; //0
		unsigned int no_use30:31; //0
	} f;
	unsigned int v;
} VOIPIMR_T;
#define VOIPIMRrv (*((regval)0xb800b004))
#define VOIPIMRdv (0x00000000)
#define RMOD_VOIPIMR(...) rset(VOIPIMR, VOIPIMRrv, __VA_ARGS__)
#define RIZS_VOIPIMR(...) rset(VOIPIMR, 0, __VA_ARGS__)
#define RFLD_VOIPIMR(fld) (*((const volatile VOIPIMR_T *)0xb800b004)).f.fld

typedef union {
	struct {
		unsigned int comp_ip:1; //0
		unsigned int mbz_0:31; //0
	} f;
	unsigned int v;
} VOIPISR_T;
#define VOIPISRrv (*((regval)0xb800b008))
#define VOIPISRdv (0x00000000)
#define RMOD_VOIPISR(...) rset(VOIPISR, VOIPISRrv, __VA_ARGS__)
#define RIZS_VOIPISR(...) rset(VOIPISR, 0, __VA_ARGS__)
#define RFLD_VOIPISR(fld) (*((const volatile VOIPISR_T *)0xb800b008)).f.fld

typedef union {
	struct {
		unsigned int function:4; //0
		unsigned int status:4; //0
		unsigned int mbz_0:24; //0
	} f;
	unsigned int v;
} VOIPCTRL_T;
#define VOIPCTRLrv (*((regval)0xb800b010))
#define VOIPCTRLdv (0x00000000)
#define RMOD_VOIPCTRL(...) rset(VOIPCTRL, VOIPCTRLrv, __VA_ARGS__)
#define RIZS_VOIPCTRL(...) rset(VOIPCTRL, 0, __VA_ARGS__)
#define RFLD_VOIPCTRL(fld) (*((const volatile VOIPCTRL_T *)0xb800b010)).f.fld

typedef union {
	struct {
		unsigned int type:3; //0
		unsigned int order:6; //0
		unsigned int divinterval:3; //0
		unsigned int num_of_grid_pt:8; //0
		unsigned int mbz_0:12; //0
	} f;
	unsigned int v;
} VOIPLSP_T;
#define VOIPLSPrv (*((regval)0xb800b014))
#define VOIPLSPdv (0x00000000)
#define RMOD_VOIPLSP(...) rset(VOIPLSP, VOIPLSPrv, __VA_ARGS__)
#define RIZS_VOIPLSP(...) rset(VOIPLSP, 0, __VA_ARGS__)
#define RFLD_VOIPLSP(fld) (*((const volatile VOIPLSP_T *)0xb800b014)).f.fld

typedef union {
	struct {
		unsigned int type:3; //0
		unsigned int num_cyc:3; //0
		unsigned int num_pulse:4; //0
		unsigned int num_step:3; //0
		unsigned int num_iter_0:2; //0
		unsigned int num_iter_1:2; //0
		unsigned int trk_0_s:3; //0
		unsigned int trk_0_e:3; //0
		unsigned int trk_1_s:3; //0
		unsigned int trk_1_e:3; //0
		unsigned int sfn:2; //0
		unsigned int mbz_0:1; //0
	} f;
	unsigned int v;
} VOIPACELP_T;
#define VOIPACELPrv (*((regval)0xb800b018))
#define VOIPACELPdv (0x00000000)
#define RMOD_VOIPACELP(...) rset(VOIPACELP, VOIPACELPrv, __VA_ARGS__)
#define RIZS_VOIPACELP(...) rset(VOIPACELP, 0, __VA_ARGS__)
#define RFLD_VOIPACELP(fld) (*((const volatile VOIPACELP_T *)0xb800b018)).f.fld

typedef union {
	struct {
		unsigned int no_use31:3; //0
		unsigned int blkptr:29; //0
	} f;
	unsigned int v;
} VOIPSBP0_T;
#define VOIPSBP0rv (*((regval)0xb800b020))
#define VOIPSBP0dv (0x00000000)
#define RMOD_VOIPSBP0(...) rset(VOIPSBP0, VOIPSBP0rv, __VA_ARGS__)
#define RIZS_VOIPSBP0(...) rset(VOIPSBP0, 0, __VA_ARGS__)
#define RFLD_VOIPSBP0(fld) (*((const volatile VOIPSBP0_T *)0xb800b020)).f.fld

typedef union {
	struct {
		unsigned int ldb:1; //0
		unsigned int mbz_0:18; //0
		unsigned int blklen:13; //0
	} f;
	unsigned int v;
} VOIPSBL0_T;
#define VOIPSBL0rv (*((regval)0xb800b024))
#define VOIPSBL0dv (0x00000000)
#define RMOD_VOIPSBL0(...) rset(VOIPSBL0, VOIPSBL0rv, __VA_ARGS__)
#define RIZS_VOIPSBL0(...) rset(VOIPSBL0, 0, __VA_ARGS__)
#define RFLD_VOIPSBL0(fld) (*((const volatile VOIPSBL0_T *)0xb800b024)).f.fld

typedef union {
	struct {
		unsigned int no_use31:2; //0
		unsigned int blkptr:30; //0
	} f;
	unsigned int v;
} VOIPSBP1_T;
#define VOIPSBP1rv (*((regval)0xb800b028))
#define VOIPSBP1dv (0x00000000)
#define RMOD_VOIPSBP1(...) rset(VOIPSBP1, VOIPSBP1rv, __VA_ARGS__)
#define RIZS_VOIPSBP1(...) rset(VOIPSBP1, 0, __VA_ARGS__)
#define RFLD_VOIPSBP1(fld) (*((const volatile VOIPSBP1_T *)0xb800b028)).f.fld

typedef union {
	struct {
		unsigned int ldb:1; //0
		unsigned int mbz_0:18; //0
		unsigned int blklen:13; //0
	} f;
	unsigned int v;
} VOIPSBL1_T;
#define VOIPSBL1rv (*((regval)0xb800b02c))
#define VOIPSBL1dv (0x00000000)
#define RMOD_VOIPSBL1(...) rset(VOIPSBL1, VOIPSBL1rv, __VA_ARGS__)
#define RIZS_VOIPSBL1(...) rset(VOIPSBL1, 0, __VA_ARGS__)
#define RFLD_VOIPSBL1(fld) (*((const volatile VOIPSBL1_T *)0xb800b02c)).f.fld

typedef union {
	struct {
		unsigned int no_use31:3; //0
		unsigned int blkptr:29; //0
	} f;
	unsigned int v;
} VOIPDBP0_T;
#define VOIPDBP0rv (*((regval)0xb800b030))
#define VOIPDBP0dv (0x00000000)
#define RMOD_VOIPDBP0(...) rset(VOIPDBP0, VOIPDBP0rv, __VA_ARGS__)
#define RIZS_VOIPDBP0(...) rset(VOIPDBP0, 0, __VA_ARGS__)
#define RFLD_VOIPDBP0(fld) (*((const volatile VOIPDBP0_T *)0xb800b030)).f.fld

typedef union {
	struct {
		unsigned int ldb:1; //0
		unsigned int mbz_0:18; //0
		unsigned int blklen:13; //0
	} f;
	unsigned int v;
} VOIPDBL0_T;
#define VOIPDBL0rv (*((regval)0xb800b034))
#define VOIPDBL0dv (0x00000000)
#define RMOD_VOIPDBL0(...) rset(VOIPDBL0, VOIPDBL0rv, __VA_ARGS__)
#define RIZS_VOIPDBL0(...) rset(VOIPDBL0, 0, __VA_ARGS__)
#define RFLD_VOIPDBL0(fld) (*((const volatile VOIPDBL0_T *)0xb800b034)).f.fld

typedef union {
	struct {
		unsigned int no_use31:2; //0
		unsigned int blkptr:30; //0
	} f;
	unsigned int v;
} VOIPDBP1_T;
#define VOIPDBP1rv (*((regval)0xb800b038))
#define VOIPDBP1dv (0x00000000)
#define RMOD_VOIPDBP1(...) rset(VOIPDBP1, VOIPDBP1rv, __VA_ARGS__)
#define RIZS_VOIPDBP1(...) rset(VOIPDBP1, 0, __VA_ARGS__)
#define RFLD_VOIPDBP1(fld) (*((const volatile VOIPDBP1_T *)0xb800b038)).f.fld

typedef union {
	struct {
		unsigned int ldb:1; //0
		unsigned int mbz_0:18; //0
		unsigned int blklen:13; //0
	} f;
	unsigned int v;
} VOIPDBL1_T;
#define VOIPDBL1rv (*((regval)0xb800b03c))
#define VOIPDBL1dv (0x00000000)
#define RMOD_VOIPDBL1(...) rset(VOIPDBL1, VOIPDBL1rv, __VA_ARGS__)
#define RIZS_VOIPDBL1(...) rset(VOIPDBL1, 0, __VA_ARGS__)
#define RFLD_VOIPDBL1(fld) (*((const volatile VOIPDBL1_T *)0xb800b03c)).f.fld

typedef union {
	struct {
		unsigned int len1:10; //0
		unsigned int len2:10; //0
		unsigned int step1:4; //0
		unsigned int step2:4; //0
		unsigned int mode:4; //0
	} f;
	unsigned int v;
} VOIPCROSS1_T;
#define VOIPCROSS1rv (*((regval)0xb800b040))
#define VOIPCROSS1dv (0x00000000)
#define RMOD_VOIPCROSS1(...) rset(VOIPCROSS1, VOIPCROSS1rv, __VA_ARGS__)
#define RIZS_VOIPCROSS1(...) rset(VOIPCROSS1, 0, __VA_ARGS__)
#define RFLD_VOIPCROSS1(fld) (*((const volatile VOIPCROSS1_T *)0xb800b040)).f.fld

typedef union {
	struct {
		unsigned int p1_inc:4; //0
		unsigned int p2_inc:4; //0
		unsigned int p1_addr:10; //0
		unsigned int p2_addr:10; //0
		unsigned int no_use03:4; //0
	} f;
	unsigned int v;
} VOIPCROSS2_T;
#define VOIPCROSS2rv (*((regval)0xb800b044))
#define VOIPCROSS2dv (0x00000000)
#define RMOD_VOIPCROSS2(...) rset(VOIPCROSS2, VOIPCROSS2rv, __VA_ARGS__)
#define RIZS_VOIPCROSS2(...) rset(VOIPCROSS2, 0, __VA_ARGS__)
#define RFLD_VOIPCROSS2(fld) (*((const volatile VOIPCROSS2_T *)0xb800b044)).f.fld

typedef union {
	struct {
		unsigned int initial_value:32; //0
	} f;
	unsigned int v;
} VOIPCROSS3_T;
#define VOIPCROSS3rv (*((regval)0xb800b048))
#define VOIPCROSS3dv (0x00000000)
#define RMOD_VOIPCROSS3(...) rset(VOIPCROSS3, VOIPCROSS3rv, __VA_ARGS__)
#define RIZS_VOIPCROSS3(...) rset(VOIPCROSS3, 0, __VA_ARGS__)
#define RFLD_VOIPCROSS3(fld) (*((const volatile VOIPCROSS3_T *)0xb800b048)).f.fld

typedef union {
	struct {
		unsigned int rme_dvse:1; //0
		unsigned int no_use30:3; //0
		unsigned int rm_dvs:4; //0
		unsigned int slight_sleep:1; //0
		unsigned int no_use22:23; //0
	} f;
	unsigned int v;
} SRAM_TEST_T;
#define SRAM_TESTrv (*((regval)0xb800b04c))
#define SRAM_TESTdv (0x00000000)
#define RMOD_SRAM_TEST(...) rset(SRAM_TEST, SRAM_TESTrv, __VA_ARGS__)
#define RIZS_SRAM_TEST(...) rset(SRAM_TEST, 0, __VA_ARGS__)
#define RFLD_SRAM_TEST(fld) (*((const volatile SRAM_TEST_T *)0xb800b04c)).f.fld

/*-----------------------------------------------------
 Extraced from file_VOIP_FFT.xml
-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int enable:1; //0
		unsigned int poll:1; //0
		unsigned int no_use29:21; //0
		unsigned int dbgsel:1; //0
		unsigned int burstsiz:2; //0
		unsigned int no_use05:6; //0
	} f;
	unsigned int v;
} FFTTOPCNR_T;
#define FFTTOPCNRrv (*((regval)0xb800b800))
#define FFTTOPCNRdv (0x00000000)
#define RMOD_FFTTOPCNR(...) rset(FFTTOPCNR, FFTTOPCNRrv, __VA_ARGS__)
#define RIZS_FFTTOPCNR(...) rset(FFTTOPCNR, 0, __VA_ARGS__)
#define RFLD_FFTTOPCNR(fld) (*((const volatile FFTTOPCNR_T *)0xb800b800)).f.fld

typedef union {
	struct {
		unsigned int comp_ie:1; //0
		unsigned int no_use30:31; //0
	} f;
	unsigned int v;
} FFTIMR_T;
#define FFTIMRrv (*((regval)0xb800b804))
#define FFTIMRdv (0x00000000)
#define RMOD_FFTIMR(...) rset(FFTIMR, FFTIMRrv, __VA_ARGS__)
#define RIZS_FFTIMR(...) rset(FFTIMR, 0, __VA_ARGS__)
#define RFLD_FFTIMR(fld) (*((const volatile FFTIMR_T *)0xb800b804)).f.fld

typedef union {
	struct {
		unsigned int comp_ip:1; //0
		unsigned int mbz_0:31; //0
	} f;
	unsigned int v;
} FFTISR_T;
#define FFTISRrv (*((regval)0xb800b808))
#define FFTISRdv (0x00000000)
#define RMOD_FFTISR(...) rset(FFTISR, FFTISRrv, __VA_ARGS__)
#define RIZS_FFTISR(...) rset(FFTISR, 0, __VA_ARGS__)
#define RFLD_FFTISR(fld) (*((const volatile FFTISR_T *)0xb800b808)).f.fld

typedef union {
	struct {
		unsigned int function:1; //0
		unsigned int length:3; //0
		unsigned int bypass:1; //0
		unsigned int mbz_0:1; //0
		unsigned int status:2; //0
		unsigned int fft_total_scale:5; //0
		unsigned int ifft_total_scale:5; //0
		unsigned int mbz_1:14; //0
	} f;
	unsigned int v;
} FFTCTST_T;
#define FFTCTSTrv (*((regval)0xb800b810))
#define FFTCTSTdv (0x00000000)
#define RMOD_FFTCTST(...) rset(FFTCTST, FFTCTSTrv, __VA_ARGS__)
#define RIZS_FFTCTST(...) rset(FFTCTST, 0, __VA_ARGS__)
#define RFLD_FFTCTST(fld) (*((const volatile FFTCTST_T *)0xb800b810)).f.fld

typedef union {
	struct {
		unsigned int no_use31:2; //0
		unsigned int blkptr:30; //0
	} f;
	unsigned int v;
} FFTSBP0_T;
#define FFTSBP0rv (*((regval)0xb800b820))
#define FFTSBP0dv (0x00000000)
#define RMOD_FFTSBP0(...) rset(FFTSBP0, FFTSBP0rv, __VA_ARGS__)
#define RIZS_FFTSBP0(...) rset(FFTSBP0, 0, __VA_ARGS__)
#define RFLD_FFTSBP0(fld) (*((const volatile FFTSBP0_T *)0xb800b820)).f.fld

typedef union {
	struct {
		unsigned int ldb:1; //0
		unsigned int mbz_0:17; //0
		unsigned int blklen:14; //0
	} f;
	unsigned int v;
} FFTSBL0_T;
#define FFTSBL0rv (*((regval)0xb800b824))
#define FFTSBL0dv (0x00000000)
#define RMOD_FFTSBL0(...) rset(FFTSBL0, FFTSBL0rv, __VA_ARGS__)
#define RIZS_FFTSBL0(...) rset(FFTSBL0, 0, __VA_ARGS__)
#define RFLD_FFTSBL0(fld) (*((const volatile FFTSBL0_T *)0xb800b824)).f.fld

typedef union {
	struct {
		unsigned int no_use31:2; //0
		unsigned int blkptr:30; //0
	} f;
	unsigned int v;
} FFTSBP1_T;
#define FFTSBP1rv (*((regval)0xb800b828))
#define FFTSBP1dv (0x00000000)
#define RMOD_FFTSBP1(...) rset(FFTSBP1, FFTSBP1rv, __VA_ARGS__)
#define RIZS_FFTSBP1(...) rset(FFTSBP1, 0, __VA_ARGS__)
#define RFLD_FFTSBP1(fld) (*((const volatile FFTSBP1_T *)0xb800b828)).f.fld

typedef union {
	struct {
		unsigned int ldb:1; //0
		unsigned int mbz_0:17; //0
		unsigned int blklen:14; //0
	} f;
	unsigned int v;
} FFTSBL1_T;
#define FFTSBL1rv (*((regval)0xb800b82c))
#define FFTSBL1dv (0x00000000)
#define RMOD_FFTSBL1(...) rset(FFTSBL1, FFTSBL1rv, __VA_ARGS__)
#define RIZS_FFTSBL1(...) rset(FFTSBL1, 0, __VA_ARGS__)
#define RFLD_FFTSBL1(fld) (*((const volatile FFTSBL1_T *)0xb800b82c)).f.fld

typedef union {
	struct {
		unsigned int no_use31:2; //0
		unsigned int blkptr:30; //0
	} f;
	unsigned int v;
} FFTDBP0_T;
#define FFTDBP0rv (*((regval)0xb800b830))
#define FFTDBP0dv (0x00000000)
#define RMOD_FFTDBP0(...) rset(FFTDBP0, FFTDBP0rv, __VA_ARGS__)
#define RIZS_FFTDBP0(...) rset(FFTDBP0, 0, __VA_ARGS__)
#define RFLD_FFTDBP0(fld) (*((const volatile FFTDBP0_T *)0xb800b830)).f.fld

typedef union {
	struct {
		unsigned int ldb:1; //0
		unsigned int mbz_0:17; //0
		unsigned int blklen:14; //0
	} f;
	unsigned int v;
} FFTDBL0_T;
#define FFTDBL0rv (*((regval)0xb800b834))
#define FFTDBL0dv (0x00000000)
#define RMOD_FFTDBL0(...) rset(FFTDBL0, FFTDBL0rv, __VA_ARGS__)
#define RIZS_FFTDBL0(...) rset(FFTDBL0, 0, __VA_ARGS__)
#define RFLD_FFTDBL0(fld) (*((const volatile FFTDBL0_T *)0xb800b834)).f.fld

typedef union {
	struct {
		unsigned int no_use31:2; //0
		unsigned int blkptr:30; //0
	} f;
	unsigned int v;
} FFTDBP1_T;
#define FFTDBP1rv (*((regval)0xb800b838))
#define FFTDBP1dv (0x00000000)
#define RMOD_FFTDBP1(...) rset(FFTDBP1, FFTDBP1rv, __VA_ARGS__)
#define RIZS_FFTDBP1(...) rset(FFTDBP1, 0, __VA_ARGS__)
#define RFLD_FFTDBP1(fld) (*((const volatile FFTDBP1_T *)0xb800b838)).f.fld

typedef union {
	struct {
		unsigned int ldb:1; //0
		unsigned int mbz_0:17; //0
		unsigned int blklen:14; //0
	} f;
	unsigned int v;
} FFTDBL1_T;
#define FFTDBL1rv (*((regval)0xb800b83c))
#define FFTDBL1dv (0x00000000)
#define RMOD_FFTDBL1(...) rset(FFTDBL1, FFTDBL1rv, __VA_ARGS__)
#define RIZS_FFTDBL1(...) rset(FFTDBL1, 0, __VA_ARGS__)
#define RFLD_FFTDBL1(fld) (*((const volatile FFTDBL1_T *)0xb800b83c)).f.fld

#endif
