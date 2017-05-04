<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>广域网访问设置</TITLE>
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
var cgi = new Object();
/********************************************************************
**          on document load
********************************************************************/
function proxySelection()
{
	if(document.mldproxy.daemon[0].checked)
	{
		document.mldproxy.ext_if.disabled = true;
	}
	else
	{
		document.mldproxy.ext_if.disabled = false;
	}
}

function on_init()
{
	//sji_docinit(document, cgi);
}
</SCRIPT>
</HEAD>

<!-------------------------------------------------------------------------------------->
<!--主页代码-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
  <blockquote>
	<DIV align="left" style="padding-left:20px; padding-top:5px">
		<form id="form" action=/boaform/admin/formMLDProxy method=POST name="mldproxy">
			<b>MLD Proxy 配置</b><br><br>
			这个页面允许你启用或者禁用MLD Proxy功能<br><br>
			
			<table border="0" cellpadding="0" cellspacing="0">
			  <tr>
			  	<td>MLD Proxy:</td>
	      	<td>
	      		<input type="radio" value="0" name="daemon" <%checkWrite("mldproxy0"); %> onClick="proxySelection()">禁用&nbsp;&nbsp;
	      		<input type="radio" value="1" name="daemon" <%checkWrite("mldproxy1"); %> onClick="proxySelection()">启用 </td>
	      </tr>
			  <tr>
			  	<td>接口:&nbsp;</td>
	      	<td> <select name="ext_if" <%checkWrite("mldproxy0d"); %>> <% if_wan_list("rtv6"); %> </select> </td>
			</table>
			<br><br>
			<input type="submit" class="button" value="保存" name="save">
			<input type="hidden" value="/app_mldProxy.asp" name="submit-url">
		</form>
	</DIV>
  </blockquote>

<script>
	initUpnpDisable = document.mldproxy.daemon[0].checked;

	ifIdx = <% getInfo("mldproxy-ext-itf"); %>;
	if (ifIdx != 65535)
		document.mldproxy.ext_if.value = ifIdx;
	else
		document.mldproxy.ext_if.selectedIndex = 0;

	proxySelection();
</script>
</body>
<%addHttpNoCache();%>
</html>
