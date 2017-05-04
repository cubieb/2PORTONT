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
var wanphytype = 1;
var serviceType = 0;
var show_ppp_vlanid = 1;
var show_iptv_vlanid = 0;
var show_voip_vlanid = 0;
var iptvMap = voipMap = 0;
var lanportconflict = 0;

function checkPPPSetting()
{
 if(document.WebWizard4.pppusername.value == "")
 {
  alert("User name cannot be empty!");
  document.WebWizard4.pppusername.focus();
  return false;
 }
 if(wanphytype == 1)
 {
  if(document.getElementById("pppVlan").style.display == "" && document.WebWizard4.ppp_vlan.value == "")
  {
   alert("Invalid VlanID value. It should be 1-4095.");
   document.WebWizard4.ppp_vlan.focus();
   return false;
  }
  if(document.WebWizard4.ppp_vlan.value != "")
  {
   if(!validateDecimalDigit(document.WebWizard4.ppp_vlan.value) || !checkDigitRange(document.WebWizard4.ppp_vlan.value,1,1,4095))
   {
    alert("Invalid VlanID value. It should be 1-4095.");
    document.WebWizard4.ppp_vlan.focus();
    return false;
   }
  }
 }
 return true;
}

function checkIPTVSetting()
{
 if(wanphytype == 1)
 {
  if(document.getElementById("iptvVlan").style.display == "" && document.WebWizard4.iptv_vlan.value == "")
  {
   alert("Invalid VlanID value. It should be 1-4095.");
   document.WebWizard4.iptv_vlan.focus();
   return false;
  }
  if(document.WebWizard4.iptv_vlan.value != "")
  {
   if(!validateDecimalDigit(document.WebWizard4.iptv_vlan.value) || !checkDigitRange(document.WebWizard4.iptv_vlan.value,1,1,4095))
   {
    alert("Invalid VlanID value. It should be 1-4095.");
    document.WebWizard4.iptv_vlan.focus();
    return false;
   }
  }
 }
 var iptv_port = 0;
 if(document.getElementById("iptv_lan1").checked == true)
  iptv_port |= 1;
 if(document.getElementById("iptv_lan2").checked == true)
  iptv_port |= 2;
 if(document.getElementById("iptv_lan3").checked == true)
  iptv_port |= 4;
 if(document.getElementById("iptv_lan4").checked == true)
  iptv_port |= 8;
 if((iptv_port & lanportconflict) != 0)
 {
  alert("Can't bind the lan port that pc connects to IPTV port, please change the IPTV port.");
  return false;
 }
 document.WebWizard4.iptv_port.value = iptv_port;

 return true;
}

function checkVOIPSetting()
{
 if(wanphytype == 1)
 {
  if(document.getElementById("voipVlan").style.display == "" && document.WebWizard4.voip_vlan.value == "")
  {
   alert("Invalid VlanID value. It should be 1-4095.");
   document.WebWizard4.voip_vlan.focus();
   return false;
  }
  if(document.WebWizard4.voip_vlan.value != "")
  {
   if(!validateDecimalDigit(document.WebWizard4.voip_vlan.value) || !checkDigitRange(document.WebWizard4.voip_vlan.value,1,1,4095))
   {
    alert("Invalid VlanID value. It should be 1-4095.");
    document.WebWizard4.voip_vlan.focus();
    return false;
   }
  }
 }
 var voip_port = 0;
 if(document.getElementById("voip_lan1").checked == true)
  voip_port |= 1;
 if(document.getElementById("voip_lan2").checked == true)
  voip_port |= 2;
 if(document.getElementById("voip_lan3").checked == true)
  voip_port |= 4;
 if(document.getElementById("voip_lan4").checked == true)
  voip_port |= 8;
 if((voip_port & lanportconflict) != 0)
 {
  alert("Can't bind the lan port that pc connects to VOIP port, please change the VOIP port.");
  return false;
 }
 document.WebWizard4.voip_port.value = voip_port;

 return true;
}

function onClickContinue(index)
{
 if(serviceType == 0)
 {
  if(!checkPPPSetting())
   return false;
  document.WebWizard4.submit();
 }
 else if(serviceType == 1)
 {
  if(!checkIPTVSetting())
   return false;
  document.WebWizard4.submit();
 }
 else if(serviceType == 2)
 {
  if(!checkVOIPSetting())
   return false;
  document.WebWizard4.submit();
 }
 else if(serviceType == 3)
 {
  if(index == 0)
  {
   if(!checkPPPSetting())
    return false;
   document.getElementById("pppoesetting").style.display = "none";
   document.getElementById("STBsetting").style.display = "";
  }
  else
  {
   if(!checkIPTVSetting())
    return false;
   document.WebWizard4.submit();
  }
 }
 else if(serviceType == 4)
 {
  if(index == 0)
  {
   if(!checkPPPSetting())
    return false;
   document.getElementById("pppoesetting").style.display = "none";
   document.getElementById("SIPsetting").style.display = "";
  }
  else
  {
   if(!checkVOIPSetting())
    return false;
   document.WebWizard4.submit();
  }
 }
 else if(serviceType == 5)
 {
  if(index == 0)
  {
   if(!checkPPPSetting())
    return false;
   document.getElementById("pppoesetting").style.display = "none";
   document.getElementById("STBsetting").style.display = "";
  }
  else if(index == 1)
  {
   if(!checkIPTVSetting())
    return false;
   document.getElementById("pppoesetting").style.display = "none";
   document.getElementById("STBsetting").style.display = "none";
   document.getElementById("SIPsetting").style.display = "";
  }
  else
  {
   if(!checkVOIPSetting())
    return false;
   var iptv_port = getDigit(document.WebWizard4.iptv_port.value, 1);
   var voip_port = getDigit(document.WebWizard4.voip_port.value, 1);
   if((iptv_port & voip_port) != 0)
   {
    alert("IPTV and VOIP can not bind the same lan port.");
    document.getElementById("voip_lan1").focus();
    return false;
   }
   document.WebWizard4.submit();
  }
 }
 return true;
}

function onload()
{
 if(serviceType == 1)
 {
  document.getElementById("pppoesetting").style.display = "none";
  document.getElementById("STBsetting").style.display = "";
  document.getElementById("SIPsetting").style.display = "none";
 }
 else if(serviceType == 2)
 {
  document.getElementById("pppoesetting").style.display = "none";
  document.getElementById("STBsetting").style.display = "none";
  document.getElementById("SIPsetting").style.display = "";
 }
 else
 {
  if((serviceType ==3 || serviceType == 5) && isIPoE == 1)
  {
   document.getElementById("pppoesetting").style.display = "none";
   document.getElementById("STBsetting").style.display = "";
   document.getElementById("SIPsetting").style.display = "none";
  }
  else if(serviceType == 4 && isIPoE == 1)
  {
   document.getElementById("pppoesetting").style.display = "none";
   document.getElementById("STBsetting").style.display = "none";
   document.getElementById("SIPsetting").style.display = "";
  }
  else
  {
   document.getElementById("pppoesetting").style.display = "";
   document.getElementById("STBsetting").style.display = "none";
   document.getElementById("SIPsetting").style.display = "none";
  }
 }

 if(wanphytype == 1)
 {
  if(show_ppp_vlanid == 1)
   document.getElementById("pppVlan").style.display = "";
  if(show_iptv_vlanid == 1)
   document.getElementById("iptvVlan").style.display = "";
  if(show_voip_vlanid == 1)
   document.getElementById("voipVlan").style.display = "";
 }

 if(iptvMap)
 {
  if(iptvMap & 1)
   document.getElementById("iptv_lan1").checked = true;
  if(iptvMap & 2)
   document.getElementById("iptv_lan2").checked = true;
  if(iptvMap & 4)
   document.getElementById("iptv_lan3").checked = true;
  if(iptvMap & 8)
   document.getElementById("iptv_lan4").checked = true;

 }
 if(voipMap)
 {
  if(voipMap & 1)
   document.getElementById("voip_lan1").checked = true;
  if(voipMap & 2)
   document.getElementById("voip_lan2").checked = true;
  if(voipMap & 4)
   document.getElementById("voip_lan3").checked = true;
  if(voipMap & 8)
   document.getElementById("voip_lan4").checked = true;

 }
 if(lanportconflict)
 {
  if(lanportconflict & 1)
  {
   disableCheckBox(document.WebWizard4.iptv_lan1);
   disableCheckBox(document.WebWizard4.voip_lan1);
  }
  if(lanportconflict & 2)
  {
   disableCheckBox(document.WebWizard4.iptv_lan2);
   disableCheckBox(document.WebWizard4.voip_lan2);
  }
  if(lanportconflict & 4)
  {
   disableCheckBox(document.WebWizard4.iptv_lan3);
   disableCheckBox(document.WebWizard4.voip_lan3);
  }
  if(lanportconflict & 8)
  {
   disableCheckBox(document.WebWizard4.iptv_lan4);
   disableCheckBox(document.WebWizard4.voip_lan4);
  }

 }
}
</SCRIPT>
</head>

<body onload="onload();">
<form action=/boaform/form2WebWizard4 method=POST name="WebWizard4">
        <div id="pppoesetting" style="display:none;">
 <div class="data_common data_common_notitle">
  <table>
   <tr>
    <th width="25%">Enter username</th>
    <th>
    <input type="text" name="pppusername">
    </th>
    <script>
     if(wanphytype == 1 && show_ppp_vlanid == 1)
     {
      document.write("<td rowspan=\"3\" class=\"data_prompt_td_info\">");
     }
     else
     {
      document.write("<td rowspan=\"2\" class=\"data_prompt_td_info\">");
     }
    </script>
     Please input username and password received from your ISP.
    </td>
   </tr>
   <tr>
    <th width="25%">Enter password</th>
    <th>
    <input type="text" name="ppppassword">
    </th>
   </tr>
   <tr id="pppVlan" style="display:none;">
    <th width="25%">VLAN ID
    &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</th>
    <th>
    <input type="text" name="ppp_vlan">
    </th>
   </tr>
  </table>
 </div>
 <br>
 <div class="adsl clearfix btn_center">
  <input class="link_bg" type="button" name="back" value="Back" onClick="window.location.href='web_wizard_1.asp';">
  <input class="link_bg" type="button" name="continue" value="Next" onClick="return onClickContinue(0);">
 </div>
 </div>
        <div id="STBsetting" style="display:none;">
 <div class="data_common data_common_notitle">
  <table>
   <tr>
    <td colspan="2" class="data_prompt_td_info" style="padding-top:40px; padding-bottom:40px;">
     Select ports that will be used to connect the set-top box. After setting up the set-top box, connect the selected device to the selected LAN port.
    </td>
   </tr>
   <tr>
    <th colspan="2">
        <div>
            <p class="portbox_title">Select port to connect STB</p>
            <div class="portbox_content">
                <p class="portbox_text">
                    <span id="span1">Lan1</span>
                    <span>Lan2</span>
                    <span>Lan3</span>
                    <span>Lan4</span>
                </p>
                <p class="portbox_checkbox">
                    <span id="span1"><input type="checkbox" id="iptv_lan1" name="iptv_lan1"></span>
                    <span><input type="checkbox" id="iptv_lan2" name="iptv_lan2"></span>
                    <span><input type="checkbox" id="iptv_lan3" name="iptv_lan3"></span>
                    <span><input type="checkbox" id="iptv_lan4" name="iptv_lan4"></span>
                    <input type="hidden" name="iptv_port">
                </p>
            </div>
        </div>
    </th>
   </tr>
   <tr id="iptvVlan" style="display:none;">
    <th width="46%" style="text-align:right;">VLAN ID
    &nbsp;&nbsp;</th>
    <th>
    <input type="text" name="iptv_vlan">
    </th>
   </tr>
  </table>
 </div>
 <br>
 <div class="adsl clearfix btn_center">
  <input class="link_bg" type="button" name="back" value="Back" onClick="window.location.href='web_wizard_1.asp';">
  <input class="link_bg" type="button" name="continue" value="Next" onClick="return onClickContinue(1);">
 </div>
 </div>
        <div id="SIPsetting" style="display:none;">
 <div class="data_common data_common_notitle" display="none">
  <table>
   <tr>
    <td colspan="2" class="data_prompt_td_info" style="padding-top:40px; padding-bottom:40px;">
     Select port to connect SIP phone
    </td>
   </tr>
   <tr>
    <th colspan="2">
        <div>
            <p class="portbox_title">Select port to connect SIP phone</p>
            <div class="portbox_content">
                <p class="portbox_text">
                    <span id="span1">Lan1</span>
                    <span>Lan2</span>
                    <span>Lan3</span>
                    <span>Lan4</span>
                </p>
                <p class="portbox_checkbox">
                    <span id="span1"><input type="checkbox" id="voip_lan1" name="voip_lan1"></span>
                    <span><input type="checkbox" id="voip_lan2" name="voip_lan2"></span>
                    <span><input type="checkbox" id="voip_lan3" name="voip_lan3"></span>
                    <span><input type="checkbox" id="voip_lan4" name="voip_lan4"></span>
                    <input type="hidden" name="voip_port">
                </p>
            </div>
        </div>
    </th>
   </tr>
   <tr id="voipVlan" style="display:none;">
    <th width="46%" style="text-align:right;">VLAN ID
    &nbsp;&nbsp;</th>
    <th>
    <input type="text" name="voip_vlan">
    </th>
   </tr>
  </table>
 </div>
 <br>
 <div class="adsl clearfix btn_center">
  <input class="link_bg" type="button" name="back" value="Back" onClick="window.location.href='web_wizard_1.asp';">
  <input class="link_bg" type="button" name="continue" value="Next" onClick="return onClickContinue(2);">
 </div>
 </div>
 <% ShowWebWizardPage(""); %>
</form>
<SCRIPT>
</SCRIPT>

</body>

</html>
