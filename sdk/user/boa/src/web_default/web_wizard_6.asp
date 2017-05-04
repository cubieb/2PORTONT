<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title>Html Wizard</title>
<link href="reset.css" rel="stylesheet" type="text/css" />
<link href="base.css" rel="stylesheet" type="text/css" />
<link href="style.css" rel="stylesheet" type="text/css" />
<script type="text/javascript" src="share.js"></script>

<SCRIPT>
</SCRIPT>
</head>

<body>
<form action=/boaform/form2WebWizard6 method=POST name="WebWizard6">
	<div class="data_common data_common_notitle">
		<table>
			<tr  class="data_prompt_info">
				<th colspan="2"><font size="6px" color="green">Configuration completed</font></th>
			</tr>
			<tr>
				<th colspan="2">Write down the connection information</th>
			</tr>
		</table>
		<table>
			<tr>
				<th width="25%">5GHz WLAN SSID name</th>
				<td><% checkWrite("5G_ssid"); %></td>
				<td id="WiFiAttention" width="50%" rowspan="6" style="display:none; color:red; font-weight: bold; font-size:18px; text-align: center;">
				    Attention, the Wi-Fi will restart. After pressing Save you should to connect to the new Wi-Fi network
				</td>
			</tr>
			<tr>
				<th width="25%">2.4GHz WLAN SSID name</th>
				<td><% checkWrite("2G_ssid"); %></td>
			</tr>
			<tr>
				<th width="25%">WiFi Password</th>
				<td><% getInfo("pskValue_Wizard"); %></td>
			</tr>
			<tr>
				<th width="25%">Device IP address</th>
				<td><% getInfo("lan-ip"); %></td>
			</tr>
			<tr>
				<th width="25%">Login</th>
				<td><% getInfo("super-user"); %></td>
			</tr>
			<tr>
				<th width="25%">GUI Password</th>
				<td><% getInfo("super-pass"); %></td>
			</tr>
		</table>
	</div>
	<br>
	<div class="adsl clearfix btn_center">
		<input class="link_bg" type="button" name="back" value="Back" onClick="window.location.href='web_wizard_5.asp';">
		<input class="link_bg" type="submit" name="save" value="Next" onClick="top.location='index.html';">
		<input type="hidden" value="/index.html" name="submit-url">
	</div>
</form>
<SCRIPT>
	document.getElementById("WiFiAttention").style.display = "";
</SCRIPT>

</body>

</html>
