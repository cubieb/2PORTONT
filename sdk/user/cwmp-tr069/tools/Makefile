#Uncomment to show dedebug messages
#LOG_LV = --loglevel=w1 #can be e, i, w[0-9], d[0-9]

#Script to generate report
REPORT = ./report.pl --include=cwmp $(LOG_LV)

#TR-181 issue 2 base document for DTinstance template
TR_181 = cwmp/tr-181-2-2-0.xml

COM_NAME = rtk-xdsl
DEV_NAME = device
DT_VER = 1-0-0
DT_NAME = $(COM_NAME)-$(DEV_NAME)
DT_SPEC = urn:$(COM_NAME):$(DEV_NAME)-$(DT_VER)


#Base profiles for template
base_PROFILES = Baseline:2 SupportedDataModel:1 Time:1 VendorLogFiles:1 \
			IPPing:1 CaptivePortal:1 DHCPv4Client:1 NAT:1 Routing:1 \
			IPInterface:2 EthernetInterface:1 EthernetLink:1 Bridge:1 PPPInterface:1 \
            Download:1 DownloadTCP:1 Upload:1 UploadTCP:1 UDPEcho:1 UDPEchoPlus:1 \
			DHCPv4Server:1 DHCPv4CondServing:1 DHCPv4Relay:1

#xDSL profiles for template
xdsl_PROFILES = ADSL:1 ADSL2:1 VDSL2:1 ATMLink:1 DSLDiagnostics:1 ADSL2Diagnostics:1 \
			VDSL2Diagnostics:1 ATMLoopback:1

#ADSL profile, a subset of xDSL
adsl_PROFILES = ADSL:1 ADSL2:1 ATMLink:1 DSLDiagnostics:1 ADSL2Diagnostics:1 ATMLoopback:1

#PTM profile
ptm_PROFILES = PTMLink:1

#WLAN profiles for template
wlan_PROFILES = WiFiRadio:1 WiFiSSID:1 WiFiAccessPoint:1

#TraceRoute profile
traceroute_PROFILES = TraceRoute:1

#Templates we want to generate
TEMPLATES = base xdsl adsl ptm wlan traceroute


all: $(foreach t,$(TEMPLATES),$(t)_only)
	@echo 'All templates are generated.'

%_only: cwmp
	@echo 'Generating $* template from $(TR_181)...'
	$(REPORT) --report=xml --outfile=$(DT_NAME)-$*-template.xml \
		 --dtspec=$(DT_SPEC) $(foreach p,$($*_PROFILES),--dtprofile=$(p)) $(TR_181)

cwmp:
	unzip cwmp.zip

cwmp_update:
	rm cwmp.zip
	wget http://broadband-forum.org/cwmp.zip

clean:
	rm -f *-template.xml
	rm -rf cwmp
