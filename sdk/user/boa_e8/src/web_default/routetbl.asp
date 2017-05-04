<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>路由表</TITLE>
<META http-equiv=pragma content=no-cache>
<META http-equiv=cache-control content="no-cache, must-revalidate">
<META http-equiv=content-type content="text/html; charset=gbk">
<META http-equiv=content-script-type content=text/javascript>
<!--系统公共css-->
<STYLE type=text/css>
@import url(/style/default.css);
</STYLE>
<!--系统公共脚本-->
</HEAD>

<body>
<blockquote>
<h2><font color="#0000FF">路由表</font></h2>

<form action=/boaform/formRefleshRouteTbl method=POST name="formRouteTbl">
<table border='1' width="80%">
<% routeList(); %>
</table>

<input type="hidden" value="/routetbl.asp" name="submit-url">
  <p><input type="submit" value="刷新" name="refresh">&nbsp;&nbsp;
  <input type="button" value="关闭" name="close" onClick="javascript: window.close();"></p>
</form>
</blockquote>
</body>

</html>
