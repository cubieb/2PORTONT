<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<META HTTP-EQUIV=Refresh CONTENT="10; URL=adsl-stats.asp">
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% checkWrite("adsl_title"); %></title>
</head>

<body>
<blockquote>
<h2><font color="#0000FF"><% checkWrite("adsl_title"); %></font></h2>

<form action=/boaform/formStatAdsl method=POST name=sts_adsl>
<table border=0 width=500 cellspacing=4 cellpadding=0>
	<tr><td><hr size=1 noshade align=top></td></tr>
</table>

<table border=0 width=500 cellspacing=4 cellpadding=0>
	<tr>
		<th align=left bgcolor=#c0c0c0><font size=2>Mode</th>
		<td bgcolor=#f0f0f0><% getInfo("adsl-drv-mode"); %></td>
	</tr>
<% checkWrite("adsl_tpstc"); %>
	<tr>
		<th align=left bgcolor=#c0c0c0><font size=2>Latency</th>
		<td bgcolor=#f0f0f0><% getInfo("adsl-drv-latency"); %></td>
	</tr>	
<% checkWrite("adsl_trellis"); %>
	<tr>
		<th align=left bgcolor=#c0c0c0><font size=2>Status</th>
		<td bgcolor=#f0f0f0><% getInfo("adsl-drv-state"); %></td>
	</tr>
	<tr>
		<th align=left bgcolor=#c0c0c0><font size=2>Power Level</th>
		<td bgcolor=#f0f0f0><% getInfo("adsl-drv-pwlevel"); %></td>
	</tr>
	<tr>
		<th align=left bgcolor=#c0c0c0><font size=2>Uptime</th>
		<td bgcolor=#f0f0f0><% DSLuptime(); %></td>
	</tr>
	<tr <% checkWrite("vdsl-cap"); %>>
		<th align=left bgcolor=#c0c0c0><font size=2>G.Vector</th>
		<td bgcolor=#f0f0f0><% getInfo("adsl-drv-vector-mode"); %></td>
	</tr>
</table>
<br>
<table border=0 width=500 cellspacing=4 cellpadding=0>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0>
		<th>Downstream<th>Upstream</th>
	</tr>
<% checkWrite("adsl_trellis_dsus"); %>	
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>SNR Margin (dB)</th>
		<td><% getInfo("adsl-drv-snr-ds"); %></td><td><% getInfo("adsl-drv-snr-us"); %></td>
	</tr>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>Attenuation (dB)</th>
		<td><% getInfo("adsl-drv-lpatt-ds"); %></td><td><% getInfo("adsl-drv-lpatt-us"); %></td>
	</tr>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>Output Power (dBm)</th>
		<td><% getInfo("adsl-drv-power-ds"); %></td><td><% getInfo("adsl-drv-power-us"); %></td>
	</tr>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>Attainable Rate (Kbps)</th>
		<td><% getInfo("adsl-drv-attrate-ds"); %></td><td><% getInfo("adsl-drv-attrate-us"); %></td>
	</tr>
<% checkWrite("adsl_ginp_dsus"); %>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>Rate (Kbps)</th>
		<td><% getInfo("adsl-drv-rate-ds"); %></td><td><% getInfo("adsl-drv-rate-us"); %></td>
	</tr>
<% checkWrite("adsl_k_dsus"); %>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>R (number of check bytes in RS code word)</th>
		<td><% getInfo("adsl-drv-pms-r-ds"); %></td><td><% getInfo("adsl-drv-pms-r-us"); %></td>
	</tr>
<% checkWrite("adsl_n_dsus"); %>
<% checkWrite("adsl_l_dsus"); %>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>S (RS code word size in DMT frame)</th>
		<td><% getInfo("adsl-drv-pms-s-ds"); %></td><td><% getInfo("adsl-drv-pms-s-us"); %></td>
	</tr>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>D (interleaver depth)</th>
		<td><% getInfo("adsl-drv-pms-d-ds"); %></td><td><% getInfo("adsl-drv-pms-d-us"); %></td>
	</tr>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>Delay (msec)</th>
		<td><% getInfo("adsl-drv-pms-delay-ds"); %></td><td><% getInfo("adsl-drv-pms-delay-us"); %></td>
	</tr>
<% checkWrite("adsl_inp_dsus"); %>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2><% checkWrite("adsl_fec_name"); %></th>
		<td><% getInfo("adsl-drv-fec-ds"); %></td><td><% getInfo("adsl-drv-fec-us"); %></td>
	</tr>
<% checkWrite("adsl_ohframe_dsus"); %>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2><% checkWrite("adsl_crc_name"); %></th>
		<td><% getInfo("adsl-drv-crc-ds"); %></td><td><% getInfo("adsl-drv-crc-us"); %></td>
	</tr>

	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>Total ES</th>
		<td><% getInfo("adsl-drv-es-ds"); %></td><td><% getInfo("adsl-drv-es-us"); %></td>
	</tr>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>Total SES</th>
		<td><% getInfo("adsl-drv-ses-ds"); %></td><td><% getInfo("adsl-drv-ses-us"); %></td>
	</tr>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>Total UAS</th>
		<td><% getInfo("adsl-drv-uas-ds"); %></td><td><% getInfo("adsl-drv-uas-us"); %></td>
	</tr>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>Total LOSS</th>
		<td><% getInfo("adsl-drv-los-ds"); %></td><td><% getInfo("adsl-drv-los-us"); %></td>
	</tr>
<% checkWrite("adsl_llr_dsus"); %>	
	<tr bgcolor=#f0f0f0>
		<th colspan=3 align=left bgcolor=#c0c0c0><font size=2></th>		
	</tr>
	
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>Full Init</th>
		<td colspan=2><% getInfo("adsl-drv-lnk-fi"); %></td>
	</tr>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>Failed Full Init</th>
		<td colspan=2><% getInfo("adsl-drv-lnk-lfi"); %></td>
	</tr>
<% checkWrite("adsl_llr"); %>
<% checkWrite("adsl_txrx_frame"); %>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>Synchronized time(Second)</th>
		<td colspan=2><% getInfo("adsl-drv-synchronized-time"); %></td>
	</tr>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>Synchronized number</th>
		<td colspan=2><% getInfo("adsl-drv-synchronized-number"); %></td>
	</tr>
</table>
<p>
</form>
</blockquote>
</body>

</html>
