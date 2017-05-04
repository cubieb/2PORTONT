<!-- add by liuxiao 2008-02-15 -->
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
<head>
<title>中国电信</title>
<meta http-equiv=pragma content=no-cache>
<meta http-equiv=cache-control content="no-cache, must-revalidate">
<meta http-equiv=content-type content="text/html; charset=gbk">
<meta http-equiv="refresh" content="5">
<meta http-equiv=content-script-type content=text/javascript>
<!--系统公共css-->
<style type=text/css>
@import url(/style/default.css);
</style>
<!--系统公共脚本-->
<script language="javascript" src="/common.js"></script>
<script language="javascript" type="text/javascript">
var cgi = new Object();
<%getifstatus();%>

/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	for(var name in cgi)
	{
		var obj = document.getElementById(name);
		if(typeof obj != "object")continue;
		obj.src = ((cgi[name]) ? "/image/signal_up.gif" : "/image/signal_down.gif");
	}
}

</script>
</head>
<!--主页代码-->
<body onLoad="on_init();">
<table cellspacing="0" cellpadding="0" width="256" align="center" border="0">
  <tr valign="top">
    <td height="1">
	  <table height="200" cellspacing="0" cellpadding="0" width="100%" align="center" border="0">
	    <tr>
	      <td vAlign=top>
	      <!--当系统上电时，显示绿灯，图片为signal_up.gif，系统参数 flag = 1；当系统不上电时，灯灭，显示图片为signal_down.gif，系统参数 flag =0 -->
	        <table cellspacing="0" cellpadding="0" border="0" width="100%">
	          <tr>
	            <td width="20%" align="center"><img id="power" src="/image/signal_up.gif" border="0"></td>
	            <td width="20%" align="center"><img id="dsl" src="/image/signal_down.gif" border="0"></td>
	            <td width="20%" align="center"><img id="wan" src="/image/signal_down.gif" border="0"></td>
	            <td width="20%" align="center"><img id="lan4" src="/image/signal_up.gif" border="0"></td>
	            <td width="20%" align="center"><img id="lan3" src="/image/signal_down.gif" border="0"></td>
	          </tr>
	          <tr>
	            <td align="center">电源</td>
	            <td align="center">DSL</td>
	            <td align="center">WAN</td>
	            <td align="center">LAN4</td>
	            <td align="center">LAN3</td>
	          </tr>
	          <tr>
	            <td align="center"><img id="iTV" src="/image/signal_down.gif" border="0"></td>
	            <td align="center"><img id="lan1" src="/image/signal_down.gif" border="0"></td>
	            <td align="center"><img id="wlan" src="/image/signal_up.gif" border="0"></td>
	            <td align="center"><img id="wps" src="/image/signal_down.gif" border="0"></td>
	            <td align="center"><img id="usb" src="/image/signal_down.gif" border="0"></td>
	          </tr>
	          <tr>
	            <td align="center">iTV</td>
	            <td align="center">LAN1</td>
	            <td align="center">WLAN</td>
	            <td align="center">WPS</td>
	            <td align="center">USB</td>
	          </tr>       
	        </table>
		  </td>
	    </tr>
	    <tr>
	      <td height="1">
	        <table height="35" cellspacing="0" cellpadding="0" width="100%" border="0">
	          <tr>
	            <td bgcolor="#427594">&nbsp;</td>
			  </tr>
			</table>
		  </td>
		</tr>
      </table>
	</td>
  </tr>
</table>
</body>
<%addHttpNoCache();%>
</html>
