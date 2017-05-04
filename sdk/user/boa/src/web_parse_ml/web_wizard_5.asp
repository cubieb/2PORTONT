<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title>Html Wizard</title>
<link href="reset.css" rel="stylesheet" type="text/css" />
<link href="base.css" rel="stylesheet" type="text/css" />
<link href="style.css" rel="stylesheet" type="text/css" />
<script type="text/javascript" src="share.js"></script>

<SCRIPT>
function checkSSIDChar(str,type)
{
 var patrn;
 if(type)
  patrn=/^[a-zA-Z0-9~`!@#$%^&*()_+|\\{}[\]:;<>?,-./='" ]+$/;
 else
  patrn=/^[a-zA-Z0-9~`!@#$%^&*()_+|\\{}[\]:;<>?,-./='" ]*$/;
 if (!patrn.exec(str))
  return false;
 return true;
}

function isValidWPAPasswd(str)
{
 var patrn=/^[a-zA-Z0-9!#$%&\'()*+,-./:;=?@[\\^_`{|}~]{1}[a-zA-Z0-9!#$%&\'()*+,-./:;=?@[\\^_`{|}~\x20]{6,61}[a-zA-Z0-9!#$%&\'()*+,-./:;=?@[\\^_`{|}~]{1}$/;
 if (!patrn.exec(str))
  return false;
 if (str.indexOf(" ") != -1)
  return false;
 if (str.indexOf(" ",0) != -1)
  return false;
 return true;
}

function onClickContinue()
{
 if (document.getElementById("wlanDisabled").checked == true)
  return true;
 else {
  if (!checkSSIDChar(document.WebWizard5.ssid5g.value, 1)) {
   alert("<% multilang("2391" "LANG_INVALID_SSID"); %>");
   document.WebWizard5.ssid5g.focus();
   return false;
  }
  if (!checkSSIDChar(document.WebWizard5.ssid2g.value, 1)) {
   alert("<% multilang("2391" "LANG_INVALID_SSID"); %>");
   document.WebWizard5.ssid2g.focus();
   return false;
  }
  var str = document.WebWizard5.psk.value;
  if (str.length < 8) {
   alert('<% multilang("2417" "LANG_PRE_SHARED_KEY_VALUE_SHOULD_BE_SET_AT_LEAST_8_CHARACTERS"); %>');
   document.WebWizard5.psk.focus();
   return false;
  }
  if (str.length > 64) {
   alert('<% multilang("2418" "LANG_PRE_SHARED_KEY_VALUE_SHOULD_BE_LESS_THAN_64_CHARACTERS"); %>');
   document.WebWizard5.psk.focus();
   return false;
  }
  if (!isValidWPAPasswd(str)) {
   alert('<% multilang("2431" "LANG_INVALID_PRE_SHARED_KEY"); %>');
   return false;
  }
 }
 return true;
}

function updateState()
{
 if(document.getElementById("wlanDisabled").checked == true)
 {
  document.WebWizard5.wlanDisabled.value="ON";
  disableTextField(document.WebWizard5.ssid5g);
  disableTextField(document.WebWizard5.ssid2g);
  disableTextField(document.WebWizard5.psk);
 }
 else
 {
  document.WebWizard5.wlanDisabled.value="OFF";
  enableTextField(document.WebWizard5.ssid5g);
  enableTextField(document.WebWizard5.ssid2g);
  enableTextField(document.WebWizard5.psk);
 }
}
</SCRIPT>
</head>

<body>
<form action=/boaform/form2WebWizard5 method=POST name="WebWizard5">
 <div class="data_common data_common_notitle">
  <table>
   <tr>
    <th width="25%">Disable Wi-Fi</th>
    <th>
    <input id="wlanDisabled" name="wlanDisabled" type="checkbox" onClick="updateState()">
    </th>
    <td rowspan="4" class="data_prompt_td_info">
     You can change the name and password for your Wi-Fi network in this window. You can also turn on Wi-Fi if necessary.
    </td>
   </tr>
   <tr>
    <th width="25%">5GHz WLAN SSID name</th>
    <th>
    <input id="ssid5g" name="ssid5g" type="text" size="20" maxlength="32" value="<% checkWrite("5G_ssid"); %>">
    </th>
   </tr>
   <tr>
    <th width="25%">2.4GHz WLAN SSID name</th>
    <th>
    <input id="ssid2g" name="ssid2g" type="text" size="20" maxlength="32" value="<% checkWrite("2G_ssid"); %>">
    </th>
   </tr>
   <tr>
    <th width="25%">Password</th>
    <th>
    <input id="psk" name="psk" type="text" size="20" maxlength="64" value="<% getInfo("pskValue"); %>">
    </th>
   </tr>
  </table>
 </div>
 <br>
 <div class="adsl clearfix btn_center">
  <input class="link_bg" type="button" name="back" value="Back" onClick="window.location.href='web_wizard_4.asp';">
  <input class="link_bg" type="submit" name="continue" value="Next" onClick="return onClickContinue();">
  <input type="hidden" value="/web_wizard_6.asp" name="submit-url">
 </div>
  <script>
 updateState();
  </script>
</form>

</body>

</html>
