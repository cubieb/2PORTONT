<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang(LANG_BRIDGING); %><% multilang(LANG_CONFIGURATION); %></title>
<script type="text/javascript" src="share.js">
</script>
<SCRIPT>
function saveChanges()
{
	if (checkDigit(document.bridge.ageingTime.value) == 0) {		
		alert('<% multilang(LANG_INVALID_AGEING_TIME); %>');
		document.bridge.ageingTime.focus();
		return false;
	}
	return true;
}

function fdbClick(url)
{
	var wide=600;
	var high=400;
	if (document.all)
		var xMax = screen.width, yMax = screen.height;
	else if (document.layers)
		var xMax = window.outerWidth, yMax = window.outerHeight;
	else
	   var xMax = 640, yMax=480;
	var xOffset = (xMax - wide)/2;
	var yOffset = (yMax - high)/3;

	var settings = 'width='+wide+',height='+high+',screenX='+xOffset+',screenY='+yOffset+',top='+yOffset+',left='+xOffset+', resizable=yes, toolbar=no,location=no,directories=no,status=no,menubar=no,scrollbars=yes';

	window.open( url, 'FDBTbl', settings );
}
	
</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF"><% multilang(LANG_BRIDGING); %><% multilang(LANG_CONFIGURATION); %></font></h2>

<form action=/boaform/formBridging method=POST name="bridge">
<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><td><font size=2>
    <% multilang(LANG_THIS_PAGE_IS_USED_TO_CONFIGURE_THE_BRIDGE_PARAMETERS_HERE_YOU_CAN_CHANGE_THE_SETTINGS_OR_VIEW_SOME_INFORMATION_ON_THE_BRIDGE_AND_ITS_ATTACHED_PORTS); %>
  </font></td></tr>
  <tr><td><hr size=1 noshade align=top></td></tr>
</table>
<table border=0 width="500" cellspacing=4 cellpadding=0>

  <tr>
      <td width="30%"><font size=2><b><% multilang(LANG_AGEING_TIME); %>:</b></td>
      <td width="70%"><input type="text" name="ageingTime" size="15" maxlength="15" value=<% getInfo("bridge-ageingTime"); %>> (<% multilang(LANG_SECONDS); %>)</td>
  </tr>

  <tr>
      <td width="30%"><font size=2><b>802.1d <% multilang(LANG_SPANNING_TREE); %>:</b></td>
      <td width="35%">
      	<input type="radio" value="0" name="stp" <% checkWrite("br-stp-0"); %>><% multilang(LANG_DISABLED); %>&nbsp;&nbsp;
     	<input type="radio" value="1" name="stp" <% checkWrite("br-stp-1"); %>><% multilang(LANG_ENABLED); %>
      </td>
  </tr>
</table>
  <br>
      <input type="submit" value="<% multilang(LANG_APPLY_CHANGES); %>" name="save" onClick="return saveChanges()">&nbsp;&nbsp;
      <input type="button" value="<% multilang(LANG_SHOW_MACS); %>" name="fdbTbl" onClick="fdbClick('/fdbtbl.asp')">
      <input type="hidden" value="/bridging.asp" name="submit-url">
 </form>
</blockquote>
</body>

</html>
