
$(TARGET_IMG):: $(target_img_dep)
	@echo "$(TARGET_IMG) is adding the 32-bit checksum and padding to $(OTTO_LPLR_SIZE) bytes"
	@util/bin/lplr_checksum -s $(OTTO_LPLR_SIZE) -c $(OTTO_LPLR_CHECKSUM_OFFSET) $(TARGET_IMG)

