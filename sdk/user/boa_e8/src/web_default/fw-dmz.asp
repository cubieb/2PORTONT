<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>DMZ Host</TITLE>
<META http-equiv=pragma content=no-cache>
<META http-equiv=cache-control content="no-cache, must-revalidate">
<META http-equiv=content-type content="text/html; charset=gbk">
<META http-equiv=content-script-type content=text/javascript>
<!--系统公共css-->
<STYLE type=text/css>
@import url(/style/default.css);
</STYLE>
<!--系统公共脚本-->
<SCRIPT language="javascript" src="common.js"></SCRIPT>
<script type="text/javascript" src="share.js"></script>
<SCRIPT language="javascript" type="text/javascript">
function skip () { this.blur(); }
function saveClick()
{
//  if (!document.formDMZ.enabled.checked)
  if (document.formDMZ.dmzcap[0].checked)
 	return true;

/*  if ( validateKey( document.formDMZ.ip.value ) == 0 ) {
	alert("Invalid IP address value. It should be the decimal number (0-9).");
	document.formDMZ.ip.focus();
	return false;
  }
  if( IsLoopBackIP( document.formDMZ.ip.value)==1 ) {
	alert("Invalid IP address value.");
	document.formDMZ.ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.formDMZ.ip.value,1,0,223) ) {
      	alert('Invalid IP address range in 1st digit. It should be 0-223.');
	document.formDMZ.ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.formDMZ.ip.value,2,0,255) ) {
      	alert('Invalid IP address range in 2nd digit. It should be 0-255.');
	document.formDMZ.ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.formDMZ.ip.value,3,0,255) ) {
      	alert('Invalid IP address range in 3rd digit. It should be 0-255.');
	document.formDMZ.ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.formDMZ.ip.value,4,1,254) ) {
      	alert('Invalid IP address range in 4th digit. It should be 1-254.');
	document.formDMZ.ip.focus();
	return false;
  }*/
  if (!checkHostIP(document.formDMZ.ip, 1))
	return false;
  return true;
}

function updateState()
{
//  if (document.formDMZ.enabled.checked) {
  if (document.formDMZ.dmzcap[1].checked) {
 	enableTextField(document.formDMZ.ip);
  }
  else {
 	disableTextField(document.formDMZ.ip);
  }
}


</SCRIPT>
</head>

<body>
<blockquote>

<DIV align="left" style="padding-left:20px; padding-top:5px">
<form action=/boaform/formDMZ method=POST name="formDMZ">
<b>NAT -- DMZ主机</b><br><br>
				家庭网关将来自广域网的IP包转发到DMZ主机。<br><br>
				选择"启用DMZ主机"并输入计算机IP地址,点击"保存/应用"，可以激活DMZ主机。<br><br>
				不选择"启用DMZ主机"并点击"保存/应用"，可以解除DMZ主机。<br>
<hr align="left" class="sep" size="1" width="90%">
<table border="0" cellpadding="0" cellspacing="0">
<tr><td>DMZ主机:</td>
      <td>
	<input type="radio" value="0" name="dmzcap" <% checkWrite("dmz-cap0"); %> onClick="updateState()">禁用&nbsp;&nbsp;
	<input type="radio" value="1" name="dmzcap" <% checkWrite("dmz-cap1"); %> onClick="updateState()">启用&nbsp;&nbsp;
      </td>
</tr>
<tr>
	<td>DMZ主机IP地址: </td>
	<td><input type="text" name="ip" size="15" maxlength="15" value=<% getInfo("dmzHost"); %> ></td>
</tr>
<tr>
	<td><input type="submit" value="保存/应用" name="save" onClick="return saveClick()">&nbsp;&nbsp;</td>
</tr>
<tr><td>
   <br>
        <!--input type="reset" value="Reset" name="reset"-->
        <input type="hidden" value="/fw-dmz.asp" name="submit-url">
</td></tr>
</table>
     <script> updateState(); </script>
</form>
</DIV>
</blockquote>
</body>
</html>
