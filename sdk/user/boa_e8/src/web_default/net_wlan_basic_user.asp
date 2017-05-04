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
var regDomain, defaultChan;

<% init_wlan_page(); %>
/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	with (document.wlanSetup) {
		/* Tsai: the initial value */
		wlEnbl.checked = enbl;
		wlSsid.value = ssid;
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
	var str;

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
			<td><input type="hidden" value="/net_wlan_basic_user.asp" name="submit-url"></td>    
			<td width="162"><input type='button' onClick='on_submit()' value='保存/应用'></td>
			<td><input type='button' onClick='on_adv()' name="advanced" value='高级'></td>
			<script>
				on_init();
			</script>
		</tr>
	</table>
	</form>
	</DIV>
</blockquote>
</body>
<%addHttpNoCache();%>
</html>
