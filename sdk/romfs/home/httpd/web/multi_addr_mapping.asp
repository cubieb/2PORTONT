<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang("564" "LANG_ADDRESS_MAPPING_RULE"); %><% multilang("212" "LANG_CONFIGURATION"); %></title>
<script type="text/javascript" src="share.js">
</script>
<SCRIPT>


function addClick()
{
 var ls, le, gs, ge;
 var ls1, le1, gs1, ge1;
 var ls2, le2, gs2, ge2;
 var ls3, le3, gs3, ge3;

 ls = getDigit(document.addressMap.lsip.value,4);
   le = getDigit(document.addressMap.leip.value,4);
   gs = getDigit(document.addressMap.gsip.value,4);
   ge = getDigit(document.addressMap.geip.value,4);

   ls1 = getDigit(document.addressMap.lsip.value,1);
   le1 = getDigit(document.addressMap.leip.value,1);
   gs1 = getDigit(document.addressMap.gsip.value,1);
   ge1 = getDigit(document.addressMap.geip.value,1);

   ls2 = getDigit(document.addressMap.lsip.value,2);
   le2 = getDigit(document.addressMap.leip.value,2);
   gs2 = getDigit(document.addressMap.gsip.value,2);
   ge2 = getDigit(document.addressMap.geip.value,2);

   ls3 = getDigit(document.addressMap.lsip.value,3);
   le3 = getDigit(document.addressMap.leip.value,3);
   gs3 = getDigit(document.addressMap.gsip.value,3);
   ge3 = getDigit(document.addressMap.geip.value,3);

  if ( document.addressMap.addressMapType.selectedIndex == 0 ) {
  //alert('You select One-to-One.');  		
    if ( !checkIP(document.addressMap.lsip)) {
     return false;
    }

    if ( !checkIP(document.addressMap.gsip)) {
     return false;
    }

   } else if ( document.addressMap.addressMapType.selectedIndex == 1 ) {
  //alert('You select Many-to-One.');  		
    if ( !checkIP(document.addressMap.lsip)) {
     return false;
    }

    if ( !checkIP(document.addressMap.leip)) {
     return false;
    }

    if ( !checkIP(document.addressMap.gsip)) {
     return false;
    }

    if ( ls1 != le1 || ls2 != le2 || ls3 != le3 ) {
     alert("<% multilang("2248" "LANG_LOCAL_START_IP_DOMAIN_IS_DIFFERENT_FROM_END_IP"); %>");
     document.addressMap.lsip.focus();
     return false;
    }

    if ( le <= ls ) {
     alert("<% multilang("1966" "LANG_INVALID_LOCAL_IP_RANGE"); %>");
     document.addressMap.lsip.focus();
     return false;
    }


   } else if ( document.addressMap.addressMapType.selectedIndex == 2 ) {
  //alert('You select Many-to-Many.');		
  if ( !checkIP(document.addressMap.lsip)) {
     return false;
    }

    if ( !checkIP(document.addressMap.leip)) {
     return false;
    }

    if ( !checkIP(document.addressMap.gsip)) {
     return false;
    }

    if ( !checkIP(document.addressMap.geip)) {
     return false;
    }

    if ( ls1 != le1 || ls2 != le2 || ls3 != le3 ) {
     alert("<% multilang("2248" "LANG_LOCAL_START_IP_DOMAIN_IS_DIFFERENT_FROM_END_IP"); %>");
     document.addressMap.lsip.focus();
     return false;
    }

    if ( gs1 != ge1 || gs2 != ge2 || gs3 != ge3 ) {
     alert("<% multilang("1968" "LANG_GLOBAL_START_IP_DOMAIN_IS_DIFFERENT_FROM_GLOBAL_END_IP"); %>");
     document.addressMap.gsip.focus();
     return false;
    }

    if ( le <= ls ) {
     alert("<% multilang("1966" "LANG_INVALID_LOCAL_IP_RANGE"); %>");
     document.addressMap.lsip.focus();
     return false;
    }

    if ( ge <= gs ) {
     alert("<% multilang("1969" "LANG_INVALID_GLOBAL_IP_RANGE"); %>");
     document.addressMap.gsip.focus();
     return false;
    }

   } else if ( document.addressMap.addressMapType.selectedIndex == 3 ) {
  //alert('You select Many One-to-Many.');		
  if ( !checkIP(document.addressMap.lsip)) {
     return false;
    }

    if ( !checkIP(document.addressMap.gsip)) {
     return false;
    }

    if ( !checkIP(document.addressMap.geip)) {
     return false;
    }

    if ( gs1 != ge1 || gs2 != ge2 || gs3 != ge3 ) {
     alert("<% multilang("1968" "LANG_GLOBAL_START_IP_DOMAIN_IS_DIFFERENT_FROM_GLOBAL_END_IP"); %>");
     document.addressMap.gsip.focus();
     return false;
    }

    if ( ge <= gs ) {
     alert("<% multilang("1969" "LANG_INVALID_GLOBAL_IP_RANGE"); %>");
     document.addressMap.gsip.focus();
     return false;
    }

   }
   //alert("Please commit and reboot this system for take effect the address mapping rule!");
}
function disableDelButton()
{

  if (verifyBrowser() != "ns") {
 disableButton(document.formAddressMapDel.deleteSelAddressMap);
 disableButton(document.formAddressMapDel.deleteAllAddressMap);
  }

}
function checkState()
{
  if ( document.addressMap.addressMapType.selectedIndex == 0 ) {
  //alert('You select One-to-One.');				
    enableTextField(document.addressMap.lsip);
    disableTextField(document.addressMap.leip);
    enableTextField(document.addressMap.gsip);
    disableTextField(document.addressMap.geip);
   } else if ( document.addressMap.addressMapType.selectedIndex == 1 ) {
  //alert('You select Many-to-One.');				
    enableTextField(document.addressMap.lsip);
    enableTextField(document.addressMap.leip);
    enableTextField(document.addressMap.gsip);
    disableTextField(document.addressMap.geip);
   } else if ( document.addressMap.addressMapType.selectedIndex == 2 ) {
  //alert('You select Many-to-Many.');
  enableTextField(document.addressMap.lsip);
    enableTextField(document.addressMap.leip);
    enableTextField(document.addressMap.gsip);
    enableTextField(document.addressMap.geip);
   } else if ( document.addressMap.addressMapType.selectedIndex == 3 ) {
  //alert('You select Many One-to-Many.');
  enableTextField(document.addressMap.lsip);
    disableTextField(document.addressMap.leip);
    enableTextField(document.addressMap.gsip);
    enableTextField(document.addressMap.geip);
   }

}


</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF"><% multilang("564" "LANG_ADDRESS_MAPPING_RULE"); %><% multilang("212" "LANG_CONFIGURATION"); %></font></h2>

<form action=/boaform/formAddressMap method=POST name="addressMap">
<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><td><font size=2>
    <% multilang("565" "LANG_THIS_PAGE_IS_USED_TO_SET_AND_CONFIGURE_THE_ADDRESS_MAPPING_RULE_FOR_YOUR_DEVICE"); %> <% multilang("566" "LANG_THE_MAXIMUM_NUMBER_OF_ENTRIES_ARE_16"); %>
  </font></td></tr>
  <tr><td><hr size=1 noshade align=top></td></tr>
</table>

<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr>
      <td width="30%"><font size=2><b><% multilang("239" "LANG_TYPE"); %>:</b>
      <select size="1" name="addressMapType" onChange="checkState()">
      <option value=1><% multilang("567" "LANG_ONE_TO_ONE"); %></option>
      <option value=2><% multilang("568" "LANG_MANY_TO_ONE"); %></option>
      <option value=3><% multilang("569" "LANG_MANY_TO_MANY_OVERLOAD"); %></option>
      <option value=4><% multilang("570" "LANG_ONE_TO_MANY"); %></option>
      </select>
      </td>
  </tr>

  <tr>
      <td width="30%"><font size=2><b><% multilang("571" "LANG_LOCAL_START_IP"); %>:</b></td>
      <td width="70%"><input type="text" name="lsip" size="15" maxlength="15" value=<% getInfo("local-s-ip"); %>></td>
  </tr>

  <tr>
      <td width="30%"><font size=2><b><% multilang("572" "LANG_LOCAL_END_IP"); %>:</b></td>
      <td width="70%"><input type="text" name="leip" size="15" maxlength="15" value=<% getInfo("local-e-ip"); %>></td>
  </tr>

  <tr>
      <td width="30%"><font size=2><b><% multilang("573" "LANG_GLOBAL_START_IP"); %>:</b></td>
      <td width="70%"><input type="text" name="gsip" size="15" maxlength="15" value=<% getInfo("global-s-ip"); %>></td>
  </tr>

  <tr>
      <td width="30%"><font size=2><b><% multilang("574" "LANG_GLOBAL_END_IP"); %>:</b></td>
      <td width="70%"><input type="text" name="geip" size="15" maxlength="15" value=<% getInfo("global-e-ip"); %>></td>
  </tr>

</table>
  <br>
      <input type="submit" value="<% multilang("195" "LANG_ADD"); %>" name="add" onClick="return addClick()">&nbsp;&nbsp;
      <input type="hidden" value="/multi_addr_mapping.asp" name="submit-url">

 <script>
 <% initPage("addressMap"); %>
  </script>

 </form>
 <form action=/boaform/formAddressMap method=POST name="formAddressMapDel">
<table border=0 width=500>
  <tr><td><hr size=1 noshade align=top></td></tr>
  <tr><font size=2><b><% multilang("575" "LANG_CURRENT_ADDRESS_MAPPING_TABLE"); %>:</b></font></tr>

<% AddressMapList(); %>
</table>

 <br><input type="submit" value="<% multilang("198" "LANG_DELETE_SELECTED"); %>" name="deleteSelAddressMap" onClick="return deleteClick()">&nbsp;&nbsp;
     <input type="submit" value="<% multilang("199" "LANG_DELETE_ALL"); %>" name="deleteAllAddressMap" onClick="return deleteAllClick()">&nbsp;&nbsp;&nbsp;
     <input type="hidden" value="/multi_addr_mapping.asp" name="submit-url">
 <script>
    <% checkWrite("AddresMapNum"); %>
 </script>

</form>


</blockquote>
</body>

</html>
