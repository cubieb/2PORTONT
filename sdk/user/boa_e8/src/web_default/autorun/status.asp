<!-- add by liuxiao 2008-02-15 -->
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
<head>
<title>中国电信</title>
<meta http-equiv=pragma content=no-cache>
<meta http-equiv=cache-control content="no-cache, must-revalidate">
<meta http-equiv=content-type content="text/html; charset=gbk">
<meta http-equiv="refresh" content="5">
<meta http-equiv=content-script-type content=text/javascript>
<style>
BODY{font-family:"华文宋体";font-weight:bolder;}
</style>
<!--系统公共脚本-->
<script language="javascript" type="text/javascript">
document.onmousedown = function(e)
{
	var tar = null;
	if (!e)e = window.event;
	if(e.button != 1)return true;
	if (e.target) tar = e.target;
	else if (e.srcElement) tar = e.srcElement;
	if(tar.tagName == "INPUT" || tar.tagName == "IMG") return true;
	document.location.href = "app:mouse:onmousedown";
}

var cgi = new Object();
<%getifstatus();%>

/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	for(var name in cgi)
	{
		var obj = document.getElementById(name);
		if(typeof obj != "object" || obj == null)continue;
		obj.innerHTML = ((cgi[name]) ? "<font color='green'><b>PASS</b></font>" : "<font color='red'><b>FAIL</b></font>");
	}
}

</script>
</head>
<!--主页代码-->
<body bgcolor="E0E0E0" onLoad="on_init();">
	<p align="center"><font size="+2">系 统 检 测</font><b></b></p>
	<table cellspacing="3" cellpadding="3" width="256px" align="center" border="0">
	  <tr><td align="left" colspan="2">测试局域网连接</tr>
	  <tr><td align="left">&nbsp;&nbsp;&nbsp;&nbsp;ENNET1连接测试:</td><td id="lan1"></td></tr>
	  <tr><td align="left">&nbsp;&nbsp;&nbsp;&nbsp;ENNET2连接测试:</td><td id="iTV"></td></tr>
	  <tr><td align="left">&nbsp;&nbsp;&nbsp;&nbsp;ENNET3连接测试:</td><td id="lan3"></td></tr>
	  <tr><td align="left">&nbsp;&nbsp;&nbsp;&nbsp;ENNET4连接测试:</td><td id="lan4"></td></tr>
	  <tr><td align="left">&nbsp;&nbsp;&nbsp;&nbsp;无线连接测试:</td><td id="wlan"></td></tr>
	  <tr height="6px"><td align="left" colspan="2">&nbsp;</tr>
	  <tr><td align="left" colspan="2">测试ADSL连接</tr>
	  <tr><td align="left">&nbsp;&nbsp;&nbsp;&nbsp;ADSL同步测试:</td><td id="dsl"></td></tr>
	</table>
</body>
</html>
<!-- add end by liuxiao 2008-02-15 -->
