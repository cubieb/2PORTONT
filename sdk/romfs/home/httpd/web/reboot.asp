<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang("473" "LANG_COMMIT_AND_REBOOT"); %></title>
<SCRIPT>
function saveClick()
{
   if ( !confirm('<% multilang("2451" "LANG_DO_YOU_REALLY_WANT_TO_COMMIT_THE_CURRENT_SETTINGS"); %>') ) {
 return false;
  }
  else
 return true;
}

function resetClick()
{
   if ( !confirm('<% multilang("2452" "LANG_DO_YOU_REALLY_WANT_TO_RESET_THE_CURRENT_SETTINGS_TO_DEFAULT"); %>') ) {
 return false;
  }
  else
 return true;
}
</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF"><% multilang("473" "LANG_COMMIT_AND_REBOOT"); %></font></h2>

<form action=/boaform/admin/formReboot method=POST name="cmboot">
<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><td><font size=2>
    <% multilang("474" "LANG_THIS_PAGE_IS_USED_TO_COMMIT_CHANGES_TO_SYSTEM_MEMORY_AND_REBOOT_YOUR_SYSTEM"); %>
  </font></td></tr>
  <tr><td><hr size=1 noshade align=top></td></tr>
</table>
<table border=0 width="500" cellspacing=4 cellpadding=0>
<!--
  <tr>
      <td width="30%"><font size=2><b>Reboot from:</b>
      <select size="1" name="rebootMode">
           <option selected value=0>Last Configuration</option>
           <option value=1>Default Configuration</option>
           <option value=2>Upgrade Configuration</option>
      </select>
      </td>
  </tr>
-->
</table>
  <br>
      <input type="submit" value="<% multilang("473" "LANG_COMMIT_AND_REBOOT"); %>" name="save" onclick="return saveClick()">&nbsp;&nbsp;
<!-- // Jenny,  buglist B031, B032, remove reset to default button from commit/reboot webpage
      <input type="submit" value="Reset to Default" name="reset" onclick="return resetClick()">
      <input type="submit" value="Reboot" name="reboot">
      <input type="hidden" value="/reboot.asp" name="submit-url">
  <script>
-->
 </form>
</blockquote>
</body>

</html>
