#ifndef __BUS_PRI_SWT_H__
#define __BUS_PRI_SWT_H__


/*-----------------------------------------------------
 Bus Priority Switch Register
 -----------------------------------------------------*/
    typedef union {
        struct {
            unsigned int mbz_31_8:24;      //0
            unsigned int park_dsp:1;       // 0
            unsigned int park_cpu:1;       // 1
            unsigned int park_pbo_usr:1;   // 0 
            unsigned int park_sel:1;       // 0        
            unsigned int mbz_3_2:2;        // 0
            unsigned int pri_swt_en:1;     // 1
            unsigned int bus_pri_swt_new:1;// 0
        } f;
        unsigned int v;
    } BUS_PRI_SWT_CTRL_T;
#define BUS_PRI_SWT_CTRLrv (*((regval)0xb8001A00))
#define BUS_PRI_SWT_CTRLdv (0x000000042)
#define RMOD_BUS_PRI_SWT_CTRL(...) rset(BUS_PRI_SWT_CTRL,BUS_PRI_SWT_CTRLrv, __VA_ARGS__)
#define RIZS_BUS_PRI_SWT_CTRL(...) rset(BUS_PRI_SWT_CTRL, 0, __VA_ARGS__)
#define RFLD_BUS_PRI_SWT_CTRL(fld) (*((const volatile BUS_PRI_SWT_CTRL_T *)0xb8001A00)).f.fld

    typedef union {
        struct {
            unsigned int pri_token:31;      //0
        } f;
        unsigned int v;
    } BUS_PRI_SWT_0_T;
#define BUS_PRI_SWT_0rv (*((regval)0xb8001A04))
#define BUS_PRI_SWT_0dv (0x000000000)
#define RMOD_BUS_PRI_SWT_0(...) rset(BUS_PRI_SWT_0,BUS_PRI_SWT_0rv, __VA_ARGS__)
#define RIZS_BUS_PRI_SWT_0(...) rset(BUS_PRI_SWT_0, 0, __VA_ARGS__)
#define RFLD_BUS_PRI_SWT_0(fld) (*((const volatile BUS_PRI_SWT_0_T *)0xb8001A04)).f.fld



    typedef union {
        struct {
            unsigned int pri_token:31;      //0
        } f;
        unsigned int v;
    } BUS_PRI_SWT_2_T;
#define BUS_PRI_SWT_2rv (*((regval)0xb8001A0C))
#define BUS_PRI_SWT_2dv (0x000000000)
#define RMOD_BUS_PRI_SWT_2(...) rset(BUS_PRI_SWT_2,BUS_PRI_SWT_2rv, __VA_ARGS__)
#define RIZS_BUS_PRI_SWT_2(...) rset(BUS_PRI_SWT_2, 0, __VA_ARGS__)
#define RFLD_BUS_PRI_SWT_2(fld) (*((const volatile BUS_PRI_SWT_2_T *)0xb8001A0C)).f.fld



    typedef union {
        struct {
            unsigned int pri_token:31;      //0
        } f;
        unsigned int v;
    } BUS_PRI_SWT_3_T;
#define BUS_PRI_SWT_3rv (*((regval)0xb8001A10))
#define BUS_PRI_SWT_3dv (0x000000000)
#define RMOD_BUS_PRI_SWT_3(...) rset(BUS_PRI_SWT_3,BUS_PRI_SWT_3rv, __VA_ARGS__)
#define RIZS_BUS_PRI_SWT_3(...) rset(BUS_PRI_SWT_3, 0, __VA_ARGS__)
#define RFLD_BUS_PRI_SWT_3(fld) (*((const volatile BUS_PRI_SWT_3_T *)0xb8001A10)).f.fld



    typedef union {
        struct {
            unsigned int pri_token:31;      //0
        } f;
        unsigned int v;
    } BUS_PRI_SWT_4_T;
#define BUS_PRI_SWT_4rv (*((regval)0xb8001A14))
#define BUS_PRI_SWT_4dv (0x000000000)
#define RMOD_BUS_PRI_SWT_4(...) rset(BUS_PRI_SWT_4,BUS_PRI_SWT_4rv, __VA_ARGS__)
#define RIZS_BUS_PRI_SWT_4(...) rset(BUS_PRI_SWT_4, 0, __VA_ARGS__)
#define RFLD_BUS_PRI_SWT_4(fld) (*((const volatile BUS_PRI_SWT_4_T *)0xb8001A14)).f.fld



    typedef union {
        struct {
            unsigned int pri_token:31;      //0
        } f;
        unsigned int v;
    } BUS_PRI_SWT_5_T;
#define BUS_PRI_SWT_5rv (*((regval)0xb8001A18))
#define BUS_PRI_SWT_5dv (0x000000000)
#define RMOD_BUS_PRI_SWT_5(...) rset(BUS_PRI_SWT_5,BUS_PRI_SWT_5rv, __VA_ARGS__)
#define RIZS_BUS_PRI_SWT_5(...) rset(BUS_PRI_SWT_5, 0, __VA_ARGS__)
#define RFLD_BUS_PRI_SWT_5(fld) (*((const volatile BUS_PRI_SWT_5_T *)0xb8001A18)).f.fld



    typedef union {
        struct {
            unsigned int pri_token:31;      //0
        } f;
        unsigned int v;
    } BUS_PRI_SWT_6_T;
#define BUS_PRI_SWT_6rv (*((regval)0xb8001A1C))
#define BUS_PRI_SWT_6dv (0x000000000)
#define RMOD_BUS_PRI_SWT_6(...) rset(BUS_PRI_SWT_6,BUS_PRI_SWT_6rv, __VA_ARGS__)
#define RIZS_BUS_PRI_SWT_6(...) rset(BUS_PRI_SWT_6, 0, __VA_ARGS__)
#define RFLD_BUS_PRI_SWT_6(fld) (*((const volatile BUS_PRI_SWT_6_T *)0xb8001A1C)).f.fld



    typedef union {
        struct {
            unsigned int pri_token:31;      //0
        } f;
        unsigned int v;
    } BUS_PRI_SWT_7_T;
#define BUS_PRI_SWT_7rv (*((regval)0xb8001A20))
#define BUS_PRI_SWT_7dv (0x000000000)
#define RMOD_BUS_PRI_SWT_7(...) rset(BUS_PRI_SWT_7,BUS_PRI_SWT_2rv, __VA_ARGS__)
#define RIZS_BUS_PRI_SWT_7(...) rset(BUS_PRI_SWT_7, 0, __VA_ARGS__)
#define RFLD_BUS_PRI_SWT_7(fld) (*((const volatile BUS_PRI_SWT_7_T *)0xb8001A20)).f.fld



    typedef union {
        struct {
            unsigned int pri_token:31;      //0
        } f;
        unsigned int v;
    } BUS_PRI_SWT_8_T;
#define BUS_PRI_SWT_8rv (*((regval)0xb8001A24))
#define BUS_PRI_SWT_8dv (0x000000000)
#define RMOD_BUS_PRI_SWT_8(...) rset(BUS_PRI_SWT_8,BUS_PRI_SWT_8rv, __VA_ARGS__)
#define RIZS_BUS_PRI_SWT_8(...) rset(BUS_PRI_SWT_8, 0, __VA_ARGS__)
#define RFLD_BUS_PRI_SWT_8(fld) (*((const volatile BUS_PRI_SWT_8_T *)0xb8001A24)).f.fld


#endif //__BUS_PRI_SWT_H__


