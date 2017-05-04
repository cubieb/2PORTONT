<!-- add by liuxiao 2008-01-16 -->
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
<head>
<title>中国电信</title>
<meta http-equiv=pragma content=no-cache>
<meta http-equiv=cache-control content="no-cache, must-revalidate">
<meta http-equiv=content-type content="text/html; charset=gbk">
<style type=text/css>
@import url(/style/default.css);
</style>
<script language="javascript" src="common.js"></script>
</head>
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000">
	<blockquote>
		<div align="left" style="padding-left:20px;"><br>
			<div align="left"><b>连接信息</b></div>
			<table class="flat" border="1" cellpadding="1" cellspacing="1">
				<tr>
					<td class="hdb">连接状态</td>
					<td><% showgpon_status(); %></td>
				</tr>
				<tr <% checkWrite("priv"); %>>
					<td class="hdb">FEC 能力</td>
					<td>支持</td>
				</tr>
				<tr <% checkWrite("priv"); %>>
					<td class="hdb">FEC 上行状态</td>
					<td><% ponGetStatus("gpon-fec-us-state"); %></td>
				</tr>
				<tr <% checkWrite("priv"); %>>
					<td class="hdb">FEC 下行状态</td>
					<td><% ponGetStatus("gpon-fec-ds-state"); %></td>
				</tr>
				<tr <% checkWrite("priv"); %>>
					<td class="hdb">加密模式</td>
					<td><% ponGetStatus("gpon-encryption"); %></td>
				</tr>
			</table>
			<br>

			<span <% checkWrite("priv"); %>>
			<div align="left"><b>链路性能统计信息</b></div>
			<table class="flat" border="1" cellpadding="1" cellspacing="1">
				<tr>
					<td class="hdb">发送字节</td>
					<td><% ponGetStatus("bytes-sent"); %></td>
				</tr>
				<tr>
					<td class="hdb">接收字节</td>
					<td><% ponGetStatus("bytes-received"); %></td>
				</tr>
				<tr>
					<td class="hdb">发送帧</td>
					<td><% ponGetStatus("packets-sent"); %></td>
				</tr>
				<tr>
					<td class="hdb">接收帧</td>
					<td><% ponGetStatus("packets-received"); %></td>
				</tr>
				<tr>
					<td class="hdb">发送单播帧</td>
					<td><% ponGetStatus("unicast-packets-sent"); %></td>
				</tr>
				<tr>
					<td class="hdb">接收单播帧</td>
					<td><% ponGetStatus("unicast-packets-received"); %></td>
				</tr>
				<tr>
					<td class="hdb">发送组播帧</td>
					<td><% ponGetStatus("multicast-packets-sent"); %></td>
				</tr>
				<tr>
					<td class="hdb">接收组播帧</td>
					<td><% ponGetStatus("multicast-packets-received"); %></td>
				</tr>
				<tr>
					<td class="hdb">发送广播帧</td>
					<td><% ponGetStatus("broadcast-packets-sent"); %></td>
				</tr>
				<tr>
					<td class="hdb">接收广播帧</td>
					<td><% ponGetStatus("broadcast-packets-received"); %></td>
				</tr>
				<tr>
					<td class="hdb">接收 FEC 错误帧</td>
					<td><% ponGetStatus("fec-errors"); %></td>
				</tr>
				<tr>
					<td class="hdb">接收 HEC 错误帧</td>
					<td><% ponGetStatus("hec-errors"); %></td>
				</tr>
				<tr>
					<td class="hdb">发送丢失帧</td>
					<td><% ponGetStatus("packets-dropped"); %></td>
				</tr>
				<tr>
					<td class="hdb">发送 PAUSE 流控制帧</td>
					<td><% ponGetStatus("pause-packets-sent"); %></td>
				</tr>
				<tr>
					<td class="hdb">接收 PAUSE 流控制帧</td>
					<td><% ponGetStatus("pause-packets-received"); %></td>
				</tr>
			</table>
			<br>

			<div align="left"><b>告警信息</b></div>
			<table class="flat" border="1" cellpadding="1" cellspacing="1">
				<tr>
					<td class="hdb">GPON 告警信息</td>
					<td><% ponGetStatus("gpon-alarm"); %></td>
				</tr>
			</table>
			<br>

			<div align="left"><b>光模块信息</b></div>
			<table class="flat" border="1" cellpadding="1" cellspacing="1">
				<tr>
					<td class="hdb">发射光功率</td>
					<td><% ponGetStatus("tx-power"); %></td>
				</tr>
				<tr>
					<td class="hdb">接收光功率</td>
					<td><% ponGetStatus("rx-power"); %></td>
				</tr>
				<tr>
					<td class="hdb">工作温度</td>
					<td><% ponGetStatus("temperature"); %></td>
				</tr>
				<tr>
					<td class="hdb">供电电压</td>
					<td><% ponGetStatus("voltage"); %></td>
				</tr>
				<tr>
					<td class="hdb">偏置电流</td>
					<td><% ponGetStatus("bias-current"); %></td>
				</tr>
			</table>
			</span>
		</div>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>
