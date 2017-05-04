<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title>PON <% multilang(LANG_STATUS); %></title>
<script type="text/javascript" src="share.js">
</script>
</head>
<body>
<blockquote>

<h2><b><font color="#0000FF">PON <% multilang(LANG_STATUS); %></font></b></h2>

<table border=0 width="500" cellspacing=0 cellpadding=0>
<tr><td><font size=2>
 <% multilang(LANG_PAGE_DESC_PON_STATUS); %>
</font></td></tr>

<tr><td><hr size=1 noshade align=top><br></td></tr>
</table>

<table width=400 border=0>
  <tr>
    <td width=100% colspan="2" bgcolor="#008000"><font color="#FFFFFF" size=2><b><% multilang(LANG_PON); %><% multilang(LANG_STATUS_1); %></b></font></td>
  </tr>
  <tr bgcolor="#DDDDDD">
    <td width=40%><font size=2><b><% multilang(LANG_VENDOR_NAME); %></b></td>
    <td width=60%><font size=2><% ponGetStatus("vendor-name"); %></td>
  </tr>
  <tr bgcolor="#DDDDDD">
    <td width=40%><font size=2><b><% multilang(LANG_PART_NUMBER); %></b></td>
    <td width=60%><font size=2><% ponGetStatus("part-number"); %></td>
  </tr>
  <tr bgcolor="#DDDDDD">
    <td width=40%><font size=2><b><% multilang(LANG_TEMPERATURE); %></b></td>
    <td width=60%><font size=2><% ponGetStatus("temperature"); %></td>
  </tr>
  <tr bgcolor="#DDDDDD">
    <td width=40%><font size=2><b><% multilang(LANG_VOLTAGE); %></b></td>
    <td width=60%><font size=2><% ponGetStatus("voltage"); %></td>
  </tr>
  <tr bgcolor="#DDDDDD">
    <td width=40%><font size=2><b><% multilang(LANG_TX_POWER); %></b></td>
    <td width=60%><font size=2><% ponGetStatus("tx-power"); %></td>
  </tr>
  <tr bgcolor="#DDDDDD">
    <td width=40%><font size=2><b><% multilang(LANG_RX_POWER); %></b></td>
    <td width=60%><font size=2><% ponGetStatus("rx-power"); %></td>
  </tr>
  <tr bgcolor="#DDDDDD">
    <td width=40%><font size=2><b><% multilang(LANG_BIAS_CURRENT); %></b></td>
    <td width=60%><font size=2><% ponGetStatus("bias-current"); %></td>
  </tr>
</table>
<br>
<table width=400 border=0>
  <% showgpon_status(); %> 
</table>
<table width=400 border=0>
  <% showepon_LLID_status(); %> 
</table>
<form action=/boaform/admin/formStatus_pon method=POST name="status_pon">
  <input type="hidden" value="/status_pon.asp" name="submit-url">
  <input type="submit" value="<% multilang(LANG_REFRESH); %>" name="refresh">&nbsp;&nbsp;
</form>
</blockquote>
</body>
</html>
