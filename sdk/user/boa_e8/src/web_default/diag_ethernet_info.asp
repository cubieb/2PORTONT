<!-- add by liuxiao 2008-01-21 -->
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
<head>
<title>中国电信</title>
<meta http-equiv=pragma content=no-cache>
<meta http-equiv=cache-control content="no-cache, must-revalidate">
<meta http-equiv=content-type content="text/html; charset=gbk">
<meta http-equiv="refresh" content="5">
<meta http-equiv=content-script-type content=text/javascript>
<style type=text/css>
@import url(/style/default.css);
</style>
<script language="javascript" src="common.js"></script>
<SCRIPT language="javascript" type="text/javascript">

var ethers = new Array();
var clts = new Array();
<% pktStatsList(); %>
<% dhcpClientList(); %>

/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	if(lsteth.rows){while(lsteth.rows.length > 2) lsteth.deleteRow(2);}
	for(var i = 0; i < ethers.length; i++)
	{
		var row = lsteth.insertRow(i + 2);
		
		row.nowrap = true;
		row.vAlign = "top";
		row.align = "center";
		
		var cell = row.insertCell(0);
		cell.innerHTML = ethers[i].ifname;
		cell = row.insertCell(1);
		cell.innerHTML = ethers[i].rx_byte;
		cell = row.insertCell(2);
		cell.innerHTML = ethers[i].rx_pkt;
		cell = row.insertCell(3);
		cell.innerHTML = ethers[i].rx_err;
		cell = row.insertCell(4);
		cell.innerHTML = ethers[i].rx_drop;
		cell = row.insertCell(5);
		cell.innerHTML = ethers[i].tx_byte;
		cell = row.insertCell(6);
		cell.innerHTML = ethers[i].tx_pkt;
		cell = row.insertCell(7);
		cell.innerHTML = ethers[i].tx_err;
		cell = row.insertCell(8);
		cell.innerHTML = ethers[i].tx_drop;
	}
	
	if(lstdev.rows){while(lstdev.rows.length > 1) lstdev.deleteRow(1);}
	for(var i = 0; i < clts.length; i++)
	{
		var row = lstdev.insertRow(i + 1);
		
		row.nowrap = true;
		row.vAlign = "top";
		row.align = "center";
		
		var cell = row.insertCell(0);
		cell.innerHTML = clts[i].devname;
		cell = row.insertCell(1);
		cell.innerHTML = clts[i].uIpAddr;
	}
}
</SCRIPT>
</head>

<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
	<blockquote>
		<div align="left" style="padding-left:20px;"><br>
			<div align="left"><b>家庭网关信息：</b></div>
			<table class="flat" border="1" cellpadding="1" cellspacing="1">
				<tr>
					<td class="hdb" width="20%">IP地址: </td>
					<td><% getInfo("lanip"); %></td>
				</tr>
				<tr>
					<td class="hdb">MAC地址:</td>
					<td><% getInfo("lanmac"); %></td>
				</tr>
			</table>
		</div>
		<br><br>
		<div align="left" style="padding-left:20px;">
			<div align="left"><b>收发包情况：</b></div>
			<table id="lsteth" class="flat" border="1" cellpadding="1" cellspacing="1">
			   <tr class="hdb" align="center" nowrap>
					<td>接口</td>
					<td colspan="4">接收</td>
					<td colspan="4">发送</td>
				</tr>
			   <tr class="hdb" align="center" nowrap>
					<td>&nbsp;</td>
					<td>Bytes</td>
					<td>Pkts</td>
					<td>Errs</td>
					<td>Drops</td>
					<td>Bytes</td>
					<td>Pkts</td>
					<td>Errs</td>
					<td>Drops</td>
				</tr>
			</table>
		</div>
		<br><br>
		<div align="left" style="padding-left:20px;">
			<div align="left"><b>内网设备信息(通过DHCP分配的设备)：</b></div>
			<table id="lstdev" class="flat" border="1" cellpadding="1" cellspacing="1">
				<tr class="hdb">
					<td>设备类型</td>
					<td>IP地址</td>
				</tr>
			</table>
		</div>
	</blockquote>
</body>
<!-- add end by liuxiao 2008-01-21 -->
<%addHttpNoCache();%>
</html>
