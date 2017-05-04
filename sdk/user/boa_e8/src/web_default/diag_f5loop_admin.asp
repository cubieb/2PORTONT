<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>USB备份配置</TITLE>
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
<script type="text/javascript" src="share.js"></script>
</head>
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000">
<blockquote>
	<form id="form" action=/boaform/admin/formOAMLB method=POST name="oamlb">
		<div align="left" style="padding-left:20px;"><br>
			<div align="left"><b>诊断测试</b></div>
			<br>
			<table width="50%" align="left" valign="middle">
				<tr><td>WAN 接口：: <% oamSelectList(); %></td></tr>
				<tr><td>
					<input type="radio" value="0" name="oam_flow" checked>F5 Segment&nbsp;&nbsp;&nbsp;&nbsp;
					<input type="radio" value="1" name="oam_flow" >F5 End-to-End
				</td></tr>
				<tr><td><br><input type="submit" value="测试" name="go"></td></tr>
			</table>
		<input type="hidden" value="/diag_f5loop_admin.asp" name="submit-url">
		</div>
	</form>
</blockquote>
</body>
<%addHttpNoCache();%>
</html>
