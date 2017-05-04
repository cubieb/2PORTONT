<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang("20" "LANG_DOMAIN_BLOCKING"); %><% multilang("212" "LANG_CONFIGURATION"); %></title>
<script type="text/javascript" src="share.js">
</script>
<SCRIPT>

function addClick()
{
 if (document.domainblk.blkDomain.value=="") {
  alert('<% multilang("2080" "LANG_PLEASE_ENTER_THE_BLOCKED_DOMAIN"); %>');
  document.domainblk.blkDomain.focus();
  return false;
 }

 if ( document.domainblk.blkDomain.value.length == 0 ) {
  if ( !confirm('<% multilang("2446" "LANG_DOMAIN_STRING_IS_EMPTY_NPLEASE_ENTER_THE_BLOCKED_DOMAIN"); %>') ) {
   document.domainblk.blkDomain.focus();
   return false;
    }
  else
   return true;
   }

   if ( includeSpace(document.domainblk.blkDomain.value)) {
  alert('<% multilang("2081" "LANG_CANNOT_ACCEPT_SPACE_CHARACTER_IN_BLOCKED_DOMAIN_PLEASE_TRY_IT_AGAIN"); %>');
  document.domainblk.blkDomain.focus();
  return false;
  }
 if (checkString(document.domainblk.blkDomain.value) == 0) {
  alert('<% multilang("2082" "LANG_INVALID_BLOCKED_DOMAIN"); %>');
  document.domainblk.blkDomain.focus();
  return false;
 }

 return true;
}



function disableDelButton()
{
  if (verifyBrowser() != "ns") {
 disableButton(document.domainblk.delDomain);
 disableButton(document.domainblk.delAllDomain);
  }
}
</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF"><% multilang("20" "LANG_DOMAIN_BLOCKING"); %><% multilang("212" "LANG_CONFIGURATION"); %></font></h2>

<form action=/boaform/formDOMAINBLK method=POST name="domainblk">
<table border=0 width="500" cellspacing=0 cellpadding=0>
  <tr><td><font size=2>
    <% multilang("363" "LANG_THIS_PAGE_IS_USED_TO_CONFIGURE_THE_BLOCKED_DOMAIN_HERE_YOU_CAN_ADD_DELETE_THE_BLOCKED_DOMAIN"); %>
  </font></td></tr>
  <tr><td><hr size=1 noshade align=top></td></tr>
</table>
<table border=0 width="500" cellspacing=0 cellpadding=0>

  <tr>
 <td><font size=2><b><% multilang("20" "LANG_DOMAIN_BLOCKING"); %>:</b></td>
 <td><font size=2>
  <input type="radio" value="0" name="domainblkcap" <% checkWrite("domainblk-cap0"); %>><% multilang("221" "LANG_DISABLE"); %>&nbsp;&nbsp;
  <input type="radio" value="1" name="domainblkcap" <% checkWrite("domainblk-cap1"); %>><% multilang("222" "LANG_ENABLE"); %>
 </td>
 <td><input type="submit" value="<% multilang("134" "LANG_APPLY_CHANGES"); %>" name="apply">&nbsp;&nbsp;</td>
  </tr>
</table>

<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><td><hr size=1 noshade align=top></td></tr>
</table>

 <font size=2><b><% multilang("364" "LANG_DOMAIN"); %>:</b>
 <input type="text" name="blkDomain" size="15" maxlength="50">&nbsp;&nbsp;
 <input type="submit" value="<% multilang("195" "LANG_ADD"); %>" name="addDomain" onClick="return addClick()">
<br>
<br>
<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><font size=2><b><% multilang("20" "LANG_DOMAIN_BLOCKING"); %><% multilang("212" "LANG_CONFIGURATION"); %>:</b></font></tr>
  <% showDOMAINBLKTable(); %>
</table>
<br>
 <input type="submit" value="<% multilang("198" "LANG_DELETE_SELECTED"); %>" name="delDomain" onClick="return deleteClick()">&nbsp;&nbsp;
 <input type="submit" value="<% multilang("199" "LANG_DELETE_ALL"); %>" name="delAllDomain" onClick="return deleteAllClick()">&nbsp;&nbsp;&nbsp;
 <input type="hidden" value="/domainblk.asp" name="submit-url">
 <script>
  <% checkWrite("domainNum"); %>
  </script>
</form>
</blockquote>
</body>

</html>
