<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang(LANG_GPON_SETTINGS); %></title>
<script type="text/javascript" src="share.js">
</script>
<script>

function applyclick()
{
	/* LOID */
	if (document.formgponconf.fmgpon_loid.value=="") {		
		alert('<% multilang(LANG_LOID_CANNOT_BE_EMPTY); %>');
		document.formgponconf.fmgpon_loid.focus();
		return false;
	}
	/* LOID Password */
	if (document.formgponconf.fmgpon_loid_password.value=="") {		
		alert('<% multilang(LANG_LOID_PASSWORD_CANNOT_BE_EMPTY); %>');
		document.formgponconf.fmgpon_loid_password.focus();
		return false;
	}
	if (includeSpace(document.formgponconf.fmgpon_loid_password.value)) {		
		alert('<% multilang(LANG_CANNOT_ACCEPT_SPACE_CHARACTER_IN_LOID_PASSWORD); %>');
		document.formgponconf.fmgpon_loid_password.focus();
		return false;
	}
	if (checkString(document.formgponconf.fmgpon_loid_password.value) == 0) {		
		alert('<% multilang(LANG_INVALID_LOID_PASSWORD); %>');
		document.formgponconf.fmgpon_loid_password.focus();
		return false;
	}
	/* PLOAM Password */
	if (document.formgponconf.fmgpon_ploam_password.value=="") {		
		alert('<% multilang(LANG_PLOAM_PASSWORD_CANNOT_BE_EMPTY); %>');
		document.formgponconf.fmgpon_ploam_password.focus();
		return false;
	}
	if (includeSpace(document.formgponconf.fmgpon_ploam_password.value)) {		
		alert('<% multilang(LANG_CANNOT_ACCEPT_SPACE_CHARACTER_IN_PLOAM_PASSWORD); %>');
		document.formgponconf.fmgpon_ploam_password.focus();
		return false;
	}
	if (checkString(document.formgponconf.fmgpon_ploam_password.value) == 0) {		
		alert('<% multilang(LANG_INVALID_PLOAM_PASSWORD); %>');
		document.formgponconf.fmgpon_ploam_password.focus();
		return false;
	}
	if( document.formgponconf.fmgpon_ploam_password.value.length>10 )
	{		
		alert('<% multilang(LANG_PLOAM_PASSWORD_SHOULD_BE_10_CHARACTERS); %>');
		document.formgponconf.fmgpon_ploam_password.focus();
		return false;
	}

	return true;
}
</script>
</head>

<body>
<blockquote>
<h2><font color="#0000FF"><% multilang(LANG_GPON_SETTINGS); %></font></h2>
<form action=/boaform/admin/formgponConf method=POST name="formgponconf">
<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><td><font size=2>
    <% multilang(LANG_THIS_PAGE_IS_USED_TO_CONFIGURE_THE_PARAMETERS_FOR_YOUR_GPON_NETWORK_ACCESS); %>
  </font></td></tr>
  <tr><td><hr size=1 noshade align=top></td></tr>
</table>
<table border=0 width="500" cellspacing=4 cellpadding=0>

<tr>
      <td width="30%"><font size=2><b><% multilang(LANG_LOID); %>:</b></td>
      <td width="70%"><input type="text" name="fmgpon_loid" size="24" maxlength="24" value="<% fmgpon_checkWrite("fmgpon_loid"); %>"></td>
  </tr>
<tr>
      <td width="30%"><font size=2><b><% multilang(LANG_LOID_PASSWORD); %>:</b></td>
      <td width="70%"><input type="text" name="fmgpon_loid_password" size="12" maxlength="12" value="<% fmgpon_checkWrite("fmgpon_loid_password"); %>"></td>
  </tr>
  
  <tr>
      <td width="30%"><font size=2><b><% multilang(LANG_PLOAM_PASSWORD); %>:</b></td>
      <td width="70%"><input type="text" name="fmgpon_ploam_password" size="10" maxlength="10" value="<% fmgpon_checkWrite("fmgpon_ploam_password"); %>" </td>
  </tr>
  <tr>
      <td width="30%"><font size=2><b><% multilang(LANG_SERIAL_NUMBER); %>:</b></td>
      <td width="70%"><font size=2><% fmgpon_checkWrite("fmgpon_sn"); %></td>
  </tr>
	<% showOMCI_OLT_mode(); %>
</table>
<br>
      <input type="submit" value="<% multilang(LANG_APPLY_CHANGES); %>" name="apply" onClick="return applyclick()">&nbsp;&nbsp;
      <input type="hidden" value="/gpon.asp" name="submit-url">
</form>
</blockquote>
</body>
</html>
