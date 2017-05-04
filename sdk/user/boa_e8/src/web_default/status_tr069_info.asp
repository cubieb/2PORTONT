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
	<blockquote>
		<div align="left" style="padding-left:20px;"><br>
			<div align="left"><b>交互建立信息</b></div>
			<br>
			<table class="flat" border="1" cellpadding="1" cellspacing="1">
				<tr>
					<td width="40%"  class="hdb">主动上报Inform情况:</td>
					<td><% getInfo("tr069Inform"); %></td>
				</tr>
				<tr>
					<td class="hdb">接受ITMS连接请求情况:</td>
					<td><% getInfo("tr069Connect"); %></td>
				</tr>
			</table>
			<br>
			<br>
		</div>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>
