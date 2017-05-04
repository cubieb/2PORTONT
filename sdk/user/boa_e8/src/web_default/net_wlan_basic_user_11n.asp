<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>中国电信-WLAN配置</TITLE>
<META http-equiv=pragma content=no-cache>
<META http-equiv=cache-control content="no-cache, must-revalidate">
<META http-equiv=content-type content="text/html; charset=gbk">
<META http-equiv=content-script-type content=text/javascript>
<!--系统公共css-->
<STYLE type=text/css>
@import url(/style/default.css);
</STYLE>
<!--系统公共脚本-->
<SCRIPT language="javascript" src="common.js"></SCRIPT>
<script type="text/javascript" src="share.js"></script>
<SCRIPT language="javascript" type="text/javascript">

var cgi_wlBssid = "wlBssid"; // wlBssid值
var enbl = true; // 取wlEnbl值；true 表示启用无线, false 则不启用。
var ssid = "ChinaNet-one"; // SSID值
//var hiddenSSID = true; // 取消广播的开关, false 表示启用广播，true 表示不启用广播
var wlanMode = 1; // WLAN模式
var ssid_2g = "<% checkWrite("2G_ssid"); %>";
var ssid_5g = "<% checkWrite("5G_ssid"); %>";
var defaultBand = <% checkWrite("band"); %>;
var Band2G5GSupport = <% checkWrite("Band2G5GSupport"); %>;
var regDomain, defaultChan;
var _wlanEnabled = new Array();
//var _band = new Array();
var _ssid = new Array();
var _bssid = new Array();
//var _chan = new Array();
//var _chanwid = new Array();
//var _ctlband = new Array();
//var _txRate = new Array();
var _hiddenSSID = new Array();
//var _wlCurrentChannel = new Array();
//var _auto = new Array();
var wlan_num = <% checkWrite("wlan_num"); %>;

<% init_wlan_page(); %>
/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	with (document.wlanSetup) {
		/* Tsai: the initial value */
		wlanEnabled.checked = _wlanEnabled[0];
		ssid.value = "<% getInfo("ssid"); %>";
		if(ssid.value.substr(0,9) == "ChinaNet-")
			ssid.value = ssid.value.substr(9);
		hiddenSSID.checked = _hiddenSSID[0];
		elements.wlan_idx.value = 0;
		/* Tsai: show or hide elements */
		wlSecCbClick(wlanEnabled);

		if(ssid_2g.substr(0,9) == "ChinaNet-")
			ssid_2g = ssid_2g.substr(9);
		if(ssid_5g.substr(0,9) == "ChinaNet-")
			ssid_5g = ssid_5g.substr(9);
	}
}

/********************************************************************
**          on document update
********************************************************************/
function wlSecCbClick(cb) 
{
	var status = cb.checked;

	with (document.wlanSetup) {
		ssid.disabled = !status;
		hiddenSSID.disabled = !status;

		wlSecInfo.style.display = status ? "block" : "none";
		adminWlinfo.style.display = status ? "block" : "none";
		advanced.style.display = status ? "block" : "none";
  	}
}

/********************************************************************
**          on document submit
********************************************************************/
function on_submit() 
{
	var str;

	with (document.wlanSetup) {
		if (wlanEnabled.checked) {
			str = ssid.value;

			//if (str.length == 0) {
			//	alert("SSID不能为空.");
			//	return;
			//}

			if (str.length > 23) {
				alert('SSID "ChinaNet-' + str + '" 不能大于32个字符。');
				return;
			}

			if (isIncludeInvalidChar(str)) {
				alert("SSID 含有非法字符，请重新输入!");
				return;
			}	
		}
		submit();
	}
}

function on_adv() 
{
	var wlan_idx = document.wlanSetup.elements["wlan_idx"].value;
	var loc;
	if(wlan_num>1)
		loc = 'net_wlan_adv.asp?wlan_idx='+wlan_idx;
	else
		loc = 'net_wlan_adv.asp';
	var code = 'location.assign("' + loc + '")';

	eval(code);
}


function BandSelected(index)
{
	document.wlanSetup.elements["wlan_idx"].value = index;
//	document.wlanSetup.wlanEnabled.checked = _wlanEnabled[index];
//	document.wlanSetup.band.value = _band[index];
	document.wlanSetup.ssid.value = _ssid[index];
	if(document.wlanSetup.ssid.value.substr(0,9) == "ChinaNet-")
		document.wlanSetup.ssid.value = document.wlanSetup.ssid.value.substr(9);
	document.getElementById('wlBssid').innerHTML = _bssid[index];
	//alert(_bssid[index]);
//	document.getElementById('cur_wlChannel').innerHTML = '当前信道: '+ _wlCurrentChannel[index];
	document.wlanSetup.hiddenSSID.checked = _hiddenSSID[index];
	document.getElementsByName('select_2g5g')[index].checked = true;
//	document.wlanSetup.chan.value = _chan[index];
//	document.wlanSetup.chanwid.selectedIndex = _chanwid[index];
//	document.wlanSetup.ctlband.selectedIndex = _ctlband[index];
//	txrate = _txRate[index];
//	auto = _auto[index];
//	updateBand(_band[index]);
//	get_rate(txrate);
//	updatePage();
//	Set_SSIDbyBand();
	//wlan_idx = index;
}
</script>
</head>
   
<!-------------------------------------------------------------------------------------->
<!--主页代码-->

<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onload="BandSelected(0)">
	<blockquote>
	<DIV align="left" style="padding-left:20px; padding-top:10px">
	<form action=/boaform/admin/formWlanSetup method="post" name="wlanSetup">
	<b>无线设置 -- 基本</b><br>
	<br>
    本页配置无线LAN 口的基本特性。 包括启用或禁用无线LAN口、从工作站的AP扫描搜索中 隐藏SSID、 设置无线网络名(即SSID)。<br>
	<br>
    点击"保存/应用"，无线设置的基本配置生效。<br>
	<br>
	<table border="0" cellpadding="4" cellspacing="0">
		<tr>
			<td valign="middle" align="center" width="30" height="30">
				<input type='checkbox' name='wlanEnabled' onClick='wlSecCbClick(this);' value="ON"></td>
			<td>启用无线</td>
		</tr>
	</table>
	<div id='wlSecInfo'>
	<table border="0" cellpadding="5" cellspacing="0">
		<tr>
		<td width=100% colspan="2">
			<% checkWrite("wlbandchoose"); %>
		</td>
		</tr>
		<tr>
			<td width="26%">SSID:</td>
			<td>
				ChinaNet-<input type='text' name='ssid' maxlength="32" size="32" value=""></td>
		</tr>
		<tr>
			<td width="26%">BSSID:</td>
			<td id='wlBssid'>
				<script language="javascript">
					document.writeln(cgi_wlBssid);
				</script>
			</td>
		</tr>
	</table>
	</div>
	<div id="adminWlinfo"  style="display:none">
	<table width="337">
		<tr>
			<td width="26%">取消广播:</td>
			<td valign="middle" width="30" height="30">
				<input type='checkbox' name='hiddenSSID' value="ON"></td>
		</tr>
		<tr> 
			<td width="26%">&nbsp;</td>
			<td colspan="2">&nbsp;</td>
		</tr>
	</table>          
	</div>                  
	<table width="295" border="0" cellpadding="4" cellspacing="0">
		<tr>
			<td><input type="hidden" value="/net_wlan_basic_user_11n.asp" name="submit-url"></td>    
			<td width="162"><input type='button' onClick='on_submit()' value='保存/应用'></td>
			<td><input type='button' onClick='on_adv()' name="advanced" value='高级'></td>
			<td><input type="hidden" name="wlan_idx" value=0></td>
			
		</tr>
		<script>
				<% initPage("wle8basic"); %>
				on_init();
				//BandSelected(0);
		</script>
	</table>
	</form>
	</DIV>
</blockquote>
</body>
<%addHttpNoCache();%>
</html>
