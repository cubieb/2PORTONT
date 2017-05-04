<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang("1054" "LANG_ETHERNETOAM_Y_1731"); %></title>
<script type="text/javascript" src="share.js">
</script>
<SCRIPT>
function saveChanges(){
 if ((document.Y1731.myid<1)) {
   alert('<% multilang("2096" "LANG_MYID_MUST_MORE_THAN_0"); %>');
   return false;
 }
 if (document.Y1731.megid.length == 0) {
   alert('<% multilang("2097" "LANG_MEGID_MUST_HAVE_VALUE"); %>');
   return false;
 }
 return true;
}
</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF"><% multilang("1054" "LANG_ETHERNETOAM_Y_1731"); %></font></h2>

<form action=/boaform/formY1731 method=POST name="Y1731">
<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><td><font size=2>
    <% multilang("1055" "LANG_HERE_YOU_CAN_CONFIGURE_ETHERNETOAM_Y_1731"); %>
  </font></td></tr>
  <tr><td><hr size=1 noshade align=top></td></tr>
</table>

<table border=0 width="500" cellspacing=4 cellpadding=0>
 <tr>
  <td width="30%"><font size=2><b><% multilang("222" "LANG_ENABLE"); %>:</b></td>
  <td width="70%"><input type="checkbox" name="oamMode" id="oamMode" value="1" checked></td>
 </tr>
 <tr>
  <td width="30%"><font size=2><b>MEG <% multilang("776" "LANG_LEVEL"); %>:</b></td>
  <td width="70%">
   <select size="1" name="meglevel" id="meglevel">
   <option value="7">7</option><option value="6">6</option><option value="5">5</option><option value="4">4</option>
   <option value="3">3</option><option value="2">2</option><option value="1">1</option><option value="0">0</option>
   </select>
  </td>
 </tr>
 <tr>
  <td width="30%"><font size=2><b>MyID:</b></td>
  <td width="70%"><input type="text" name="myid" id="myid" size="5" maxlength="5"></td>
 </tr>
 <tr>
  <td width="30%"><font size=2><b>MEG ID:</b></td>
  <td width="70%"><input type="text" name="megid" id="megid" size="14" maxlength="14"></td>
 </tr>
  <tr>
  <td width="30%"><font size=2><b>CCM Interval:</b></td>
  <td width="70%">
   <select size="1" name="ccminterval" id="ccminterval">
   <option value="1">3.33ms</option><option value="2">10ms</option><option value="3">100ms</option>
   <option value="4">1s</option><option value="5">10s</option><option value="6">1min</option><option value="7">10min</option>
   </select>
  </td>
 </tr>
 <tr>
  <td width="30%"><font size=2><b>Log Level:</b></td>
  <td width="70%">
   <select size="1" name="loglevel" id="loglevel">
   <option value="none">none</option><option value="medium">medium</option><option value="xtra">extra</option><option value="all">all</option>
   </select>
  </td>
 </tr>
</table>
 <br>
 <input type=submit value="<% multilang("134" "LANG_APPLY_CHANGES"); %>" name="save" onClick="return saveChanges()">
 <input type=hidden value="/ethoam.asp" name="submit-url">
<script>
  document.getElementById('oamMode').checked = <% getInfo("y1731_mode"); %>;
  document.getElementById('myid').value = "<% getInfo("y1731_myid"); %>";
  document.getElementById('meglevel').value = "<% getInfo("y1731_meglevel"); %>";
  document.getElementById('megid').value = "<% getInfo("y1731_megid"); %>";
  document.getElementById('ccminterval').value = "<% getInfo("y1731_ccminterval"); %>";
  document.getElementById('loglevel').value = "<% getInfo("y1731_loglevel"); %>";
</script>
</form>
</blockquote>
</body>

</html>
