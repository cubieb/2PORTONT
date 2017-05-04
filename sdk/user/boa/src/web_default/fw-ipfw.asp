<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang(LANG_NAT_IP_FORWARDING); %></title>
<script type="text/javascript" src="share.js">
</script>
<script>
function addClick()
{
  if (!document.formIPFwAdd.enabled.checked)
  	return true;
	
  if (document.formIPFwAdd.l_ip.value=="" && document.formIPFwAdd.r_ip.value=="" )
	return true;

  if (document.formIPFwAdd.l_ip.value=="") {	
	alert('<% multilang(LANG_EMPTY_LOCAL_IP_ADDRESS); %>');
	document.formIPFwAdd.l_ip.focus();
	return false;
  }
  if ( validateKey( document.formIPFwAdd.l_ip.value ) == 0 ) {	
	alert('<% multilang(LANG_INVALID_LOCAL_IP_ADDRESS_VALUE_); %>');
	document.formIPFwAdd.l_ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.formIPFwAdd.l_ip.value,1,0,255) ) {      
	alert('<% multilang(LANG_INVALID_LOCAL_IP_ADDRESS_RANGE_IN_1ST_DIGIT); %>');
	document.formIPFwAdd.l_ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.formIPFwAdd.l_ip.value,2,0,255) ) {      	
	alert('<% multilang(LANG_INVALID_LOCAL_IP_ADDRESS_RANGE_IN_2ND_DIGIT); %>');
	document.formIPFwAdd.l_ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.formIPFwAdd.l_ip.value,3,0,255) ) {      	
	alert('<% multilang(LANG_INVALID_LOCAL_IP_ADDRESS_RANGE_IN_3RD_DIGIT); %>');
	document.formIPFwAdd.l_ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.formIPFwAdd.l_ip.value,4,1,254) ) {      	
	alert('<% multilang(LANG_INVALID_LOCAL_IP_ADDRESS_RANGE_IN_4TH_DIGIT); %>');
	document.formIPFwAdd.l_ip.focus();
	return false;
  }

  if (document.formIPFwAdd.r_ip.value=="") {	
	alert('<% multilang(LANG_EMPTY_EXTERNAL_IP_ADDRESS); %>');
	document.formIPFwAdd.r_ip.focus();
	return false;
  }
  if ( validateKey( document.formIPFwAdd.r_ip.value ) == 0 ) {	
	alert('<% multilang(LANG_INVALID_EXTERNAL_IP_ADDRESS_VALUE_); %>');
	document.formIPFwAdd.r_ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.formIPFwAdd.r_ip.value,1,0,255) ) {      
	alert('<% multilang(LANG_INVALID_EXTERNAL_IP_ADDRESS_RANGE_IN_1ST_DIGIT); %>');
	document.formIPFwAdd.r_ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.formIPFwAdd.r_ip.value,2,0,255) ) {      	
	alert('<% multilang(LANG_INVALID_EXTERNAL_IP_ADDRESS_RANGE_IN_2ND_DIGIT); %>');
	document.formIPFwAdd.r_ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.formIPFwAdd.r_ip.value,3,0,255) ) {      	
	alert('<% multilang(LANG_INVALID_EXTERNAL_IP_ADDRESS_RANGE_IN_3RD_DIGIT); %>');
	document.formIPFwAdd.r_ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.formIPFwAdd.r_ip.value,4,1,254) ) {      
	alert('<% multilang(LANG_INVALID_EXTERNAL_IP_ADDRESS_RANGE_IN_4TH_DIGIT); %>');
	document.formIPFwAdd.r_ip.focus();
	return false;
  }

   return true;
}

function disableDelButton()
{
  if (verifyBrowser() != "ns") {
	disableButton(document.formIPFwDel.delSelEntry);
	disableButton(document.formIPFwDel.delAllEntry);
  }
}

function updateState()
{
  if (document.formIPFwAdd.enabled.checked) {
 	enableTextField(document.formIPFwAdd.l_ip);
	enableTextField(document.formIPFwAdd.r_ip);
  }
  else {
 	disableTextField(document.formIPFwAdd.l_ip);
	disableTextField(document.formIPFwAdd.r_ip);
  }
}

</script>
</head>

<body>
<blockquote>
<h2><font color="#0000FF"><% multilang(LANG_NAT_IP_FORWARDING); %></font></h2>

<table border=0 width="500" cellspacing=4 cellpadding=0>
<tr><td><font size=2>
 <% multilang(LANG_ENTRIES_IN_THIS_TABLE_ALLOW_YOU_TO_AUTOMATICALLY_REDIRECT_TRAFFIC_TO_A_SPECIFIC_MACHINE_BEHIND_THE_NAT_FIREWALL_THESE_SETTINGS_ARE_ONLY_NECESSARY_IF_YOU_WISH_TO_HOST_SOME_SORT_OF_SERVER_LIKE_A_WEB_SERVER_OR_MAIL_SERVER_ON_THE_PRIVATE_LOCAL_NETWORK_BEHIND_YOUR_GATEWAY_S_NAT_FIREWALL); %>
</font></td></tr>
<tr><td><hr size=1 noshade align=top></td></tr>
</table>

<table border=0 width="500" cellspacing=4 cellpadding=0>

<form action=/boaform/formIPFw method=POST name="formIPFwAdd">

<tr><td><font size=2><b>
   	<input type="checkbox" name="enabled" value="ON" <% checkWrite("ipFwEn"); %>
   	 ONCLICK=updateState()>&nbsp;&nbsp;<% multilang(LANG_ENABLE); %> <% multilang(LANG_NAT_IP_FORWARDING); %></b><br>
    </td>
</tr>

<tr><td>
    <font size=2><b><% multilang(LANG_LOCAL); %> <% multilang(LANG_IP_ADDRESS); %>:</b> <input type="text" name="l_ip" size="10" maxlength="15"></td>
</tr>
<tr><td>
    <font size=2><b><% multilang(LANG_EXTERNAL); %> <% multilang(LANG_IP_ADDRESS); %>:</b> <input type="text" name="r_ip" size="10" maxlength="15"></td>
</tr>
<tr><td>
  <input type="submit" value="<% multilang(LANG_APPLY_CHANGES); %>" name="addEntry" onClick="return addClick()">&nbsp;&nbsp;
  <input type="hidden" value="/fw-ipfw.asp" name="submit-url">
</td></tr>
  <script> updateState(); </script>
</form>
</table>


<br>
<form action=/boaform/formIPFw method=POST name="formIPFwDel">
<table border=0 width=500>
  <tr><font size=2><b><% multilang(LANG_CURRENT_NAT_IP_FORWARDING_TABLE); %>:</b></font></tr>
  <% ipFwList(); %>
</table>

 <br><input type="submit" value="<% multilang(LANG_DELETE_SELECTED); %>" name="delSelEntry" onClick="return deleteClick()">&nbsp;&nbsp;
     <input type="submit" value="<% multilang(LANG_DELETE_ALL); %>" name="delAllEntry" onClick="return deleteAllClick()">&nbsp;&nbsp;&nbsp;
     <input type="reset" value="<% multilang(LANG_RESET); %>" name="reset">
 <script>
   	<% checkWrite("ipFwNum"); %>
 </script>
     <input type="hidden" value="/fw-ipfw.asp" name="submit-url">
</form>

</td></tr></table>

</blockquote>
</body>
</html>

