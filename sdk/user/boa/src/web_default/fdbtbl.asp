<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang(LANG_BRIDGE_FORWARDING_DATABASE); %></title>
</head>

<body>
<blockquote>
<h2><font color="#0000FF"><% multilang(LANG_BRIDGE_FORWARDING_DATABASE); %></font></h2>

<table border=0 width="480" cellspacing=0 cellpadding=0>
  <tr><td><font size=2>
  <% multilang(LANG_PAGE_DESC_MAC_TABLE_INFO); %>
  </font></td></tr>
  <tr><td><hr size=1 noshade align=top></td></tr>
</table>


<form action=/boaform/formRefleshFdbTbl method=POST name="formFdbTbl">
<table border='1' width="80%">
<tr bgcolor=#7f7f7f> <td width="10%"><font size=2><b><% multilang(LANG_PORT); %></b></td>
<td width="20%"><font size=2><b><% multilang(LANG_MAC_ADDRESS); %></b></td>
<td width="10%"><font size=2><b><% multilang(LANG_IS_LOCAL); %>?</b></td>
<td width="10%"><font size=2><b><% multilang(LANG_AGEING_TIMER); %></b></td></tr>
<% bridgeFdbList(); %>
</table>

<input type="hidden" value="/fdbtbl.asp" name="submit-url">
  <p><input type="submit" value="<% multilang(LANG_REFRESH); %>" name="refresh">&nbsp;&nbsp;
  <input type="button" value="<% multilang(LANG_CLOSE); %>" name="close" onClick="javascript: window.close();"></p>
</form>
</blockquote>
</body>

</html>
