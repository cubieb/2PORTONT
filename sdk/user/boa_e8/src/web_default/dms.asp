<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>DMS Settings</TITLE>
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
<SCRIPT language="javascript" type="text/javascript">
function dmsSelection()
{	
	return true;
}
</SCRIPT>
</head>

<body>
<blockquote>
<DIV align="left" style="padding-left:20px; padding-top:5px">
<form action=/boaform/formDMSConf method=POST name="formDMSconf">
<table border=0 width="500" cellspacing=4 cellpadding=0>
  <b>DMS 配置</b><br><br>
  <tr>
    这个页面允许你启用或者禁用DMS功能
  </tr>
  <tr><hr size=1 noshade align=top></tr>
  <tr>
      <td width="30%">Digital Media Server:</td>
      <td width="70%">
	      <input type="radio" name="enableDMS" value=0 <% fmDMS_checkWrite("fmDMS-enable-dis"); %> onClick="dmsSelection()" >禁用&nbsp;&nbsp;
	      <input type="radio" name="enableDMS" value=1 <% fmDMS_checkWrite("fmDMS-enable-en"); %>  onClick="dmsSelection()" >启用
      </td>
  </tr>  
</table>
<br>
      <input type="submit" value="保存/应用" name="apply">&nbsp;&nbsp;
      <!--<input type="reset" value="Undo" name="reset" onClick="window.location.reload()">-->
      <input type="hidden" value="/dms.asp" name="submit-url">
</form>
</DIV>
</blockquote>
</body>
</html>
