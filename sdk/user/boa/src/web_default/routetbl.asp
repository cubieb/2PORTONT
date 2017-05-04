<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang(LANG_IP_ROUTE_TABLE); %></title>
</head>

<body>
<blockquote>
<h2><font color="#0000FF"><% multilang(LANG_IP_ROUTE_TABLE); %></font></h2>

<table border=0 width="480" cellspacing=0 cellpadding=0>
  <tr><td><font size=2>
  <% multilang(LANG_THIS_TABLE_SHOWS_A_LIST_OF_DESTINATION_ROUTES_COMMONLY_ACCESSED_BY_YOUR_NETWORK); %>
  </font></td></tr>
  <tr><td><hr size=1 noshade align=top></td></tr>
</table>


<form action=/boaform/formRefleshRouteTbl method=POST name="formRouteTbl">
<table border='1' width="80%">
<% routeList(); %>
</table>

<input type="hidden" value="/routetbl.asp" name="submit-url">
  <p><input type="submit" value="<% multilang(LANG_REFRESH); %>" name="refresh">&nbsp;&nbsp;
  <input type="button" value="<% multilang(LANG_CLOSE); %>" name="close" onClick="javascript: window.close();"></p>
</form>
</blockquote>
</body>

</html>
