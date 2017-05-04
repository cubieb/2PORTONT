<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title>PPTP VPN <% multilang(LANG_CONFIGURATION); %></title>
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

function pptpSelection()
{
	if (document.pptp.pptpen[0].checked) {
		document.pptp.server.disabled = true;
		document.pptp.username.disabled = true;
		document.pptp.password.disabled = true;
		document.pptp.auth.disabled = true;
		document.pptp.defaultgw.disabled = true;
		document.pptp.addPPtP.disabled = true;
		document.pptp.enctype.disabled = true;
	}
	else {
		document.pptp.server.disabled = false;
		document.pptp.username.disabled = false;
		document.pptp.password.disabled = false;
		document.pptp.auth.disabled = false;
		document.pptp.defaultgw.disabled = false;
		document.pptp.addPPtP.disabled = false;
		document.pptp.enctype.disabled = true;
	}
}


function encryClick()
{
	if (document.pptp.auth.value==3) {
		document.pptp.enctype.disabled = false;
	}else
		document.pptp.enctype.disabled = true;
}

function onClickPPtpEnable()
{
	pptpSelection();
	document.pptp.lst.value = "enable";
	document.pptp.submit();
}

function addPPtPItf()
{
	if(document.pptp.pptpen[0].checked)
		return false;
	
	if (document.pptp.server.value=="") {
		alert("<% multilang(LANG_PLEASE_ENTER_PPTP_SERVER_ADDRESS); %>");
		document.pptp.server.focus();
		return false;
	}
	
	if(!checkTextStr(document.pptp.server.value))
	{
		alert("<% multilang(LANG_INVALID_VALUE_IN_SERVER_ADDRESS); %>");
		document.pptp.server.focus();
		return false;		
	}
	
	if (document.pptp.username.value=="")
	{
		alert("<% multilang(LANG_PLEASE_ENTER_PPTP_CLIENT_USERNAME); %>");
		document.pptp.username.focus();
		return false;
	}
	if(!checkTextStr(document.pptp.username.value))
	{
		alert("<% multilang(LANG_INVALID_VALUE_IN_USERNAME); %>");
		document.pptp.username.focus();
		return false;		
	}
	if (document.pptp.password.value=="") {
		alert("<% multilang(LANG_PLEASE_ENTER_PPTP_CLIENT_PASSWORD); %>");
		document.pptp.password.focus();
		return false;
	}
	if(!checkTextStr(document.pptp.password.value))
	{
		alert("<% multilang(LANG_INVALID_VALUE_IN_PASSWORD); %>");
		document.pptp.password.focus();
		return false;		
	}

	return true;
}

</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">PPTP VPN <% multilang(LANG_CONFIGURATION); %></font></h2>

<form action=/boaform/formPPtP method=POST name="pptp">
<table border=0 width="500" cellspacing=0 cellpadding=0>
  <tr><font size=2>
    <% multilang(LANG_THIS_PAGE_IS_USED_TO_CONFIGURE_THE_PARAMETERS_FOR_PPTP_MODE_VPN); %>
  </tr>
  <tr><hr size=1 noshade align=top></tr>

  <tr>
      <td width="30%"><font size=2><b>PPTP VPN:</b></td>
      <td width="70%"><font size=2>
      	<input type="radio" value="0" name="pptpen" <% checkWrite("pptpenable0"); %> onClick="onClickPPtpEnable()"><% multilang(LANG_DISABLE); %>&nbsp;&nbsp;
     	<input type="radio" value="1" name="pptpen" <% checkWrite("pptpenable1"); %> onClick="onClickPPtpEnable()"><% multilang(LANG_ENABLE); %>
      </td>
  </tr>
</table>
<input type="hidden" id="lst" name="lst" value="">
<br>

<table border=0 width="500" cellspacing=0 cellpadding=0>
  <tr>
    <td width="30%"><font size=2><b><% multilang(LANG_SERVER); %>:</b></td>
    <td width="70%"><input type="text" name="server" size="32" maxlength="256"></td>
  </tr>
  <tr>
    <td width="30%"><font size=2></b><% multilang(LANG_USER); %><% multilang(LANG_NAME); %>:</b></td>
    <td width="70%"><input type="text" name="username" size="15" maxlength="35"></td>
  </tr>
  <tr>
    <td width="30%"><font size=2></b><% multilang(LANG_PASSWORD); %>:</b></td>
    <td width="70%"><input type="text" name="password" size="15" maxlength="35"></td>
  </tr>
  <tr>
    <td width="30%"><font size=2></b><% multilang(LANG_AUTHENTICATION); %>:</b></td>
    <td width="70%"><select name="auth" onClick="encryClick()">
      <option value="0"><% multilang(LANG_AUTO); %></option>
      <option value="1">PAP</option>
      <option value="2">CHAP</option>
      <option value="3">CHAPMSV2</option>
      </select>
    </td>
  </tr>
  <tr>
    <td width="30%"><font size=2></b><% multilang(LANG_ENCRYPTION); %>:</b></td>
    <td width="70%"><select name="enctype" >
      <option value="0"><% multilang(LANG_NONE); %></option>
      <option value="1">MPPE</option>
      <option value="2">MPPC</option>
      <option value="3">MPPE&MPPC</option>
      </select>
    </td>
  </tr>
  <tr>
    <td width="30%"><font size=2><b><% multilang(LANG_DEFAULT_GATEWAY); %>:</b></td>
    <td width="70%"><input type="checkbox" name="defaultgw"></td>
  </tr>
</table>

<table border=0 width="500" cellspacing=0 cellpadding=0>
  </tr>
      <td><input type="submit" value="<% multilang(LANG_APPLY_CHANGES); %>" name="addPPtP" onClick="return addPPtPItf()">&nbsp;&nbsp;</td>
  </tr>
</table>
<br><br>

<table border=0 width="600" cellspacing=4 cellpadding=0>
  <tr><font size=2><b>PPTP <% multilang(LANG_TABLE); %>:</b></font></tr>
  <tr>
    <td align=center width="3%" bgcolor="#808080"><font size=2><% multilang(LANG_SELECT); %></font></td>
    <td align=center width="5%" bgcolor="#808080"><font size=2><% multilang(LANG_INTERFACE); %></font></td>
    <td align=center width="5%" bgcolor="#808080"><font size=2><% multilang(LANG_SERVER); %></font></td>
    <td align=center width="8%" bgcolor="#808080"><font size=2><% multilang(LANG_ACTION); %></font></td>
  </tr>
	<% pptpList(); %>
</table>
<br>
<input type="submit" value="<% multilang(LANG_DELETE_SELECTED); %>" name="delSel" onClick="return deleteClick()">&nbsp;&nbsp;
<input type="hidden" value="/pptp.asp" name="submit-url">
<script>
	pptpSelection();
</script>
</form>
</blockquote>
</body>
</html>
