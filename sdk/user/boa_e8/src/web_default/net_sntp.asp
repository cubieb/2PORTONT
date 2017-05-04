<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<html>
<head>
<title>中国电信—时间管理服务器</title>
<meta http-equiv=pragma content=no-cache>
<meta http-equiv=cache-control content="no-cache, must-revalidate">
<meta http-equiv=content-type content="text/html; charset=gbk">
<meta http-equiv=content-script-type content=text/javascript>
<!--系统公共css-->
<style type=text/css>
@import url(/style/default.css);
</style>
<!--系统公共脚本-->
<script language="javascript" src="common.js"></script>
<script language="javascript" type="text/javascript">

var ntpServers = new Array();

ntpServers.push("clock.fmt.he.net");
ntpServers.push("clock.nyc.he.net");
ntpServers.push("clock.sjc.he.net");
ntpServers.push("clock.via.net");
ntpServers.push("ntp1.tummy.com");
ntpServers.push("time.cachenetworks.com");
ntpServers.push("time.nist.gov");

function ntpChange(optionlist, textbox)
{
	textbox.disabled = (optionlist.value == "Other") ? false : true;
}

function ntpEnblChange()
{
	if (document.forms[0].ntpEnabled.checked)
		status = 'visible';
	else
		status = 'hidden';

	if (document.getElementById)
		document.getElementById('ntpConfig').style.visibility = status;
	else if (!document.layers)
		document.all.ntpConfig.style.visibility = status;
}

function writeNtpList(select, other, server, needed)
{
	var flag = 0;

	if (!needed)
		select.add(new Option("None"));

	for (var i = 0; i < ntpServers.length; i++) {
		if (server.value == ntpServers[i]) {
			select.add(new Option(ntpServers[i], ntpServers[i], true, true));
			flag = 1;
		} else {
			select.add(new Option(ntpServers[i]));
		}
	}

	if (flag || !needed && server.value.length == 0) {
		select.add(new Option("Other"));
		other.disabled = true;
		other.value = "";
	} else {
		select.add(new Option("Other", "Other", true, true));
		other.disabled = false;
		other.value = server.value;
	}
}

function if_typeChange()
{
	var if_type = document.forms[0].if_type.value;
	var if_wan = document.forms[0].if_wan;
	var keyword, i;

	switch (if_type) {
		case "0":
			keyword = "INTERNET";
			break;
		case "1":
			keyword = "VOICE";
			break;
		case "2":
			keyword = "TR069";
			break;
		case "3":
		default:
			keyword = "Other";
			break;
	}

	for (i = 0; i < if_wan.options.length; i++) {
		if (if_wan.options[i].text.search(keyword) == -1) {
			//if_wan.options[i].style.display = "none";		//this is not workable with IE
			if_wan.options[i].disabled = true;
		} else {
			//if_wan.options[i].style.display = "block";
			if_wan.options[i].disabled = false;
		}
	}

	/* deselect when the previous selected interface does not match the new selected if_type */
	if (if_wan.selectedIndex == -1 || (if_wan.selectedIndex >= 0 && if_wan.options[if_wan.selectedIndex].text.search(keyword) == -1))
	{
		if_wan.selectedIndex = -1;

		/* Find first available interface */
		for (i = 0; i < if_wan.options.length; i++)
		{
			if (if_wan.options[i].text.search(keyword) > 0)
				if_wan.selectedIndex = i;
		}
	}
}

/********************************************************************
**          on document submit
********************************************************************/
function on_submit()
{
	with (document.forms[0]) {
		if (ntpEnabled.checked) {
			if (ntpServerHost1.value == "Other") {
				if (ntpServerOther1.value.length == 0) {	// == Other
					alert
					    ('第一时间服务器设置为其他，但其他栏为空。');
					return;
				} else {
					server1.value = ntpServerOther1.value;
				}
			} else {
				server1.value = ntpServerHost1.value;
			}

			if (ntpServerHost2.value == "Other") {
				if (ntpServerOther2.value.length == 0) {	// == Other
					alert
					    ('第二时间服务器设置为其他，但其他栏为空。');
					return;
				} else {
					server2.value = ntpServerOther2.value;
				}
			} else {
				if (ntpServerHost2.selectedIndex > 0)
					server2.value = ntpServerHost2.value;
				else
					server2.value = "";
			}

			if (interval.value == 0) {
				alert("同步间隔设置不可为 0。");
				return;
			}
		}
	}
}
</script>
</head>

<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000">
	<blockquote>
	<div align="left" style="padding-left:20px; padding-top:10px">
	  <form action=/boaform/admin/formTimezone method=POST name="timeZone">
		<b>时间设定</b><br><br>
		本页允许您配置路由器的时间.<br><br>
		<table>
			<tr>
			  <td width='150'>当前日期和时间:</td>
			  <td colspan=2>
			  	<% getInfo("date"); %>
			  </td>
			</tr>
			<tr>
			  <td width='150'>时区:</td>
			  <td colspan=2>
			  <select name='tmzone' size="1">
				  <% timeZoneList(); %>
			  </select>
			  </td>
			</tr>
		</table>
		<br>
		<input type='checkbox' name='dstEnabled'>
		启用日光节约时间.<br><br>
		<input type='checkbox' name='ntpEnabled' onClick='ntpEnblChange()'>
		与因特网时间服务器自动同步.<br><br>
		<div id='ntpConfig'>
		  <table border="0" cellpadding="0" cellspacing="0">
			<tr>
			  <td width='150'>NTP第一时间服务器:</td>
			  <td>
			  <select name='ntpServerHost1' size="1" onChange='ntpChange(ntpServerHost1, ntpServerOther1)'>
			  </select>
			  </td>
			  <td><input type='text' name='ntpServerOther1' disabled></td>
			</tr>
			<tr>
			  <td>NTP第二时间服务器:</td>
			  <td>
			  <select name='ntpServerHost2' size="1" onChange='ntpChange(ntpServerHost2, ntpServerOther2)'>
			  </select>
			  </td>
			  <td><input type='text' name='ntpServerOther2' disabled></td>
			</tr>
		  </table>
		  <br>
		  <table border="0" cellpadding="0" cellspacing="0">
			<tr>
			  <td width='150'>同步通道:</td>
			  <td colspan=2>
			  <select name='if_type' onClick='if_typeChange()'>
				<option value = "0" selected>INTERNET</option>
				<option value = "1">VOICE</option>
				<option value = "2">TR069</option>
				<option value = "3">Other</option>
			  </select>
			  </td>
			</tr>
			<tr>
			  <td width='150'>同步的WAN连接:</td>
			  <td colspan=2>
			  <select name="if_wan">
			  <% if_wan_list("rt"); %>
			  </select>
			  </td>
			</tr>
			<tr>
			  <td width='150'>同步间隔:</td>
			  <td colspan=2><input type='text' name='interval'> 秒</td>
			</tr>
		  </table>
		  <br><br>
		</div>
		<input type='submit' class="button" value="保存/应用" onClick='on_submit();'>
		<input type='hidden' name="server1" value="">
		<input type='hidden' name="server2" value="">
		<input type="hidden" name="submit-url" value="/net_sntp.asp">
	  </form>
	  <script language="javascript">
		  with (document.forms[0]) {
			  <% init_sntp_page(); %>
			  writeNtpList(ntpServerHost1, ntpServerOther1, server1, true);
			  writeNtpList(ntpServerHost2, ntpServerOther2, server2, false);
			  ntpEnblChange();
		  }
	  </script>
	  </div>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>
