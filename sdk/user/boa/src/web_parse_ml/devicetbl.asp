<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang("4" "LANG_DEVICE"); %> <% multilang("668" "LANG_IP_RANGE"); %><% multilang("1061" "LANG_TABLE_2"); %></title>
<script type="text/javascript" src="share.js">
</script>
<SCRIPT>
var pool_ipprefix;
<% initPage("dhcp-iprange"); %>

function postsetting(devname,devicetype, startip, endip, gwip,option,optCode,optStr)
{
 document.ipRange.name.value = devname;
 document.ipRange.sIp.value = startip;
 document.ipRange.dIp.value = endip;
 document.ipRange.gwIp.value = gwip;
 document.ipRange.Opt60.value = option;
 document.ipRange.dhcpDeviceType.value = devicetype;

 deviceTypeSelect();
 if(document.ipRange.dhcpDeviceType.selectedIndex==3) //STB
 {
  document.ipRange.OptStr240.value = optStr;
 }
 else if(document.ipRange.dhcpDeviceType.selectedIndex!=5) //PC Phone Camera HGW ...
 {
  document.ipRange.reservedOption.value = optCode;
  document.ipRange.OptStr.value = optStr;
 }
}

function checkInputIP(client)
{
 var pool_ip, mask;
 var i, mask_d, ip_d, pool_d;
 var pool_ip_d, client_d;

 if (pool_ipprefix) {
  pool_ip = document.ipRange.lan_ip.value;
  mask = document.ipRange.lan_mask.value;
 }
 else {
  pool_ip = document.ipRange.lan_dhcpRangeStart.value;
  mask = document.ipRange.lan_dhcpSubnetMask.value;
 }

 for( i=1;i<5;i++ ) {
  mask_d = getDigit(mask, i);
  pool_ip_d = getDigit(pool_ip, i);
  client_d = getDigit(client, i);

  if( (pool_ip_d & mask_d) != (client_d & mask_d ) ) {
   return false;
  }
 }

 if (pool_ipprefix) {
  if( (parseInt(document.ipRange.lan_dhcpRangeStart.value, 10) > client_d) ||
   (parseInt(document.ipRange.lan_dhcpRangeEnd.value, 10) < client_d) ) {
   return false;
  }
 }

 return true;
}

function deviceTypeSelect()
{
 if(document.ipRange.dhcpDeviceType.selectedIndex==3) //STB
 {
  if (document.getElementById) // DOM3 = IE5, NS6
  {
   document.getElementById('option240').style.display = 'block';
   document.getElementById('option241').style.display = 'none';
  }
  else {
   if (document.layers == false) // IE4
   {
    document.all.option240.style.display = 'block';
    document.all.option241.style.display = 'none';
   }
  }
 }
 else if(document.ipRange.dhcpDeviceType.selectedIndex==5)//Unkown 
 {
  if (document.getElementById) // DOM3 = IE5, NS6
  {
   document.getElementById('option240').style.display = 'none';
   document.getElementById('option241').style.display = 'none';
  }
  else {
   if (document.layers == false) // IE4
   {
    document.all.option240.style.display = 'none';
    document.all.option241.style.display = 'none';
   }
  }
 }
 else //other device
 {
  if (document.getElementById) // DOM3 = IE5, NS6
  {
   document.getElementById('option240').style.display = 'none';
   document.getElementById('option241').style.display = 'block';
  }
  else {
   if (document.layers == false) // IE4
   {
    document.all.option240.style.display = 'none';
    document.all.option241.style.display = 'block';
   }
  }
 }
}

function addClick()
{
 if (document.ipRange.name.value=="") {
  alert('<% multilang("2007" "LANG_DEVICE_NAME_CANNOT_BE_NULL"); %>');
  document.ipRange.name.focus();
  return false;
 }

 if (checkString(document.ipRange.name.value) == 0) {
  alert('<% multilang("2008" "LANG_INVALID_DEVICE_NAME"); %>');
  document.ipRange.name.focus();
  return false;
 }
 if (checkPrintableString(document.ipRange.Opt60.value) == 0) {
  alert('<% multilang("2009" "LANG_INVALID_OPTION60_STRING"); %>');
  document.ipRange.Opt60.focus();
  return false;
 }
 if (checkPrintableString(document.ipRange.OptStr.value) == 0) {
  alert('<% multilang("2010" "LANG_INVALID_OPTION_STRING"); %>');
  document.ipRange.OptStr.focus();
  return false;
 }
 if (!checkHostIP(document.ipRange.sIp, 1)) {
  document.ipRange.sIp.focus();
  return false;
 }

 if (!checkHostIP(document.ipRange.dIp, 1)) {
  document.ipRange.dIp.focus();
  return false;
 }

 if (document.ipRange.Opt60.value=="") {
  alert('<% multilang("2011" "LANG_OPTION60_CANNOT_BE_NULL"); %>');
  document.ipRange.Opt60.focus();
  return false;
 }

 if ( !checkInputIP(document.ipRange.sIp.value ) ) {
  alert('<% multilang("2012" "LANG_INVALID_SOURCE_RANGE_OF_IP_ADDRESS_IT_SHOULD_BE_IN_IP_POOL_RANGE"); %>');
  document.ipRange.sIp.focus();
  return false;
 }

 if ( !checkInputIP(document.ipRange.dIp.value ) ) {
  alert('<% multilang("2013" "LANG_INVALID_DESTINATION_RANGE_OF_IP_ADDRESS_IT_SHOULD_BE_IN_IP_POOL_RANGE"); %>');
  document.ipRange.dIp.focus();
  return false;
 }

 if(document.ipRange.dhcpDeviceType.selectedIndex==3) {
  if (document.ipRange.stbdns1.value == "") {
   alert('<% multilang("2014" "LANG_ENTER_STBS_DNS_VALUE"); %>');
   document.ipRange.stbdns1.focus();
   return false;
  }
  if (!checkIP(document.ipRange.stbdns1)) {
   document.ipRange.stbdns1.focus();
   return false;
  }
  if (document.ipRange.stbdns2.value != "") {
   if (!checkIP(document.ipRange.stbdns2)) {
    document.ipRange.stbdns2.focus();
    return false;
   }
  }
  if (document.ipRange.opchaddr.value == "") {
   alert('<% multilang("2015" "LANG_ENTER_STBS_OPCH_ADDRESS_VALUE"); %>');
   document.ipRange.opchaddr.focus();
   return false;
  }
  if (!checkIP(document.ipRange.opchaddr)) {
   document.ipRange.opchaddr.focus();
   return false;
  }
  if (document.ipRange.opchport.value == "") {
   alert('<% multilang("2016" "LANG_ENTER_STBS_OPCH_PORT_VALUE"); %>');
   document.ipRange.opchport.focus();
   return false;
  }
  if ( !checkDigitRange(document.ipRange.opchport.value,1,1,65535) ) {
   alert('<% multilang("2017" "LANG_INVALID_OPCH_PORT_IT_SHOULD_BE_1_65535"); %>');
   document.ipRange.opchport.focus();
   return false;
  }
 }
 else if(document.ipRange.dhcpDeviceType.selectedIndex!=5)
 {
  if(document.ipRange.OptStr.value == "") {
   alert('<% multilang("2018" "LANG_ENTER_OPTION_STRING_VALUE"); %>');
   document.ipRange.OptStr.focus();
   return false;
  }
 }
 return true;
}

</SCRIPT>
</head>


<body>
<blockquote>
<h2><font color="#0000FF"><% multilang("4" "LANG_DEVICE"); %> <% multilang("668" "LANG_IP_RANGE"); %><% multilang("1061" "LANG_TABLE_2"); %></font></h2>

<table border=0 width="500" cellspacing=0 cellpadding=0>
  <tr><td><font size=2>
 <% multilang("755" "LANG_THIS_PAGE_IS_USED_TO_CONFIGURE_THE_IP_ADDRESS_RANGE_BASED_ON_DEVICE_TYPE"); %>
  </font></td></tr>
  <tr><td><hr size=1 noshade align=top></td></tr>
</table>

<form action=/boaform/formIpRange method=POST name="ipRange">

<input type="hidden" name="lan_ip" value=<% getInfo("dhcplan-ip"); %>>
<input type="hidden" name="lan_mask" value=<% getInfo("dhcplan-subnet"); %>>
<input type="hidden" name="lan_dhcpRangeStart" value=<% getInfo("lan-dhcpRangeStart"); %>>
<input type="hidden" name="lan_dhcpRangeEnd" value=<% getInfo("lan-dhcpRangeEnd"); %>>
<input type="hidden" name="lan_dhcpSubnetMask" value=<% getInfo("lan-dhcpSubnetMask"); %>>
<table class="flat" border="0" cellpadding="0" cellspacing="0" width=500>
<tr>
    <td width=30%><font size=2><% multilang("4" "LANG_DEVICE"); %><% multilang("619" "LANG_NAME"); %>:&nbsp;&nbsp;</td>
    <td><input type="text" name="name" size="20" maxlength="20"></td>
</tr>
    <td><font size=2><% multilang("756" "LANG_START_ADDRESS"); %>:&nbsp;&nbsp;</td>
    <td><input type="text" name="sIp" size="20" maxlength="15"></td>
</tr>
<tr>
    <td><font size=2><% multilang("757" "LANG_END_ADDRESS"); %>:&nbsp;&nbsp;</td>
    <td><input type="text" name="dIp" size="20" maxlength="15"></td>
</tr>
<tr>
    <td><font size=2><% multilang("304" "LANG_GATEWAY_ADDRESS"); %>:&nbsp;&nbsp;</td>
    <td><input type="text" name="gwIp" size="20" maxlength="15"></td>
</tr>
<tr>
    <td><font size=2><% multilang("758" "LANG_OPTION60"); %>:&nbsp;&nbsp;</td>
    <td><input type="text" name="Opt60" size="20" maxlength="100"></td>
</tr>
<!--<tr>
    <td><font size=2>STB:&nbsp;&nbsp;</td>
    <td><input type="checkbox" name="isSTB" value="ON"></td>
</tr> -->
<tr>
 <td width=30%><font size=2><% multilang("759" "LANG_DEVICE_TYPE"); %>:&nbsp;&nbsp;</td>
 <td><select size="1" name="dhcpDeviceType" onChange="deviceTypeSelect()">
  <option selected value=0><% multilang("760" "LANG_PC"); %></option>
  <option value=1><% multilang("761" "LANG_CAMERA"); %></option>
  <option value=2><% multilang("762" "LANG_HGW"); %></option>
  <option value=3><% multilang("763" "LANG_STB"); %></option>
  <option value=4><% multilang("764" "LANG_PHONE"); %></option>
  <option value=100><% multilang("765" "LANG_UNKNOWN"); %></option></select>
 </td>
</tr>
</table>

<div id="option241" style="display:block">
<table border=0 width="500" cellspacing=0 cellpadding=0>
  <tr>
  <td width=30%><font size=2><% multilang("766" "LANG_RESERVED_OPTION"); %>:&nbsp;&nbsp;</td>
  <td><select size="1" name="reservedOption">
   <option selected value=241>Option 241</option>
   <option value=242>Option 242</option>
   <option value=243>Option 243</option>
   <option value=244>Option 244</option>
   <option value=245>Option 245</option></select>
  </td>
  </tr>
  <tr>
  <td><font size=2><% multilang("767" "LANG_OPTION_STRING"); %></td>
  <td><input type="text" name="OptStr" size="20" maxlength="100"></td>
  </tr>
</table>
</div>

<div id="option240" style="display:none">
<table border=0 width="500" cellspacing=0 cellpadding=0>
 <tr>
  <td width=30%><font size=2><% multilang("766" "LANG_RESERVED_OPTION"); %>:&nbsp;&nbsp;</td>
  <td><input type="text" name="reservedOption240" size="20" maxlength="20" value="Option 240" readonly></td>
  </tr>
  <tr>
  <td><font size=2><% multilang("767" "LANG_OPTION_STRING"); %>:&nbsp;&nbsp;</td>
  <td><input type="text" name="OptStr240" size="20" maxlength="100"></td>
  </tr>

  <tr>
   <td><font size=2><% multilang("768" "LANG_DNS1"); %>:</td>
   <td><input type="text" name="stbdns1" value=<% getInfo("stb-dns1"); %>></td>
  </tr>
  <tr>
   <td><font size=2><% multilang("769" "LANG_DNS2"); %>:</td>
   <td><input type="text" name="stbdns2" value=<% getInfo("stb-dns2"); %>></td>
  </tr>
</table>
</div>

<input type="submit" value="<% multilang("195" "LANG_ADD"); %>" name="add" onClick="return addClick()">&nbsp;&nbsp;
<input type="submit" value="<% multilang("254" "LANG_DELETE"); %>" name="del">&nbsp;&nbsp;
<input type="submit" value="<% multilang("276" "LANG_MODIFY"); %>" name="mod" onClick="return addClick()">&nbsp;&nbsp
<input type="hidden" value="/devicetbl.asp" name="submit-url">
<input type="button" value="<% multilang("659" "LANG_CLOSE"); %>" name="close" onClick="javascript: window.close();">

<p>
<tr><hr size=1 noshade align=top></tr>
<table border=0 width="500" cellspacing=4 cellpadding=0>
<tr><font size=2><b><% multilang("4" "LANG_DEVICE"); %> <% multilang("668" "LANG_IP_RANGE"); %><% multilang("1061" "LANG_TABLE_2"); %>:</b></font></tr>
    <% showDeviceIpTable(); %>
</table>

<script>
</script>
</form>
</blockquote>
</body>

</html>
