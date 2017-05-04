<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<META HTTP-EQUIV=Refresh CONTENT="60; URL=ddns.asp">
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang("316" "LANG_DYNAMIC"); %> DNS <% multilang("212" "LANG_CONFIGURATION"); %></title>
<script type="text/javascript" src="share.js">
</script>
<SCRIPT>
selected=0;

function deSelected()
{
 /*
	if (document.ddns.select) {
		var len = document.ddns.select.length;
		if (len == undefined)
			document.ddns.select.checked = false;
		else {
			for (var i=0; i < len; i++)
				document.ddns.select[i].checked = false;
		}
	}
	*/
}

function addClick()
{
 if (document.ddns.hostname.value=="") {
  alert('<% multilang("1997" "LANG_PLEASE_ENTER_HOSTNAME_FOR_THIS_ACCOUNT"); %>');
  document.ddns.hostname.focus();
  deSelected();
  return false;
 }
 if (includeSpace(document.ddns.hostname.value)) {
  alert('<% multilang("1998" "LANG_INVALID_HOST_NAME"); %>');
  document.ddns.hostname.focus();
  return false;
 }
 if (checkString(document.ddns.hostname.value) == 0) {
  alert('<% multilang("1998" "LANG_INVALID_HOST_NAME"); %>');
  document.ddns.hostname.focus();
  return false;
 }

 if (document.ddns.ddnsProv.value=="0") {
  if (document.ddns.username.value=="") {
   alert('<% multilang("1999" "LANG_PLEASE_ENTER_USERNAME_FOR_THIS_ACCOUNT"); %>');
   document.ddns.username.focus();
   deSelected();
   return false;
  }
  if (includeSpace(document.ddns.username.value)) {
   alert('<% multilang("1976" "LANG_INVALID_USER_NAME"); %>');
   document.ddns.username.focus();
   return false;
  }
  if (checkString(document.ddns.username.value) == 0) {
   alert('<% multilang("1976" "LANG_INVALID_USER_NAME"); %>');
   document.ddns.username.focus();
   return false;
  }

  if (document.ddns.password.value=="") {
   alert('<% multilang("2000" "LANG_PLEASE_ENTER_PASSWORD_FOR_THIS_ACCOUNT"); %>');
   document.ddns.password.focus();
   deSelected();
   return false;
  }
    if ( includeSpace(document.ddns.password.value)) {
   alert('<% multilang("1978" "LANG_INVALID_PASSWORD"); %>');
   document.ddns.password.focus();
   return false;
   }
  if (checkString(document.ddns.password.value) == 0) {
   alert('<% multilang("1978" "LANG_INVALID_PASSWORD"); %>');
   document.ddns.password.focus();
   return false;
  }
 }

 if (document.ddns.ddnsProv.value=="1") {
  if (document.ddns.email.value=="") {
   alert('<% multilang("2001" "LANG_PLEASE_ENTER_EMAIL_FOR_THIS_ACCOUNT"); %>');
   document.ddns.email.focus();
   deSelected();
   return false;
  }
  if (includeSpace(document.ddns.email.value)) {
   alert('<% multilang("2002" "LANG_INVALID_EMAIL"); %>');
   document.ddns.email.focus();
   return false;
  }
  if (checkString(document.ddns.email.value) == 0) {
   alert('<% multilang("2002" "LANG_INVALID_EMAIL"); %>');
   document.ddns.email.focus();
   return false;
  }

  if (document.ddns.key.value=="") {
   alert('<% multilang("2003" "LANG_PLEASE_ENTER_KEY_FOR_THIS_ACCOUNT"); %>');
   document.ddns.key.focus();
   deSelected();
          return false;
         }
  if (includeSpace(document.ddns.key.value)) {
   alert('<% multilang("2004" "LANG_INVALID_KEY"); %>');
   document.ddns.key.focus();
   return false;
  }
  if (checkString(document.ddns.key.value) == 0) {
   alert('<% multilang("2004" "LANG_INVALID_KEY"); %>');
   document.ddns.key.focus();
   return false;
  }
 }

 return true;
}

function modifyClick()
{
 if (!selected) {
  alert('<% multilang("2005" "LANG_PLEASE_SELECT_AN_ENTRY_TO_MODIFY"); %>');
  return false;
 }
 return addClick();
}

function removeClick()
{
 if (!selected) {
  alert('<% multilang("2006" "LANG_PLEASE_SELECT_AN_ENTRY_TO_DELETE"); %>');
  return false;
 }
 return true;
}

function updateState()
{
  if (document.ddns.ddnsProv.value=="0" || document.ddns.ddnsProv.value=="2") {
  enableTextField(document.ddns.username);
  enableTextField(document.ddns.password);
  disableTextField(document.ddns.email)
  disableTextField(document.ddns.key)
  }
  else {
   enableTextField(document.ddns.email);
   enableTextField(document.ddns.key);
  disableTextField(document.ddns.username);
  disableTextField(document.ddns.password);
  }
}

function postEntry(enabled, pvd, host, user, passwd, intf)
{
 if (enabled)
  document.ddns.enable.checked = true;
 else
  document.ddns.enable.checked = false;
 document.ddns.hostname.value = host;
 if (pvd == 'dyndns') {
  document.ddns.ddnsProv.value = 0;
  document.ddns.username.value = user;
  document.ddns.password.value = passwd;
  document.ddns.email.value = '';
  document.ddns.key.value = '';
  document.ddns.interface.value = intf;
 }
 else {
  document.ddns.ddnsProv.value = 1;
  document.ddns.username.value = '';
  document.ddns.password.value = '';
  document.ddns.email.value = user;
  document.ddns.key.value = passwd;
  document.ddns.interface.value = intf;
 }
 updateState();
 selected = 1;
}

</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF"><% multilang("316" "LANG_DYNAMIC"); %> DNS <% multilang("212" "LANG_CONFIGURATION"); %></font></h2>

<form action=/boaform/admin/formDDNS method=POST name="ddns">
<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><td><font size=2>
    <% multilang("317" "LANG_PAGE_DESC_CONFIGURE_DYNAMIC_DNS_ADDRESS"); %>
  </font></td></tr>
  <tr><td><hr size=1 noshade align=top></td></tr>
</table>
<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr>
      <td width="30%"><font size=2><b><% multilang("222" "LANG_ENABLE"); %>:</b></td>
      <td width="70%"><input type="checkbox" name="enable" value="1" checked></td>
  </tr>

  <tr>
      <td width="30%"><font size=2><b>DDNS <% multilang("318" "LANG_PROVIDER"); %>:</b></td>
      <td width="70%">
      <select size="1" name="ddnsProv" onChange='updateState()'>
      <option selected value="0">DynDNS.org</option>
      <option value="1">TZO</option>
      <option value="2">No-IP</option>
      </select>
      </td>
  </tr>

  <tr>
      <td width="30%"><font size=2><b><% multilang("319" "LANG_HOSTNAME"); %>:</b></td>
      <td width="70%"><input type="text" name="hostname" size="35" maxlength="35"></td>
  </tr>

  <tr><td width="30%"><font size=2><b><% multilang("66" "LANG_INTERFACE"); %></b></td>
  <td width="35%">
   <select name="interface" >
      <% if_wan_list("rt");
      %>
      <!--<option value=100>LAN/br0</option>-->
   </select>
  </td></tr>

</table>

<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><td><hr size=1 noshade align=top></td></tr>
  <tr><font size=2><b>DynDns/No-IP <% multilang("320" "LANG_SETTINGS"); %>:</b></font></tr>
  <tr>
      <td width="30%"><font size=2><b><% multilang("751" "LANG_USER"); %><% multilang("619" "LANG_NAME"); %>:</b></td>
      <td width="70%"><input type="text" name="username" size="35" maxlength="35"></td>
  </tr>

  <tr>
      <td width="30%"><font size=2><b><% multilang("63" "LANG_PASSWORD"); %>:</b></td>
      <td width="70%"><input type="password" name="password" size="35" maxlength="35"></td>
  </tr>
</table>

<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><td><hr size=1 noshade align=top></td></tr>
  <tr><font size=2><b>TZO <% multilang("320" "LANG_SETTINGS"); %>:</b></font></tr>
  <tr>
      <td width="30%"><font size=2><b><% multilang("321" "LANG_EMAIL"); %>:</b></td>
      <td width="70%"><input type="text" name="email" size="35" maxlength="35"></td>
  </tr>

  <tr>
      <td width="30%"><font size=2><b><% multilang("217" "LANG_KEY"); %>:</b></td>
      <td width="70%"><input type="password" name="key" size="35" maxlength="35"></td>
  </tr>
</table>

  <br><br>
  <!--<input type="hidden" name="interface" value="all">-->
  <input type="submit" value="<% multilang("195" "LANG_ADD"); %>" name="addacc" onClick="return addClick()">
  <input type="submit" value="<% multilang("276" "LANG_MODIFY"); %>" name="modify" onClick="return modifyClick()">
  <input type="submit" value="<% multilang("322" "LANG_REMOVE"); %>" name="delacc" onClick="return removeClick()">
  </tr>

  <br><br>

<table border=0 width="800" cellspacing=4 cellpadding=0>
  <tr><font size=2><b><% multilang("316" "LANG_DYNAMIC"); %> DNS <% multilang("359" "LANG__TABLE"); %>:</b></font></tr>
  <% showDNSTable(); %>
</table>
  <br>
      <input type="hidden" value="/ddns.asp" name="submit-url">

  <script>
 updateState();
 deSelected();
  </script>
</form>
</blockquote>
</body>

</html>
