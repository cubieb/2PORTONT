<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title>IPIP VPN <% multilang("212" "LANG_CONFIGURATION"); %></title>
<script type="text/javascript" src="share.js">
</script>
<SCRIPT>
function checkTextStr(str)
{
 for (var i=0; i<str.length; i++)
 {
  if ( str.charAt(i) == '%' || str.charAt(i) =='&' ||str.charAt(i) =='\\' || str.charAt(i) =='?' || str.charAt(i)=='"')
   return false;
 }
 return true;
}

function ipipSelection()
{
 if (document.ipip.ipipen[0].checked) {
  document.ipip.remote.disabled = true;
  document.ipip.local.disabled = true;
  document.ipip.defaultgw.disabled = true;
  document.ipip.addIPIP.disabled = true;
 }
 else {
  document.ipip.remote.disabled = false;
  document.ipip.local.disabled = false;
  document.ipip.defaultgw.disabled = false;
  document.ipip.addIPIP.disabled = false;
 }
}

function onClickIPIPEnable()
{
 ipipSelection();

 if (document.ipip.ipipen[0].checked)
  document.ipip.lst.value = "disable";
 else
  document.ipip.lst.value = "enable";

 document.ipip.submit();
}

function addIPIPItf()
{
 if (document.ipip.ipipen[0].checked)
  return false;
 /*
	if (document.ipip.tun_name.value=="") {
		alert("Please enter tunnel name!");
		document.ipip.tun_name.focus();
		return false;
	}

	// tunnel name can not begin with "p". which is for ppp connection.
	if (!checkTextStr(document.ipip.tun_name.value) || (document.ipip.tun_name.value.charAt(0) == 'p'))
	{
		alert("Invalid value in tunnel name!");
		document.ipip.tun_name.focus();
		return false;

	}
	*/

 if (!checkHostIP(document.ipip.remote, 1))
 {
  alert('<% multilang("2152" "LANG_INVALID_VALUE_IN_REMOTE_ADDRESS"); %>');
  document.ipip.remote.focus();
  return false;
 }

 if (!checkHostIP(document.ipip.local, 1))
 {
  alert('<% multilang("2153" "LANG_INVALID_VALUE_IN_LOCAL_ADDRESS"); %>');
  document.ipip.local.focus();
  return false;
 }

 return true;
}

</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">IPIP VPN <% multilang("212" "LANG_CONFIGURATION"); %></font></h2>

<form action=/boaform/formIPIP method=POST name="ipip">
<table border=0 width="500" cellspacing=0 cellpadding=0>
  <tr><font size=2>
    <% multilang("636" "LANG_THIS_PAGE_IS_USED_TO_CONFIGURE_THE_PARAMETERS_FOR_IPIP_MODE_VPN"); %>
  </tr>
  <tr><hr size=1 noshade align=top></tr>

  <tr>
      <td width="40%"><font size=2><b>IPIP VPN:</b></td>
      <td width="60%"><font size=2>
       <input type="radio" value="0" name="ipipen" <% checkWrite("ipipenable0"); %> onClick="onClickIPIPEnable()"><% multilang("221" "LANG_DISABLE"); %>&nbsp;&nbsp;
      <input type="radio" value="1" name="ipipen" <% checkWrite("ipipenable1"); %> onClick="onClickIPIPEnable()"><% multilang("222" "LANG_ENABLE"); %>
      </td>
  </tr>
</table>
<input type="hidden" id="lst" name="lst" value="">
<br>

<table border=0 width="500" cellspacing=0 cellpadding=0>
  <tr>
    <td width="40%"><font size=2><b><% multilang("256" "LANG_LOCAL"); %> <% multilang("83" "LANG_IP_ADDRESS"); %>:</b></td>
    <td width="60%"><input type="text" name="local" size="20" maxlength="30"></td>
  </tr>
  <tr>
    <td width="40%"><font size=2><b><% multilang("277" "LANG_REMOTE"); %> <% multilang("83" "LANG_IP_ADDRESS"); %>:</b></td>
    <td width="60%"><input type="text" name="remote" size="20" maxlength="30"></td>
  </tr>
  <tr>
    <td width="40%"><font size=2><b><% multilang("79" "LANG_DEFAULT_GATEWAY"); %>:</b></td>
    <td width="60%"><input type="checkbox" name="defaultgw"></td>
</table>

<table border=0 width="500" cellspacing=0 cellpadding=0>
  </tr>
      <td><input type="submit" value="<% multilang("134" "LANG_APPLY_CHANGES"); %>" name="addIPIP" onClick="return addIPIPItf()">&nbsp;&nbsp;</td>
  </tr>
</table>
<br><br>

<table border=0 width="600" cellspacing=4 cellpadding=0>
  <tr><font size=2><b>IPIP <% multilang("275" "LANG_TABLE"); %>:</b></font></tr>
  <tr>
    <td align=center width="3%" bgcolor="#808080"><font size=2><% multilang("200" "LANG_SELECT"); %></font></td>
    <td align=center width="5%" bgcolor="#808080"><font size=2><% multilang("619" "LANG_NAME"); %></font></td>
    <td align=center width="5%" bgcolor="#808080"><font size=2><% multilang("256" "LANG_LOCAL"); %></font></td>
    <td align=center width="5%" bgcolor="#808080"><font size=2><% multilang("277" "LANG_REMOTE"); %></font></td>
    <td align=center width="5%" bgcolor="#808080"><font size=2><% multilang("79" "LANG_DEFAULT_GATEWAY"); %></font></td>
    <td align=center width="5%" bgcolor="#808080"><font size=2><% multilang("629" "LANG_ACTION"); %></font></td>
  </tr>
 <% ipipList(); %>
</table>
<br>
<input type="submit" value="Delete Selected" name="delSel" onClick="return deleteClick()">&nbsp;&nbsp;
<input type="hidden" value="/ipip.asp" name="submit-url">
<script>
 ipipSelection();
</script>
</form>
</blockquote>
</body>
</html>
