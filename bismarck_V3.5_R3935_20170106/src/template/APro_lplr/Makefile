$(TARGET_IMG):: $(target_img_dep)
	@echo "$(TARGET_IMG) is adding the 32-bit checksum and padding to $(OTTO_LPLR_SIZE) bytes"
	@util/bin/lplr_checksum -s $(OTTO_LPLR_SIZE) -c $(OTTO_LPLR_CHECKSUM_OFFSET) $(TARGET_IMG)
	@dd bs=16K conv=sync if=$@ 2>/dev/null | hexdump -v -e '16/1 "%02X" " " 16/1 "%02X" "\n"' > $@.16k_rom.256_hex.lst
	@cut -d ' ' -f1 $@.16k_rom.256_hex.lst > $@.L_hex.lst
	@cut -d ' ' -f2 $@.16k_rom.256_hex.lst > $@.R_hex.lst
