<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang("30" "LANG_LANDING_PAGE"); %><% multilang("212" "LANG_CONFIGURATION"); %></title>
<script>
function saveClick()
{
 alert('<% multilang("2187" "LANG_PLEASE_COMMIT_AND_REBOOT_THIS_SYSTEM_FOR_TAKE_EFFECT_THE_LANDING_PAGE"); %>');
}

</script>

</head>

<body>
<blockquote>
<h2><font color="#0000FF"><% multilang("30" "LANG_LANDING_PAGE"); %><% multilang("212" "LANG_CONFIGURATION"); %></font></h2>

<form action=/boaform/formLanding method=POST name="landing">
<table border=0 width="500" cellspacing=0 cellpadding=0>
  <tr><td><font size=2>
    <% multilang("686" "LANG_THIS_PAGE_IS_USED_TO_CONFIGURE_THE_TIME_INTERVAL_OF_LANDING_PAGE"); %>
  </font></td></tr>
  <tr><td><hr size=1 noshade align=top></td></tr>
</table>

<table border=0 width="500" cellspacing=0 cellpadding=0>
  <tr>
      <td width="30%"><font size=2><b><% multilang("687" "LANG_TIME_INTERVAL"); %>:</b></font></td>
      <td width="70%"><input type="text" name="interval" size="15" maxlength="15" value=<% getInfo("landing-page-time"); %>>(<% multilang("302" "LANG_SECONDS"); %>)</td>
  </tr>
</table>
      <br>
      <input type="submit" value="<% multilang("134" "LANG_APPLY_CHANGES"); %>" name="save" onClick="return saveClick()">&nbsp;&nbsp;
      <input type="hidden" value="/landing.asp" name="submit-url">

</form>
</blockquote>
</body>

</html>
