<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang("475" "LANG_BACKUP_AND_RESTORE_SETTINGS"); %></title>
<script>
function resetClick()
{
 return confirm("<% multilang("2453" "LANG_DO_YOU_REALLY_WANT_TO_RESET_THE_CURRENT_SETTINGS_TO_FACTORY_DEFAULT"); %>");
}

function uploadClick()
{
    if (document.saveConfig.binary.value.length == 0) {
  alert('<% multilang("476" "LANG_CHOOSE_FILE"); %>!');
  document.saveConfig.binary.focus();
  return false;
 }
 return true;
}

</script>

</head>
<body>
<blockquote>
<h2><font color="#0000FF"><% multilang("475" "LANG_BACKUP_AND_RESTORE_SETTINGS"); %></font></h2>
  <table border="0" cellspacing="4" width="500">
  <tr><td><font size=2>
 <% multilang("477" "LANG_THIS_PAGE_ALLOWS_YOU_TO_BACKUP_CURRENT_SETTINGS_TO_A_FILE_OR_RESTORE_THE_SETTINGS_FROM_THE_FILE_WHICH_WAS_SAVED_PREVIOUSLY_BESIDES_YOU_COULD_RESET_THE_CURRENT_SETTINGS_TO_FACTORY_DEFAULT"); %>
  </font></td></tr>
  <tr><td><hr size=1 noshade align=top></td></tr>
  </table>

  <table border="0" cellspacing="4" width="500">
  <form action=/boaform/formSaveConfig method=POST name="saveCSConfig">
  <tr>
    <td width="40%"><font size=2><b><% multilang("478" "LANG_BACKUP_SETTINGS_TO_FILE"); %>:</b></font></td>
    <td width="30%"><font size=2>
      <input type="submit" value="<% multilang("481" "LANG_BACKUP"); %>..." name="save_cs">
    </font></tr>
  </form>

  <!--
  <form action=/boaform/formSaveConfig method=POST name="saveDSConfig">
  <tr>
    <td width="40%"><font size=2><b><% multilang("479" "LANG_BACKUP_DEFAULT_SETTINGS_TO_FILE"); %>:</b></font></td>
    <td width="30%"><font size=2>
      <input type="submit" value="<% multilang("481" "LANG_BACKUP"); %>..." name="save_ds">
    </font></tr>
  </form>

  <form action=/boaform/formSaveConfig method=POST name="saveHSConfig">
  <tr>
    <td width="40%"><font size=2><b><% multilang("480" "LANG_BACKUP_HARDWARE_SETTINGS_TO_FILE"); %>:</b></font></td>
    <td width="30%"><font size=2>
      <input type="submit" value="<% multilang("481" "LANG_BACKUP"); %>..." name="save_hs">
    </font></tr>
  </form>
  -->

  <form action=/boaform/formSaveConfig enctype="multipart/form-data" method=POST name="saveConfig">
  <tr>
    <td width="40%"><font size=2><b><% multilang("482" "LANG_RESTORE_SETTINGS_FROM_FILE"); %>:</b></font></td>
    <td width="30%"><font size=2><input type="file" value="<% multilang("476" "LANG_CHOOSE_FILE"); %>" name="binary" size=24></font></td>
    <td width="20%"><font size=2><input type="submit" value="<% multilang("483" "LANG_RESTORE"); %>" name="load" onclick="return uploadClick()"></font></td>
    <input type="hidden" value="/saveconf.asp" name="submit-url">
  </tr>
  </form>

  <form action=/boaform/formSaveConfig method=POST name="resetConfig">
  <tr>
    <td width="70%"><font size=2><b><% multilang("484" "LANG_RESET_SETTINGS_TO_DEFAULT"); %>:</b></font></td>
    <td width="30%"><font size=2>
    <input type="submit" value="<% multilang("196" "LANG_RESET"); %>" name="reset" onclick="return resetClick()"></font></td>
    <input type="hidden" value="/saveconf.asp" name="submit-url">
   </tr>
  </form>
</table>
</blockquote>
</body>
</html>
