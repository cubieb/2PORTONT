<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>Firmware Update</TITLE>
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
<SCRIPT language="javascript" type="text/javascript">
</SCRIPT>

</head>
<BODY>
<blockquote>
<form action=/boaform/admin/formUpgradePop method=POST>
<table border="0" cellspacing="4" width="300">
  <tr><td align=left><font size=2>检测到有新版本，是否进行升级?</td></tr>
  <tr><td>
  <input type="submit" value="升级" name="doit" onClick="window.close()">&nbsp;&nbsp;
  <input type="submit" value="不升级" name="nodo" onClick="window.close()">&nbsp;&nbsp;
  <input type="submit" value="暂不升级" name="holdover" onClick="window.close()">
  </td></tr>
 <tr>
</table>
  <p>
  <input type="hidden" value="/admin/upgrade_pop.asp" name="submit-url">
  </p>
 </form>
 </blockquote>
</body>
<%addHttpNoCache();%>
</html>
