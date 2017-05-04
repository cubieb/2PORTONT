<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<html>
<head>
<title>系统安全日志</title>
<meta http-equiv=pragma content=no-cache>
<meta http-equiv=cache-control content="no-cache, must-revalidate">
<meta http-equiv=content-type content="text/html; charset=gbk">
<meta http-equiv=content-script-type content=text/javascript>
<!--系统公共css-->
<style type=text/css>
@import url(/style/default.css);
</style>
<!--系统公共脚本-->
<script language="javascript" src="common.js"></script>
<script language="javascript" type="text/javascript">

var rcs = new Array();
<% sysLogList(); %>
/*
cgi.mf = "ASB";
cgi.pc = "V1.0";
cgi.sn = "03100200000100100000007404010203";
cgi.ip = "192.168.2.1";
cgi.hv = "V1.0";
cgi.sv = "RG100A_V1.0";
*/
/*
rcs.push(new Array("1900-01-07 01:26:37", "Informational", "kernel: Freeing unused kernel memory: 144k freed"));
rcs.push(new Array("1900-01-07 01:26:37", "Warning", "kernel: EHCI: port status=0x00001000"));
*/

/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	if (lstrc.rows) {
		while (lstrc.rows.length > 1)
			lstrc.deleteRow(1);
	}
	for (var i = 0; i < rcs.length; i++) {
		var row = lstrc.insertRow(i + 1);

		row.nowrap = true;
		row.vAlign = "top";

		var cell = row.insertCell(0);
		cell.innerHTML = rcs[i][0];
		cell = row.insertCell(1);
		cell.innerHTML = rcs[i][1];
		cell = row.insertCell(2);
		cell.innerHTML = rcs[i][2];
	}
}

</script>
</head>

<!-------------------------------------------------------------------------------------->
<!--主页代码-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
	<div align="left" style="padding-left:20px; padding-top:10px">
		<form id="form">
			<b>系统安全日志</b><br>
			<hr align="left" class="sep" size="1" width="90%">
			<table border="0" cellpadding="0" cellspacing="0" style="padding-left:4px; padding-right:4px ">
				<tr><td><b>Manufacturer:</b></td><td id="mf"><% getInfo("manufacture"); %></td></tr>
				<tr><td><b>ProductClass:</b></td><td id="strModel"><% getInfo("devModel"); %></td></tr>
				<tr><td><b>SerialNumber:</b></td><td id="strId"><% getInfo("devId"); %></td></tr>
				<tr><td><b>IP:</b></td><td id="ip"><% getInfo("lan-ip"); %></td></tr>
				<tr><td><b>HWVer:</b></td><td id="strHdVer"><% getInfo("hdVer"); %></td></tr>
				<tr><td><b>SWVer:</b></td><td id="strStVer"><% getInfo("stVer"); %></td></tr>
			</table>
			<hr align="left" class="sep" size="1" width="90%">
			<input type="button" class="button" onClick="location.href=location.href;" value="刷新">
			<input type="button" class="button" onClick="window.close();" value="关闭">
			<table class="flat" id="lstrc" border="1" cellpadding="0" cellspacing="1" width="90%">
			   <tr class="hdb" align="center">
				  <td>日期/时间</td>
				  <td>严重程度</td>
				  <td>信息</td>
			   </tr>
			</table>
		</form>
	</div>
</body>
<%addHttpNoCache();%>
</html>
