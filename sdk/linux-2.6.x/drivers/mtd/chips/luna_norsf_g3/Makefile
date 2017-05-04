EXTRA_CFLAGS = -I$(src) -I$(src)/nor_spi

obj-y += spi_probe.o spi_cmd.o

norsf_flist := nor_spif_core.o
norsf_flist += nor_spif_micron_rev.o
norsf_flist += nor_spif_mxic_3b_mode.o nor_spif_mxic_4b_cmd.o
norsf_flist := $(addprefix nor_spi/,$(norsf_flist))

obj-y += $(norsf_flist)
