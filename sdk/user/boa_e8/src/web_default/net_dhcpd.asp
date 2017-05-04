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
<SCRIPT language="javascript" type="text/javascript">

var popUpWin=0;

var cgi = new Object();
<%init_dhcpmain_page();%>

var old_lan_ip = null;

/********************************************************************
**          on document load
********************************************************************/

function on_init()
{
	sji_docinit(document, cgi);
	typeClick();
	old_lan_ip = document.forms[0].uIp.value;
}

/********************************************************************
**          on document update
********************************************************************/
function typeClick(obj)
{
	with ( document.forms[0])
	{
		if(uDhcpType[0].checked  == true)
		{
			dhcpRangeStart.disabled = true;
			dhcpRangeEnd.disabled = true;
			//ipMask.disabled = true;//LGD_FOR_TR098
			ulTime.disabled = true;
			uServerIp.disabled = true;
		}
		else if(uDhcpType[1].checked  == true)
		{
			dhcpRangeStart.disabled = false;
			dhcpRangeEnd.disabled = false;
			//ipMask.disabled = false;//LGD_FOR_TR098
			ulTime.disabled = false;
			uServerIp.disabled = true;
		}
		else
		{
			dhcpRangeStart.disabled = true;
			dhcpRangeEnd.disabled = true;
			//ipMask.disabled = true;//LGD_FOR_TR098
			ulTime.disabled = true;
			uServerIp.disabled = false;
		}
	}
}

function popUpWindow(URLStr, left, top, width, height)
{
	if(popUpWin)
	{
		if(!popUpWin.closed) popUpWin.close();
	}
	popUpWin = open(URLStr, "popUpWin", "toolbar=yes,location=no,directories=no,status=no,menubar=yes,scrollbars=yes,resizable=yes,copyhistory=yes,width="+width+",height="+height+",left="+left+", top="+top+",screenX="+left+",screenY="+top+"");
}

function dhcpDevice()
{
	var loc = "net_dhcpdevice.asp";
	var code = "location=\"" + loc + "\"";
	eval(code);
}

function isSameSubNet(lan1Ip, lan1Mask, lan2Ip, lan2Mask)
{
	var count = 0;

	lan1a = lan1Ip.split(".");
	lan1m = lan1Mask.split(".");
	lan2a = lan2Ip.split(".");
	lan2m = lan2Mask.split(".");

	for (i = 0; i < 4; i++)
	{
		l1a_n = parseInt(lan1a[i]);
		l1m_n = parseInt(lan1m[i]);
		l2a_n = parseInt(lan2a[i]);
		l2m_n = parseInt(lan2m[i]);
		if ((l1a_n & l1m_n) == (l2a_n & l2m_n))
			count++;
	}
	if (count == 4)
		return true;
	else
		return false;
}

/********************************************************************
**          on document submit
********************************************************************/

function on_submit(reboot)
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
			if ( sji_checkvip(uIp.value) == false )
			{
				uIp.focus();
				alert("IP地址 \"" + uIp.value + "\" 是无效的IP地址.");
				return;
			}
			if ( sji_checkmask(uMask.value) == false )
			{
				uMask.focus();
				alert("子网掩码 \"" + uMask.value + "\" 是无效的子网掩码.");
				return;
			}
			if ( uDhcpType[1].checked == true )
			{
				/*
				//LGD_FOR_TR098
				if ( sji_checkmask(ipMask.value) == false )
				{
					ipMask.focus();
					alert("DHCP 子网掩码 \"" + ipMask.value + "\" 是无效的子网掩码.");
					return;
				}
				*/

				if (sji_checkvip(dhcpRangeStart.value) == false || !(isSameSubNet(uIp.value, uMask.value, dhcpRangeStart.value, uMask.value)))
				{
					dhcpRangeStart.focus();
					alert("初始IP地址\"" + dhcpRangeStart.value + "\"是无效IP地址.");
					return;
				}
				if ( sji_checkvip(dhcpRangeEnd.value) == false || !(isSameSubNet(uIp.value, uMask.value, dhcpRangeEnd.value, uMask.value)))
				{
					dhcpRangeEnd.focus();
					alert("终止IP地址\"" + dhcpRangeEnd.value + "\"是无效IP地址.");
					return;
				}
				if (sji_ipcmp(dhcpRangeStart.value, dhcpRangeEnd.value) > 0)
				{
					alert("终止IP地址必须等于或大于初始IP地址.");
					return;
				}
			}
			else if ( uDhcpType[2].checked == true )
			{
				if ( sji_checkvip(uServerIp.value) == false )
				{
					uServerIp.focus();
					alert("DHCP服务器IP地址 \"" + uServerIp.value + "\" 是无效的IP地址.");
					return;
				}
			}
			if(old_lan_ip != uIp.value)
				alert("您已经将 IP 地址修改成 \"" + uIp.value + "\"，之后请由此 IP 地址连入路由器。也请记得修改装置的DHCP地址区间，确保其他装置可以顺利联网。");
			submit();
		}
	}
}
</script>
</head>
<!-------------------------------------------------------------------------------------->
<!--主页代码-->
	<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
		<blockquote>
			<DIV align="left" style="padding-left:20px; padding-top:10px">
				<form action=/boaform/formDhcpServer method=POST name="dhcpd">
					<b>局域网(LAN)&nbsp;设置</b><br>
					<br>
					为局域网配置家庭网关IP地址和子网掩码接口。&nbsp;点击"应用"按钮使LAN配置生效。<!--&nbsp; "重启"按钮保存局域网配置数据和重启路由器以使新的配置持久生效。--><br>
					<div id="rstip" style="display:none;"><font color="red">提示：本页面的设置，需要重启路由器才能生效！</font><br></div>
					<hr align="left" class="sep" size="1" width="90%">
					<table border="0" cellpadding="0" cellspacing="0">
						<tr>
						<td width="150">IP地址:</td>
						<td><input type="text" name="uIp" value=<% getInfo("dhcplan-ip"); %>></td>
						</tr>
						<tr>
						<td>子网掩码:</td>
						<td><input type="text" name="uMask" value=<% getInfo("dhcplan-subnet"); %>></td>
						</tr>
					</table>
					<br>
					<div id="dhcpInfo">
					<table border="0" cellpadding="0" cellspacing="0">
						<tr>
						<td colspan="2"><input type="radio" name="uDhcpType" value = "0" onClick="typeClick(this);">&nbsp;&nbsp;禁用DHCP服务器</td>
						</tr>
						<tr>
						<td colspan="2"><input type="radio" name="uDhcpType" value = "1" onClick="typeClick(this);">&nbsp;&nbsp;启用DHCP服务器</td>
						</tr>
						<tr>
						<td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;初始IP地址:</td>
						<td><input type="text" name="dhcpRangeStart" value=<% getInfo("lan-dhcpRangeStart") %>></td>
						</tr>
						<tr>
						<td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;终止IP地址:</td>
						<td><input type="text" name="dhcpRangeEnd" value=<% getInfo("lan-dhcpRangeEnd") %>></td>
						</tr>
<!--
						<tr>
						<td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;子网掩码:</td>
						<td><input type="text" name="ipMask"></td>
						<td><input type="hidden" name="ipMask"></td>
						</tr>
-->
						<tr>
						<td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;延续时间:</td>
						<td><select size="1" name="ulTime">
						<option value="60">一分钟</option>
						<option value="3600">一小时</option>
						<option  value="86400" >一天</option>
						<option value="604800">一周</option>
						</select>
						</td>
						</tr>
					</table>
					</div>

					<div id="relayInfo">
					<table border="0" cellpadding="0" cellspacing="0">
						<tr>
						<td colspan="2"><input type="radio" name="uDhcpType" value = "2" onClick="typeClick(this)">&nbsp;&nbsp;启用DHCP服务器中继</td>
						</tr>
						<tr>
						<td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;DHCP服务器IP地址:&nbsp;&nbsp;</td>
						<td><input type="text" name="uServerIp" value=<% getInfo("wan-dhcps"); %>></td>
						</tr>
					</table>
					</div>
					<br>
<!--
					<b>&nbsp;&nbsp;预留IP地址</b><br>
					<br>
					&nbsp;&nbsp;选择"编辑预留IP地址列表" 来配置预留IP地址列表。<br>
					&nbsp;&nbsp;注意1:最多预留10个ip地址和特殊的mac。 <br>
					&nbsp;&nbsp;注意2:当新增加了一个预留IP，必须重启设备使配置生效。<br>
					<br>
-->
					<table border="0" width="520">
						<tr>
						<td>
						<INPUT type="button" class="button" onClick="popUpWindow('net_mopreipaddr.asp',350,350,500,300);" value="编辑预留IP地址列表">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
						<INPUT type="button" class="button" onClick="dhcpDevice()" value="编辑DHCP地址区间">
						</td>
						</tr>
					</table>
					<br>

					<hr align="left" class="sep" size="1" width="90%">
					<br>
					<input type="button" class="button" onClick="on_submit(0);" value="应用">&nbsp; &nbsp; &nbsp; &nbsp;
					<!--<input type="button" class="button" onClick="on_submit(1);" value="重启">-->
					<input type="hidden" name="submit-url" value="">
				</form>
			</div>
		</blockquote>
	</body>
<%addHttpNoCache();%>
<script>
	<% initPage("dhcp-mode"); %>

	var lease_time = <% getInfo("lan-dhcpLTime"); %>;

	if(lease_time == 60)
		document.dhcpd.ulTime.selectedIndex = 0;
	else if(lease_time == 3600)
		document.dhcpd.ulTime.selectedIndex = 1;
	else if(lease_time == 86400)
		document.dhcpd.ulTime.selectedIndex = 2;
	else if(lease_time == 604800)
		document.dhcpd.ulTime.selectedIndex = 3;
	else
		document.dhcpd.ulTime.selectedIndex = -1;
</script>
</html>
