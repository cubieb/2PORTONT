<!-- add by liuxiao 2008-01-16 -->
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
<head>
<title>中国电信—我的E家</title>
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
			<div align="left"><b>宽带语音信息</b></div>
			<table class="flat" border="1" cellpadding="1" cellspacing="1">
				<tr>
					<td width="20%"  class="hdb">业务注册状态</td>
					<% voip_e8c_get("registerStatus"); %>
				</tr>
				<tr>
					<td class="hdb">电话号码</td>
					<% voip_e8c_get("sip_number"); %>
				</tr>
			</table>
		</div>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>
