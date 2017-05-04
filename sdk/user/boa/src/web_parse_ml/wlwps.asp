<html>
<! Copyright (c) Realtek Semiconductor Corp., 2007. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang("201" "LANG_WI_FI_PROTECTED_SETUP"); %></title>
<script type="text/javascript" src="share.js"> </script>
<style>
.on {display:on}
.off {display:none}
</style>
<script>
var wps20;
var wps_version_configurable;
var isClient;
var isConfig;
var hidden_ssid=<% getInfo("hidden_ssid"); %>;
var wpa_cipher=<% getInfo("wpa_cipher"); %>;
var wpa2_cipher=<% getInfo("wpa2_cipher"); %>;
var encrypt=<% getInfo("encrypt"); %>;
var enable1x=<% getInfo("enable1X"); %>;
var wpa_auth=<% getInfo("wpaAuth"); %>;
var mode=<% getInfo("wlanMode"); %>;
var is_adhoc=<% getInfo("networkType"); %>;
var autolockdown_stat=<% getInfo("lockdown_stat"); %>;
var wpsUseVersion=<% getInfo("wpsUseVersion"); %>;
var warn_msg1='WPS was disabled automatically because wireless mode setting could not be supported. ' +
    'You need go to Wireless/Basic page to modify settings to enable WPS.';
var warn_msg2='WPS was disabled automatically because Radius Authentication could not be supported. ' +
    'You need go to Wireless/Security page to modify settings to enable WPS.';
var warn_msg3="PIN number was generated. You have to click \'Apply Changes\' button to make change effectively.";
var warn_msg4='WPS was disabled automatically because broadcast SSID is disabled. ' +
    'You need go to Wireless/Advanced Settings page to modify settings to enable WPS.';
var warn_msg5='WPS was disabled automatically because Encryption type could not be WEP. ' +
    'You need go to Wireless/Security page to modify settings to enable WPS.';
var warn_msg6='WPS was disabled automatically because WPA only or TKIP only is not supported. ' +
    'You need go to Wireless/Security page to modify settings to enable WPS.';
var disable_all=0;

function noenter(e)
{
 if (window.event) //IE
  return !(e.keyCode == 13);
 else if (e.which) //Netscape/Firefox/Opera
  return !(e.which == 13);
}

function triggerPBCClicked()
{
   return true;
}

// WPS2DOTX , for brute force attack unlock
function unlockdownClicked()
{
 return true;
}

function triggerPINClicked()
{
 return(saveChangesWPS(document.formWsc));
}

function compute_pin_checksum(val)
{
 var accum = 0;
 var code = parseInt(val)*10;

 accum += 3 * (parseInt(code / 10000000) % 10);
 accum += 1 * (parseInt(code / 1000000) % 10);
 accum += 3 * (parseInt(code / 100000) % 10);
 accum += 1 * (parseInt(code / 10000) % 10);
 accum += 3 * (parseInt(code / 1000) % 10);
 accum += 1 * (parseInt(code / 100) % 10);
 accum += 3 * (parseInt(code / 10) % 10);
 accum += 1 * (parseInt(code / 1) % 10);
 var digit = (parseInt(accum) % 10);
 return ((10 - digit) % 10);
}

function genPinClicked()
{
 var num_str="1";
 var rand_no;
 var num;

 while (num_str.length != 7) {
  rand_no = Math.random()*1000000000;
  num = parseInt(rand_no, 10);
  num = num%10000000;
  num_str = num.toString();
 }

 num = num*10 + compute_pin_checksum(num);
 num = parseInt(num, 10);
 document.formWsc.elements["localPin"].value = num;
 alert(warn_msg3);
}

function validate_pin_code(code)
{
 var accum=0;

 accum += 3 * (parseInt(code / 10000000) % 10);
 accum += 1 * (parseInt(code / 1000000) % 10);
 accum += 3 * (parseInt(code / 100000) % 10);
 accum += 1 * (parseInt(code / 10000) % 10);
 accum += 3 * (parseInt(code / 1000) % 10);
 accum += 1 * (parseInt(code / 100) % 10);
 accum += 3 * (parseInt(code / 10) % 10);
 accum += 1 * (parseInt(code / 1) % 10);
 return (0 == (accum % 10));
}

function check_pin_code(str)
{
 var i;
 var code_len;

 code_len = str.length;
 if (code_len != 8 && code_len != 4)
  return 1;

 for (i=0; i<code_len; i++) {
  if ((str.charAt(i) < '0') || (str.charAt(i) > '9'))
   return 2;
 }

 if (code_len == 8) {
  var code = parseInt(str, 10);
  if (!validate_pin_code(code))
   return 3;
  else
   return 0;
 }
 else
  return 0;
}

function setPinClicked(form)
{
 var ret;
 var str = form.elements["peerPin"].value;
 form.elements["peerPin"].value = str.replace(/[(\s)-]/g, "");

 ret = check_pin_code(form.elements["peerPin"].value);
 if (ret == 1) {
  alert('<% multilang("2432" "LANG_INVALID_ENROLLEE_PIN_LENGTH_THE_DEVICE_PIN_IS_USUALLY_FOUR_OR_EIGHT_DIGITS_LONG"); %>');
  form.peerPin.focus();
  return false;
 }
 else if (ret == 2) {
  alert('<% multilang("2433" "LANG_INVALID_ENROLLEE_PIN_ENROLLEE_PIN_MUST_BE_NUMERIC_DIGITS"); %>');
  form.peerPin.focus();
  return false;
 }
 else if (ret == 3) {
  if ( !confirm('<% multilang("2459" "LANG_CHECKSUM_FAILED_USE_PIN_ANYWAY"); %>') ) {
   form.peerPin.focus();
   return false;
    }
 }
 return true;
}


function checkWPSstate(form)
{
 if (autolockdown_stat && !form.elements["disableWPS"].checked) {
  enableButton(form.elements["unlockautolockdown"]);
 }else{
  disableButton(form.elements["unlockautolockdown"]);
 }
 if (disable_all || form.elements["wlanDisabled"].value == "ON") {
  disableCheckBox(form.elements["disableWPS"]);
  disableButton(form.elements["save"]);
  disableButton(form.elements["reset"]);
 }
 if (disable_all || form.elements["disableWPS"].checked || form.elements["wlanDisabled"].value == "ON") {
   disableTextField(form.elements["localPin"]);
   disableTextField(form.elements["peerPin"]);
   disableButton(form.elements["setPIN"]);
  disableButton(form.elements["triggerPIN"]);
  disableButton(form.elements["triggerPBC"]);
  disableButton(form.elements["genPIN"]);
   }
 else {
  enableTextField(form.elements["localPin"]);
  enableTextField(form.elements["peerPin"]);
   enableButton(form.elements["setPIN"]);
  enableButton(form.elements["genPIN"]);
  enableButton(form.elements["triggerPIN"]);
  enableButton(form.elements["triggerPBC"]);
 }
 disableRadioGroup(form.elements["config"]);
 return true;
}

function saveChangesWPS(form)
{
 var wps_enc_status;
 var str = form.elements["localPin"].value;
 form.elements["localPin"].value = str.replace(/[(\s)-]/g, "");

 ret = check_pin_code(form.elements["localPin"].value);
 if (ret == 1) {
  alert('<% multilang("2434" "LANG_INVALID_PIN_LENGTH_THE_DEVICE_PIN_IS_USUALLY_FOUR_OR_EIGHT_DIGITS_LONG"); %>');
  form.localPin.focus();
  return false;
 }
 else if (ret == 2) {
  alert('<% multilang("2435" "LANG_INVALID_PIN_THE_DEVICE_PIN_MUST_BE_NUMERIC_DIGITS"); %>');
  form.localPin.focus();
  return false;
 }
 else if (ret == 3) {
  alert('<% multilang("2436" "LANG_INVALID_PIN_CHECKSUM_ERRO"); %>');
  form.localPin.focus();
  return false;
 }

 if (wps20 && wpsUseVersion != 0)
 {
     wps_enc_status = check_wps_enc(encrypt, enable1x, wpa_auth, hidden_ssid, wpa_cipher, wpa2_cipher, wps20);
 }
 else
 {
  wps_enc_status = check_wps_enc(encrypt, enable1x, wpa_auth, hidden_ssid, wpa_cipher, wpa2_cipher, wpsUseVersion);
 }
 if (wps_enc_status) {
  alert('<% multilang("2437" "LANG_WPS2_0_DOES_NOT_SUPPORT_THIS_ENCRYPTION_MODE"); %>');
  return false;
 }
    return true;
}

function updatewpsUseVersionIndex()
{
 wpsUseVersion=document.formWsc.wpsUseVersion.selectedIndex;
}
</script>
</head>

<body>
<blockquote>
<h2><font color="#0000FF"><% multilang("201" "LANG_WI_FI_PROTECTED_SETUP"); %></font></h2>

<form action=/boaform/formWsc method=POST name="formWsc">
<table border=0 width="500" cellspacing=4 cellpadding=0>
<tr><td><font size=2>
 <% multilang("202" "LANG_PAGE_DESC_WPS_SETTING"); %>
</font></td></tr>
<tr><td><hr size=1 noshade align=top><td></tr>
</table>

<table border=0 width="500" cellspacing=4 cellpadding=0>
 <input type=hidden name="wlanDisabled" value=<% wlanStatus(); %>>
<script>
    <% checkWrite("wlanMode"); %>
 <% checkWrite("wpsVer"); %>
 <% checkWrite("wpsVerConfig"); %>
 if (mode == 0 || mode == 3)
 {
  if (wps20)
  {
      disable_all = check_wps_enc(encrypt, enable1x, wpa_auth, hidden_ssid, wpa_cipher, wpa2_cipher, wpsUseVersion);
  }
  else
  {
   disable_all = check_wps_enc(encrypt, enable1x, wpa_auth, hidden_ssid, wpa_cipher, wpa2_cipher, wps20);
  }
 } else {
  disable_all = check_wps_wlanmode(mode, is_adhoc);
 }
</script>
<tr>
  <td width="100%" colspan=3><font size=2><b>
    <input type="checkbox" name="disableWPS" value="ON" <% checkWrite("wscDisable"); %> ONCLICK="checkWPSstate(document.formWsc)">&nbsp;&nbsp;<% multilang("203" "LANG_DISABLE_WPS"); %>
  </td>
</tr>
<script>
  if (isClient) {
 document.write("</table>\n");
 document.write("<span id = \"hide_div\" class = \"off\">\n");
 document.write("<table border=\"0\" width=500>\n");
  }
</script>
<tr>
  <td width="40%"><font size="2"><b>WPS <% multilang("3" "LANG_STATUS"); %>:</b></font></td>
  <td width="60%"><font size="2">
   <input type="radio" name="config" value="on" <% checkWrite("wscConfig-1"); %>><% multilang("204" "LANG_CONFIGURED"); %>&nbsp;&nbsp;
   <input type="radio" name="config" value="off" <% checkWrite("wscConfig-0"); %>><% multilang("205" "LANG_UNCONFIGURED"); %>
 </font></td>
</tr>
<script>
  if (isClient) {
 document.write("</table>\n");
 document.write("</span>\n");
 document.write("<table border=\"0\" width=500>\n");
  }
</script>
<script>
  if (wps20 && wps_version_configurable)
   document.write("<tr>");
  else
   document.write("<tr style=\"display: none;\">");
</script>
  <td width="40%"><font size="2"><b>WPS <% multilang("622" "LANG_VERSION"); %>:</b></font></td>
  <td width="60%"><font size="2">
   <select size="1" name="wpsUseVersion" onChange="updatewpsUseVersionIndex()"><% checkWrite("wpsUseVersion"); %></select>
   <SCRIPT>document.formWsc.wpsUseVersion.selectedIndex = wpsUseVersion;</SCRIPT>
 </font></td>
</tr>
<!-- for brute force attack unlock -->
<tr <% checkWrite("wlan_qtn_hidden_function"); %>>
  <td width="40%"><font size="2"><b><% multilang("206" "LANG_AUTO_LOCK_DOWN_STATE"); %>:</b></font></td>
  <td width="60%"><font size="2">
<script>
  if (autolockdown_stat) {
 document.write("<% multilang("207" "LANG_LOCKED"); %>");
  }else{
 document.write("<% multilang("208" "LANG_UNLOCKED"); %>");
  }
</script>
 &nbsp;&nbsp;
   <input type="submit" value="<% multilang("209" "LANG_UNLOCK"); %>" name="unlockautolockdown" onClick="return unlockdownClicked()">
 </font></td>
</tr>
<tr>
  <td width="40%"><font size="2"><b><% multilang("210" "LANG_SELF_PIN_NUMBER"); %>:</b></font></td>
  <td width="60%"><font size="2"><input type="text" name="localPin" size="12" maxlength="10" onkeypress="return noenter(event)" value=<% getInfo("wscLoocalPin"); %>>
   &nbsp;&nbsp;<input type="button" value="<% multilang("211" "LANG_REGENERATE_PIN"); %>" name="genPIN" onClick="return genPinClicked()"></td>
</tr>

<script>
  if (!isClient) {
 document.write("</table>\n");
 document.write("<span id = \"hide_div\" class = \"off\">\n");
 document.write("<table border=\"0\" width=500>\n");
  }
</script>
<tr>
  <td width="40%"><font size="2"><b>PIN <% multilang("212" "LANG_CONFIGURATION"); %>:</b></font></td>
  <td width="60%"><font size="2">
   <input type="submit" value="<% multilang("213" "LANG_START_PIN"); %>" name="triggerPIN" onClick="return triggerPINClicked()"></td>
 </font></td>
</tr>
<script>
  if (!isClient) {
 document.write("</table>\n");
 document.write("</span>\n");
 document.write("<table border=\"0\" width=500>\n");
  }
</script>

<tr>
  <td width="40%"><font size="2"><b><% multilang("214" "LANG_PUSH_BUTTON"); %> <% multilang("212" "LANG_CONFIGURATION"); %>:</b></font></td>
  <td width="60%"><font size="2">
   <input type="submit" value="<% multilang("215" "LANG_START_PBC"); %>" name="triggerPBC" onClick="return triggerPBCClicked()"></td>
 </font></td>
</tr>

<input type="hidden" value="/wlwps.asp" name="submit-url">
<tr>
   <td width="100%" colspan="2" height=40><input type="submit" value="<% multilang("134" "LANG_APPLY_CHANGES"); %>" name="save" onClick="return saveChangesWPS(document.formWsc)">&nbsp;&nbsp;
  <input type="reset" value="<% multilang("196" "LANG_RESET"); %>" name="reset"></td>
</tr>

 <script>
  if (disable_all) {
   document.write("<tr><td colspan=\"2\" height=\"55\"><font size=2><em>");
     if (disable_all == 1)
      document.write(warn_msg1);
     else if(disable_all == 2)
      document.write(warn_msg2);
  else if(disable_all == 4)
      document.write(warn_msg4);
  else if(disable_all == 5)
      document.write(warn_msg5);
  else if(disable_all == 6)
      document.write(warn_msg6);
  document.write("</td></tr>");
  }
</script>

</table>

<script>
    <% checkWrite("wscConfig-A"); %>
 if (isClient || !isConfig)
  document.write("<span id = \"hide_div\" class = \"off\">\n");
</script>
<table border='0' width="500">
<tr><td><font size=2><b><% multilang("216" "LANG_CURRENT_KEY_INFO"); %>:</b></td></tr>
<table border='1' width="500">
<tr bgcolor=#7f7f7f>
   <td width="30%"><font size=2><b><% multilang("176" "LANG_AUTHENTICATION"); %></b></td>
   <td width="20%"><font size=2><b><% multilang("175" "LANG_ENCRYPTION"); %></b></td>
   <td width="50%"><font size=2><b><% multilang("217" "LANG_KEY"); %></b></td>
</tr>

<tr>
   <td width="30%"><font size=2>
     <% checkWrite("wps_auth"); %></td>
   <td width="20%"><font size=2>
     <% checkWrite("wps_enc"); %></td>
   <td width="50%"><font size=2>
     <% getInfo("wps_key"); %></td>
</tr>
</table><br></table>

<script>
  if (isClient || !isConfig) {
 document.write("</span>\n");
 document.write("<table border=\"0\" width=500>\n");
  }
  if (isClient)
 document.write("<span id = \"hide_div\" class = \"off\">\n");
</script>
<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><td><hr size=1 noshade align=top></td></tr>
</table>
<table border=0 width="500" cellspacing=4 cellpadding=0>
<tr>
  <td width="40%"><font size="2"><b><% multilang("218" "LANG_CLIENT_PIN_NUMBER"); %>:</b></font></td>
  <td width="60%"><font size="2"><input type="text" name="peerPin" size="12" maxlength="10" onkeypress="return noenter(event)" value="">
   &nbsp;&nbsp;<input type="submit" value="<% multilang("213" "LANG_START_PIN"); %>" name="setPIN" onClick="return setPinClicked(document.formWsc)">
 <input type="hidden" name="wlan_idx" value=<% checkWrite("wlan_idx"); %>>
   </td>
</tr>
</table>
<script>
  if (isClient)
 document.write("</span>\n");
   checkWPSstate(document.formWsc);
</script>

</form>
</blockquote>
</body>

</html>
