<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>静态路由</TITLE>
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
<STYLE type=text/css>
@import url(/style/default.css);
</STYLE>
<SCRIPT language="javascript" type="text/javascript">
function postGW( enable, destNet, subMask, nextHop, metric, interface, select )
{
	document.route.enable.checked = enable;
	document.route.destNet.value=destNet;
	document.route.subMask.value=subMask;
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
	/*if (document.route.destNet.value=="") {
		alert("Enter Destination Network ID !");
		document.route.destNet.focus();
		return false;
	}
	
	if ( validateKey( document.route.destNet.value ) == 0 ) {
		alert("Invalid Destination value.");
		document.route.destNet.focus();
		return false;
	}
	if ( !checkDigitRange(document.route.destNet.value,1,0,255) ) {
		alert('Invalid Destination range in 1st digit. It should be 0-255.');
		document.route.destNet.focus();
		return false;
	}
	if ( !checkDigitRange(document.route.destNet.value,2,0,255) ) {
		alert('Invalid Destination range in 2nd digit. It should be 0-255.');
		document.route.destNet.focus();
		return false;
	}
	if ( !checkDigitRange(document.route.destNet.value,3,0,255) ) {
		alert('Invalid Destination range in 3rd digit. It should be 0-255.');
		document.route.destNet.focus();
		return false;
	}
	if ( !checkDigitRange(document.route.destNet.value,4,0,254) ) {
		alert('Invalid Destination range in 4th digit. It should be 0-254.');
		document.route.destNet.focus();
		return false;
	}
	
	if (document.route.subMask.value=="") {
		alert("Enter Subnet Mask !");
		document.route.subMask.focus();
		return false;
	}
	
	if ( validateKey( document.route.subMask.value ) == 0 ) {
		alert("Invalid Subnet Mask value.");
		document.route.subMask.focus();
		return false;
	}
	if ( !checkDigitRange(document.route.subMask.value,1,0,255) ) {
		alert('Invalid Subnet Mask range in 1st digit. It should be 0-255.');
		document.route.subMask.focus();
		return false;
	}
	if ( !checkDigitRange(document.route.subMask.value,2,0,255) ) {
		alert('Invalid Subnet Mask range in 2nd digit. It should be 0-255.');
		document.route.subMask.focus();
		return false;
	}
	if ( !checkDigitRange(document.route.subMask.value,3,0,255) ) {
		alert('Invalid Subnet Mask range in 3rd digit. It should be 0-255.');
		document.route.subMask.focus();
		return false;
	}
	if ( !checkDigitRange(document.route.subMask.value,4,0,255) ) {
		alert('Invalid Subnet Mask range in 4th digit. It should be 0-255.');
		document.route.subMask.focus();
		return false;
	}
	if (document.route.interface.value==65535) {
	if (document.route.nextHop.value=="" ) {
		alert("Enter Next Hop IP or select a GW interface!");
		document.route.nextHop.focus();
		return false;
	}
	
	if ( validateKey( document.route.nextHop.value ) == 0 ) {
		alert("Invalid Next Hop value.");
		document.route.nextHop.focus();
		return false;
	}
	if ( !checkDigitRange(document.route.nextHop.value,1,0,255) ) {
		alert('Invalid Next Hop range in 1st digit. It should be 0-255.');
		document.route.nextHop.focus();
		return false;
	}
	if ( !checkDigitRange(document.route.nextHop.value,2,0,255) ) {
		alert('Invalid Next Hop range in 2nd digit. It should be 0-255.');
		document.route.nextHop.focus();
		return false;
	}
	if ( !checkDigitRange(document.route.nextHop.value,3,0,255) ) {
		alert('Invalid Next Hop range in 3rd digit. It should be 0-255.');
		document.route.nextHop.focus();
		return false;
	}
	if ( !checkDigitRange(document.route.nextHop.value,4,1,254) ) {
		alert('Invalid Next Hop range in 4th digit. It should be 1-254.');
		document.route.nextHop.focus();
		return false;
	}*/
	if (checkDest(document.route.destNet, document.route.subMask) == true) {
		alert('(目的 & 子网掩码) != 目的.');
		document.route.subMask.focus();
		return false;
	}
	if (!checkHostIP(document.route.destNet, 1))
		return false;
	if (!checkNetmask(document.route.subMask, 1))
		return false;
	if (document.route.interface.value==65535) {
		if (document.route.nextHop.value=="" ) {
			alert("关闭地址不能是空的!");
			document.route.nextHop.focus();
			return false;
		}

		if (!checkHostIP(document.route.nextHop, 0))
			return false;
	}
	if ( !checkDigitRange(document.route.metric.value,1,0,16) ) {
		alert('Metric无效. 合理范围是 0~16.');
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

<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000">
<blockquote>
<DIV align="left" style="padding-left:20px; padding-top:5px">

<form action=/boaform/formRouting method=POST name="route">
<div class="tip" style="width:90% ">
	<b>路由 -- 静态路由</b><br><br>	
</div>
<table border=0 width="600" cellspacing=4 cellpadding=0>
  <tr><hr size=1 noshade align=top></tr>
  <tr>
      <td width="30%">使能:</td>
      <td width="70%"><input type="checkbox" name="enable" value="1" checked></td>
  </tr>
  <tr>
      <td width="30%">目的:</td>
      <td width="70%"><input type="text" name="destNet" size="15" maxlength="15"></td>
  </tr>
  <tr>
      <td width="30%">子网掩码:</td>
      <td width="70%"><input type="text" name="subMask" size="15" maxlength="15"></td>
  </tr>
  <tr>
      <td width="30%">网关:</td>
      <td width="70%"><input type="text" name="nextHop" size="15" maxlength="15"></td>
  </tr>
  <tr>
      <td width="30%">Metric:</td>
      <td width="70%"><input type="text" name="metric" size="5" maxlength="5"></td>
  </tr>
  <tr>
      <td width="30%">接口:</td>
      <td width="70%"><select name="interface">
          <%  if_wan_list("rt-any");%>
      	</select></td>
  </tr>
  <input type="hidden" value="" name="select_id">
</table>
  <input type="submit" value="添加" name="addRoute" onClick="return addClick()">&nbsp;&nbsp;
  <input type="submit" value="更新" name="updateRoute" onClick="return addClick()">&nbsp;&nbsp;
  <input type="submit" value="删除" name="delRoute" onClick="return deleteClick()">&nbsp;&nbsp;
  <input type="button" value="显示路由" name="showRoute" onClick="routeClick('/routetbl.asp')">
  </tr>
  <tr><hr size=1 noshade align=top></tr>
<table border=0 width="600" cellspacing=4 cellpadding=0>
  <tr>静态路由表:</tr>
  <% showStaticRoute(); %>
</table>
  <br>
      <input type="hidden" value="/routing.asp" name="submit-url">
		<!--
		<% GetDefaultGateway(); %>
		-->
</form>
</DIV>
</blockquote>
</body>

</html>
