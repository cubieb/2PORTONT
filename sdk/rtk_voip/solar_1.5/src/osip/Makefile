include ../build/Makeparm.cfg
include ../build/Makeparm.def

OBJS		=	fsm_misc.o ict.o ict_fsm.o ist.o ist_fsm.o nict.o nict_fsm.o nist.o	\
				nist_fsm.o osip.o osip_dialog.o osip_event.o osip_negotiation.o		\
				osip_time.o osip_transaction.o port_condv.o port_fifo.o port_sema.o	\
				port_thread.o

OBJS		+=	osip_accept.o osip_accept_encoding.o osip_accept_language.o			\
				osip_alert_info.o osip_allow.o osip_authentication_info.o			\
				osip_authorization.o osip_body.o osip_call_id.o osip_call_info.o	\
				osip_contact.o osip_content_disposition.o osip_content_encoding.o	\
				osip_content_length.o osip_content_type.o osip_cseq.o				\
				osip_error_info.o osip_from.o osip_header.o osip_list.o				\
				osip_md5c.o osip_message.o osip_message_parse.o						\
				osip_message_to_str.o osip_mime_version.o osip_parser_cfg.o			\
				osip_port.o osip_proxy_authenticate.o								\
				osip_proxy_authentication_info.o osip_proxy_authorization.o			\
				osip_record_route.o osip_rfc3264.o osip_route.o osip_to.o			\
				osip_uri.o osip_via.o osip_www_authenticate.o sdp_accessor.o		\
				sdp_message.o
OBJS		+= osip_replaces.o


LIB_OBJ		= $(foreach obj,$(OBJS),$(OBJ_DIR)/$(obj))
DEPENDENCY	= $(LIB_OBJ:.o=.d)

lib: $(OBJ_DIR) $(SIP_LIB)

clean:
	rm -f $(SIP_LIB) $(LIB_OBJ) $(DEPENDENCY)

$(OBJ_DIR):
	mkdir -p $@

$(SIP_LIB): $(LIB_OBJ)
	$(AR) rcs $(SIP_LIB) $^

# pull in dependency info for existing .o files
-include $(DEPENDENCY)
