<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<META HTTP-EQUIV=Refresh CONTENT="10; URL=lan_port_status.asp">
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang("94" "LANG_LAN_PORT_STATUS"); %></title>
<script type="text/javascript" src="share.js">
</script>
<script>

</script>
</head>
<body>
<blockquote>

<h2><b><font color="#0000FF"><% multilang("94" "LANG_LAN_PORT_STATUS"); %></font></b></h2>

<table border=0 width="500" cellspacing=0 cellpadding=0>
<tr><td><font size=2>
 <% multilang("95" "LANG_THIS_PAGE_SHOWS_THE_CURRENT_LAN_PORT_STATUS"); %>
</font></td></tr>

<tr><td><hr size=1 noshade align=top><br></td></tr>
</table>

<form action=/boaform/admin/formLANPortStatus method=POST name="status3">
<table width=400 border=0>
  <tr>
    <td width=100% colspan="2" bgcolor="#008000"><font color="#FFFFFF" size=2><b><% multilang("94" "LANG_LAN_PORT_STATUS"); %></b></font></td>
  </tr>
  <% showLANPortStatus(); %>
<!-- <tr bgcolor="#EEEEEE">
    <td width=40%><font size=2><b>LAN1</b></td>
    <td width=60%><font size=2><% getInfo("lan1-status"); %></td>
  </tr>
  <tr bgcolor="#DDDDDD">
    <td width=40%><font size=2><b>LAN2</b></td>
    <td width=60%><font size=2><% getInfo("lan2-status"); %></td>
  </tr>
  <tr bgcolor="#EEEEEE">
    <td width=40%><font size=2><b>LAN3</b></td>
    <td width=60%><font size=2><% getInfo("lan3-status"); %></td>
  </tr>
   <tr bgcolor="#EEEEEE">
    <td width=40%><font size=2><b>LAN4</b></td>
    <td width=60%><font size=2><% getInfo("lan4-status"); %></td>
  </tr>
 -->
</table>
<input type="hidden" value="/lan_port_status.asp" name="submit-url">
<input type="submit" value="Refresh" name="refresh">&nbsp;&nbsp;
</form>
<br>
</blockquote>
</body>
</html>
