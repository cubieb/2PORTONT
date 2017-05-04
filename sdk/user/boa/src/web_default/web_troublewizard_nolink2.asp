<VWS_FUNCTION (void*)SendWebMetaStr();>
<VWS_FUNCTION (void*)SendWebCssStr();>
<title>Html Wizard</title>

</head>

<body>
	<div class="data_common data_common_notitle">
		<table>
			<tr class="data_prompt_info">
				<th colspan="4" style="color:red; font-size:20px;">
					Please check the correctness of support of JSC "Rostelecom"
				</th>
			</tr>
			<tr>
				<td colspan="4" class="data_prompt_td_info">
					No connection to the Internet. Try again later. If the problem persists, contact technical support of JSC "Rostelecom".
				</td>
			</tr>
			<tr>
				<th width="25%" style="padding-top:50px;">Device model</th>
				<td width="25%" style="padding-top:50px;">
				<VWS_SCREEN (char*)xscrnRoseModelName[];>
				</td>
				<th width="25%" style="padding-top:50px;">PPPoE user name</th>
				<td width="25%" style="padding-top:50px;">
				<VWS_SCREEN (char*)xscrnRoseTroublePPPUsername[];>
				</td>
			</tr>
			<tr>
				<th width="25%">Firmware version</th>
				<td>
				<VWS_SCREEN (char*)xscrnHwVersion[];>		
				</td>
				<th>PPPoE password</th>
				<td>
				<VWS_SCREEN (char*)xscrnRoseTroublePPPPassword[];>
				</td>
			</tr>
			<tr>
				<th width="25%">Software version</th>
				<td>
				<VWS_SCREEN (char*)xscrnAppVersion[];>			
				</td>
				<th>Service information</th>
				<td>
				<VWS_SCREEN (char*)xscrnRoseTroubleServiceInfo[];>
				</td>
			</tr>
			<tr>
				<th width="25%">Mac address</th>
				<td>
				<VWS_SCREEN (char*)xscrnRoseWANMAC[];>
				</td>
				<th>Serial number</th>
				<td>
				<VWS_SCREEN (char*)xscrnRoseSerial[];>
				</td>
			</tr>
		</table>
	</div>
	<br>
	<div class="adsl clearfix btn_center">
		<input class="link_bg" type="button" name="close" value="Configure device manually" onClick="top.location.href='index.htm';" >
	</div>

</body>

</html>
