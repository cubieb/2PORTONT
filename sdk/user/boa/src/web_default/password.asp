<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta HTTP-EQUIV='Pragma' CONTENT='no-cache'>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang(LANG_PASSWORD_CONFIGURATION); %></title>
<script type="text/javascript" src="share.js">
</script>
<SCRIPT>

function saveChanges()
{
/*   if ( document.password.username.value.length == 0 ) {
	if ( !confirm('<% multilang(LANG_USER_ACCOUNT_IS_EMPTY_NDO_YOU_WANT_TO_DISABLE_THE_PASSWORD_PROTECTION); %>') ) {
		document.password.username.focus();
		return false;
  	}
	else
		return true;
  }*/

   if ( document.password.newpass.value != document.password.confpass.value) {
	alert("<% multilang(LANG_PASSWORD_IS_NOT_MATCHED_PLEASE_TYPE_THE_SAME_PASSWORD_BETWEEN_NEW_AND_CONFIRMED_BOX); %>");
	document.password.newpass.focus();
	return false;
  }

//  if ( document.password.username.value.length > 0 &&
//  		document.password.newpass.value.length == 0 ) {
  if (	document.password.newpass.value.length == 0) {
	alert("<% multilang(LANG_PASSWORD_CANNOT_BE_EMPTY_PLEASE_TRY_IT_AGAIN); %>");
	document.password.newpass.focus();
	return false;
  }

/*  if ( includeSpace(document.password.username.value)) {
	alert("<% multilang(LANG_CANNOT_ACCEPT_SPACE_CHARACTER_IN_USER_NAME_PLEASE_TRY_IT_AGAIN); %>");
	document.password.username.focus();
	return false;
  }*/

  if (includeSpace(document.password.newpass.value)) {
	alert("<% multilang(LANG_CANNOT_ACCEPT_SPACE_CHARACTER_IN_PASSWORD_PLEASE_TRY_IT_AGAIN); %>");
	document.password.newpass.focus();
	return false;
  }
  if (checkString(document.password.newpass.value) == 0) {
	alert("<% multilang(LANG_INVALID_PASSWORD); %>");
	document.password.newpass.focus();
	return false;
  }

  return true;
}

</SCRIPT>
</head>

<BODY>
<blockquote>
<h2><font color="#0000FF"><% multilang(LANG_PASSWORD_CONFIGURATION); %></font></h2>

<form action=/boaform/formPasswordSetup method=POST name="password">
 <table border="0" cellspacing="4" width="500">
  <tr><td><font size=2>
 <% multilang(LANG_PAGE_DESC_SET_ACCOUNT_PASSWORD); %>
  </font></td></tr>
  <tr><td><hr size=1 noshade align=top></td></tr>
  </table>

  <table border="0" cellspacing="4" width="500">
    <tr>
      <td width="20%"><font size=2><b><% multilang(LANG_USER); %><% multilang(LANG_NAME); %>:</b>
      <select size="1" name="userMode">
      <% checkWrite("userMode"); %>
      </select>
      </td>
    </tr>
    <tr>
      <td width="20%"><font size=2><b><% multilang(LANG_OLD_PASSWORD); %>:</b></td>
      <td width="50%"><font size=2><input type="password" name="oldpass" size="20" maxlength="30"></td>
    </tr>
    <tr>
      <td width="20%"><font size=2><b><% multilang(LANG_NEW_PASSWORD); %>:</b></td>
      <td width="50%"><font size=2><input type="password" name="newpass" size="20" maxlength="30"></td>
    </tr>
    <tr>
      <td width="20%"><font size=2><b><% multilang(LANG_CONFIRMED_PASSWORD); %>:</b></td>
      <td width="50%"><font size=2><input type="password" name="confpass" size="20" maxlength="30"></td>
    </tr>
  </table>
   <input type="hidden" value="/password.asp" name="submit-url">
  <p><input type="submit" value="<% multilang(LANG_APPLY_CHANGES); %>" name="save" onClick="return saveChanges()">&nbsp;&nbsp;
  <input type="reset" value="  <% multilang(LANG_RESET); %>  " name="reset"></p>
</form>
<blockquote>
</body>
</html>


