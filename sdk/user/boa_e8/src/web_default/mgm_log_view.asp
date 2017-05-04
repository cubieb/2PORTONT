<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<html>
<head>
<title>日志</title>
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
<% initPageSysLog(); %>

/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	sji_docinit(document, cgi);
	if (cgi.issu == false) {
		form.newrec.style.display = "none";
		//form.clrrec.style.display = "none";
	}
}

function btnView()
{
	var options =
	    "menubar=yes,resizable=yes,scrollbars=yes,titlebar=yes,toolbar=no,width=640,height=600";
	window.open("mgm_log_view_access.asp", "系统日志", options);
}

function btnSecView()
{
	var options =
	    "menubar=yes,resizable=yes,scrollbars=yes,titlebar=yes,toolbar=no,width=640,height=600";

	window.open("mgm_log_view_sec.asp", "系统日志", options);
}

/********************************************************************
**          on document submit
********************************************************************/
function on_submit(act)
{
	with (document.forms[0]) {
		action.value = act;
		submit();
	}
}

</script>
</head>

<!-------------------------------------------------------------------------------------->
<!--主页代码-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
	<blockquote>
		<div align="left" style="padding-left:20px; padding-top:5px">
			<form id="form" action="/boaform/admin/formSysLog" method="post">
				<b>系统记录</b><br>
				<br>
				您可以查看系统记录.<br>
				<br>
				点击“访问记录”查看系统访问记录.<br>
				<br>
				点击“安全记录”查看系统安全记录.<br>
				<br>
				另外在启用日志记录后，您可以清理系统记录 .<br>
				<br>
				<hr align="left" class="sep" size="1" width="90%">
				<input type="button" class="button" name="sysrec" onClick="btnView();" value="访问记录">
				<!--<input type="button" class="button" name="secrec" onClick="btnSecView();" value="安全记录">-->
				<!--<input type="button" class="button" name="newrec" onClick="on_submit('new');" value="创建日志文件">-->
				<input type="button" class="button" name="clrrec" onClick="on_submit('clr');" value="清除记录">
				<input type="button" class="button" name="saverec" onClick="on_submit('saveLog');" value="日志下载">
				<input type="hidden" id="action" name="action" value="none">
				<input type="hidden" name="submit-url" value="">
			</form>
		</div>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>
