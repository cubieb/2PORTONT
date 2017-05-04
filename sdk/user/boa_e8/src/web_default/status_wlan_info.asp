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

var rcs = new Array();
<% wlStatsList(); %>
<% checkWrite("wl_txRate"); %>	

/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	if (lsteth.rows) {
		while (lsteth.rows.length > 2)
			lsteth.deleteRow(2);
	}

	for (var i = 0; i < rcs.length; i++) {
		var row = lsteth.insertRow(i + 2);

		row.nowrap = true;
		row.style.verticalAlign = "top";
		row.style.textAlign = "center";

		var cell = row.insertCell(0);
		cell.innerHTML = rcs[i].ifname;
		cell = row.insertCell(1);
		cell.innerHTML = rcs[i].rx_packets;
		cell = row.insertCell(2);
		cell.innerHTML = rcs[i].rx_bytes;
		cell = row.insertCell(3);
		cell.innerHTML = rcs[i].rx_errors;
		cell = row.insertCell(4);
		cell.innerHTML = rcs[i].rx_dropped;
		cell = row.insertCell(5);
		cell.innerHTML = rcs[i].tx_packets;
		cell = row.insertCell(6);
		cell.innerHTML = rcs[i].tx_bytes;
		cell = row.insertCell(7);
		cell.innerHTML = rcs[i].tx_errors;
		cell = row.insertCell(8);
		cell.innerHTML = rcs[i].tx_dropped;
	}
}

function get_rate(isMulticast, curRate)
{
	var rate_mask = new Array(3,1,1,1,1,2,2,2,2,2,2,2,2);
	var rate_name = new Array("Auto","1M","2M","5.5M","11M","6M","9M","12M","18M","24M","36M","48M","54M");
	var rate, mask, i, defidx, idx;
	var isAuto; 

	if(isMulticast) {
		if( curRate == 0)
			isAuto = 1;
	}
	else {
		isAuto = auto;
	}
	
	mask=0;
	if (isAuto)
		curRate=0;
	if (band & 1)
		mask |= 1;
	if ((band&2) || (band&4))
		mask |= 2;
	defidx=0;
	for (idx=0, i=0; i<=12; i++) {
		if (rate_mask[i] & mask) {
			if (i == 0)
				rate = 0;
			else
				rate = (1 << (i-1));
			if (curRate == rate)
				defidx = idx;
			idx++;
		}
	}
	document.write(rate_name[defidx]);
}
    
</script>
</head>
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
	<blockquote>
		<div align="left" style="padding-left:20px;"><br>
			<div align="left"><b>WLAN接口信息</b></div>
			<div align="left" style="width:768px;"><br>
			<table class="flat" border="1" cellpadding="0" cellspacing="1">
				<tr>
					<td width="20%" class="hdb">无线网络连接状态:</td>
					<td><% getInfo("wlanState"); %></td>
				</tr>
				<tr>
					<td class="hdb">信道:</td>
					<td><% getInfo("wlDefChannel"); %></td>
				</tr>
				<tr>
					<td class="hdb">54g&#8482;速率:</td>
					<td>
						<SCRIPT>
							get_rate(0, txrate);						
						</SCRIPT>
					</td>
				</tr>
				<tr>
					<td class="hdb">多播速率:</td>
					<td>
						<SCRIPT>
							get_rate(1, mulrate);		
						</SCRIPT>
					</td>
				</tr>
				<tr>
					<td class="hdb">54g&#8482;模式:</td>
					<td><% getInfo("wlanMode"); %></td>
				</tr>
				<tr>
					<td class="hdb">发送功率:</td>
					<td><% getInfo("wlTxPower"); %></td>
				</tr>
				<tr>
					<td class="hdb">BSSID:</td>
					<td><% getInfo("wlanBssid"); %></td>
				</tr>
				<tr>
					<td class="hdb">SSID隐藏属性:</td>
					<td><% getInfo("wlanSsidAttr"); %></td>
				</tr>
				<tr>
					<td class="hdb">SSID-1名称:</td>
					<td><% getInfo("ssidName"); %></td>
				</tr>
				<tr>
					<td class="hdb">SSID-1加密状态:</td>
					<td><% getInfo("encryptState"); %></td>
				</tr>
			</table>
			</div>
		</div>
		<br><br>
		<!--
		<div align="left" style="padding-left:20px;"><br>
			<div align="left"><b>收发包情况：</b></div>
			<div align="left" style="width:768px;"><br>
			<table class="flat" border="1" cellpadding="0" cellspacing="1">
			</table>
			</div>
		</div>
		-->
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
					<td>Pkts</td>
					<td>Errs</td>
					<td>Drops</td>
					<td>Bytes</td>
					<td>Pkts</td>
					<td>Errs</td>
					<td>Drops</td>
					<td>Bytes</td>
				</tr>
			</table>
		</div>
	</blockquote>
</body>
<!-- add end by liuxiao 2008-01-21 -->
<%addHttpNoCache();%>
</html>
