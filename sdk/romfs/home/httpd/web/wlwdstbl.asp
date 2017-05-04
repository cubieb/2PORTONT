<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title>WDS AP <% multilang("275" "LANG_TABLE"); %></title>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">WDS AP <% multilang("275" "LANG_TABLE"); %></font></h2>


<table border=0 width="500" cellspacing=0 cellpadding=0>
  <tr><td><font size=2>
 <% multilang("741" "LANG_THIS_TABLE_SHOWS_THE_MAC_ADDRESS_TRANSMISSION_RECEPTION_PACKET_COUNTERS_AND_STATE_INFORMATION_FOR_EACH_CONFIGURED_WDS_AP"); %>
  </font></td></tr>
  <tr><td><hr size=1 noshade align=top></td></tr>
</table>
<form action=/boaform/formWirelessTbl method=POST name="formWirelessTbl">
<table border='1' width="500">
<tr bgcolor=#7f7f7f><td width="30%"><font size=2><b><% multilang("86" "LANG_MAC_ADDRESS"); %></b></td>
<td width="15%"><font size=2><b><% multilang("140" "LANG_TX_PACKETS"); %></b></td>
<td width="15%"><font size=2><b><% multilang("742" "LANG_TX_ERRORS"); %></b></td>
<td width="15%"><font size=2><b><% multilang("141" "LANG_RX_PACKETS"); %></b></td>
<td width="25%"><font size=2><b><% multilang("142" "LANG_TX_RATE_MBPS"); %></b></td></tr>
<% wdsList(); %>
</table>

<input type="hidden" name="wlan_idx" value=<% checkWrite("wlan_idx"); %>>
<input type="hidden" value="/wlwdstbl.asp" name="submit-url">
  <p><input type="submit" value="<% multilang("377" "LANG_REFRESH"); %>" name="refresh">&nbsp;&nbsp;
  <input type="button" value="<% multilang("659" "LANG_CLOSE"); %>" name="close" onClick="javascript: window.close();"></p>
</form>
</blockquote>
</body>

</html>
