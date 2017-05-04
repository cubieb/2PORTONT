<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang(LANG_EPON_SETTINGS); %></title>
<script type="text/javascript" src="share.js">
</script>
<script>
function applyclick()
{
	var mac_addr = document.formepon_llid_mac_mapping.elements["mac_addr[]"];

	for(var i=0;i<mac_addr.length;i++)
	{
		if ( (mac_addr[i].value=="") || (!mac_addr[i].value.contains(":")) || (mac_addr[i].value.length!=17))
		{				
				alert('<% multilang(LANG_INVALID_MAC_ADDRESS); %>');
				mac_addr[i].focus();
				return false;
		}
	}
	return true;
}
</script>
</head>
<body>
<blockquote>
<h2><font color="#0000FF"><% multilang(LANG_EPON_SETTINGS); %></font></h2>
<form action=/boaform/admin/formeponConf method=POST name="formeponconf">
<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><td><font size=2>
    <% multilang(LANG_THIS_PAGE_IS_USED_TO_CONFIGURE_THE_PARAMETERS_FOR_EPON_NETWORK_ACCESS); %>
  </font></td></tr>
  <tr><td><hr size=1 noshade align=top></td></tr>
</table>
<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr>
      <td width="30%"><font size=2><b><% multilang(LANG_LOID); %>:</b></td>
      <td width="70%"><input type="text" name="fmepon_loid" size="24" maxlength="24" value="<% fmepon_checkWrite("fmepon_loid"); %>"></td>
  </tr>
<tr>
      <td width="30%"><font size=2><b><% multilang(LANG_LOID_PASSWORD); %>:</b></td>
      <td width="70%"><input type="text" name="fmepon_loid_password" size="12" maxlength="12" value="<% fmepon_checkWrite("fmepon_loid_password"); %>"></td>
  </tr>

</table>
      <input type="submit" value="<% multilang(LANG_APPLY_CHANGES); %>" name="apply" onclick="return applyclick()">&nbsp;&nbsp;
      <input type="hidden" value="/epon.asp" name="submit-url">
</form>
<table border=0 width="600" cellspacing=4 cellpadding=0>
  <tr><font size=2><b><% multilang(LANG_LLID_MAC_MAPPING_TABLE); %>:</b></font></tr>
<form action=/boaform/admin/formepon_llid_mac_mapping method=POST name="formepon_llid_mac_mapping">
  <% showepon_LLID_MAC(); %> <br>
<tr><td>
      <input type="submit" value="<% multilang(LANG_APPLY_CHANGES); %>" name="apply" onClick="return applyclick()">&nbsp;&nbsp;
      <input type="hidden" value="/epon.asp" name="submit-url">
</td></tr>
</form>
</table>
</blockquote>
</body>
</html>
