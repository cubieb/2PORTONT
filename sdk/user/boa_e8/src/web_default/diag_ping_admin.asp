<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>PING测试</TITLE>
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
<SCRIPT language="javascript" type="text/javascript">
var links = new Array();
with(links){<% listWanif("rt"); %>}

/********************************************************************
**          on document load
********************************************************************/
function on_Apply(){
	if( !sji_checkvip(document.forms[0].target_addr.value) && !sji_checkhostname(document.forms[0].target_addr.value)
		&& !isIPv6(document.forms[0].target_addr.value)){
		alert("目标地址错误！");
		document.forms[0].target_addr.focus();
		return false;
	}

	return true;
}
</SCRIPT>
</HEAD>

<!-------------------------------------------------------------------------------------->
<!--主页代码-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000">
	<blockquote>
	<form id="form" action=/boaform/admin/formPing method=POST>
		<div align="left" style="padding-left:20px;"><br>
			<div align="left"><b>PING测试</b></div>
			<br>
			<table width="50%" align="left" valign="middle">
			<tr><td>目标地址：<input type="text" name="target_addr" maxlength="60" /></td></tr>
			<tr><td>WAN 接口：<select name="waninf"><% checkWrite("wan-interface-name"); %></select></td></tr>
			<tr><td><br><input type="submit" value="Ping" width="100px" onClick="return on_Apply();" /></td></tr>
			</table>
		</div>
	</form>
	</blockquote>
</body>
</html>
