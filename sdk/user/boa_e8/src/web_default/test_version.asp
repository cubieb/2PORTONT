<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
<head>
<title>版本信息</title>
<meta http-equiv=pragma content=no-cache>
<meta http-equiv=cache-control content="no-cache, must-revalidate">
<meta http-equiv=content-type content="text/html; charset=gbk">
<style type=text/css>
@import url(/style/default.css);
</style>
<script language="javascript" src="common.js"></script>
</head>
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000">
 <table width=100%  border=0 cellspacing=0 cellpadding=0>
   <tr><td width=50% align=right><strong>VERSION:&nbsp;</strong></td><td><% getInfo("imgVersion"); %></td></tr>
   <tr><td width=50% align=right><strong>BUILD DATE:&nbsp;</strong></td><td><% getInfo("imgBuildDate"); %> </td></tr>
   <tr><td width=50% align=right><strong>BUILD BY:&nbsp;</strong></td><td><% getInfo("imgBuilder"); %></td></tr>
   <tr><td align=right><strong>FLASH SIZE:&nbsp;</strong></td><td><% getInfo("imgFlashSize"); %></td></tr>
 </table>
 </body>
<%addHttpNoCache();%>
</html>
