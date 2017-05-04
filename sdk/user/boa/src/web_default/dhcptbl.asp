<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang(LANG_ACTIVE_DHCP_CLIENTS); %></title>
</head>

<body>
<blockquote>
<h2><font color="#0000FF"><% multilang(LANG_ACTIVE_DHCP_CLIENTS); %></font></h2>

<table border=0 width="480" cellspacing=0 cellpadding=0>
  <tr><td><font size=2>
  <% multilang(LANG_THIS_TABLE_SHOWS_THE_ASSIGNED_IP_ADDRESS_MAC_ADDRESS_AND_TIME_EXPIRED_FOR_EACH_DHCP_LEASED_CLIENT); %>
  </font></td></tr>
  <tr><td><hr size=1 noshade align=top></td></tr>
</table>


<form action=/boaform/formReflashClientTbl method=POST name="formClientTbl">
<table border='1' width="80%">
<tr bgcolor=#7f7f7f> <td width="30%"><font size=2><b><% multilang(LANG_IP_ADDRESS); %></b></td>
<td width="40%"><font size=2><b><% multilang(LANG_MAC_ADDRESS); %></b></td>
<td width="30%"><font size=2><b><% multilang(LANG_EXPIRED_TIME_SEC); %></b></td></tr>
<% dhcpClientList(); %>
</table>

<input type="hidden" value="/dhcptbl.asp" name="submit-url">
  <p><input type="submit" value="<% multilang(LANG_REFRESH); %>" name="refresh">&nbsp;&nbsp;
  <input type="button" value="<% multilang(LANG_CLOSE); %>" name="close" onClick="javascript: window.close();"></p>
</form>
</blockquote>
</body>

</html>
