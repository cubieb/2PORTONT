<!-- add by liuxiao 2008-01-16 -->
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
<head>
<title>中国电信</title>
<meta http-equiv=pragma content=no-cache>
<meta http-equiv=cache-control content="no-cache, must-revalidate">
<meta http-equiv=content-type content="text/html; charset=gbk">
<style type=text/css>
@import url(/style/default.css);
</style>
<script language="javascript" src="common.js"></script>
</head>
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000">
	<form id="form" action="/boaform/admin/formTr069Diagnose" method="post">
	<blockquote>
		<div align="left" style="padding-left:20px;"><br>
			<div align="left"><b>Inform诊断</b></div>
			<br>
			<div align="left" style="padding-left:20px;">Inform诊断测试，该功能测试需要等待10秒。<div>
			<input type="submit" class="button"  value="开始诊断"></left>
			<input type="hidden" name="submit-url" value="/diagnose_tr069.asp">

			<br>
			<br>
			<br>
			<div align="left"><b>Inform手动上报诊断结果:</b></div>
			<table class="flat" border="1" cellpadding="1" cellspacing="1">
				<tr>
					<td width="40%"  class="hdb">手动上报Inform情况:</td>
					<td><% getInfo("tr069Inform"); %></td>
				</tr>
			</table>
			<input type="button" class="button" onClick="location.href=location.href;" value="刷新结果">
		</div>
	</blockquote>
	</form>
</body>
<%addHttpNoCache();%>
</html>
