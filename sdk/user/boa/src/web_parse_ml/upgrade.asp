<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang("64" "LANG_FIRMWARE_UPGRADE"); %></title>
<script>
function sendClicked()
{
 if (document.password.binary.value=="") {
  alert("<% multilang("2149" "LANG_SELECTED_FILE_CANNOT_BE_EMPTY"); %>");
  document.password.binary.focus();
  return false;
 }

 if (!confirm('<% multilang("495" "LANG_PAGE_DESC_UPGRADE_CONFIRM")%>'))
  return false;
 else
  return true;
}

</script>

</head>
<BODY>
<blockquote>
<h2><font color="#0000FF"><% multilang("64" "LANG_FIRMWARE_UPGRADE"); %></font></h2>
<form action=/boaform/admin/formUpload method=POST enctype="multipart/form-data" name="password">
<table border="0" cellspacing="4" width="500">
 <tr><td><font size=2>
 <% multilang("490" "LANG_PAGE_DESC_UPGRADE_FIRMWARE"); %>
 </font></td></tr>
 <tr><td><hr size=1 noshade align=top></td></tr>
</table>
<table border="0" cellspacing="4" width="500">
  <tr>
      <td><font size=2><input type="file" value="<% multilang("476" "LANG_CHOOSE_FILE"); %>" name="binary" size=20></td>
  </tr>
  </table>
    <p> <input type="submit" value="<% multilang("491" "LANG_UPGRADE"); %>" name="send" onclick="return sendClicked()">&nbsp;&nbsp;
 <input type="reset" value="<% multilang("196" "LANG_RESET"); %>" name="reset">
 <input type="hidden" value="/admin/upgrade.asp" name="submit-url">
    </p>
 </form>
 </blockquote>
</body>
</html>
