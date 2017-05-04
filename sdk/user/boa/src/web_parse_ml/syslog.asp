<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang("61" "LANG_SYSTEM_LOG"); %></title>
<script type="text/javascript" src="share.js">
</script>
<script language="javascript">
var addr = '<% getInfo("syslog-server-ip"); %>';
var port = '<% getInfo("syslog-server-port"); %>';
function getLogPort() {
 var portNum = parseInt(port);
 if (isNaN(portNum) || portNum == 0)
  portNum = 514; // default system log server port is 514

 return portNum;
}

function hideInfo(hide) {
 var status = 'visible';

 if (hide == 1) {
  status = 'hidden';
  document.forms[0].logAddr.value = '';
  document.forms[0].logPort.value = '';
  changeBlockState('srvInfo', true);
 } else {
  changeBlockState('srvInfo', false);
  document.forms[0].logAddr.value = addr;
  document.forms[0].logPort.value = getLogPort();
 }
}

function hidesysInfo(hide) {
 var status = false;

 if (hide == 1) {
  status = true;
 }
 changeBlockState('sysgroup', status);
}

function changelogstatus() {
 with (document.forms[0]) {
  if (logcap[1].checked) {
   hidesysInfo(0);
   if (logMode.selectedIndex == 0) {
    hideInfo(1);
   } else {
    hideInfo(0);
   }
  } else {
   hidesysInfo(1);
   hideInfo(1);
  }
 }
}

function cbClick(obj) {
 var idx = obj.selectedIndex;
 var val = obj.options[idx].value;

 /* 1: Local, 2: Remote, 3: Both */
 if (val == 1)
  hideInfo(1);
 else
  hideInfo(0);
}

function check_enable()
{
 if (document.formSysLog.logcap[0].checked) {
  //disableTextField(document.formSysLog.msg);
  disableButton(document.formSysLog.refresh);
 }
 else {
  //enableTextField(document.formSysLog.msg);
  enableButton(document.formSysLog.refresh);
 }
}

/*function scrollElementToEnd (element) {
   if (typeof element.scrollTop != 'undefined' &&
       typeof element.scrollHeight != 'undefined') {
     element.scrollTop = element.scrollHeight;
   }
}*/

function saveClick()
{
 <% RemoteSyslog("check-ip"); %>
//	if (document.forms[0].logAddr.disabled == false && !checkIP(document.formSysLog.logAddr))
//		return false;
//	alert("Please commit and reboot this system for take effect the System log!");
 return true;
}

</script>
</head>

<body>
<blockquote>
<h2><font color="#0000FF"><% multilang("61" "LANG_SYSTEM_LOG"); %></font></h2>
<form action=/boaform/formSysLog method=POST name=formSysLog>
<table border="0" cellspacing="4" width="500">
<tr><hr size=1 noshade align=top></tr>
<tr>
 <td width="25%"><font size=2><b><% multilang("61" "LANG_SYSTEM_LOG"); %>&nbsp;:</b></td>
 <td width="30%"><font size=2>
  <input type="radio" value="0" name="logcap" onClick='changelogstatus()' <% checkWrite("log-cap0"); %>><% multilang("221" "LANG_DISABLE"); %>&nbsp;&nbsp;
  <input type="radio" value="1" name="logcap" onClick='changelogstatus()' <% checkWrite("log-cap1"); %>><% multilang("222" "LANG_ENABLE"); %>
 </td>
</tr>
<% ShowPPPSyslog("syslogppp"); %>
<TBODY id='sysgroup'>
<tr>
 <td><font size=2><b><% multilang("770" "LANG_LOG_LEVEL"); %>&nbsp;:</b></td>
 <td><select name='levelLog' size="1">
  <% checkWrite("syslog-log"); %>
 </select></td>
</tr>
<tr>
 <td><font size=2><b><% multilang("771" "LANG_DISPLAY_LEVEL"); %>&nbsp;:</b></td>
 <td ><select name='levelDisplay' size="1">
  <% checkWrite("syslog-display"); %>
 </select></td>
</tr>
<% RemoteSyslog("syslog-mode"); %>
<tbody id='srvInfo'>
<% RemoteSyslog("server-info"); %>
</tbody>
</TBODY>
<tr>
 <td width="45%"> <input type="submit" value="<% multilang("134" "LANG_APPLY_CHANGES"); %>" name="apply" onClick="return saveClick()"></td>
</tr>

<tr>
 <td width="25%"><font size=2><b><% multilang("772" "LANG_SAVE_LOG_TO_FILE"); %>:</b></td>
 <td width="30%"><font size=2><input type="submit" value="<% multilang("773" "LANG_SAVE"); %>..." name="save_log"></td>
</tr>
<tr>
 <td width="25%"><font size=2><b><% multilang("774" "LANG_CLEAR_LOG"); %>:</b></td>
 <td width="30%"><font size=2><input type="submit" value="<% multilang("196" "LANG_RESET"); %>" name="clear_log"></td>
</tr>
</table>
<table border="0" cellspacing="4" width="500">
<tr><hr size=1 noshade align=top></tr>
<tr>
 <td width="25%"><font size=2><b><% multilang("61" "LANG_SYSTEM_LOG"); %></b></td>
 <td width="30%"><font size=2><input type="button" value="Refresh" name="refresh" onClick="javascript: window.location.reload()"></td>
</tr>
<tr>
 <td>
 <div style="overflow: auto; height: 500px; width: 500px; PADDING-LEFT: 10px; PADDING-TOP: 10px; PADDING-RIGHT: 10px; PADDING-BOTTOM: 10px">
 <table border="0" width="100%"><% sysLogList(); %></table>
 </td>
</tr>
</table>


<input type="hidden" value="/syslog.asp" name="submit-url">
<script>
 check_enable();
 //scrollElementToEnd(this.formSysLog.msg);
</script>
</form>
<script>
 <% initPage("syslog"); %>
 <% initPage("pppSyslog"); %>
</script>
</blockquote>
</body>
</html>
