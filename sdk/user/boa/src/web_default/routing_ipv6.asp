<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang(LANG_ROUTING); %><% multilang(LANG_CONFIGURATION); %></title>
<script type="text/javascript" src="share.js">
</script>
<SCRIPT>

function postGW( enable, destNet, nextHop, metric, interface, select )
{
	document.route.enable.checked = enable;
	document.route.destNet.value=destNet;
	document.route.nextHop.value=nextHop;
	document.route.metric.value=metric;
	document.route.interface.value=interface;	
	document.route.select_id.value=select;	
}

function checkDest(ip, mask)
{
	var i, dip, dmask, nip;

	for (i=1; i<=4; i++) {
		dip = getDigit(ip.value, i);
		dmask = getDigit(mask.value,  i);
		nip = dip & dmask;
		if (nip != dip)
			return true;
	}
	return false;
}

function addClick()
{

    //check destination
	if ( validateKeyV6IP( document.route.destNet.value ) == 0 ) {
           if(! validateKeyV6Prefix( document.route.destNet.value) )
           {
				alert("<% multilang(LANG_DESTNET_IS_NOT_VALID_IPV6_NET); %>");
				document.route.destNet.focus();
				return false;
		   }
	}
    else if (! isGlobalIpv6Address( document.route.destNet.value) )
	{
		alert("<% multilang(LANG_DESTNET_IS_NOT_VALID_IPV6_HOST); %>");
		document.route.destNet.focus();
		return false;
	}

    //check Next Hop
	if (document.route.nextHop.value=="" ) {
		alert("<% multilang(LANG_ENTER_NEXT_HOP_IP_OR_SELECT_A_GW_INTERFACE); %>");
		document.route.nextHop.focus();
		return false;
	}

	if ( validateKeyV6IP( document.route.nextHop.value ) == 0 ) {
		alert("<% multilang(LANG_INVALID_NEXT_HOP_VALUE); %>");
		document.route.nextHop.focus();
		return false;
	}
    else
	{
		if (! isGlobalIpv6Address( document.route.nextHop.value) ){
			alert("<% multilang(LANG_NEXTHOP_IS_NOT_VALID_GLOBAL_AND_UNICAST_IPV6_ADDRESS); %>");
			document.route.nextHop.focus();
			return false;
		}
	}

	//check metric
	if ( !checkDigitRange(document.route.metric.value,1,0,16) ) {
		alert("<% multilang(LANG_INVALID_METRIC_RANGE_IT_SHOULD_BE_0_16); %>");
		document.route.metric.focus();
		return false;
	}
	
	return true;
}

function routeClick(url)
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

	window.open( url, 'RouteTbl', settings );
}
	
</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF"><% multilang(LANG_IPV6); %> <% multilang(LANG_STATIC); %> <% multilang(LANG_ROUTING); %><% multilang(LANG_CONFIGURATION); %></font></h2>

<form action=/boaform/formIPv6Routing method=POST name="route">
<table border=0 width="600" cellspacing=4 cellpadding=0>
  <tr><td><font size=2>
    <% multilang(LANG_THIS_PAGE_IS_USED_TO_CONFIGURE_THE_IPV6_STATIC_ROUTING_INFORMATION_HERE_YOU_CAN_ADD_DELETE_STATIC_IP_ROUTES); %>
  </font></td></tr>
  <tr><td><hr size=1 noshade align=top></td></tr>
</table>
<table border=0 width="600" cellspacing=4 cellpadding=0>
  <tr>
      <td width="30%"><font size=2><b><% multilang(LANG_ENABLE); %>:</b></td>
      <td width="70%"><input type="checkbox" name="enable" value="1" checked></td>
  </tr>
  <tr>
      <td width="30%"><font size=2><b><% multilang(LANG_DESTINATION); %>:</b></td>
      <td width="70%"><input type="text" name="destNet" size="40" maxlength="40"></td>
  </tr>
  <tr>
      <td width="30%"><font size=2><b><% multilang(LANG_NEXT_HOP); %>:</b></td>
      <td width="70%"><input type="text" name="nextHop" size="40" maxlength="48"></td>
  </tr>
  <tr>
      <td width="30%"><font size=2><b><% multilang(LANG_METRIC); %>:</b></td>
      <td width="70%"><input type="text" name="metric" size="5" maxlength="5"></td>
  </tr>
  <tr>
      <td width="30%"><font size=2><b><% multilang(LANG_INTERFACE); %>:</b></td>
      <td width="70%"><select name="interface">
          <% if_wan_list("rtv6-any-vpn"); %>
      	</select></td>
  </tr>
  <input type="hidden" value="" name="select_id">
</table>
  <input type="submit" value="<% multilang(LANG_ADD_ROUTE); %>" name="addV6Route" onClick="return addClick()">&nbsp;&nbsp;
  <input type="submit" value="<% multilang(LANG_UPDATE); %>" name="updateV6Route" onClick="return addClick()">&nbsp;&nbsp; 
  <input type="submit" value="<% multilang(LANG_DELETE_SELECTED); %>" name="delV6Route" onClick="return deleteClick()">&nbsp;&nbsp;
  <input type="submit" value="<% multilang(LANG_DELETE_ALL); %>" name="delAllV6Route" onClick="return deleteAllClick()">&nbsp;&nbsp;
  <input type="button" value="<% multilang(LANG_SHOW_ROUTES); %>" name="showV6Route" onClick="routeClick('/routetbl_ipv6.asp')"> 
<table border=0 width="600" cellspacing=4 cellpadding=0>
  <tr><td><hr size=1 noshade align=top></td></tr>
</table>
<table border=0 width="600" cellspacing=4 cellpadding=0>
  <tr><font size=2><b><% multilang(LANG_STATIC_IPV6_ROUTE_TABLE); %>:</b></font></tr>
  <% showIPv6StaticRoute(); %>
</table>
  <br>
      <input type="hidden" value="/routing_ipv6.asp" name="submit-url">
</form>
</blockquote>
</body>

</html>
