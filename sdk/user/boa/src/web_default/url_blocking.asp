<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title>URL <% multilang(LANG_BLOCKING); %><% multilang(LANG_CONFIGURATION); %></title>
<script type="text/javascript" src="share.js">
</script>
<SCRIPT>

function addClick()
{	
	return true;
}

function addFQDNClick()
{	
	if (document.url.urlFQDN.value=="") {
		alert("<% multilang(LANG_PLEASE_ENTER_THE_BLOCKED_FQDN); %>");
		document.url.urlFQDN.focus();
		return false;
	}
	if (document.url.urlFQDN.value.length == 0 ) {
		if (!confirm('<% multilang(LANG_FQDN_IS_EMPTY_NPLEASE_ENTER_THE_BLOCKED_FQDN); %>') ) {
			document.url.urlFQDN.focus();
			return false;
  		}
		else
			return true;
  	}
  	if (includeSpace(document.url.urlFQDN.value)) {
		alert("<% multilang(LANG_CANNOT_ACCEPT_SPACE_CHARACTER_IN_BLOCKED_FQDN_PLEASE_TRY_IT_AGAIN); %>");
		document.url.urlFQDN.focus();
		return false;
 	}
	if (checkString(document.url.urlFQDN.value) == 0) {
		alert("<% multilang(LANG_INVALID_BLOCKED_FQDN); %>");
		document.url.urlFQDN.focus();
		return false;
	}
	return true;
}

function addKeywordClick()
{	
	if (document.url.Keywd.value=="") {
		alert("<% multilang(LANG_PLEASE_ENTER_THE_BLOCKED_KEYWORD); %>");
		document.url.Keywd.focus();
		return false;
	}
	if (document.url.Keywd.value.length == 0 ) {
		if (!confirm('<% multilang(LANG_KEYWORD_IS_EMPTY_NPLEASE_ENTER_THE_BLOCKED_KEYWORD); %>') ) {
			document.url.Keywd.focus();
			return false;
  		}
		else
			return true;
  	}
  	if (includeSpace(document.url.Keywd.value)) {
		alert("<% multilang(LANG_CANNOT_ACCEPT_SPACE_CHARACTER_IN_BLOCKED_KEYWORD_PLEASE_TRY_IT_AGAIN); %>");
		document.url.Keywd.focus();
		return false;
 	}
	if (checkString(document.url.Keywd.value) == 0) {
		alert("<% multilang(LANG_INVALID_BLOCKED_KEYWORD); %>");
		document.url.Keywd.focus();
		return false;
	}
	return true;
}
	
function disableDelFQDNButton()
{
  if (verifyBrowser() != "ns") {
	disableButton(document.url.delFQDN);
	disableButton(document.url.delFAllQDN);
  }
}
	
function disableDelKeywdButton()
{
  if (verifyBrowser() != "ns") {
	disableButton(document.url.delKeywd);
	disableButton(document.url.delAllKeywd);
  }
}
</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">URL<% multilang(LANG_BLOCKING); %></font></h2>

<form action=/boaform/formURL method=POST name="url">
<table border=0 width="500" cellspacing=0 cellpadding=0>
  <tr><td><font size=2>
    <% multilang(LANG_THIS_PAGE_IS_USED_TO_CONFIGURE_THE_BLOCKED_FQDN_SUCH_AS_TW_YAHOO_COM_AND_FILTERED_KEYWORD_HERE_YOU_CAN_ADD_DELETE_FQDN_AND_FILTERED_KEYWORD); %>
  </font></td></tr>
  <tr><td><hr size=1 noshade align=top></td></tr>
</table>
<table border=0 width="500" cellspacing=0 cellpadding=0>

<tr>
	<td><font size=2><b>URL <% multilang(LANG_BLOCKING); %>:</b></td>
	<td><font size=2>
		<input type="radio" value="0" name="urlcap" <% checkWrite("url-cap0"); %>><% multilang(LANG_DISABLE); %>&nbsp;&nbsp;
		<input type="radio" value="1" name="urlcap" <% checkWrite("url-cap1"); %>><% multilang(LANG_ENABLE); %>&nbsp;&nbsp;
	</td>
	<td><input type="submit" value="<% multilang(LANG_APPLY_CHANGES); %>" name="apply">&nbsp;&nbsp;<td>
</tr>       
<tr><td colspan=3><hr size=1 noshade align=top></td></tr>
</table>
<tr>
	<td><font size=2><b><% multilang(LANG_FQDN); %>:</b></font><td>
	<td><input type="text" name="urlFQDN" size="15" maxlength="125">&nbsp;&nbsp;</td>
	<td><input type="submit" value="<% multilang(LANG_ADD); %>" name="addFQDN" onClick="return addFQDNClick()"></td>
</tr>
<br>
<br>
<table border=0 width="500" cellspacing=4 cellpadding=0>
<tr><font size=2><b>URL <% multilang(LANG_BLOCKING); %><% multilang(LANG_TABLE_2); %>:</b></font></tr>
  <% showURLTable(); %>
</table>
<br>
<table border=0 width="500" cellspacing=0 cellpadding=0>
<tr><td>
<input type="submit" value="<% multilang(LANG_DELETE_SELECTED); %>" name="delFQDN" onClick="return deleteClick()">&nbsp;&nbsp;
<input type="submit" value="<% multilang(LANG_DELETE_ALL); %>" name="delFAllQDN" onClick="return deleteAllClick()"></td>
</tr>
<tr><td><hr size=1 noshade align=top></td></tr>
</table>
 <script>
 	<% checkWrite("FQDNNum"); %>
  </script>
<tr>
	<td><font size=2><b><% multilang(LANG_KEYWORD); %>:</b></td>
	<td><input type="text" name="Keywd" size="15" maxlength="18">&nbsp;&nbsp;</td>
	<td><input type="submit" value="<% multilang(LANG_ADD); %>" name="addKeywd" onClick="return addKeywordClick()"></td>
</tr>
<br>
<br>
<table border=0 width="500" cellspacing=4 cellpadding=0>
<tr><font size=2><b><% multilang(LANG_KEYWORD_FILTERING_TABLE); %>:</b></font></tr>
  <% showKeywdTable(); %>
</table>
<br>
<input type="submit" value="<% multilang(LANG_DELETE_SELECTED); %>" name="delKeywd" onClick="return deleteClick()">&nbsp;&nbsp;
<input type="submit" value="<% multilang(LANG_DELETE_ALL); %>" name="delAllKeywd" onClick="return deleteAllClick()">&nbsp;&nbsp;&nbsp;
<input type="hidden" value="/url_blocking.asp" name="submit-url">
 <script>
 	<% checkWrite("keywdNum"); %>
  </script>
</form>
</blockquote>
</body>

</html>
