<html>
<! Copyright (c) Realtek Semiconductor Corp., 2006. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang(LANG_ARP_TABLE); %></title>
</head>

<body>
<blockquote>
<h2><font color="#0000FF"><% multilang(LANG_USER_LIST); %></font></h2>

<table border=0 width="480" cellspacing=0 cellpadding=0>
  <tr><td><font size=2>
 <% multilang(LANG_PAGE_DESC_MAC_TABLE_INFO); %>
  </font></td></tr>
  <tr><td><hr size=1 noshade align=top></td></tr>
</table>


<form action=/boaform/formRefleshFdbTbl method=POST name="formFdbTbl">
<table border='1' width="500" >
<tr bgcolor=#7f7f7f> <td width="20%"><font size=2><b><% multilang(LANG_IP_ADDRESS); %></b></td>
<td width="20%"><font size=2><b><% multilang(LANG_MAC_ADDRESS); %></b></td>
<% ARPTableList(); %>
</table>

<input type="hidden" value="/arptable.asp" name="submit-url">
  <p><input type="submit" value="<% multilang(LANG_REFRESH); %>" name="refresh">&nbsp;&nbsp;
</form>
</blockquote>
</body>

</html>
