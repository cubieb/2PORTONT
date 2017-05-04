<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang(LANG_WLAN_ACCESS_CONTROL); %></title>
<script type="text/javascript" src="share.js">
</script>
<script>

function skip () { this.blur(); }
function addClick()
{
//  var str = document.formWlAcAdd.mac.value;

// if (!document.formWlAcAdd.wlanAcEnabled.checked)
//  if (!document.formWlAcAdd.wlanAcEnabled.selectedIndex)
//	return true;

	if (!checkMac(document.formWlAcAdd.mac, 1))
		return false;
	return true;
/*  if ( str.length == 0)
  	return true;

  if ( str.length < 12) {
	alert("<% multilang(LANG_INVALID_MAC_ADDR_NOT_COMPLETE); %>");
	document.formWlAcAdd.mac.focus();
	return false;
  }

  for (var i=0; i<str.length; i++) {
    if ( (str.charAt(i) >= '0' && str.charAt(i) <= '9') ||
			(str.charAt(i) >= 'a' && str.charAt(i) <= 'f') ||
			(str.charAt(i) >= 'A' && str.charAt(i) <= 'F') )
			continue;

	alert("<% multilang(LANG_INVALID_MAC_ADDRESS_IT_SHOULD_BE_IN_HEX_NUMBER_0_9_OR_A_F_); %>");
	document.formWlAcAdd.mac.focus();
	return false;
  }
  return true;*/
}

function disableDelButton()
{
	disableButton(document.formWlAcDel.deleteSelFilterMac);
	disableButton(document.formWlAcDel.deleteAllFilterMac);
}

function enableAc()
{
  enableTextField(document.formWlAcAdd.mac);
}

function disableAc()
{
  disableTextField(document.formWlAcAdd.mac);
}

function updateState()
{
  if(wlanDisabled || wlanMode == 1 || wlanMode ==2){
	disableDelButton();
	//disableButton(document.formWlAcDel.reset);
	disableButton(document.formWlAcAdd.reset);
	disableButton(document.formWlAcAdd.setFilterMode);
	disableButton(document.formWlAcAdd.addFilterMac);
  	disableTextField(document.formWlAcAdd.wlanAcEnabled);
  	disableAc();
  } 
  else{
    if (document.formWlAcAdd.wlanAcEnabled.selectedIndex) {
	enableButton(document.formWlAcAdd.reset);
	enableButton(document.formWlAcAdd.addFilterMac);
 	enableAc();
    }
    else {
	disableButton(document.formWlAcAdd.reset);
	disableButton(document.formWlAcAdd.addFilterMac);
  	disableAc();
    }
  }
}

</script>
</head>
<body>
<blockquote>
<h2><font color="#0000FF"><% multilang(LANG_WLAN_ACCESS_CONTROL); %></font></h2>
<form action=/boaform/admin/formWlAc method=POST name="formWlAcAdd">
<table border=0 width="500" cellspacing=4 cellpadding=0>
<tr><td><font size=2>
 <% multilang(LANG_PAGE_DESC_WLAN_ALLOW_DENY_LIST); %>
</font></td></tr>
<tr><td><hr size=1 noshade align=top></td></tr>
</table>

<table border=0 width="500" cellspacing=4 cellpadding=0>

<tr>
   <td><font size=2><b>
   	<% multilang(LANG_MODE); %>: &nbsp;&nbsp;&nbsp;&nbsp;
	<select size="1" name="wlanAcEnabled" onclick="updateState()">
          <option value=0 ><% multilang(LANG_DISABLED); %></option>
          <option value=1 selected ><% multilang(LANG_ALLOW_LISTED); %></option>
          <option value=2 ><% multilang(LANG_DENY_LISTED); %></option>
        </select></font></b>
   </td>
   <td><input type="submit" value="<% multilang(LANG_APPLY_CHANGES); %>" name="setFilterMode">&nbsp;&nbsp;</td>
</tr>
<tr>
</table>
<td>

<table border=0 width="500" cellspacing=4 cellpadding=0>
<tr><td><hr size=1 noshade align=top></td></tr>
<tr><p><font size=2><b><% multilang(LANG_MAC_ADDRESS); %>: </b><input type="text" name="mac" size="15" maxlength="12">
     &nbsp;&nbsp;(ex. 00E086710502)</font></p>
     <p><input type="submit" value="<% multilang(LANG_ADD); %>" name="addFilterMac" onClick="return addClick()">&nbsp;&nbsp;
        <input type="reset" value="<% multilang(LANG_RESET); %>" name="reset">&nbsp;&nbsp;&nbsp;
        <input type="hidden" value="/admin/wlactrl.asp" name="submit-url">
        <input type="hidden" name="wlan_idx" value=<% checkWrite("wlan_idx"); %>>
     </p>
  </form>
<br>
<form action=/boaform/admin/formWlAc method=POST name="formWlAcDel">
  <table border="0" width=440>
  <tr><font size=2><b><% multilang(LANG_CURRENT_ACCESS_CONTROL_LIST); %>:</b></font></tr>
  <% wlAcList(); %>
  </table>
  <br>
  <input type="submit" value="<% multilang(LANG_DELETE_SELECTED); %>" name="deleteSelFilterMac" onClick="return deleteClick()">&nbsp;&nbsp;
  <input type="submit" value="<% multilang(LANG_DELETE_ALL); %>" name="deleteAllFilterMac" onClick="return deleteAllClick()">&nbsp;&nbsp;&nbsp;
  <input type="hidden" value="/admin/wlactrl.asp" name="submit-url">
  <input type="hidden" name="wlan_idx" value=<% checkWrite("wlan_idx"); %>>
 <script>
 	<% checkWrite("wlanAcNum"); %>
	<% initPage("wlactrl"); %>
	updateState();
 </script>
</form>

</blockquote>
</body>
</html>
