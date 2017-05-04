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
<% E8BPktStatsList(); %>
<% E8BDhcpClientList(); %>

/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	if (lsteth.rows) {
		while (lsteth.rows.length > 2)
			lsteth.deleteRow(2);
	}

	for (var i = 0; i < ethers.length; i++) {
		var row = lsteth.insertRow(i + 2);

		row.nowrap = true;
		row.style.verticalAlign = "top";
		row.style.textAlign = "center";

		var cell = row.insertCell(0);
		cell.innerHTML = ethers[i].ifname;
		cell = row.insertCell(1);
		cell.innerHTML = ethers[i].rx_packets;
		cell = row.insertCell(2);
		cell.innerHTML = ethers[i].rx_bytes;
		cell = row.insertCell(3);
		cell.innerHTML = ethers[i].rx_errors;
		cell = row.insertCell(4);
		cell.innerHTML = ethers[i].rx_dropped;
		cell = row.insertCell(5);
		cell.innerHTML = ethers[i].tx_packets;
		cell = row.insertCell(6);
		cell.innerHTML = ethers[i].tx_bytes;
		cell = row.insertCell(7);
		cell.innerHTML = ethers[i].tx_errors;
		cell = row.insertCell(8);
		cell.innerHTML = ethers[i].tx_dropped;
	}

	if (lstdev.rows) {
		while (lstdev.rows.length > 1)
			lstdev.deleteRow(1);
	}

	for (var i = 0; i < clts.length; i++) {
		var row = lstdev.insertRow(i + 1);

		row.nowrap = true;
		row.style.verticalAlign = "top";
		row.style.textAlign = "center";

		var cell = row.insertCell(0);
		cell.innerHTML = clts[i].devname;
		cell = row.insertCell(1);
		cell.innerHTML = clts[i].macAddr;
		cell = row.insertCell(2);
		cell.innerHTML = clts[i].ipAddr;
		cell = row.insertCell(3);
		cell.innerHTML = clts[i].liveTime;
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
					<td class="hdb" width="20%">IP地址</td>
					<td><% getInfo("lan-ip"); %></td>
				</tr>
				<tr>
					<td class="hdb">MAC地址</td>
					<td><% getInfo("elan-Mac"); %></td>
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
					<td>Packets</td>
					<td>Bytes</td>
					<td>Errors</td>
					<td>Dropped</td>
					<td>Packets</td>
					<td>Bytes</td>
					<td>Errors</td>
					<td>Dropped</td>
				</tr>
			</table>
		</div>
		<br><br>
		<div align="left" style="padding-left:20px;">
			<div align="left"><b>内网设备信息(通过DHCP分配的设备)：</b></div>
			<table id="lstdev" class="flat" border="1" cellpadding="1" cellspacing="1">
				<tr class="hdb" align="center" nowrap>
					<td>设备类型</td>
					<td>MAC地址</td>
					<td>IP地址</td>
					<td>剩余租借期</td>
				</tr>
			</table>
		</div>
	</blockquote>
</body>
<!-- add end by liuxiao 2008-01-21 -->
<%addHttpNoCache();%>
</html>
