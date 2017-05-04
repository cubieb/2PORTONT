<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang("138" "LANG_ACTIVE_WLAN_CLIENTS"); %></title>
</head>

<body>
<blockquote>
<h2><font color="#0000FF"><% multilang("138" "LANG_ACTIVE_WLAN_CLIENTS"); %></font></h2>


<table border=0 width="500" cellspacing=0 cellpadding=0>
  <tr><td><font size=2>
 <% multilang("139" "LANG_THIS_TABLE_SHOWS_THE_MAC_ADDRESS"); %>
  </font></td></tr>
  <tr><td><hr size=1 noshade align=top></td></tr>
</table>
<form action=/boaform/admin/formWirelessTbl method=POST name="formWirelessTbl">
<table border='1' width="500">
<tr bgcolor=#7f7f7f><td width="25%"><font size=2><b><% multilang("86" "LANG_MAC_ADDRESS"); %></b></td>
<td width="15%"><font size=2><b><% multilang("140" "LANG_TX_PACKETS"); %></b></td>
<td width="15%"><font size=2><b><% multilang("141" "LANG_RX_PACKETS"); %></b></td>
<td width="15%"><font size=2><b><% multilang("142" "LANG_TX_RATE_MBPS"); %></b></td>
<td width="15%"><font size=2><b><% multilang("143" "LANG_POWER_SAVING"); %></b></td>
<td width="15%"><font size=2><b><% multilang("144" "LANG_EXPIRED_TIME_SEC"); %></b></td></tr>
<% wirelessClientList(); %>
</table>
<input type="hidden" name="wlan_idx" value=<% checkWrite("wlan_idx"); %>>
<input type="hidden" value="/admin/wlstatbl.asp" name="submit-url">
  <p><input type="submit" value="Refresh" name="refresh">&nbsp;&nbsp;
  <input type="button" value=" Close " name="close" onClick="javascript: window.close();"></p>
</form>
</blockquote>
</body>

</html>
