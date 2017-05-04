EXTRA_CFLAGS = -I$(src) -I$(src)/spi_nand -I$(src)/ecc

spin_flist := spi_nand_common.o
spin_flist += spi_nand_ctrl.o
spin_flist += spi_nand_winbond.o
spin_flist += spi_nand_gd.o
spin_flist += spi_nand_zentel.o
spin_flist += spi_nand_toshiba.o
spin_flist += spi_nand_mxic.o
spin_flist := $(addprefix spi_nand/,$(spin_flist))

ecc_flist := ecc_ctrl.o
ecc_flist += bch6_software_encode.o
ecc_flist := $(addprefix ecc/,$(ecc_flist))

obj-y += $(spin_flist)
obj-y += $(ecc_flist)
obj-y += luna_spi_nand.o spi_nand_base_luna.o

