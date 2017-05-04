/* start_fon.c */

#include <rtk/utility.h>

#define FON_TEST

void createChilliCronAdmin(char *name)
{
	FILE *fp;

	fp = fopen(name, "w");
	if(fp) {
		fputs("40 4 * * * sh /etc/chilli_radconfig.sh\n",fp);
		//fputs("*/5 * * * * sh /etc/chilli_radconfig.sh\n",fp);
		//fputs("*/5 * * * * echo chilli_radconfig\n", fp);
		fclose(fp);
	}
#ifndef FON_TEST
	va_cmd("/bin/crond",0,0);
#endif
	return;
}

void createChilliconf(char *name)
{
	FILE *fp;

	fp = fopen(name, "w");
	if(fp) {
		fputs("radiusserver1 radius01.fon.com\n",fp);
		fputs("radiusserver2 radius02.fon.com\n",fp);
		fputs("radiussecret garrafon\n", fp);
		fputs("uamserver https://www.fon.com/login/gateway\n", fp);
		fputs("uamsecret garrafon\n", fp);
		fputs("uamallowed www.fon.com,www.paypal.com,www.paypalobjects.com,www.skype.com\n", fp);
		fclose(fp);
	}
	return;
}

void createFonWhitelist(char *name)
{		
	FILE *fp;

	fp = fopen(name, "w");
	if(fp) {
		fputs("*.google.com\n\
*.flickr.com\n\
*.fon.com\n\
secure.nuguya.com\n\
ssl.google-analytics.com\n\
www.skype.com\n\
inilite.inicis.com\n\
fon-en.custhelp.com\n\
*.statcounter.com\n\
www.excite.co.jp\n\
image.excite.co.jp\n\
adimp.excite.co.jp\n\
!wifi.fon.com\n\
*.yimg.com\n\
a4.g.akamai.net\n\
*.youtube.com\n\
*.paypal.com\n\
*.paypalobjects.com\n\
*.youthwant.com.tw\n\
*.youthwant.com\n\
*.iyw.tw\n\
*.clickandbuy.com\n\
*.livedoor.com\n\
*.datahotel.ne.jp\n\
*.pcpf.jp\n\
*.nowa.jp\n\
petamap.jp\n\
www.fon.ne.jp\n\
www.simyo.es\n\
*.twitxr.com\n\
www.pangya.jp\n\
tenjin.kyushu-wifi.net\n\
*.iketeru.org\n\
*.lokalisten.de\n\
www.wireless-campus.it\n\
www.uniurb.it\n\
*.btfon.com\n\
*.nintendowifi.net\n\
*.wii.com\n\
*.playstation.org\n\
*.playstation.com\n\
*.scei.co.jp\n\
freewifi.com.hk\n", fp);
	fclose(fp);
	}
	return;
}
void createFonKeywordFile(char *fon_keyword, char *fon_mac)
{
	unsigned char keyword[32];
	unsigned char devAddr[6];
	char keyword_string[65]; 
	char macAddr[13];
	FILE *fp;
	mib_get(MIB_HW_FON_KEYWORD, (void *)keyword);
	sprintf(keyword_string, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\
%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", 
		keyword[0], keyword[1], keyword[2], keyword[3], keyword[4], keyword[5], keyword[6], keyword[7], 
		keyword[8], keyword[9], keyword[10], keyword[11], keyword[12], keyword[13], keyword[14], keyword[15], 
		keyword[16], keyword[17], keyword[18], keyword[19], keyword[20], keyword[21], keyword[22], keyword[23], 
		keyword[24], keyword[25], keyword[26], keyword[27], keyword[28], keyword[29], keyword[30], keyword[31]);
	mib_get(MIB_ELAN_MAC_ADDR, (void *)devAddr);
	sprintf(macAddr, "%02x-%02x-%02x-%02x-%02x-%02x",
		devAddr[0], devAddr[1], devAddr[2],
#ifdef CONFIG_USER_IPV6READYLOGO_ROUTER
		//Set vc and nas mac with br0's mac plus 1
		devAddr[3], devAddr[4], devAddr[5]+1);
#else
		devAddr[3], devAddr[4], devAddr[5]);
#endif
	fp = fopen(fon_keyword, "w");
	if(fp) {
		fputs(keyword_string, fp);
		fclose(fp);
	}
	fp = fopen(fon_mac, "w");
	if(fp) {
		fputs(macAddr, fp);
		fclose(fp);
	}
}

void readFonKeyMac(char *fon_keyword, char *fon_mac)
{
	unsigned char keyword[32];
	unsigned char devAddr[6];
	FILE *fp;
	mib_get(MIB_HW_FON_KEYWORD, (void *)keyword);
	sprintf(fon_keyword, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\
%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", 
		keyword[0], keyword[1], keyword[2], keyword[3], keyword[4], keyword[5], keyword[6], keyword[7], 
		keyword[8], keyword[9], keyword[10], keyword[11], keyword[12], keyword[13], keyword[14], keyword[15], 
		keyword[16], keyword[17], keyword[18], keyword[19], keyword[20], keyword[21], keyword[22], keyword[23], 
		keyword[24], keyword[25], keyword[26], keyword[27], keyword[28], keyword[29], keyword[30], keyword[31]);
	mib_get(MIB_ELAN_MAC_ADDR, (void *)devAddr);
	sprintf(fon_mac, "%02x-%02x-%02x-%02x-%02x-%02x",
		devAddr[0], devAddr[1], devAddr[2],
#ifdef CONFIG_USER_IPV6READYLOGO_ROUTER
		//Set vc and nas mac with br0's mac plus 1
		devAddr[3], devAddr[4], devAddr[5]+1);
#else
		devAddr[3], devAddr[4], devAddr[5]);
#endif

}

void startFonsmcd()
{
	char fon_keyword[65]; 
	char fon_mac[18];
	readFonKeyMac(fon_keyword, fon_mac);
	printf("fonsmcd start.\n");
#ifdef FON_TEST
	va_cmd_no_echo("/bin/fonsmcd", 4, 0, "-m", "00-18-84-a0-a2-59", "-w", fon_keyword);
#else
	va_cmd_no_echo("/bin/fonsmcd", 4, 0, "-m", fon_mac, "-w", fon_keyword);
#endif
}
