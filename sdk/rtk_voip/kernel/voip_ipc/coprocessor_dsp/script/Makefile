ifeq ($(CONFIG_RTK_VOIP_COPROCESS_DSP),y)
always += coprocess_dsp_include
$(obj)/coprocess_dsp_include:
	@if [ ! -d $(obj)/../include ]; then	\
		echo -e "\tInvalid include link";	\
		rm -f $(obj)/../include;	\
		if [ -d ../../../aipc/include ]; then	\
			echo -e "\tCreate eCos type";	\
			ln -s ../../../../../../aipc/include/ $(obj)/../include;	\
		elif [ -d drivers/char/aipc/include/ ]; then	\
			ln -s ../../../drivers/char/aipc/include/ $(obj)/../include;	\
			echo -e "\t Create Linux type";	\
		fi	\
	fi
endif

obj-y += dummy.o 

