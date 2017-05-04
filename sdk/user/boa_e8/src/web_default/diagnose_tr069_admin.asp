<!-- add by liuxiao 2008-01-16 -->
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
<head>
<title>手动上报 Inform</title>
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
			<div align="left"><b>手动上报 Inform</b></div>
			<br>
			<table class="flat" border="1" cellpadding="1" cellspacing="1">
				<tr>
					<td class="hdb">手动上报 Inform 状态</td>
					<td><% getInfo("tr069Inform"); %></td>
				</tr>
			</table>
			<br>
			<input type="submit" class="button"  value="开始上报">
			<input type="button" class="button" onClick="location.reload();" value="刷新结果">
			<input type="hidden" name="submit-url" value="/diagnose_tr069_admin.asp">
		</div>
	</blockquote>
	</form>
</body>
<%addHttpNoCache();%>
</html>
