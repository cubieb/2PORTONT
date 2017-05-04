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

var cgi_wlCurrentChannel = "11"; // 当前信道
var cgi_wlBssid = "wlBssid"; // wlBssid值
var enbl = true; // 取wlEnbl值；true 表示启用无线, false 则不启用。
var ssid = "ChinaNet-one"; // SSID值
var txPower = "100"; // 发送功率
var hiddenSSID = true; // 取消广播的开关, false 表示启用广播，true 表示不启用广播
var wlanMode = 1; // WLAN模式
var ssid_2g = "<% checkWrite("2G_ssid"); %>";
var ssid_5g = "<% checkWrite("5G_ssid"); %>";
var defaultBand = <% checkWrite("band"); %>;
var Band2G5GSupport = <% checkWrite("Band2G5GSupport"); %>;
var regDomain, defaultChan;

<% init_wlan_page(); %>
<% checkWrite("wl_txRate"); %>
<% checkWrite("wl_chno"); %>
/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	with (document.wlanSetup) {
		/* Tsai: the initial value */
		wlEnbl.checked = enbl;
		wlSsid.value = ssid;
		wlTxPower.selectedIndex = txPower;
		wlHide.checked = hiddenSSID;

		/* Tsai: show or hide elements */
		wlSecCbClick(wlEnbl);
	}
}

/********************************************************************
**          on document update
********************************************************************/
function wlSecCbClick(cb) 
{
	var status = cb.checked;

	with (document.wlanSetup) {
		wlSsid.disabled = !status;
		wlHide.disabled = !status;

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
	var str, band, basicRate = 0, operRate = 0;

	with (document.wlanSetup) {
		if (wlEnbl.checked) {
			str = wlSsid.value;

			if (str.length == 0) {
				alert("SSID不能为空.");
				return;
			}

			if (str.length > 32) {
				alert('SSID "' + str + '" 不能大于32个字符。');
				return;
			}

			if (str.indexOf("ChinaNet-") != 0) {
				alert('SSID "' + str + '" 不是以ChinaNet-开头，请重新输入。');
				return;
			}

			if (isIncludeInvalidChar(str)) {
				alert("SSID 含有非法字符，请重新输入!");
				return;
			}	

			band = parseInt(document.wlanSetup.wlgMode.value, 10) + 1;

			/* band:
			   bit 0 == 802.11b, bit 1 == 802.11g,
			   bit 2 == 802.11a, bit 3 == 802.11n
			*/

			/* basicRate, operRate:
			   bit 0 == 1M
			   bit 1 == 2M
			   bit 2 == 5M
			   bit 3 == 11M
			   bit 4 == 6M
			   bit 5 == 9M
			   bit 6 == 12M
			   bit 7 == 18M
			   bit 8 == 24M
			   bit 9 == 36M
			   bit 10 == 48M
			   bit 11 == 54M
			*/ 

			/* 802.11b */
			if (band & 1) {
				operRate |= 0xf;
				basicRate |= 0xf;
			}

			/* 802.11g, 802.11a */
			if ((band & 2) || (band & 4)) {
				operRate |= 0xff0;
				if  (!(band & 1))
					basicRate |= 0xff0;
			}

			operRate |= basicRate;
			wlBasicrates.value = basicRate;
			wlOperrates.value = operRate;
		}
		submit();
	}
}

function on_adv() 
{
	var loc = 'net_wlan_adv.asp';
	var code = 'location.assign("' + loc + '")';

	eval(code);
}

function get_rate(curRate)
{
	var rate_mask = new Array(15,1,1,1,1,2,2,2,2,2,2,2,2,4,4,4,4,4,4,4,4,8,8,8,8,8,8,8,8);
	var rate_name = new Array("Auto","1M","2M","5.5M","11M","6M","9M","12M","18M","24M","36M","48M","54M", "MCS0", "MCS1",
		"MCS2", "MCS3", "MCS4", "MCS5", "MCS6", "MCS7", "MCS8", "MCS9", "MCS10", "MCS11", "MCS12", "MCS13", "MCS14", "MCS15");
	var rate, mask, i, idx = 0, band;

	band = parseInt(document.wlanSetup.wlgMode.value, 10) + 1;

	/* band:
	   bit 0 == 802.11b, bit 1 == 802.11g,
	   bit 2 == 802.11a, bit 3 == 802.11n
	*/

	mask = 0;
	if (auto)
		curRate = 0;
	if (band & 1)
		mask |= 1;
	if ((band & 2) || (band & 4))
		mask |= 2;

	/* Tsai: remove all options under wlRate */
	document.wlanSetup.wlRate.length = 0;

	for (i = 0; i < rate_mask.length; i++) {
		if (rate_mask[i] & mask) {
			rate = (i == 0) ? 0 : 1 << (i - 1);
			document.wlanSetup.wlRate.options[idx++] = new Option(rate_name[i], i, false, curRate == rate);
		}
	}
}

function updatePage()
{
	updateChannel(false);
}

function updateChannel(byHand)
{
	/* Tsai: change channel by hand ? */
	if (byHand) {
		wlan_channel = parseInt(document.wlanSetup.wlChannel.value, 10);
	} else {
		wlan_channel = defaultChan;
	}

	updateChannelBound();

	/* Tsai: update wlan_channel because wlChannel may be changed */
	wlan_channel = parseInt(document.wlanSetup.wlChannel.value, 10);
}

function updateChannelBound()
{
	var band = parseInt(document.wlanSetup.wlgMode.value, 10) + 1;

	/* band:
	   bit 0 == 802.11b, bit 1 == 802.11g,
	   bit 2 == 802.11a, bit 3 == 802.11n
	*/

	if (band & 4) {
		/* Tsai: 802.11a */
		showChannel5G();
	} else {
		/* Tsai: 802.11b and 802.11g */
		showChannel2G(band);
	}
}

function showChannel5G()
{
	var chan, idx = 0;

	/* Tsai: remove all options under wlChannel */
	document.wlanSetup.wlChannel.length = 0;

	document.wlanSetup.wlChannel.options[idx++] = new Option("Auto", 0, false, 0 == wlan_channel);

	if (regDomain == 6) {	// MKK
		for (chan = 34; chan <= 64; chan += 2) {
			if ((chan == 50) || (chan == 54) || (chan == 58) || (chan == 62)) {
				continue;
			}

			document.wlanSetup.wlChannel.options[idx++] = new Option(chan, chan, false, chan == wlan_channel);
		}
	} else {
		var start, end;

		start = 36;
		end = 64;

		for (chan = start; chan <= end; chan += 4) {
			document.wlanSetup.wlChannel.options[idx++] = new Option(chan, chan, false, chan == wlan_channel);
		}
	}
}

function showChannel2G(band)
{
	var start = 0, end = 0, idx = 0;

	/* band:
	   bit 0 == 802.11b, bit 1 == 802.11g,
	   bit 2 == 802.11a, bit 3 == 802.11n
	*/

	if (regDomain == 1 || regDomain == 2) {
		start = 1;
		end = 11;
	} else if (regDomain == 3) {
		start = 1;
		end = 13;
	} else if (regDomain == 4) {
		start = 10;
		end = 11;
	} else if (regDomain == 5) {
		start = 10;
		end = 13;
	} else if (regDomain == 6) {
		start = 1;
		end = 14;
	} else {		//wrong regDomain ?
		start = 1;
		end = 11;
	}

	/* Tsai: remove all options under wlChannel */
	document.wlanSetup.wlChannel.length = 0;

	document.wlanSetup.wlChannel.options[idx++] = new Option("Auto", 0, false, 0 == wlan_channel);

	for (chan = start; chan <= end; chan++) {
		document.wlanSetup.wlChannel.options[idx++] = new Option(chan, chan, false, chan == wlan_channel);
	}
}

function updateBand(band)
{
	var idx;

	for (idx = 0; idx < document.wlanSetup.wlgMode.length; idx++) {
		if (document.wlanSetup.wlgMode.options[idx].value == band) {
			document.wlanSetup.wlgMode.selectedIndex = idx;
			break;
		}
	}
}

function Set_SSIDbyBand()
{
	var c;

	with (document.wlanSetup) {
		c = wlgMode.options[wlgMode.selectedIndex].text.charAt(0);

		if (c == "5")
			wlSsid.value = ssid_5g;
		else if (c == "2")
			wlSsid.value = ssid_2g;
	}
}

</script>
</head>
   
<!-------------------------------------------------------------------------------------->
<!--主页代码-->

<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000"><!--onLoad='on_init()'-->
	<blockquote>
	<DIV align="left" style="padding-left:20px; padding-top:10px">
	<form action=/boaform/formWlanSetup method="post" name="wlanSetup">
	<b>无线设置 -- 基本</b><br>
	<br>
    本页配置无线LAN 口的基本特性。 包括启用或禁用无线LAN口、从工作站的AP扫描搜索中 隐藏SSID、 设置无线网络名(即SSID)。<br>
	<br>
    点击"保存/应用"，无线设置的基本配置生效。<br>
	<br>
	<table border="0" cellpadding="4" cellspacing="0">
		<tr>
			<td valign="middle" align="center" width="30" height="30">
				<input type='checkbox' name='wlEnbl' onClick='wlSecCbClick(this);' value="ON"></td>
			<td>启用无线</td>
		</tr>
	</table>
	<div id='wlSecInfo'>
	<table border="0" cellpadding="5" cellspacing="0">
		<tr> 
			<td width="26%">模式:</td>
			<td>
				<select name="wlgMode" size="1" onChange="updatePage(); Set_SSIDbyBand(); get_rate(txrate);">
					<% checkWrite("wlband"); %>
				</select>
				<SCRIPT>updateBand(defaultBand);</SCRIPT>
			</td>
		</tr>
		<tr>
			<td width="26%">SSID 索引:</td>
			<td><select name="wlSsidIdx">
				<option value="0">SSID1</option>
			</select></td>
		</tr>
		<tr>
			<td width="26%">SSID:</td>
			<td>
				<input type='text' name='wlSsid' maxlength="32" size="32" value="<% getInfo("ssid"); %>"></td>
		</tr>
		<tr>
			<td width="26%">BSSID:</td>
			<td>
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
			<td width="26%">信道:</td>
			<td><select name='wlChannel' onChange="updateChannel(true)">
			</select></td>
			<td width="26%">当前信道:
				<script language="javascript">
					document.writeln(cgi_wlCurrentChannel);
				</script>
			</td>
		</tr>
		<tr> 
			<td width="26%">速率:</td>
			<td colspan="2"><select name="wlRate">
				<SCRIPT>
					get_rate(txrate);
				</SCRIPT>
			</select></td> 
		</tr> 
		<tr> 
			<td width="26%">发送功率:</td>
			<td colspan="2"><select name="wlTxPower" size="1">
				<% checkWrite("txpower"); %> 
			</select></td> 
		</tr>
		<tr>
			<td width="26%">取消广播:</td>
			<td valign="middle" width="30" height="30">
				<input type='checkbox' name='wlHide' value="ON"></td>
		</tr>
		<tr> 
			<td width="26%">&nbsp;</td>
			<td colspan="2">&nbsp;</td>
		</tr>
	</table>          
	</div>                  
	<table width="295" border="0" cellpadding="4" cellspacing="0">
		<tr>
			<td><input type="hidden" value="/net_wlan_basic.asp" name="submit-url"></td>    
			<td width="162"><input type='button' onClick='on_submit()' value='保存/应用'></td>
			<td><input type='button' onClick='on_adv()' name="advanced" value='高级'></td>
			<td><input type="hidden" name="wlBasicrates" value=0></td>
			<td><input type="hidden" name="wlOperrates" value=0></td>
			<script>
				<% initPage("wlbasic"); %>
				on_init();
				updatePage();
			</script>
		</tr>
	</table>
	</form>
	</DIV>
</blockquote>
</body>
<%addHttpNoCache();%>
</html>
