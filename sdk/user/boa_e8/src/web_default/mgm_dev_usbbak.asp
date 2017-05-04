<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<html>
<head>
<title>usb备份配置</title>
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

var cgi = new Object();
var rcs = new Array();
with (rcs) {
	<% listUsbDevices(); %>
}

/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	sji_docinit(document, cgi);

	for (var i = 0; i < rcs.length; i++) {
		form.usbdev.options.add(new Option(rcs[i].path, rcs[i].path));
	}
	if (rcs.length > 0)
		form.usbdev.selectedIndex = 0;
	else
		form.usbdev.disabled = true;
}

/********************************************************************
**          on document submit
********************************************************************/
function on_action(act)
{
	form.action.value = act;

	if (act == "bk") {
		with(document.forms[0]) {
			if (usbdev.value == "") {
				alert("请插入USB设备, 并刷新本页面！");
				return;
			}
			submit();
		}
	} else if (act == "en") {
		document.forms[0].submit();
	}
}
</script>
</head>

<!-------------------------------------------------------------------------------------->
<!--主页代码-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
	<blockquote>
		<div align="left" style="padding-left:20px; padding-top:5px">
			<form id="form" action="/boaform/admin/formUSBbak" method="post">
				<table align="left"> 
					<tr>
						<td width="110px" height="31px">USB分区选择：</td>
						<td><select size="1" name="usbdev" style="width:140px "></select></td>
					</tr>
				</table>
				<br><br>
				<input type="button" class="button" onClick="on_action('bk')" value="备份配置">
				<br><br>
				<hr align="left" class="sep" size="1" width="90%">
				<br>
				<table align="left" border="0" cellpadding="2" cellspacing="0">
					<tr>
						<td width="110px" height="31px">配置快速恢复：</td>
						<td><input type="radio" name="cfgFastRestoreEnable" value="off" onClick="on_action('en')" <% checkWrite("usb-res0"); %>>&nbsp;&nbsp;禁用</td>
						<td><input type="radio" name="cfgFastRestoreEnable" value="on" onClick="on_action('en')" <% checkWrite("usb-res1"); %>>&nbsp;&nbsp;启用</td>
					</tr>
				</table>
				<input type="hidden" id="action" name="action" value="none">
				<input type="hidden" name="submit-url" value="">
			</form>
		</div>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>
