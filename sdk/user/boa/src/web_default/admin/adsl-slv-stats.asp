<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<META HTTP-EQUIV=Refresh CONTENT="10; URL=adsl-slv-stats.asp">
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% checkWrite("adsl_slv_title"); %></title>
</head>

<body>
<blockquote>
<h2><font color="#0000FF"><% checkWrite("adsl_slv_title"); %></font></h2>

<form action=/boaform/formStatAdsl method=POST name=sts_adsl>
<table border=0 width=500 cellspacing=4 cellpadding=0>
	<tr><td><hr size=1 noshade align=top></td></tr>
</table>

<table border=0 width=500 cellspacing=4 cellpadding=0>
	<tr>
		<th align=left bgcolor=#c0c0c0><font size=2>Mode</th>
		<td bgcolor=#f0f0f0><% getInfo("adsl-slv-drv-mode"); %></td>
	</tr>
<% checkWrite("adsl_slv_tpstc"); %>
	<tr>
		<th align=left bgcolor=#c0c0c0><font size=2>Latency</th>
		<td bgcolor=#f0f0f0><% getInfo("adsl-slv-drv-latency"); %></td>
	</tr>	
<% checkWrite("adsl_slv_trellis"); %>
	<tr>
		<th align=left bgcolor=#c0c0c0><font size=2>Status</th>
		<td bgcolor=#f0f0f0><% getInfo("adsl-slv-drv-state"); %></td>
	</tr>
	<tr>
		<th align=left bgcolor=#c0c0c0><font size=2>Power Level</th>
		<td bgcolor=#f0f0f0><% getInfo("adsl-slv-drv-pwlevel"); %></td>
	</tr>
	<tr>
		<th align=left bgcolor=#c0c0c0><font size=2>Uptime</th>
		<td bgcolor=#f0f0f0><% DSLSlvuptime(); %></td>
	</tr>
</table>
<br>
<table border=0 width=500 cellspacing=4 cellpadding=0>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0>
		<th>Downstream<th>Upstream</th>
	</tr>
<% checkWrite("adsl_slv_trellis_dsus"); %>	
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>SNR Margin (dB)</th>
		<td><% getInfo("adsl-slv-drv-snr-ds"); %></td><td><% getInfo("adsl-slv-drv-snr-us"); %></td>
	</tr>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>Attenuation (dB)</th>
		<td><% getInfo("adsl-slv-drv-lpatt-ds"); %></td><td><% getInfo("adsl-slv-drv-lpatt-us"); %></td>
	</tr>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>Output Power (dBm)</th>
		<td><% getInfo("adsl-slv-drv-power-ds"); %></td><td><% getInfo("adsl-slv-drv-power-us"); %></td>
	</tr>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>Attainable Rate (Kbps)</th>
		<td><% getInfo("adsl-slv-drv-attrate-ds"); %></td><td><% getInfo("adsl-slv-drv-attrate-us"); %></td>
	</tr>
<% checkWrite("adsl_slv_ginp_dsus"); %>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>Rate (Kbps)</th>
		<td><% getInfo("adsl-slv-drv-rate-ds"); %></td><td><% getInfo("adsl-slv-drv-rate-us"); %></td>
	</tr>
<% checkWrite("adsl_slv_k_dsus"); %>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>R (number of check bytes in RS code word)</th>
		<td><% getInfo("adsl-slv-drv-pms-r-ds"); %></td><td><% getInfo("adsl-slv-drv-pms-r-us"); %></td>
	</tr>
<% checkWrite("adsl_slv_n_dsus"); %>
<% checkWrite("adsl_slv_l_dsus"); %>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>S (RS code word size in DMT frame)</th>
		<td><% getInfo("adsl-slv-drv-pms-s-ds"); %></td><td><% getInfo("adsl-slv-drv-pms-s-us"); %></td>
	</tr>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>D (interleaver depth)</th>
		<td><% getInfo("adsl-slv-drv-pms-d-ds"); %></td><td><% getInfo("adsl-slv-drv-pms-d-us"); %></td>
	</tr>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>Delay (msec)</th>
		<td><% getInfo("adsl-slv-drv-pms-delay-ds"); %></td><td><% getInfo("adsl-slv-drv-pms-delay-us"); %></td>
	</tr>
<% checkWrite("adsl_slv_inp_dsus"); %>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2><% checkWrite("adsl_fec_name"); %></th>
		<td><% getInfo("adsl-slv-drv-fec-ds"); %></td><td><% getInfo("adsl-slv-drv-fec-us"); %></td>
	</tr>
<% checkWrite("adsl_slv_ohframe_dsus"); %>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2><% checkWrite("adsl_crc_name"); %></th>
		<td><% getInfo("adsl-slv-drv-crc-ds"); %></td><td><% getInfo("adsl-slv-drv-crc-us"); %></td>
	</tr>

	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>Total ES</th>
		<td><% getInfo("adsl-slv-drv-es-ds"); %></td><td><% getInfo("adsl-slv-drv-es-us"); %></td>
	</tr>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>Total SES</th>
		<td><% getInfo("adsl-slv-drv-ses-ds"); %></td><td><% getInfo("adsl-slv-drv-ses-us"); %></td>
	</tr>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>Total UAS</th>
		<td><% getInfo("adsl-slv-drv-uas-ds"); %></td><td><% getInfo("adsl-slv-drv-uas-us"); %></td>
	</tr>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>Total LOSS</th>
		<td><% getInfo("adsl-slv-drv-los-ds"); %></td><td><% getInfo("adsl-slv-drv-los-us"); %></td>
	</tr>
<% checkWrite("adsl_slv_llr_dsus"); %>	
	<tr bgcolor=#f0f0f0>
		<th colspan=3 align=left bgcolor=#c0c0c0><font size=2></th>		
	</tr>
	
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>Full Init</th>
		<td colspan=2><% getInfo("adsl-slv-drv-lnk-fi"); %></td>
	</tr>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>Failed Full Init</th>
		<td colspan=2><% getInfo("adsl-slv-drv-lnk-lfi"); %></td>
	</tr>
<% checkWrite("adsl_slv_llr"); %>
<% checkWrite("adsl_slv_txrx_frame"); %>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>Synchronized time(Second)</th>
		<td colspan=2><% getInfo("adsl-slv-drv-synchronized-time"); %></td>
	</tr>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>Synchronized number</th>
		<td colspan=2><% getInfo("adsl-slv-drv-synchronized-number"); %></td>
	</tr>
</table>
<p>
</form>
</blockquote>
</body>

</html>
