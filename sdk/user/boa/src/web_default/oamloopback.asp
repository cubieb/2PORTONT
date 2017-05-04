<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang(LANG_ATM_LOOPBACK_DIAGNOSTICS); %></title>
<script type="text/javascript" src="share.js">
</script>
<SCRIPT>
function isHexDecimal(num)
{
	var string="1234567890ABCDEF";
	if (string.indexOf(num.toUpperCase()) != -1)
	{
		return true;
	}

	return false;
}

function isValidID(val)
{
	for(var i=0; i < val.length; i++)
	{
		if  ((!isHexDecimal(val.charAt(i))))
		{
			return false;
		}
	}

	return true;
}

function goClick()
{
	retval = isValidID(document.oamlb.oam_llid.value);
	if((document.oamlb.oam_llid.value=="")|| (retval==false))
	{
		alert("<% multilang(LANG_INVALID_LOOPBACK_LOCATION_ID); %>");
		document.oamlb.oam_llid.focus()
		return false
	}

	return true;
}
</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF"><% multilang(LANG_ATM_LOOPBACK_DIAGNOSTICS); %> - <% multilang(LANG_CONNECTIVITY_VERIFICATION); %></font></h2>

<form action=/boaform/formOAMLB method=POST name="oamlb">
<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><td><font size=2>
    <% multilang(LANG_CONNECTIVITY_VERIFICATION_IS_SUPPORTED_BY_THE_USE_OF_THE_ATM_OAM_LOOPBACK_CAPABILITY_FOR_BOTH_VP_AND_VC_CONNECTIONS_THIS_PAGE_IS_USED_TO_PERFORM_THE_VCC_LOOPBACK_FUNCTION_TO_CHECK_THE_CONNECTIVITY_OF_THE_VCC); %>
  </font></td></tr>
  <tr><td><hr size=1 noshade align=top></td></tr>
</table>

<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><td>
      <font size=2><b><% multilang(LANG_SELECT); %> PVC:</b></font>
        <% oamSelectList(); %>
      </td>
  </tr>
</table>

<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr>
  	<td>
		<font size=2><b><% multilang(LANG_FLOW_TYPE); %>:</b>
		</td>
		<td>
		<input type="radio" value="3" name="oam_flow"><% multilang(LANG_F4_SEGMENT); %>&nbsp;&nbsp;&nbsp;&nbsp;
		<input type="radio" value="4" name="oam_flow" ><% multilang(LANG_F4_END_TO_END); %>
		</td>
  </tr>
  <tr>
  	<td>&nbsp;</td>
		<td>
		<input type="radio" value="0" name="oam_flow" checked><% multilang(LANG_F5_SEGMENT); %>&nbsp;&nbsp;&nbsp;&nbsp;
		<input type="radio" value="1" name="oam_flow" ><% multilang(LANG_F5_END_TO_END); %>
		</td>
  </tr>

  <tr><td colspan=2>
      <font size=2><b><% multilang(LANG_LOOPBACK_LOCATION_ID); %>: </b>
      <input type="text" name="oam_llid" value="FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF" size=40 maxlength=32 onFocus="this.select()">
      </td>
  </tr>
</table>
  <br>
      <input type="submit" value=" <% multilang(LANG_GO); %> ! " name="go" onClick="return goClick()">
      <input type="hidden" value="/oamloopback.asp" name="submit-url">
</form>
</blockquote>
</body>

</html>
