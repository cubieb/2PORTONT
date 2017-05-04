<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title>MAC <% multilang("324" "LANG_FILTERING"); %></title>
<script type="text/javascript" src="share.js">
</script>
<script>
function skip () { this.blur(); }
function addClick()
{
//  if (document.formFilterAdd.srcmac.value=="" )
//	return true;
  if (document.formFilterAdd.srcmac.value=="" && document.formFilterAdd.dstmac.value=="") {
 alert('<% multilang("2128" "LANG_INPUT_MAC_ADDRESS"); %>');
 return false;
  }

 if (document.formFilterAdd.srcmac.value != "") {
  if (!checkMac(document.formFilterAdd.srcmac, 0))
   return false;
 }
 if (document.formFilterAdd.dstmac.value != "") {
  if (!checkMac(document.formFilterAdd.dstmac, 0))
   return false;
 }
 return true;
/*  var str = document.formFilterAdd.srcmac.value;
  if ( str.length < 12) {
	alert("Input MAC address is not complete. It should be 12 digits in hex.");
	document.formFilterAdd.srcmac.focus();
	return false;
  }

  for (var i=0; i<str.length; i++) {
    if ( (str.charAt(i) >= '0' && str.charAt(i) <= '9') ||
			(str.charAt(i) >= 'a' && str.charAt(i) <= 'f') ||
			(str.charAt(i) >= 'A' && str.charAt(i) <= 'F') )
			continue;

	alert("Invalid MAC address. It should be in hex number (0-9 or a-f).");
	document.formFilterAdd.srcmac.focus();
	return false;
  }
  return true;*/
}

function disableDelButton()
{
  if (verifyBrowser() != "ns") {
 disableButton(document.formFilterDel.deleteSelFilterMac);
 disableButton(document.formFilterDel.deleteAllFilterMac);
  }
}

</script>
</head>

<body>
<blockquote>
<h2><font color="#0000FF"><% multilang("18" "LANG_MAC_FILTERING"); %></font></h2>

<table border=0 width="500" cellspacing=0 cellpadding=0>
<tr><td><font size=2>
 <% multilang("339" "LANG_PAGE_DESC_LAN_TO_INTERNET_DATA_PACKET_FILTER_TABLE"); %>
</font></td></tr>

<tr><td><hr size=1 noshade align=top></td></tr>
</table>

<table border=0 width="500" cellspacing=0 cellpadding=0>
<form action=/boaform/admin/formFilter method=POST name="formFilterDefault">
<tr><td><font size=2><b><% multilang("340" "LANG_DEFAULT_ACTION"); %></b>&nbsp;&nbsp;
    <input type="radio" name="outAct" value=0 <% checkWrite("macf_out_act0"); %>><% multilang("327" "LANG_DENY"); %>&nbsp;&nbsp;
    <input type="radio" name="outAct" value=1 <% checkWrite("macf_out_act1"); %>><% multilang("328" "LANG_ALLOW"); %>&nbsp;&nbsp;
    <input type="submit" value="<% multilang("134" "LANG_APPLY_CHANGES"); %>" name="setMacDft">&nbsp;&nbsp;
 <input type="hidden" value="/admin/fw-macfilter_rg.asp" name="submit-url">
</font></td><tr>
</form>
<tr><td><hr size=1 noshade align=top></td></tr>
</table>
<table border=0 width="500" cellspacing=0 cellpadding=0>
<form action=/boaform/admin/formFilter method=POST name="formFilterAdd">
<!--<br>
<tr>
 <td><font size=2>
 <b><% multilang("330" "LANG_DIRECTION"); %>: </b></font>
 </td>
 <td>
 <select name=dir>
  <option select value=0><% multilang("342" "LANG_BOTH"); %></option>
  <option value=1><% multilang("334" "LANG_SOURCE"); %></option>
  <option value=2><% multilang("335" "LANG_DESTINATION"); %></option>
 </select>
 </td>
</tr> -->
<tr>
 <td><font size=2>
 <b><% multilang("86" "LANG_MAC_ADDRESS"); %>: </b></font>
 <input type="text" name="srcmac" size="15" maxlength="12">&nbsp;&nbsp;
 <font size=2>
 <input type="submit" value="<% multilang("195" "LANG_ADD"); %>" name="addFilterMac" onClick="return addClick()">&nbsp;&nbsp;
 <input type="hidden" value="/admin/fw-macfilter_rg.asp" name="submit-url"></font>
 </td>
</tr>
<!--<tr><td>&nbsp;</td></tr>-->
<tr><td><hr size=1 noshade align=top></td></tr>
</form>
</table>

<form action=/boaform/admin/formFilter method=POST name="formFilterDel">
 <table border="0" width=400>
 <tr><font size=2><b><% multilang("336" "LANG_CURRENT_FILTER_TABLE"); %>:</b></font></tr>
 <% macFilterList(); %>
 </table>
 <br>
 <input type="submit" value="<% multilang("198" "LANG_DELETE_SELECTED"); %>" name="deleteSelFilterMac" onClick="return deleteClick()">&nbsp;&nbsp;
 <input type="submit" value="<% multilang("199" "LANG_DELETE_ALL"); %>" name="deleteAllFilterMac" onClick="return deleteAllClick()">&nbsp;&nbsp;&nbsp;
 <input type="hidden" value="/admin/fw-macfilter_rg.asp" name="submit-url">
 <script>
  <% checkWrite("macFilterNum"); %>
 </script>
</form>

</blockquote>
</body>
</html>
