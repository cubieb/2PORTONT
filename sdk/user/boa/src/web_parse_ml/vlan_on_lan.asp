<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title>VLAN on LAN <% multilang("212" "LANG_CONFIGURATION"); %></title>
<script type="text/javascript" src="share.js">
</script>
<script>
function saveClick()
{
 if (document.formVLANonLAN.lan1_vid_cap[1].checked == true) {
  if (document.formVLANonLAN.lan1_vid.value == "") {
   alert("<% multilang("2360" "LANG_VID1_SHOULD_NOT_BE_EMPTY"); %>");
   document.formVLANonLAN.lan1_vid.focus();
   return false;
  }
  if (!isNumber(document.formVLANonLAN.lan1_vid.value)) {
   alert("<% multilang("2361" "LANG_VID1_SHOULD_BE_NUMBER"); %>");
   document.formVLANonLAN.lan1_vid.focus();
   return false;
  }
  if (document.formVLANonLAN.lan1_vid.value >= 7 && document.formVLANonLAN.lan1_vid.value <= 9) {
   alert("<% multilang("2362" "LANG_VID1_7_9_ARE_RESERVED"); %>");
   document.formVLANonLAN.lan1_vid.focus();
   return false;
  }
  if (document.formVLANonLAN.lan1_vid.value < 0 || document.formVLANonLAN.lan1_vid.value >= 4096) {
   alert("<% multilang("2363" "LANG_VID1_SHOULD_BE_0_4095"); %>");
   document.formVLANonLAN.lan1_vid.focus();
   return false;
  }
 }

 if (document.formVLANonLAN.lan2_vid_cap[1].checked == true) {
  if (document.formVLANonLAN.lan2_vid.value == "") {
   alert("<% multilang("2364" "LANG_VID2_SHOULD_NOT_BE_EMPTY"); %>");
   document.formVLANonLAN.lan2_vid.focus();
   return false;
  }
  if (!isNumber(document.formVLANonLAN.lan2_vid.value)) {
   alert("<% multilang("2365" "LANG_VID2_SHOULD_BE_NUMBER"); %>");
   document.formVLANonLAN.lan2_vid.focus();
   return false;
  }
  if (document.formVLANonLAN.lan2_vid.value >= 7 && document.formVLANonLAN.lan2_vid.value <= 9) {
   alert("<% multilang("2366" "LANG_VID2_7_9_ARE_RESERVED"); %>");
   document.formVLANonLAN.lan2_vid.focus();
   return false;
  }
  if (document.formVLANonLAN.lan2_vid.value < 0 || document.formVLANonLAN.lan2_vid.value >= 4096) {
   alert("<% multilang("2367" "LANG_VID2_SHOULD_BE_0_4095"); %>");
   document.formVLANonLAN.lan2_vid.focus();
   return false;
  }
 }

 if (document.formVLANonLAN.lan3_vid_cap[1].checked == true) {
  if (document.formVLANonLAN.lan3_vid.value == "") {
   alert("<% multilang("2368" "LANG_VID3_SHOULD_NOT_BE_EMPTY"); %>");
   document.formVLANonLAN.lan3_vid.focus();
   return false;
  }
  if (!isNumber(document.formVLANonLAN.lan3_vid.value)) {
   alert("<% multilang("2369" "LANG_VID3_SHOULD_BE_NUMBER"); %>");
   document.formVLANonLAN.lan3_vid.focus();
   return false;
  }
  if (document.formVLANonLAN.lan3_vid.value >= 7 && document.formVLANonLAN.lan3_vid.value <= 9) {
   alert("<% multilang("2370" "LANG_VID3_7_9_ARE_RESERVED"); %>");
   document.formVLANonLAN.lan3_vid.focus();
   return false;
  }
  if (document.formVLANonLAN.lan3_vid.value < 0 || document.formVLANonLAN.lan3_vid.value >= 4096) {
   alert("<% multilang("2371" "LANG_VID3_SHOULD_BE_0_4095"); %>");
   document.formVLANonLAN.lan3_vid.focus();
   return false;
  }
 }

 if (document.formVLANonLAN.lan4_vid_cap[1].checked == true) {
  if (document.formVLANonLAN.lan4_vid.value == "") {
   alert("<% multilang("2372" "LANG_VID4_SHOULD_NOT_BE_EMPTY"); %>");
   document.formVLANonLAN.lan4_vid.focus();
   return false;
  }
  if (!isNumber(document.formVLANonLAN.lan4_vid.value)) {
   alert("<% multilang("2373" "LANG_VID4_SHOULD_BE_NUMBER"); %>");
   document.formVLANonLAN.lan4_vid.focus();
   return false;
  }
  if (document.formVLANonLAN.lan4_vid.value >= 7 && document.formVLANonLAN.lan4_vid.value <= 9) {
   alert("<% multilang("2374" "LANG_VID4_7_9_ARE_RESERVED"); %>");
   document.formVLANonLAN.lan4_vid.focus();
   return false;
  }
  if (document.formVLANonLAN.lan4_vid.value < 0 || document.formVLANonLAN.lan4_vid.value >= 4096) {
   alert("<% multilang("2375" "LANG_VID4_SHOULD_BE_0_4095"); %>");
   document.formVLANonLAN.lan4_vid.focus();
   return false;
  }
 }

 return true;
}

function updateState()
{
 if (document.formVLANonLAN.lan1_vid_cap[1].checked)
  enableTextField(document.formVLANonLAN.lan1_vid);
 else
  disableTextField(document.formVLANonLAN.lan1_vid);

 if (document.formVLANonLAN.lan2_vid_cap[1].checked)
  enableTextField(document.formVLANonLAN.lan2_vid);
 else
  disableTextField(document.formVLANonLAN.lan2_vid);

 if (document.formVLANonLAN.lan3_vid_cap[1].checked)
  enableTextField(document.formVLANonLAN.lan3_vid);
 else
  disableTextField(document.formVLANonLAN.lan3_vid);

 if (document.formVLANonLAN.lan4_vid_cap[1].checked)
  enableTextField(document.formVLANonLAN.lan4_vid);
 else
  disableTextField(document.formVLANonLAN.lan4_vid);
}
</script>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">VLAN on LAN <% multilang("212" "LANG_CONFIGURATION"); %></font></h2>

<table border=0 width="500" cellspacing=0 cellpadding=0>
 <tr><td><font size=2>
 <% multilang("368" "LANG_THIS_PAGE_BE_USED_TO_CONFIGURE_VLAN_ON_LAN"); %>
 </font></td></tr>
 <tr><td><hr size=1 noshade align=top></td></tr>
</table>

<form action=/boaform/formVLANonLAN method=POST name="formVLANonLAN">
 <table border=0 width="500" cellspacing=0 cellpadding=0>
 <tr>
  <td><font size=2><b>LAN1 <% multilang("232" "LANG_VLAN"); %> <% multilang("617" "LANG_ID"); %>: </b></font></td>
  <td><input type="text" name="lan1_vid" size="10" maxlength="15" value=<% getInfo("lan1-vid"); %>></td>
  <td><font size=2>
  <input type="radio" value="0" name="lan1_vid_cap" <% checkWrite("lan1-vid-cap0"); %> onClick="updateState()"><% multilang("221" "LANG_DISABLE"); %>&nbsp;&nbsp;
  <input type="radio" value="1" name="lan1_vid_cap" <% checkWrite("lan1-vid-cap1"); %> onClick="updateState()"><% multilang("222" "LANG_ENABLE"); %>&nbsp;&nbsp;
  </font></td>
 </tr>
 <tr>
  <td><font size=2><b>LAN2 <% multilang("232" "LANG_VLAN"); %> <% multilang("617" "LANG_ID"); %>: </b></font></td>
  <td><input type="text" name="lan2_vid" size="10" maxlength="15" value=<% getInfo("lan2-vid"); %>></td>
  <td><font size=2>
  <input type="radio" value="0" name="lan2_vid_cap" <% checkWrite("lan2-vid-cap0"); %> onClick="updateState()"><% multilang("221" "LANG_DISABLE"); %>&nbsp;&nbsp;
  <input type="radio" value="1" name="lan2_vid_cap" <% checkWrite("lan2-vid-cap1"); %> onClick="updateState()"><% multilang("222" "LANG_ENABLE"); %>&nbsp;&nbsp;
  </font></td>
 </tr>
 <tr>
  <td><font size=2><b>LAN3 <% multilang("232" "LANG_VLAN"); %> <% multilang("617" "LANG_ID"); %>: </b></font></td>
  <td><input type="text" name="lan3_vid" size="10" maxlength="15" value=<% getInfo("lan3-vid"); %>></td>
  <td><font size=2>
  <input type="radio" value="0" name="lan3_vid_cap" <% checkWrite("lan3-vid-cap0"); %> onClick="updateState()"><% multilang("221" "LANG_DISABLE"); %>&nbsp;&nbsp;
  <input type="radio" value="1" name="lan3_vid_cap" <% checkWrite("lan3-vid-cap1"); %> onClick="updateState()"><% multilang("222" "LANG_ENABLE"); %>&nbsp;&nbsp;
  </font></td>
 </tr>
 <tr>
  <td><font size=2><b>LAN4 <% multilang("232" "LANG_VLAN"); %> <% multilang("617" "LANG_ID"); %>: </b></font></td>
  <td><input type="text" name="lan4_vid" size="10" maxlength="15" value=<% getInfo("lan4-vid"); %>></td>
  <td><font size=2>
  <input type="radio" value="0" name="lan4_vid_cap" <% checkWrite("lan4-vid-cap0"); %> onClick="updateState()"><% multilang("221" "LANG_DISABLE"); %>&nbsp;&nbsp;
  <input type="radio" value="1" name="lan4_vid_cap" <% checkWrite("lan4-vid-cap1"); %> onClick="updateState()"><% multilang("222" "LANG_ENABLE"); %>&nbsp;&nbsp;
  </font></td>
 </tr>
 </table>
 <br>
 <input type="submit" value="<% multilang("134" "LANG_APPLY_CHANGES"); %>" name="save" onClick="return saveClick()">
 <input type="hidden" value="/vlan_on_lan.asp" name="submit-url">
 <script>updateState();</script>
</form>
</blockquote>
</body>
</html>
