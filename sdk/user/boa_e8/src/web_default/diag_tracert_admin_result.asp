<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>Tracert测试结果</TITLE>
<META http-equiv=pragma content=no-cache>
<META http-equiv = 'refresh' content = '2;url=/diag_tracert_admin_result.asp' >
<META http-equiv=cache-control content="no-cache, must-revalidate">
<META http-equiv=content-type content="text/html; charset=gbk">
<META http-equiv=content-script-type content=text/javascript>
<!--系统公共css-->
<STYLE type=text/css>
@import url(/style/default.css);
</STYLE>
<!--系统公共脚本-->
<SCRIPT language="javascript" src="common.js"></SCRIPT>
</HEAD>

<!-------------------------------------------------------------------------------------->
<!--主页代码-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000">
	<blockquote>
	<form>
	<div align="left" style="padding-left:20px;"><br>
<% dumpTraceInfo(); %>
		<input type=button value="  back  " OnClick=window.location.replace("/diag_tracert_admin.asp")>
	</div>
	</form>
	</blockquote>
</body>
</html>
