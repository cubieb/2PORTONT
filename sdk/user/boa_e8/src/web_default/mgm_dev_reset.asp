<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<html>
<head>
<title>恢复出厂设置</title>
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
</head>

<!-------------------------------------------------------------------------------------->
<!--主页代码-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000">
	<blockquote>
		<div align="left" style="padding-left:20px; padding-top:5px">
			<form id="form" action="/boaform/admin/formReboot" method="post">
				<center>
					<b>配置 -- 恢复到默认配置</b><br>
					<br>
					将家庭网关配置恢复到出厂值。<br>
 				</center>
				<br>
				<p align="center"><input type="submit" value="恢复默认配置"></p>
				<input type="hidden" value="/mgm_dev_reset.asp" name="submit-url">
				<input type="hidden" value="1" name="reset">
			</form>
		</div>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>
