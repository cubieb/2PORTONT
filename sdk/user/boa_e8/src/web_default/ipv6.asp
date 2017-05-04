<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>中国电信-DHCP</TITLE>
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
<SCRIPT language="javascript" src="share.js"></SCRIPT>
<SCRIPT language="javascript" type="text/javascript">

var popUpWin=0;

//var cgi = new Object();

/********************************************************************
**          on document load
********************************************************************/

function on_init()
{
//	sji_docinit(document, cgi);
}

/********************************************************************
**          on document update
********************************************************************/

function popUpWindow(URLStr, left, top, width, height)
{
	if(popUpWin)
	{
		if(!popUpWin.closed) popUpWin.close();
	}
	popUpWin = open(URLStr, "popUpWin", "toolbar=yes,location=no,directories=no,status=no,menubar=yes,scrollbars=yes,resizable=yes,copyhistory=yes,width="+width+",height="+height+",left="+left+", top="+top+",screenX="+left+",screenY="+top+"");
}


/********************************************************************
**          on document submit
********************************************************************/

function on_lanipv6addrform_submit(reboot)
{
	if(reboot)
	{
		var loc = "mgm_dev_reboot.asp";
		var code = "location.assign(\"" + loc + "\")";
		eval(code);
	}
	else
	{
		with ( document.forms[0] )
		{
			if ( document.lanipv6addrform.lanIpv6addr.value == "" )
			{
				document.lanipv6addrform.lanIpv6addr.focus();
				alert("IP地址 \"" + document.lanipv6addrform.lanIpv6addr.value + "\" 是无效的IP地址.");
				return false;
			}
			else
			{
				if ( isLinkLocalIpv6Address(document.lanipv6addrform.lanIpv6addr.value) == 0) {
					alert("无效的LAN IPv6地址!");	//Invalid LAN IPv6 address!
					document.lanipv6addrform.lanIpv6addr.focus();
					document.lanipv6addrform.lanIpv6addr.value ="<% checkWrite("lanipv6addr"); %>";
					return false;
				}

			}
			submit();
		}
	}
}




function prefixModeChange()
{
	with ( document.lanipv6prefixmodeform )
	{
		var prefix_mode =ipv6lanprefixmode.value;
		
		v6delegated_WANConnection.style.display = 'none';
		staticipv6prefix.style.display = 'none';
		switch(prefix_mode){
			case '0': //WANDelegated
					v6delegated_WANConnection.style.display = 'block';
					break;
			case '1': //Static
					staticipv6prefix.style.display = 'block';
					break;
					
		}
	}
}

function dnsModeChange()
{
	with ( document.lanipv6dnsform )
	{
		var dns_mode =ipv6landnsmode.value;
		
		v6dns_WANConnection.style.display = 'none';	
		v6dns_Staic.style.display = 'none';
		switch(dns_mode){
			case '0': //HGWProxy
					break;
			case '1': //WANConnection
					v6dns_WANConnection.style.display = 'block';
					break;
			case '2': //Static
					v6dns_Staic.style.display = 'block';
					if(Ipv6Dns1.value == "::") //clear the value
						Ipv6Dns1.value ="";
					if(Ipv6Dns2.value == "::")
						Ipv6Dns2.value ="";

					break;
					
		}
	}

}

//Handle DNSv6 mode
function on_lanipv6dnsform_submit(reboot)
{

	with ( document.lanipv6dnsform )
	{
	
		if ( ipv6landnsmode.value==2 ){  //static
				if(Ipv6Dns1.value == "" && Ipv6Dns2.value == "" )  //Both DNS setting is NULL
				{
					Ipv6Dns1.focus();
					alert("IPv6　DNS 地址 " + Ipv6Dns1.value + "\" 是无效的IPv6 Prefix 地址.");
					return false;
				}
				else if (Ipv6Dns1.value != "" || Ipv6Dns2.value != ""){
					if(Ipv6Dns1.value != "" ){
						if (! isUnicastIpv6Address( Ipv6Dns1.value) ){
								alert("首选 IPv6 DNS 地址\"" + Ipv6Dns1.value + "\"为无效地址，请重新输入！");
								Ipv6Dns1.focus();
								return false;
						}
					}
					if(Ipv6Dns2.value != "" ){
						if (! isUnicastIpv6Address( Ipv6Dns2.value) ){
								alert("首选 IPv6 DNS 地址\"" + Ipv6Dns2.value + "\"为无效地址，请重新输入！");
								Ipv6Dns2.focus();
								return false;
						}
					}
				}
		}	
		submit();
	}	
}

//Handle Prefix v6 mode
function on_lanipv6prefixmodeform_submit(reboot)
{
	with ( document.lanipv6prefixmodeform )
	{
			if ( ipv6lanprefixmode.value==1 ){
				if(document.lanipv6prefixmodeform.lanIpv6prefix.value == "" )
				{
					document.lanipv6prefixmodeform.lanIpv6prefix.focus();
					alert("IP地址 \"" + document.lanipv6prefixmodeform.lanIpv6prefix.value + "\" 是无效的IPv6 Prefix 地址.");
					return false;
				}
				else if ( validateKeyV6Prefix(document.lanipv6prefixmodeform.lanIpv6prefix.value) == 0) { //check if is valid ipv6 address
					alert("无效的LAN IPv6 前缀长度!");	
					document.lanipv6prefixmodeform.lanIpv6prefix.focus();
					return false;
				}
			}
			submit();
	}
}

/*
function on_lanipv6prefixform_submit(reboot)
{
	if(reboot)
	{
		var loc = "mgm_dev_reboot.asp";
		var code = "location.assign(\"" + loc + "\")";
		eval(code);
	}
	else
	{
		with ( document.lanipv6prefixform )
		{
			if ( document.lanipv6prefixform.lanIpv6prefix.value == "" )
			{
				document.lanipv6addrform.lanIpv6prefix.focus();
				alert("IP地址 \"" + document.lanipv6prefixform.lanIpv6prefix.value + "\" 是无效的IPv6 Prefix 地址.");
				return false;
			}
		
			else
			{
				if ( validateKeyV6Prefix(document.lanipv6prefixform.lanIpv6prefix.value) == 0) {
					alert("无效的LAN IPv6 前缀长度!");	
					document.formlanipv6prefix.lanIpv6prefix.focus();
					document.formlanipv6prefix.lanIpv6prefix.value ="<% checkWrite("lanIpv6prefix"); %>";
					return false;
				}
			}
					
			submit();
		}
	}
}
*/

</script>
</head>
<!-------------------------------------------------------------------------------------->
<!--主页代码-->
	<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
		<blockquote>
			<DIV align="left" style="padding-left:20px; padding-top:10px">
				<form action=/boaform/formlanipv6 method=POST name="lanipv6addrform">
					<b>局域网(LAN) IPv6地址&nbsp;设置</b><br>
					<br>
					为局域网配置家庭网关IPv6地址。&nbsp;点击"应用"按钮使LAN IPv6地址配置生效。<!--&nbsp; "重启"按钮保存局域网配置数据和重启路由器以使新的配置持久生效。--><br>
					<div id="rstip" style="display:none;"><br></div>
					<hr align="left" class="sep" size="1" width="90%">
					<table border="0" cellpadding="0" cellspacing="0">
						<tr>
						<td width="150">IPv6地址:</td>
						<td><input type="text" name="lanIpv6addr" value=<% checkWrite("lanipv6addr"); %>></td>
						</tr>
					</table>

					<hr align="left" class="sep" size="1" width="90%">
					<br>
					<input type="button" class="button" onClick="on_lanipv6addrform_submit(0);" value="保存/应用">&nbsp; &nbsp; &nbsp; &nbsp;
					<input type="hidden" value="/ipv6.asp" name="submit-url">
				</form>
			</div>
			
			<DIV align="left" style="padding-left:20px; padding-top:10px">
				<form action=/boaform/formlanipv6dns method=POST name="lanipv6dnsform">
					<b>局域网(LAN) DNS Server&nbsp;设置</b><br>
					<br>
					为局域网配置家庭网关 LAN IPv6 DNS Server。&nbsp;点击"应用"按钮使LAN IPv6 DNS Server配置生效。<br><!--&nbsp; "重启"按钮保存局域网配置数据和重启路由器以使新的配置持久生效。--><br>
					<div id="rstip" style="display:none;"><br></div>
					<hr align="left" class="sep" size="1" width="90%">
					<table border="0" cellpadding="0" cellspacing="0">
						<tr>
						<td width="150">LAN侧DNS获取方式</td>
						<td><select name="ipv6landnsmode"  onChange="dnsModeChange()">
  							<option value="0">HGWProxy</option>
							<option value="1">WANConnection</option>
  							<option value="2">Static</option>
							</select></td> 
						</tr>
						<tr><td><div id='v6dns_WANConnection' style="display:none;"> 接口: <select name="ext_if" > <% if_wan_list("rtv6"); %> </select></div> </td><tr>
						
						
					</table>
					
					<div id='v6dns_Staic' style="display:none;"> 
					<table border="0" cellpadding="0" cellspacing="0">
						<tr nowrap>		
					<tr>
						<td width="150px">首选 IPv6 DNS:</td>
						<td><input type="text" name="Ipv6Dns1" size="36" maxlength="39" value=<% getInfo("wan-dnsv61"); %> style="width:150px"></td>
					</tr>
					<tr>
						<td width="150px">备选 IPv6 DNS:</td>
						<td><input type=text name="Ipv6Dns2" size="36" maxlength="39" value=<% getInfo("wan-dnsv62"); %> style="width:150px"></td>
					</tr>
					</table>
						</div>

					<hr align="left" class="sep" size="1" width="90%">
					<br>
					<input type="button" class="button" onClick="on_lanipv6dnsform_submit(0);" value="保存/应用">&nbsp; &nbsp; &nbsp; &nbsp;
					<input type="hidden" value="/ipv6.asp" name="submit-url">
				</form>
			</div>
			
			
									<DIV align="left" style="padding-left:20px; padding-top:10px">
				<form action=/boaform/formlanipv6prefix method=POST name="lanipv6prefixmodeform">
					<b>前辍来源设置</b><br>
					<br>
					为局域网配置家庭网关 LAN IPv6 前辍来源;点击"应用"按钮使LAN IPv6 前辍来源配置生效。<br>
					<div id="rstip" style="display:block;"><br></div>
					<hr align="left" class="sep" size="1" width="90%">
					<table border="0" cellpadding="0" cellspacing="0">
						<tr>
						<td width="150">LAN侧前辍获取方式</td>
						<td><select name="ipv6lanprefixmode"  onChange="prefixModeChange()">
  							<option value="0">WANDelegated </option>
							<option value="1">Static</option>
							</select></td> 
						</tr>
						<tr><td><div id='v6delegated_WANConnection' style="display:none;"> 接口: <select name="ext_if" > <% if_wan_list("rtv6"); %> </select></div> </td><tr>
						
						
					</table>
					
					<div id="staticipv6prefix" style="display:none;">
					<table border="0" cellpadding="0" cellspacing="0">
						<tr>
						<td width="150">IPv6 前缀 : (例: 2001::/64)</td>
						<td><input type="text" name="lanIpv6prefix" value=<% checkWrite("lanipv6prefix"); %>></td>
						</tr>
					</table>
					</div>

					<hr align="left" class="sep" size="1" width="90%">
					<br>
					<input type="button" class="button" onClick="on_lanipv6prefixmodeform_submit(0);" value="保存/应用">&nbsp; &nbsp; &nbsp; &nbsp;
					<input type="hidden" value="/ipv6.asp" name="submit-url">
				</form>
			</div>
		</blockquote>
		
		
<script>

	ifIdx = <% getInfo("prefix-delegation-wan-conn"); %>;
	if (ifIdx != 65535)
		document.lanipv6prefixmodeform.ext_if.value = ifIdx;
	else
		document.lanipv6prefixmodeform.ext_if.selectedIndex = 0;

	document.lanipv6prefixmodeform.ipv6lanprefixmode.value = <% getInfo("prefix-mode"); %>;
	
		ifIdx = <% getInfo("dns-wan-conn"); %>;
	if (ifIdx != 65535)
		document.lanipv6dnsform.ext_if.value = ifIdx;
	else
		document.lanipv6dnsform.ext_if.selectedIndex = 0;
	
	document.lanipv6dnsform.ipv6landnsmode.value=<% getInfo("dns-mode"); %>;
	
	prefixModeChange();
	dnsModeChange();
</script>
	</body>
</html>
