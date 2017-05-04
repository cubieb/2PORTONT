<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta HTTP-EQUIV='Pragma' CONTENT='no-cache'>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang("63" "LANG_PASSWORD"); %><% multilang(LANG_WAN_CONFIGURATION); %></title>
<script type="text/javascript" src="share.js">
</script>
<SCRIPT>

function saveChanges()
{
   if ( document.password.username.value.length == 0 ) {
 if ( !confirm('User account is empty.\nDo you want to disable the password protection?') ) {
  document.password.username.focus();
  return false;
   }
 else
  return true;
  }

   if ( document.password.newpass.value != document.password.confpass.value) {
 alert('<% multilang("1973" "LANG_PASSWORD_IS_NOT_MATCHED_PLEASE_TYPE_THE_SAME_PASSWORD_BETWEEN_NEW_AND_CONFIRMED_BOX"); %>');
 document.password.newpass.focus();
 return false;
  }

  if ( document.password.username.value.length > 0 &&
   document.password.newpass.value.length == 0 ) {
 alert('<% multilang("1974" "LANG_PASSWORD_CANNOT_BE_EMPTY_PLEASE_TRY_IT_AGAIN"); %>');
 document.password.newpass.focus();
 return false;
  }

  if ( includeSpace(document.password.username.value)) {
 alert('<% multilang("1975" "LANG_CANNOT_ACCEPT_SPACE_CHARACTER_IN_USER_NAME_PLEASE_TRY_IT_AGAIN"); %>');
 document.password.username.focus();
 return false;
  }
  if (checkString(document.password.username.value) == 0) {
 alert('<% multilang("1976" "LANG_INVALID_USER_NAME"); %>');
 document.password.username.focus();
 return false;
  }

  if (includeSpace(document.password.newpass.value)) {
 alert('<% multilang("1977" "LANG_CANNOT_ACCEPT_SPACE_CHARACTER_IN_PASSWORD_PLEASE_TRY_IT_AGAIN"); %>');
 document.password.newpass.focus();
 return false;
  }
  if (checkString(document.password.newpass.value) == 0) {
 alert('<% multilang("1978" "LANG_INVALID_PASSWORD"); %>');
 document.password.newpass.focus();
 return false;
  }

  return true;
}

</SCRIPT>
</head>

<BODY>
<blockquote>
<h2><font color="#0000FF"><% multilang("63" "LANG_PASSWORD"); %><% multilang(LANG_WAN_CONFIGURATION); %></font></h2>

<form action=/boaform/admin/formUserPasswordSetup method=POST name="password">
 <table border="0" cellspacing="4" width="500">
  <tr><font size=2>
 This page is used to set the account to access the web server of ADSL Router.
 Empty user name and password will disable the protection.
  </tr>
  <tr><hr size=1 noshade align=top></tr>

  <table border="0" cellspacing="4" width="500">
    <tr>
      <td width="20%"><font size=2><b><% multilang("728" "LANG_LOGIN_USER"); %>:</b></td>
      <td width="50%"><font size=2><% getInfo("login-user"); %></td>
    </tr>
    <tr>
      <td width="20%"><font size=2><b><% multilang("486" "LANG_OLD_PASSWORD"); %>:</b></td>
      <td width="50%"><font size=2><input type="password" name="oldpass" size="20" maxlength="30"></td>
    </tr>
    <tr>
      <td width="20%"><font size=2><b><% multilang("487" "LANG_NEW_PASSWORD"); %>:</b></td>
      <td width="50%"><font size=2><input type="password" name="newpass" size="20" maxlength="30"></td>
    </tr>
    <tr>
      <td width="20%"><font size=2><b><% multilang("488" "LANG_CONFIRMED_PASSWORD"); %>:</b></td>
      <td width="50%"><font size=2><input type="password" name="confpass" size="20" maxlength="30"></td>
    </tr>
  </table>
   <input type="hidden" value="/admin/user-password.asp" name="submit-url">
  <p><input type="submit" value="<% multilang("134" "LANG_APPLY_CHANGES"); %>" name="save" onClick="return saveChanges()">&nbsp;&nbsp;
  <input type="reset" value=" <% multilang("196" "LANG_RESET"); %> " name="reset"></p>
</form>
<blockquote>
</body>
</html>
