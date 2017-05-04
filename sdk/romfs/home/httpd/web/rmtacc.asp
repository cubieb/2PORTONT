<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang("41" "LANG_REMOTE_ACCESS"); %><% multilang("212" "LANG_CONFIGURATION"); %></title>
<script type="text/javascript" src="share.js">
</script>

<script>
function addClick()
{
 dTelnet = getDigit(document.acc.w_telnet_port.value, 1);
 dFtp = getDigit(document.acc.w_ftp_port.value, 1);
 dWeb = getDigit(document.acc.w_web_port.value, 1);
 if (dTelnet == dFtp || dTelnet == dWeb) {
  alert("<% multilang("2291" "LANG_DUPLICATED_PORT_NUMBER"); %>");
  document.acc.w_telnet_port.focus();
  return false;
 }
 if (dFtp == dWeb) {
  alert("<% multilang("2291" "LANG_DUPLICATED_PORT_NUMBER"); %>");
  document.acc.w_ftp_port.focus();
  return false;
 }

 if (document.acc.w_telnet.checked) {
  if (document.acc.w_telnet_port.value=="") {
   alert("<% multilang("2292" "LANG_PORT_RANGE_CANNOT_BE_EMPTY_YOU_SHOULD_SET_A_VALUE_BETWEEN_1_65535"); %>");
   document.acc.w_telnet_port.focus();
   return false;
  }
  if ( validateKey( document.acc.w_telnet_port.value ) == 0 ) {
   alert("<% multilang("2134" "LANG_INVALID_PORT_NUMBER_IT_SHOULD_BE_THE_DECIMAL_NUMBER_0_9"); %>");
   document.acc.w_telnet_port.focus();
   return false;
  }
  //d1 = getDigit(document.acc.w_telnet_port.value, 1);
  //if (d1 > 65535 || d1 < 1) {
  if (dTelnet > 65535 || dTelnet < 1) {
   alert("<% multilang("2135" "LANG_INVALID_PORT_NUMBER_YOU_SHOULD_SET_A_VALUE_BETWEEN_1_65535"); %>");
   document.acc.w_telnet_port.focus();
   return false;
  }
  }

 if (document.acc.w_ftp.checked) {
  if (document.acc.w_ftp_port.value=="") {
   alert("<% multilang("2292" "LANG_PORT_RANGE_CANNOT_BE_EMPTY_YOU_SHOULD_SET_A_VALUE_BETWEEN_1_65535"); %>");
   document.acc.w_ftp_port.focus();
   return false;
  }
  if ( validateKey( document.acc.w_ftp_port.value ) == 0 ) {
   alert("<% multilang("2134" "LANG_INVALID_PORT_NUMBER_IT_SHOULD_BE_THE_DECIMAL_NUMBER_0_9"); %>");
   document.acc.w_ftp_port.focus();
   return false;
  }
  //d1 = getDigit(document.acc.w_ftp_port.value, 1);
  //if (d1 > 65535 || d1 < 1) {
  if (dFtp > 65535 || dFtp < 1) {
   alert("<% multilang("2135" "LANG_INVALID_PORT_NUMBER_YOU_SHOULD_SET_A_VALUE_BETWEEN_1_65535"); %>");
   document.acc.w_ftp_port.focus();
   return false;
  }
 }

 if (document.acc.w_web.checked) {
  if (document.acc.w_web_port.value=="") {
   alert("<% multilang("2292" "LANG_PORT_RANGE_CANNOT_BE_EMPTY_YOU_SHOULD_SET_A_VALUE_BETWEEN_1_65535"); %>");
   document.acc.w_web_port.focus();
   return false;
  }
  if ( validateKey( document.acc.w_web_port.value ) == 0 ) {
   alert("<% multilang("2134" "LANG_INVALID_PORT_NUMBER_IT_SHOULD_BE_THE_DECIMAL_NUMBER_0_9"); %>");
   document.acc.w_web_port.focus();
   return false;
  }
  //d1 = getDigit(document.acc.w_web_port.value, 1);
  //if (d1 > 65535 || d1 < 1) {
  if (dWeb > 65535 || dWeb < 1) {
   alert("<% multilang("2135" "LANG_INVALID_PORT_NUMBER_YOU_SHOULD_SET_A_VALUE_BETWEEN_1_65535"); %>");
   document.acc.w_web_port.focus();
   return false;
  }
 }

 return true;

}

</script>
</head>

<body>
<blockquote>
<h2><font color="#0000FF"><% multilang("41" "LANG_REMOTE_ACCESS"); %><% multilang("212" "LANG_CONFIGURATION"); %></font></h2>

<table border=0 width=500 cellspacing=4 cellpadding=0>
<tr><td colspan=4><font size=2>
 <% multilang("390" "LANG_THIS_PAGE_IS_USED_TO_ENABLE_DISABLE_MANAGEMENT_SERVICES_FOR_THE_LAN_AND_WAN"); %>
</font></td></tr>

<tr><td><hr size=1 noshade align=top></td></tr>
</table>
<form action=/boaform/formSAC method=POST name=acc>
<table border=0 cellpadding=3 cellspacing=0>
<tr>
 <td width=150 align=left><font size=2><b><% multilang("323" "LANG_SERVICE"); %><% multilang("619" "LANG_NAME"); %></b></td>
 <td width=80 align=center><font size=2><b><% multilang("6" "LANG_LAN"); %></b></td>
 <td width=80 align=center><font size=2><b><% multilang("11" "LANG_WAN"); %></b></td>
 <td width=80 align=center><font size=2><b><% multilang("391" "LANG_WAN_PORT"); %></b></td>
</tr>
<% rmtaccItem(); %>
</table>
<br>
<tr>
 <td><input type="submit" value="<% multilang("134" "LANG_APPLY_CHANGES"); %>" name="set" onClick="return addClick()"></td>
 <td><input type="hidden" value="/rmtacc.asp" name="submit-url"></td>
</tr>
<script>
 <% accPost(); %>
</script>
</form>
</blockquote>
</body>
</html>
