<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<html>
<head>
<title>PING测试结果</title>
<meta http-equiv=pragma content=no-cache>
<meta http-equiv=refresh content="2">
<meta http-equiv=cache-control content="no-cache, must-revalidate">
<meta http-equiv=content-type content="text/html; charset=gbk">
<meta http-equiv=content-script-type content=text/javascript>
<!--系统公共css-->
<style type=text/css>
@import url(/style/default.css);
</style>
<!--系统公共脚本-->
<script language="javascript" src="common.js"></script>
</head>

<!-------------------------------------------------------------------------------------->
<!--主页代码-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000">
	<blockquote>
	<form>
	<div align="left" style="padding-left:20px;"><br>
<% dumpPingInfo(); %>
		<input type=button value="back" onClick=window.location.replace("/diag_ping_admin.asp")>
	</div>
	</form>
	</blockquote>
</body>
</html>
