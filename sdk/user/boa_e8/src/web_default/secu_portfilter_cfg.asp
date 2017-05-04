<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>端口过滤</TITLE>
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
<%ipPortFilterConfig();%>
/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	sji_docinit(document, cgi);

	if(cgi.ipfilterEnable == false)
	{
		form.ipFilterMode[0].disabled = true;
		form.ipFilterMode[1].disabled = true;
		policy_frame.location.href = "about:blank";
	}
	else
	{
		on_mode();
	}
}

function on_action()
{
	with(form)
	{
		submit();
	}
}

function on_mode()
{
	var surl = ( (form.ipFilterMode[0].checked == true)? "secu_portfilter_blk.asp" : "secu_portfilter_wht.asp");
	if(policy_frame.location)policy_frame.location.href = surl;
	else policy_frame.src = surl;
}

</SCRIPT>
</HEAD>
<!-------------------------------------------------------------------------------------->
<!--主页代码-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
	<blockquote>
		<DIV align="left" style="padding-left:20px; padding-top:5px">
			<form id="form" action=/boaform/admin/formPortFilter method=POST name="form">
				<b>端口过滤 -- 最多允许您添加 16条规则.</b><br><br>
				<!--
				<div id="rstip" style="display:none;"><font color="red">提示：本页面的设置，需要重启路由器才能生效！</font><br></div>
				-->
				<hr align="left" class="sep" size="1" width="90%">
				<table border="0" cellpadding="2" cellspacing="0">
					<tr>
						<td>IP地址过滤:</td>
						<td><input type="radio" name="ipfilterEnable" value="off" onClick="on_action()">&nbsp;&nbsp;禁用</td>
						<td><input type="radio" name="ipfilterEnable" value="on" onClick="on_action()">&nbsp;&nbsp;启用</td>
						<!--
						<td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<font color="red"><b>注：有Internet连接才能启用！</b></font></td>
						-->
					</tr>
					<tr>
						<td>过滤模式:</td>
						<td><input type="radio" name="ipFilterMode" onClick="on_mode();" checked>&nbsp;&nbsp;黑名单</td>
						<td><input type="radio" name="ipFilterMode" onClick="on_mode();">&nbsp;&nbsp;白名单</td>
						<td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<font color="red"><b>注：黑白名单是同时工作的！</b></font></td>
					</tr>
				</table>
				<br>
				<hr align="left" class="sep" size="1" width="90%">
				<input type="hidden" id="action" name="action" value="sw">
				<input type="hidden" name="submit-url" value="" >
			</form>
		</div>
	</blockquote>
	<iframe src="about:blank" id="policy_frame" width="90%" frameborder="0" style="border-style:none; height:80%"></iframe>
</body>
<%addHttpNoCache();%>
</html>
