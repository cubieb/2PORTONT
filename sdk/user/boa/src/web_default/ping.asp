<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title>Ping <% multilang(LANG_DIAGNOSTICS); %></title>
<script type="text/javascript" src="share.js">
</script>
<SCRIPT>
function goClick()
{
	if (!checkHostIP(document.ping.pingAddr, 1))
		return false;
/*	if (document.ping.pingAddr.value=="") {
		alert("Enter host address !");
		document.ping.pingAddr.focus();
		return false;
	}
	
	return true;*/
}
</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">Ping <% multilang(LANG_DIAGNOSTICS); %></font></h2>

<form action=/boaform/formPing method=POST name="ping">
<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><td><font size=2>
    <% multilang(LANG_PAGE_DESC_ICMP_DIAGNOSTIC); %>
  </font></td></tr>
  <tr><td><hr size=1 noshade align=top></td></tr>
</table>
<table border=0 width="500" cellspacing=4 cellpadding=0>

  <tr>
      <td width="30%"><font size=2><b><% multilang(LANG_HOST_ADDRESS); %>: </b></td>
      <td width="70%"><input type="text" name="pingAddr" size="15" maxlength="30"></td>
  </tr>

</table>
  <br>
      <input type="submit" value=" <% multilang(LANG_GO); %>" name="go" onClick="return goClick()">
      <input type="hidden" value="/ping.asp" name="submit-url">
 </form>
</blockquote>
</body>

</html>
