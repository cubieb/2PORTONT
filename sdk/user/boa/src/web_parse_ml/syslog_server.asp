<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang("61" "LANG_SYSTEM_LOG"); %></title>
<script type="text/javascript" src="share.js">
</script>
<script>
function check_enable()
{
 if (document.formSysLog.logcap[0].checked) {
  disableTextField(document.formSysLog.msg);
  disableButton(document.formSysLog.refresh);
 }
 else {
  enableTextField(document.formSysLog.msg);
  enableButton(document.formSysLog.refresh);
 }
}

function scrollElementToEnd (element) {
   if (typeof element.scrollTop != 'undefined' &&
       typeof element.scrollHeight != 'undefined') {
     element.scrollTop = element.scrollHeight;
   }
}

function saveClick()
{
 if (!checkIP(document.formSysLog.ip))
  return false;

 alert("<% multilang("2330" "LANG_PLEASE_COMMIT_AND_REBOOT_THIS_SYSTEM_FOR_TAKE_EFFECT_THE_SYSTEM_LOG"); %>");
}

</script>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">System Log</font></h2>
<form action=/boaform/formSysLog method=POST name=formSysLog>
<table border="0" cellspacing="4" width="500">
<tr>
 <td width="25%"><font size=2><b>System Log</b></td>
 <td width="30%"><font size=2>
  <input type="radio" value="0" name="logcap" <% checkWrite("log-cap0"); %>>Disable&nbsp;&nbsp;
  <input type="radio" value="1" name="logcap" <% checkWrite("log-cap1"); %>>Enable
 </td>
 <!--
 <td width="45%"> <input type="submit" value="Apply Changes" name="apply" onClick="return saveClick()"></td>
        -->
</tr>
<tr>
       <td width="25%"><font size=2><b>Log Server(FTP Server):</b></td>
       <td width="30%"><input type="text" name="ip" size="15" maxlength="15" value=<% getInfo("log-server-ip"); %>></td>
</tr>

<tr>
      <td width="25%"><font size=2><b>User Name:</b></td>
      <td width="30%"><font size=2><input type="text" name="username" size="20" maxlength="30" value=<% getInfo("log-server-username"); %>></td>
</tr>

<tr>
      <td width="25%"><font size=2><b>Password:</b></td>
      <td width="30%"><font size=2><input type="password" name="passwd" size="20" maxlength="30"></td>
</tr>

<tr>
 <td width="45%"> <input type="submit" value="Apply Changes" name="apply" onClick="return saveClick()"></td>
</tr>

<tr>
 <td width="25%"><font size=2><b>Save Log to File:</b></td>
 <td width="30%"><font size=2><input type="submit" value="Save..." name="save_log"></td>
</tr>
<tr>
 <td width="25%"><font size=2><b>Clear Log:</b></td>
 <td width="30%"><font size=2><input type="submit" value="Reset" name="clear_log"></td>
</tr>
</table>
<textarea rows="15" name="msg" cols="80" wrap="virtual"><% sysLogList(); %></textarea>

<p>
<input type="button" value="Refresh" name="refresh" onClick="javascript: window.location.reload()">
<input type="hidden" value="/syslog_server.asp" name="submit-url">
<script>
 check_enable();
 scrollElementToEnd(this.formSysLog.msg);
</script>
</form>
</blockquote>
</body>
</html>
