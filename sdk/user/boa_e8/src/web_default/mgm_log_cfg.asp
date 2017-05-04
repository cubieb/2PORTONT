<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<html>
<head>
<title>写入等级设置</title>
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
<script type="text/javascript" src="share.js"></script>
<script language="javascript" type="text/javascript">
var addr = '<% getInfo("syslog-server-ip"); %>';
var port = <% getInfo("syslog-server-port"); %>;

function getLogPort()
{
	if (isNaN(port) || port == 0)
		port = 514; // default system log server port is 514

	return port;
}

/********************************************************************
**          on document update
********************************************************************/
function on_updatectrl() 
{
	with (document.forms[0]) {
		if (syslogEnable[0].checked) {
			recordLevel.disabled = true;
			dispLevel.disabled = true;
			sysMode.disabled = true;
			logAddr.disabled = true;
			logPort.disabled = true;
			logAddr.value = '';
			logPort.value = '';
		} else {
			recordLevel.disabled = false;
			dispLevel.disabled = false;
			sysMode.disabled = false;
			logAddr.disabled = false;
			logPort.disabled = false;
			logAddr.value = addr;
			logPort.value = getLogPort();
		}
		srvInfo.style.display = (sysMode.value & 2) ? "block" : "none";
	}
}

/********************************************************************
**          on document submit
********************************************************************/
function on_submit()
{
	with (document.forms[0]) {
		if (syslogEnable[1].checked == true && sysMode.value & 2) {
			if (sji_checkvip(logAddr.value) == false) {
				logAddr.focus();
				alert("服务器IP地址\"" + logAddr.value +
				      "\"为无效地址，请重新输入！");
				return;
			}
			if (sji_checkdigitrange(logPort.value, 1, 65535) ==
			    false) {
				logPort.focus();
				alert("服务器UDP端口\"" + logPort.value +
				      "\"为无效端口，请重新输入！");
				return;
			}
		}
		submit();
	}
}
</script>
</head>

<!-------------------------------------------------------------------------------------->
<!--主页代码-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000">
	<blockquote>
		<div align="left" style="padding-left:20px; padding-top:5px">
			<form id="form" action="/boaform/admin/formSysLogConfig" method="post">
				<b>系统记录 -- 配置<br></b>
				<br>
				如果记录模式已启用，系统将开始记录所有已选择的事件。对于记录的等级，所有与已选等级相等或更高等级的事件将被记录。<br>
				对于显示的等级，所有与已选等级相等或更高等级的事件将被显示。<br>
				如果选择的模式是"Remote"或"Both，事件将被送到远程系统记录服务器的特定IP地址和UDP端口。<br>
				如果选择的模式是"Local"或"Both"，事件将被记录在本地存储器。<br>
				<br>
					选择需要的值，然后点击"保存/应用"来配置系统记录选项。<br>
				<hr align="left" class="sep" size="1" width="90%">
				<table border="0" cellpadding="0" cellspacing="0">
				   <tr>
					  <td width="80">记录:</td>
					  <td><input name="syslogEnable" type="radio" value="0" onClick="on_updatectrl();" <% checkWrite("log-cap0"); %>>禁用</td>
					  <td><input name="syslogEnable" type="radio" value="1" onClick="on_updatectrl();"<% checkWrite("log-cap1"); %>>启用</td>
				   </tr>
				</table>
				<br>
				<table border="0" cellpadding="0" cellspacing="0">
				   <tr>
					  <td>记录等级:</td>
					  <td colspan="2">
				  		<select name="recordLevel" size="1" style="width:120px ">
						<% checkWrite("syslog-log"); %>
						</select>
					 </td>
				   </tr>
				   <tr>
					  <td>显示等级:</td>
					  <td colspan="2">
					  	<select name="dispLevel" size="1" style="width:120px ">
						<% checkWrite("syslog-display"); %>
						</select>
					  </td>
				   </tr>
				   <tr>
					  <td width="120">模式:</td>
					  <td colspan="2">
				  		<select name="sysMode" size="1" onChange="if (this.selectedIndex == 0) { srvInfo.style.display = 'none'; } else { srvInfo.style.display = 'block'; }" style="width:120px ">
						<% checkWrite("syslog-mode"); %>
						</select>
					 </td>
				   </tr>
				</table>
				<div id="srvInfo">
				   <table border="0" cellpadding="0" cellspacing="0">
					  <tr>
						 <td width="120">服务器IP地址:</td>
						 <td><input type="text" name="logAddr"></td>
					  </tr>
					  <tr>
						 <td>服务器UDP端口:</td>
						 <td><input type="text" name="logPort"></td>
					  </tr>
				   </table>
				</div>
				<hr align="left" class="sep" size="1" width="90%">
				<input type="button" class="button" onClick="on_submit()" value="保存/应用">
				<input type="hidden" name="submit-url" value="/mgm_log_cfg.asp">
<script>
	on_updatectrl();
</script>			</form>
		</div>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>
