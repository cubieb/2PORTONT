<!-- add by liuxiao 2008-01-21 -->
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
<head>
<title>下挂设备实施带宽监测</title>
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

var lanBandwidthInfo = new Array();
with(lanBandwidthInfo){<% initPageLanBandwidthMonitor(); %>}

/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	
	if (lstLanHost.rows) {
		while (lstLanHost.rows.length > 1)
			lstLanHost.deleteRow(1);
	}

	for (var i = 0; i < lanBandwidthInfo.length; i++) {
		var row = lstLanHost.insertRow(i + 1);

		row.nowrap = true;
		row.style.verticalAlign = "top";
		row.style.textAlign = "center";

		var cell = row.insertCell(0);;
		cell.innerHTML = lanBandwidthInfo[i].mac;
		cell = row.insertCell(1);
		cell.innerHTML = lanBandwidthInfo[i].cur_usBand;
		cell = row.insertCell(2);
		cell.innerHTML = lanBandwidthInfo[i].cur_dsBand;
	}
}
</SCRIPT>
</head>

<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
	<blockquote>
		<div align="left" style="padding-left:20px;">
			<br>
			<div align="left"><b>家庭网关下挂设备实施带宽监测:</b></div>
			<br>
				<table id="lstLanHost" class="flat" border="1" cellpadding="1" cellspacing="1">
					<tr class="hdb" align="center" nowrap>
						<td width="120px">MAC地址</td>
						<td width="120px" id="cur_usBand">上行实时带宽(bps)</td>
						<td width="120px" id="cur_dsBand">下行实时带宽(bps)</td>
					</tr>
				</table>	
		</div>
	</blockquote>
</body>
<!-- add end by martin ZHU 2016-01-13 -->
<%addHttpNoCache();%>
</html>
