<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% checkWrite("adsl_slv_diag_title"); %></title>
</head>

<body>
<blockquote>
<h2><font color="#0000FF"><% checkWrite("adsl_slv_diag_title"); %></font></h2>

<form action=/boaform/formDiagAdsl method=POST name=diag_adsl>
<table border=0 width=500 cellspacing=4 cellpadding=0>
	<tr><td><font size=2>
	  <% checkWrite("adsl_slv_diag_cmt"); %>
	</font></td></tr>
	<tr><td><hr size=1 noshade align=top></td></tr>
</table>

<input type=submit value="  <% multilang(LANG_START); %>  " name=start disabled="disabled">
<input type=hidden value="/admin/adsl-slv-diag.asp" name="submit-url">
<input type=hidden value="1" name="slaveid">
<table border=0 cellspacing=4 cellpadding=0>
	<% adslToneDiagTbl("1"); %>
</table>
<p>

<% vdslBandStatusTbl("1"); %>

<table border=0 width=500 cellspacing=4 cellpadding=0>
<% adslToneDiagList("1"); %>
</table>
  <br>
<script>
	<% initPage("diagdslslv"); %>
</script>
</form>
</blockquote>
</body>

</html>
