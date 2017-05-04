<!-- add by liuxiao 2008-01-21 -->
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
<head>
<title>中国电信</title>
<meta http-equiv=pragma content=no-cache>
<meta http-equiv=cache-control content="no-cache, must-revalidate">
<meta http-equiv=content-type content="text/html; charset=gbk">
<meta http-equiv="refresh" content="5">
<meta http-equiv=content-script-type content=text/javascript>
<style type=text/css>
@import url(/style/default.css);
</style>
<script language="javascript" src="common.js"></script>
<SCRIPT language="javascript" type="text/javascript">

var rcs = new Array();
<% wlStatsList(); %>

/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	if (lsteth.rows) {
		while (lsteth.rows.length > 2)
			lsteth.deleteRow(2);
	}

	for (var i = 0; i < rcs.length; i++) {
		var row = lsteth.insertRow(i + 2);

		row.nowrap = true;
		row.style.verticalAlign = "top";
		row.style.textAlign = "center";

		var cell = row.insertCell(0);
		cell.innerHTML = rcs[i].ifname;
		cell = row.insertCell(1);
		cell.innerHTML = rcs[i].rx_packets;
		cell = row.insertCell(2);
		cell.innerHTML = rcs[i].rx_bytes;
		cell = row.insertCell(3);
		cell.innerHTML = rcs[i].rx_errors;
		cell = row.insertCell(4);
		cell.innerHTML = rcs[i].rx_dropped;
		cell = row.insertCell(5);
		cell.innerHTML = rcs[i].tx_packets;
		cell = row.insertCell(6);
		cell.innerHTML = rcs[i].tx_bytes;
		cell = row.insertCell(7);
		cell.innerHTML = rcs[i].tx_errors;
		cell = row.insertCell(8);
		cell.innerHTML = rcs[i].tx_dropped;
	}
}

function get_rate(curRate, band, auto, rf_num)
{
	var rate, mask, i;
	var rate_mask =
	    new Array(15, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 4, 4, 4, 4, 4, 4,
		      4, 4, 8, 8, 8, 8, 8, 8, 8, 8);
	var rate_name =
	    new Array("Auto", "1M", "2M", "5.5M", "11M", "6M", "9M", "12M",
		      "18M", "24M", "36M", "48M", "54M", "MCS0", "MCS1",
		      "MCS2", "MCS3", "MCS4", "MCS5", "MCS6", "MCS7", "MCS8",
		      "MCS9", "MCS10", "MCS11", "MCS12", "MCS13", "MCS14",
		      "MCS15");
	var vht_rate_name =
	    ["NSS1-MCS0", "NSS1-MCS1", "NSS1-MCS2", "NSS1-MCS3", "NSS1-MCS4",
	     "NSS1-MCS5", "NSS1-MCS6", "NSS1-MCS7", "NSS1-MCS8", "NSS1-MCS9",
	     "NSS2-MCS0", "NSS2-MCS1", "NSS2-MCS2", "NSS2-MCS3", "NSS2-MCS4",
	     "NSS2-MCS5", "NSS2-MCS6", "NSS2-MCS7", "NSS2-MCS8", "NSS2-MCS9"];

	mask = 0;
	if (auto)
		curRate = 0;
	/* 802.11B */
	if (band & 1)
		mask |= 1;
	/* 802.11G or 802.11A */
	if ((band & 2) || (band & 4))
		mask |= 2;
	/* 802.11N */
	if (band & 8) {
		if (rf_num == 2)
			mask |= 12;
		else
			mask |= 4;
	}
	/* 802.11ac */
	if (band & 64) {
		mask |= 16;
	}
	for (i = 0; i < rate_mask.length; i++) {
		if (rate_mask[i] & mask) {
			rate = (i == 0) ? 0 : 1 << (i - 1);
			if (curRate == rate) {
				document.write(rate_name[i]);
			}
		}
	}

	for (i = 0; i < vht_rate_name.length; i++) {
		rate = (((1 << 31) >>> 0) + i);
		if (curRate == rate) {
			document.write(vht_rate_name[i]);
		}
	}
}

var wlan_num = <% checkWrite("wlan_num"); %>;
var auto=new Array();
var txrate=new Array();
var band=new Array();
var ssid_drv=new Array();
var channel_drv=new Array();
var bssid_drv=new Array();
var wlanDisabled=new Array();
var wlanSsidAttr=new Array();
var encryptState=new Array();
var wlDefChannel=new Array();
var rf_used=new Array();

var mssid_num;
var mssid_disable=[[],[]];
var mssid_bssid_drv=[[],[]];
var mssid_band=[[],[]];
var mssid_ssid_drv=[[],[]];
var mssid_wlanSsidAttr=[[],[]];
var mssid_encryptState=[[],[]];
var Band2G5GSupport=new Array();
   
</script>
</head>
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
	<blockquote>
		<div align="left" style="padding-left:20px;"><br>
			<div align="left"><b>WLAN接口信息：</b></div>
			<div align="left" style="width:768px;"><br>
			<table class="flat" border="1" cellpadding="0" cellspacing="1">
<script>
<% wlStatus_parm(); %>
document.write('\
<tr>\
<td width="20%" class="hdb">无线网络连接状态</td>\
<td>');
if(wlanDisabled[0])
	document.write('Disabled');
else
	document.write('Enabled');
document.write(	'</td></tr>');

for(i=0; i < wlan_num; i++){
	document.write('<tr><td class="hdb">SSID-'+(i*(mssid_num+1)+1)+'名称</td><td>'+ssid_drv[i]+'</td></tr>');
	document.write('<tr><td class="hdb">信道</td><td>'+wlDefChannel[i]+'</td></tr>');
	document.write('<tr><td class="hdb">速率</td><td>');
	get_rate(txrate[i], band[i], auto[i], rf_used[i]);
	document.write('<tr><td class="hdb">SSID隐藏属性</td><td>'+wlanSsidAttr[i]+'</td></tr>');
	
	/* band */
	document.write('<tr>\
	    	<td class="hdb">模式</td>\
	    	<td>');
	if (band[i] == 1)
   		document.write( "2.4 GHz (B)");
	if (band[i] == 2)
		document.write( "2.4 GHz (G)");
	if (band[i] == 3)
		document.write( "2.4 GHz (B+G)");
	if (band[i] == 4)
		document.write( "5 GHz (A)");
	if (band[i] == 8) {
		if(Band2G5GSupport[i] == 1)	//PHYBAND_2G
			document.write( "2.4 GHz (N)");
		else if(Band2G5GSupport[i] == 2)	//PHYBAND_5G
			document.write( "5 GHz (N)");
	}
	if (band[i] == 10)
   		document.write( "2.4 GHz (G+N)");
   	if (band[i] == 11)
   		document.write( "2.4 GHz (B+G+N)");
   	if (band[i] == 12)
   		document.write( "5 GHz (A+N)");
   	if (band[i] == 76)
   		document.write( "5 GHz (A+N+AC)");

	document.write('</td></tr>');
	
	document.write('<tr><td class="hdb">SSID-'+(i*(mssid_num+1)+1)+'加密状态</td><td>');
	document.write(encryptState[i]+'</td></tr>');
  	document.write('<tr><td class="hdb">BSSID</td><td>'+bssid_drv[i]+'</td></tr>');

	if (!wlanDisabled[i]) {
		for (idx=0; idx<mssid_num; idx++) {
			if (!mssid_disable[i][idx]) {

			//	document.write('<tr><td width=100% colspan="2">SSID-'+(idx+2+(mssid_num+1)*i)+'</td></tr>');
				
				document.write('<tr><td class="hdb">SSID-'+(idx+2+(mssid_num+1)*i)+'名称</td><td>'+mssid_ssid_drv[i][idx]+'</td></tr>');
				document.write('<tr>\
	    			<td class="hdb">模式</td>\
			    	<td>');
				if (mssid_band[i][idx] == 1)
					document.write( "2.4 GHz (B)");
				if (mssid_band[i][idx] == 2)
					document.write( "2.4 GHz (G)");
				if (mssid_band[i][idx] == 3)
					document.write( "2.4 GHz (B+G)");
				if (mssid_band[i][idx] == 4)
					document.write( "5 GHz (A)");
				if (mssid_band[i][idx] == 8) {
					if(Band2G5GSupport[i] == 1)	//PHYBAND_2G
						document.write( "2.4 GHz (N)");
					else if(Band2G5GSupport[i] == 2)	//PHYBAND_5G
						document.write( "5 GHz (N)");
				}
				if (mssid_band[i][idx] == 10)
					document.write( "2.4 GHz (G+N)");
				if (mssid_band[i][idx] == 11)
					document.write( "2.4 GHz (B+G+N)");
				if (mssid_band[i][idx] == 12)
					document.write( "5 GHz (A+N)");
				if (mssid_band[i][idx] == 76)
					document.write( "5 GHz (A+N+AC)");

				document.write('</td></tr>');
				document.write('<tr><td class="hdb">SSID-'+(idx+2+(mssid_num+1)*i)+'加密状态</td>\
					<td>'+mssid_encryptState[i][idx]+'</td></tr>');
				document.write('<tr><td class="hdb">BSSID</td><td>'+mssid_bssid_drv[i][idx]+'</td></tr>');
			}
		}
	}
}
</script>
			</table>
			</div>
		</div>
		<br><br>
		<div align="left" style="padding-left:20px;">
			<div align="left"><b>接入设备情况：</b></div>
			<table class="flat" border="1" cellpadding="0" cellspacing="1">
			<tr class="hdb" align="center" nowrap><font size=2>
			<td width="25%"><b>MAC地址</b></td>
			<td width="15%"><b>发送封包数</b></td>
			<td width="15%"><b>接收封包数</b></td>
			<td width="15%"><b>发送速率 (Mbps)</b></td>
			<td width="15%"><b>省电</b></td>
			<td width="15%"><b>到期时间 (sec)</b></td>
			</font></tr>
			<% wirelessClientList(); %>
			</table>
		</div>
		<br><br>
		<div align="left" style="padding-left:20px;">
			<div align="left"><b>收发包情况：</b></div>
			<table id="lsteth" class="flat" border="1" cellpadding="1" cellspacing="1">
			   <tr class="hdb" align="center" nowrap>
					<td>接口</td>
					<td colspan="4">接收</td>
					<td colspan="4">发送</td>
				</tr>
			   <tr class="hdb" align="center" nowrap>
					<td>&nbsp;</td>
					<td>Packets</td>
					<td>Bytes</td>
					<td>Errors</td>
					<td>Dropped</td>
					<td>Packets</td>
					<td>Bytes</td>
					<td>Errors</td>
					<td>Dropped</td>
				</tr>
			</table>
		</div>
	</blockquote>
</body>
<!-- add end by liuxiao 2008-01-21 -->
<%addHttpNoCache();%>
</html>
