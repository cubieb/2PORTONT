<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang(LANG_ETHERNET_LINK_SPEED_DUPLEX_MODE); %></title>
<SCRIPT>
function postit(port, pvc)
{
	switch(port) {
	case 0:
		if (pvc != 255)
			document.eth2pvc.vc0.value = pvc;
		else
			document.eth2pvc.vc0.selectedIndex = 0;
		break;
	case 1:
		if (pvc != 255)
			document.eth2pvc.vc1.value = pvc;
		else
			document.eth2pvc.vc1.selectedIndex = 0;
		break;
	case 2:
		if (pvc != 255)
			document.eth2pvc.vc2.value = pvc;
		else
			document.eth2pvc.vc2.selectedIndex = 0;
		break;
	case 3:
		if (pvc != 255)
			document.eth2pvc.vc3.value = pvc;
		else
			document.eth2pvc.vc3.selectedIndex = 0;
	default: break;
	}
}

function linkOption()
{
	document.write('<option value=0>10M <% multilang(LANG_HALF_MODE); %></option>\n');
	document.write('<option value=1>10M <% multilang(LANG_FULL_MODE); %></option>\n');
	document.write('<option value=2>100M <% multilang(LANG_HALF_MODE); %></option>\n');
	document.write('<option value=3>100M <% multilang(LANG_FULL_MODE); %></option>\n');
	document.write('<option value=4><% multilang(LANG_AUTO_MODE); %></option>\n');
}
	
</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF"><% multilang(LANG_ETHERNET_LINK_SPEED_DUPLEX_MODE); %></font></h2>

<form action=/boaform/formLinkMode method=POST name=link>
<table border=0 width=500 cellspacing=4 cellpadding=0>
  <tr><font size=2>
    <% multilang(LANG_SET_THE_ETHERNET_LINK_SPEED_DUPLEX_MODE); %>
  </tr>
  <tr><hr size=1 noshade align=top></tr>
  <% show_lan(); %>
</table>
  <br>
      <input type="submit" value="<% multilang(LANG_APPLY_CHANGES); %>" name="save">&nbsp;&nbsp;
      <input type="hidden" value="/linkmode.asp" name="submit-url">
<script>
	<% initPage("linkMode"); %>
</script>
</form>
</blockquote>
</body>

</html>
