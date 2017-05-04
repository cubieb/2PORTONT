<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<META HTTP-EQUIV=Refresh CONTENT="10; URL=status.asp">
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang(LANG_DEVICE_STATUS); %></title>
<script type="text/javascript" src="share.js">
</script>
<script>
var getObj = null;
function modifyClick(url)
{
	var wide=600;
	var high=400;
	if (document.all)
		var xMax = screen.width, yMax = screen.height;
	else if (document.layers)
		var xMax = window.outerWidth, yMax = window.outerHeight;
	else
	   var xMax = 640, yMax=480;
	var xOffset = (xMax - wide)/2;
	var yOffset = (yMax - high)/3;

	var settings = 'width='+wide+',height='+high+',screenX='+xOffset+',screenY='+yOffset+',top='+yOffset+',left='+xOffset+', resizable=yes, toolbar=no,location=no,directories=no,status=no,menubar=no,scrollbars=yes';

	window.open( url, 'Status_Modify', settings );
}

function disButton(id)
{
       getObj = document.getElementById(id);

       window.setTimeout("getObj.disabled=true", 100);
	return false;
}

function on_init()
{
	// Mason Yu for IPv6
	if (!<% checkWrite("IPv6Show"); %>) {
		if (document.getElementById)  // DOM3 = IE5, NS6
		{
			document.getElementById('ipv6DefaultGW').style.display = 'none';
		}
		else {
			if (document.layers == false) // IE4
			{
				document.all.ipv6DefaultGW.style.display = 'none';
			}
		}
	}
	return true;
}
</script>
</head>
<body onLoad="on_init();">
<blockquote>

<h2><b><font color="#0000FF"><% multilang(LANG_DEVICE_STATUS); %></font></b></h2>

<table border=0 width="500" cellspacing=0 cellpadding=0>
<tr><td><font size=2>
	<% multilang(LANG_PAGE_DESC_DEVICE_STATUS_SETTING); %>
</font></td></tr>

<tr><td><hr size=1 noshade align=top><br></td></tr>
</table>

<form action=/boaform/admin/formStatus method=POST name="status2">
<table width=400 border=0>
  <tr>
    <td width=100% colspan="2" bgcolor="#008000"><font color="#FFFFFF" size=2><b><% multilang(LANG_SYSTEM); %></b></font></td>
  </tr>
  <tr bgcolor="#EEEEEE">
    <td width=40%><font size=2><b><% multilang(LANG_DEVICE_NAME); %></b></td>
    <td width=60%><font size=2><% getInfo("name"); %></td>
  </tr>
  <tr bgcolor="#DDDDDD">
    <td width=40%><font size=2><b><% multilang(LANG_UPTIME); %></b></td>
    <td width=60%><font size=2><% getInfo("uptime"); %></td>
  </tr>
<!--
  <tr bgcolor="#EEEEEE">
    <td width=40%><font size=2><b><% multilang(LANG_DATE); %>/<% multilang(LANG_TIME); %></b></td>
    <td width=60%><font size=2><% getInfo("date"); %></td>
  </tr>
-->
  <tr bgcolor="#EEEEEE">
    <td width=40%><font size=2><b><% multilang(LANG_FIRMWARE_VERSION); %></b></td>
    <td width=60%><font size=2><% getInfo("fwVersion"); %></td>
  </tr>
<% DSLVer(); %>
   <tr bgcolor="#EEEEEE" <% checkWrite("bridge-only"); %>>
    <td width=40%><font size=2><b><% multilang(LANG_NAME_SERVERS); %></b></td>
    <td width=60%><font size=2><% getNameServer(); %></td>
  </tr>
  <tr bgcolor="#DDDDDD" <% checkWrite("bridge-only"); %>>
    <td width=40%><font size=2><b>IPv4 <% multilang(LANG_DEFAULT_GATEWAY); %></b></td>
    <td width=60%><font size=2><% getDefaultGW(); %></td>
  </tr>
  <tr id='ipv6DefaultGW' bgcolor="#EEEEEE" <% checkWrite("bridge-only"); %>>
    <td width=40%><font size=2><b>IPv6 <% multilang(LANG_DEFAULT_GATEWAY); %></b></td>
    <td width=60%><font size=2><% getDefaultGW_ipv6(); %></td>
  </tr>
</table>

<table width=400 border=0>
	<!--
  <tr>
    <td width=100% colspan="2" bgcolor="#008000"><font color="#FFFFFF" size=2><b><% multilang(LANG_ADSL); %></b></font></td>
  </tr>
  <tr bgcolor="#EEEEEE">
    <td width=40%><font size=2><b><% multilang(LANG_OPERATIONAL_STATUS); %></b></td>
    <td width=60%><font size=2><% getInfo("dslstate"); %></td>
  </tr>
  <tr bgcolor="#DDDDDD">
    <td width=40%><font size=2><b><% multilang(LANG_UPSTREAM_SPEED); %></b></td>
    <td width=60%><font size=2><% getInfo("adsl-drv-rate-us"); %>&nbsp;kbps
       &nbsp;</td>
  </tr>
  <tr bgcolor="#EEEEEE">
    <td width=40%><font size=2><b><% multilang(LANG_DOWNSTREAM_SPEED); %></b></td>
    <td width=60%><font size=2><% getInfo("adsl-drv-rate-ds"); %>&nbsp;kbps
       &nbsp;</td>
  </tr>
  -->
  <% DSLStatus(); %>
  <tr>
    <td width=100% colspan="2" bgcolor="#008000"><font color="#FFFFFF" size=2><b><% multilang(LANG_LAN); %><% multilang(LANG_CONFIGURATION); %></b></font></td>
  </tr>
  <tr bgcolor="#EEEEEE">
    <td width=40%><font size=2><b><% multilang(LANG_IP_ADDRESS); %></b></td>
    <td width=60%><font size=2><% getInfo("lan-ip"); %></td>
  </tr>
  <tr bgcolor="#DDDDDD">
    <td width=40%><font size=2><b><% multilang(LANG_SUBNET_MASK); %></b></td>
    <td width=60%><font size=2><% getInfo("lan-subnet"); %></td>
  </tr>
  <% DHCPSrvStatus(); %>
  <tr bgcolor="#DDDDDD">
    <td width=40%><font size=2><b><% multilang(LANG_MAC_ADDRESS); %></b></td>
    <td width=60%><font size=2><% getInfo("elan-Mac"); %></td>
  </tr>
</table>
</form>
<br>
<form action=/boaform/admin/formStatus method=POST name="status">
<table width=600 border=0 <% checkWrite("bridge-only"); %>>
 <tr>
    <td width=100% colspan=7 bgcolor="#008000"><font color="#FFFFFF" size=2><b><% multilang(LANG_WAN); %><% multilang(LANG_CONFIGURATION); %></b></font></td>
  </tr>
  <% wanConfList(); %>
</table>
  <% wan3GTable(); %>
  <% wanPPTPTable(); %>
  <% wanL2TPTable(); %>
  <% wanIPIPTable(); %>
  <input type="hidden" value="/admin/status.asp" name="submit-url">
  <input type="submit" value="<% multilang(LANG_REFRESH); %>" name="refresh">&nbsp;&nbsp;
  <!--
  <input type="button" value="Modify" name="modify" onClick="modifyClick('/admin/date.asp')">
  -->
</form>

</blockquote>

</body>

</html>
