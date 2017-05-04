<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang(LANG_FRAMWORK_INFO); %></title>

</head>
<body>
<blockquote>

<h2><b><font color="#0000FF"><% multilang(LANG_FRAMWORK_INFO); %></font></b></h2>

<table border=0 width="500" cellspacing=0 cellpadding=0>
<tr><td><font size=2>
 <% multilang("1127" "LANG_THIS_PAGE_SHOWS_THE_OSGI_FRAMWEORK_OF_THE_DEVICE"); %>
</font></td></tr>

<tr><td><hr size=1 noshade align=top><br></td></tr>
</table>

<form action=/boaform/admin/formStatus method=POST name="status2">
<table width=400 border=0>
  <tr>
    <td width=100% colspan="2" bgcolor="#008000"><font color="#FFFFFF" size=2><b><% multilang("1128" "LANG_OSGI_FRAMEWORK_INFORMATION"); %></b></font></td>
  </tr>
  <tr bgcolor="#EEEEEE">
    <td width=40%><font size=2><b><% multilang("1129" "LANG_FRAMEWORK_NAME"); %></b></td>
    <td width=60%><font size=2><% getOSGIInfo("fwname"); %></td>
  </tr>
  <tr bgcolor="#EEEEEE">
    <td width=40%><font size=2><b><% multilang("1130" "LANG_FRAMEWORK_VERSION"); %></b></td>
    <td width=60%><font size=2><% getOSGIInfo("fwver"); %></td>
  </tr>
   <tr bgcolor="#EEEEEE">
    <td width=40%><font size=2><b><% multilang("1131" "LANG_FRAMEWORK_STATUS"); %></b></td>
    <td width=60%><font size=2><% getOSGIInfo("fwstatus"); %></td>
  </tr>
</table>
</br>
 <% getOSGIBundleList("0"); %>
</form>
<br>


</blockquote>

</body>

</html>
