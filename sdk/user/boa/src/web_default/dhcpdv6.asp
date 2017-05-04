<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title>DHCPv6 <% multilang(LANG_SETTINGS); %></title>
<script type="text/javascript" src="share.js">
</script>
<SCRIPT>

function showDhcpv6Svr()
{
	var html;

	if (document.dhcpd.dhcpdenable[0].checked == true)
		document.getElementById('displayDhcpSvr').innerHTML=
			'<input type="submit" value="<% multilang(LANG_APPLY_CHANGES); %>" name="save">&nbsp;&nbsp;';
	else if (document.dhcpd.dhcpdenable[1].checked == true)
		document.getElementById('displayDhcpSvr').innerHTML=
			'<table border=0 width="500" cellspacing=4 cellpadding=0>'+
			'<tr><font color="#000000" size=2>'+
			'<% multilang(LANG_THIS_PAGE_IS_USED_TO_CONFIGURE_THE_UPPER_INTERFACE_SERVER_LINK_FOR_DHCPV6_RELAY); %>'+
			'</font></tr>'+
			'<tr><td><hr size=1 noshade align=top></td></tr>'+
			'</table>'+
			'<table border=0 width="500" cellspacing=4 cellpadding=0>'+
			'<tr>'+
			'<td width="30%"><font size=2><b><% multilang(LANG_UPPER_INTERFACE); %>:</b></font></td>'+
			'<td width="35%">'+
			'<select name="upper_if">'+
			'<% if_wan_list("all2"); %>'+    	                           
			'</select>'+     	
			'</td>'+
			'</tr>'+			   
			'</table>'+			
			'<input type="submit" value="<% multilang(LANG_APPLY_CHANGES); %>" name="save">&nbsp;&nbsp;';

	else if (document.dhcpd.dhcpdenable[2].checked == true) {
		html=
			'<table border=0 width="500" cellspacing=4 cellpadding=0>'+
			'<tr><font color="#000000" size=2>'+
			'<% multilang(LANG_ENABLE_THE_DHCPV6_SERVER_IF_YOU_ARE_USING_THIS_DEVICE_AS_A_DHCPV6_SERVER_THIS_PAGE_LISTS_THE_IP_ADDRESS_POOLS_AVAILABLE_TO_HOSTS_ON_YOUR_LAN_THE_DEVICE_DISTRIBUTES_NUMBERS_IN_THE_POOL_TO_HOSTS_ON_YOUR_NETWORK_AS_THEY_REQUEST_INTERNET_ACCESS); %>'+

			'</table>'+
			'<table border=0 width="500" cellspacing=4 cellpadding=0>'+
			'<tr>'+
			'<td width="30%"><font size=2><b><% multilang(LANG_IP_POOL_RANGE); %>:</b></td>';
		html+=
			'<td width="70%"><input type="text" name="dhcpRangeStart" size=40 maxlength=39 value="<% getInfo("dhcpv6s_range_start"); %>">'+
			'<font face="Arial" size="5">- <input type="text" name="dhcpRangeEnd" size=40 maxlength=39 value="<% getInfo("dhcpv6s_range_end"); %>">&nbsp;';
		html+=	'</td>'+
			'</tr>';		

		html += '<tr>'+
			'<td width="30%"><font size=2><b><% multilang(LANG_PREFIX_LENGTH); %>:</b></font></td>'+
			'<td width="70%"><font size=2>'+
			'<input type="text" name="prefix_len" size=10 maxlength=3 value="<% getInfo("dhcpv6s_prefix_length"); %>">'+
			'</td>'+
			'</tr>';
		html += '<tr>'+
			'<td width="30%"><font size=2><b><% multilang(LANG_VALID_LIFETIME); %>:</b></td>'+
			'<td width="70%"><font size=2>'+
			'<input type="text" name="Dltime" size=10 maxlength=9 value="<% getInfo("dhcpv6s_default_LTime"); %>"><b> <% multilang(LANG_SECONDS); %></b>'+
			'</td>'+
			'</tr>'+
			'<tr>'+
			'<td width="30%"><font size=2><b><% multilang(LANG_PREFERRED_LIFETIME); %>:</b></td>'+
			'<td width="70%"><font size=2>'+
			'<input type="text" name="PFtime" size=10 maxlength=9 value="<% getInfo("dhcpv6s_preferred_LTime"); %>"><b> <% multilang(LANG_SECONDS); %></b>'+
			'</td>'+
			'</tr>'+
			'<tr>'+
			'<td width="30%"><font size=2><b><% multilang(LANG_RENEW_TIME); %>:</b></td>'+
			'<td width="70%"><font size=2>'+
			'<input type="text" name="RNtime" size=10 maxlength=9 value="<% getInfo("dhcpv6s_renew_time"); %>"><b> <% multilang(LANG_SECONDS); %></b>'+
			'</td>'+
			'</tr>'+
			'<tr>'+
			'<td width="30%"><font size=2><b><% multilang(LANG_REBIND_TIME); %>:</b></td>'+
			'<td width="70%"><font size=2>'+
			'<input type="text" name="RBtime" size=10 maxlength=9 value="<% getInfo("dhcpv6s_rebind_time"); %>"><b> <% multilang(LANG_SECONDS); %></b>'+
			'</td>'+
			'</tr>'+
			'<tr>'+
			'<td width="30%"><font size=2><b><% multilang(LANG_CLIENT); %> DUID:</b></td>'+
			'<td width="70%"><font size=2>'+
			'<input type="text" name="clientID" size=42 maxlength=41 value="<% getInfo("dhcpv6s_clientID"); %>">'+
			'</td>'+
			'</tr>'+							
			'</table>'+
			'<input type="submit" value="<% multilang(LANG_APPLY_CHANGES); %>" name="save" onClick="return saveChanges()">&nbsp;&nbsp;'+

			'<tr><hr size=1 noshade align=top></tr>'+
			'<tr>'+
			'<td><font size=2><b><% multilang(LANG_DOMAIN); %>:</b></td>'+
			'<td><input type="text" name="domainStr" size="15" maxlength="50">&nbsp;&nbsp;</td>'+
			'<td><input type="submit" value="<% multilang(LANG_ADD); %>" name="addDomain">&nbsp;&nbsp;</td>'+
			'</tr>'+ 
			'<br>'+
			'<br>'+
			'<table border=0 width="300" cellspacing=4 cellpadding=0>'+
			'<tr><font size=2><b><% multilang(LANG_DOMAIN_SEARCH_TABLE); %>:</b></font></tr>'+
			<% showDhcpv6SDOMAINTable(); %>
			'</table>'+
			'<br>'+	
			'<input type="submit" value="<% multilang(LANG_DELETE_SELECTED); %>" name="delDomain">&nbsp;&nbsp;'+
			'<input type="submit" value="<% multilang(LANG_DELETE_ALL); %>" name="delAllDomain">&nbsp;&nbsp;&nbsp;'+

			'<br>'+
			'<br>'+	
			'<tr><hr size=1 noshade align=top></tr>'+

			'<tr>'+
			'<td><font size=2><b><% multilang(LANG_NAME_SERVER); %> IP:</b></td>'+
			'<td><input type="text" name="nameServerIP" size="15" maxlength="40">&nbsp;&nbsp;</td>'+
			'<td><input type="submit" value="<% multilang(LANG_ADD); %>" name="addNameServer">&nbsp;&nbsp;</td>'+
			'</tr>'+ 
			'<br>'+
			'<br>'+
			'<table border=0 width="300" cellspacing=4 cellpadding=0>'+
			'<tr><font size=2><b><% multilang(LANG_NAME_SERVER_TABLE); %>:</b></font></tr>'+
			<% showDhcpv6SNameServerTable(); %>
			'</table>'+
			'<br>'+	
			'<input type="submit" value="<% multilang(LANG_DELETE_SELECTED); %>" name="delNameServer">&nbsp;&nbsp;'+
			'<input type="submit" value="<% multilang(LANG_DELETE_ALL); %>" name="delAllNameServer">&nbsp;&nbsp;&nbsp;';

		document.getElementById('displayDhcpSvr').innerHTML=html;		
	}
	else if (document.dhcpd.dhcpdenable[3].checked == true)
		document.getElementById('displayDhcpSvr').innerHTML=
			'<tr><font color="#000000" size=2>'+
			'<% multilang(LANG_AUTO_CONFIG_BY_PREFIX_DELEGATION_FOR_DHCPV6_SERVER); %>'+
			'</tr>'+
			'<input type="submit" value="<% multilang(LANG_APPLY_CHANGES); %>" name="save">&nbsp;&nbsp;';
}

function checkDigitRange_leaseTime(str, min)
{
  d = parseInt(str, 10);
  if ( d < min || d == 0)
      	return false;
  return true;
}

function validateKey_leasetime(str)
{
   for (var i=0; i<str.length; i++) {
    if ( (str.charAt(i) >= '0' && str.charAt(i) <= '9') ||
    		(str.charAt(i) == '-' ) )
			continue;
	return 0;
  }
  return 1;
}


function saveChanges()
{
	if (document.dhcpd.dhcpRangeStart.value =="") {		
		alert('<% multilang(LANG_START_IP_ADDRESS_CANNOT_BE_EMPTY_FORMAT_IS_IPV6_ADDRESS_FOR_EXAMPLE_2000_0200_10); %>');
		document.dhcpd.dhcpRangeStart.value = document.dhcpd.dhcpRangeStart.defaultValue;
		document.dhcpd.dhcpRangeStart.focus();
		return false;
	} else {
		if ( validateKeyV6IP(document.dhcpd.dhcpRangeStart.value) == 0) {				
			alert('<% multilang(LANG_INVALID_START_IP); %>');
			document.dhcpd.dhcpRangeStart.focus();
			return false;
		}
	}
	
	if (document.dhcpd.dhcpRangeEnd.value =="") {		
		alert('<% multilang(LANG_END_IP_ADDRESS_CANNOT_BE_EMPTY_FORMAT_IS_IPV6_ADDRESS_FOR_EXAMPLE_2000_0200_20); %>');
		document.dhcpd.dhcpRangeEnd.value = document.dhcpd.dhcpRangeEnd.defaultValue;
		document.dhcpd.dhcpRangeEnd.focus();
		return false;
	} else {
		if ( validateKeyV6IP(document.dhcpd.dhcpRangeEnd.value) == 0) {				
			alert('<% multilang(LANG_INVALID_END_IP); %>');
			document.dhcpd.dhcpRangeEnd.focus();
			return false;
		}
	}
	
	if ( document.dhcpd.prefix_len.value=="") {		
		alert('<% multilang(LANG_PLEASE_INPUT_IP_PREFIX_LENGTH); %>');
		document.dhcpd.prefix_len.focus();
		return false;
	}
	if ( document.dhcpd.Dltime.value=="") {		
		alert('<% multilang(LANG_PLEASE_INPUT_DHCP_DEFAULT_LEASE_TIME); %>');
		document.dhcpd.Dltime.focus();
		return false;
	}
	if ( validateKey_leasetime( document.dhcpd.Dltime.value ) == 0 ) {		
		alert('<% multilang(LANG_INVALID_DHCP_SERVER_DEFAULT_LEASE_TIME_NUMBER); %>');
		document.dhcpd.Dltime.value = document.dhcpd.Dltime.defaultValue;
		document.dhcpd.Dltime.focus();
		return false;
	}
	if ( !checkDigitRange_leaseTime(document.dhcpd.Dltime.value, 0) ) {	  	
		alert('<% multilang(LANG_INVALID_DHCP_SERVER_DEFAULT_LEASE_TIME); %>');
		document.dhcpd.Dltime.value = document.dhcpd.Dltime.defaultValue;
		document.dhcpd.Dltime.focus();
		return false;
	}	 	
	
	if ( document.dhcpd.PFtime.value=="") {		
		alert('<% multilang(LANG_PLEASE_INPUT_DHCP_PREFERED_LIFETIME); %>');
		document.dhcpd.PFtime.focus();
		return false;
	}
	if ( validateKey_leasetime( document.dhcpd.PFtime.value ) == 0 ) {		
		alert('<% multilang(LANG_INVALID_DHCP_SERVER_PREFERED_LIFETIME_NUMBER); %>');
		document.dhcpd.PFtime.value = document.dhcpd.PFtime.defaultValue;
		document.dhcpd.PFtime.focus();
		return false;
	}
	if ( !checkDigitRange_leaseTime(document.dhcpd.PFtime.value, 0) ) {	  	
		alert('<% multilang(LANG_INVALID_DHCP_SERVER_PREFERED_LIFETIME); %>');
		document.dhcpd.PFtime.value = document.dhcpd.PFtime.defaultValue;
		document.dhcpd.PFtime.focus();
		return false;
	}
	if ( document.dhcpd.RNtime.value=="") {		
		alert('<% multilang(LANG_PLEASE_INPUT_DHCP_RENEW_TIME); %>');
		document.dhcpd.RNtime.focus();
		return false;
	}
	if ( document.dhcpd.RBtime.value=="") {
		alert('<% multilang(LANG_PLEASE_INPUT_DHCP_REBIND_TIME); %>');		
		document.dhcpd.RBtime.focus();
		return false;
	}
	if ( document.dhcpd.clientID.value=="") {		
		alert('<% multilang(LANG_PLEASE_INPUT_DHCP_CLIENT_OUID); %>');
		document.dhcpd.clientID.focus();
		return false;
	}
	
	return true;
}

function enabledhcpd()
{
	document.dhcpd.dhcpdenable[2].checked = true;
	//ip = ShowIP(document.dhcpd.lan_ip.value);
	showDhcpv6Svr();
}

function disabledhcpd()
{
	document.dhcpd.dhcpdenable[0].checked = true;
	showDhcpv6Svr();
}

function enabledhcprelay()
{
	document.dhcpd.dhcpdenable[1].checked = true;
	showDhcpv6Svr();
}

function autodhcpd()
{
	document.dhcpd.dhcpdenable[3].checked = true;
	showDhcpv6Svr();
}

</SCRIPT>
</head>

<body>
<blockquote>

<h2><font color="#0000FF">DHCPv6 <% multilang(LANG_SETTINGS); %></font></h2>

<form action=/boaform/formDhcpv6Server method=POST name="dhcpd">
<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><td><font color="#000000" size=2>
    <% multilang(LANG_THIS_PAGE_IS_USED_TO_CONFIGURE_DHCPV6_SERVER_AND_DHCPV6_RELAY); %>
  </font></td></tr>
  <tr><td><hr size=1 noshade align=top></td></tr>

</table>

<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr>
  <td><font size=2><b>DHCPv6 <% multilang(LANG_MODE); %>: </b>
  <% checkWrite("dhcpV6Mode"); %>

  </td>
  </tr>
</table>

<table border="0" width="500" cellpadding="0" cellspacing="0">
  <tr><td><hr size=2 noshade align=top></td></tr>
  <tr><td ID="displayDhcpSvr"></td></tr>
</table>

   <br>
      <input type="hidden" value="/dhcpdv6.asp" name="submit-url">

<script>
	<% initPage("dhcpv6-mode"); %>
	showDhcpv6Svr();
</script>


 </form>
</blockquote>
</body>

</html>
