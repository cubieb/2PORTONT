<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="refresh" content="10">
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang(LANG_PRINTER); %> URL(s)</title>
<script type="text/javascript" src="share.js">
</script>
<script>

</script>
</head>

<body>
<blockquote>
<h2><font color="#0000FF"><% multilang(LANG_PRINTER); %> URL(s)</font></h2>
<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><td><font size=2>
    <% multilang(LANG_THIS_PAGE_IS_USED_TO_SHOW_PRINTER_URL_S); %>
  </font></td></tr>
  <tr><td><hr size=1 noshade align=top></td></tr>
</table>
<table>
	<% printerList(); %>
</table>
<br>
	<input type="button" value="<% multilang(LANG_REFRESH); %>" name="refresh" onClick="location.reload();">
</blockquote>
</body>
</html>
