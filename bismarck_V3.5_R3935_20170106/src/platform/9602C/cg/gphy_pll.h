#ifndef __GPHY_PLL_H__
#define __GPHY_PLL_H__


typedef union {
	struct {
		unsigned int mbz:16;
		unsigned int cfg_div_lx:2;      //0:/5, 1: /6, 2: /7, 3: /8
		unsigned int cfg_div_spif:2;    //0:/4, 1: /5, 2: /6, 3: /7
		unsigned int reg3x_pll_11_0:12; 
	} f;
	unsigned int v;
} PHY_RG3X_PLL_T;
#define PHY_RG3X_PLLrv (*((regval)0xBB01F044))
#define PHY_RG3X_PLLdv (0x00000000)
#define RMOD_PHY_RG3X_PLL(...) rset(PHY_RG3X_PLL, PHY_RG3X_PLLrv, __VA_ARGS__)
#define RIZS_PHY_RG3X_PLL(...) rset(PHY_RG3X_PLL, 0, __VA_ARGS__)
#define RFLD_PHY_RG3X_PLL(fld) (*((const volatile PHY_RG3X_PLL_T *)0xBB01F044)).f.fld




#endif  //__GPHY_PLL_H__


