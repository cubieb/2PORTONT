<VWS_FUNCTION (void*)SendWebMetaStr();>
<VWS_FUNCTION (void*)SendWebCssStr();>
<title>Html Wizard</title>

<SCRIPT>
#if defined(CONFIG_ADSLUP) && defined(CONFIG_MULTI_ETHUP)
	<VWS_FUNCTION (void*) vmsGetPhytype(); >
#elif defined(CONFIG_ADSLUP)
	var wanphytype = 0;
#else
	var wanphytype = 1;
#endif
</SCRIPT>
</head>

<body>
	<div class="data_common data_common_notitle">
		<table>
#ifdef CONFIG_VENDOR_SAGEMCOM
			<tr class="data_prompt_info">
				<th colspan="4" style="color:red; font-size:20px;">
					Internet is KO.
				</th>
			</tr>
			<tr>
				<td colspan="4" class="data_prompt_td_info">
					No connection to the Internet. Try again try to connect later. If the problem persists, contact technical support of JSC "Rostelecom".
			        </td>
			</tr>
#else
			<tr class="data_prompt_info">
				<th colspan="4" style="color:red; font-size:20px;">
					Unable to connect network
				</th>
			</tr>
			<tr>
				<td colspan="4" class="data_prompt_td_info">
<script>
#if defined(CONFIG_MULTI_ETHUP)
				if(wanphytype == 1)
					document.write("Please make sure that Ethernet cable connection connected properly as shown below. If the problem persists, contact technical support of JSC \"Rostelecom\".");
#endif
#if defined(CONFIG_ADSLUP) && defined(CONFIG_MULTI_ETHUP)
				else
#endif
#if defined(CONFIG_ADSLUP)
				if(wanphytype == 0)
					document.write("Please make sure that ADSL cable connection connected properly as shown below. If the problem persists, contact technical support of JSC \"Rostelecom\".");
#endif
</script>
				</td>
			</tr>
#endif
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
#ifdef CONFIG_RESERVE_DEFAULT_MAC
				<VWS_SCREEN (char*)xscrnRoseReserveMAC[];>
#else
				<VWS_SCREEN (char*)xscrnRoseWANMAC[];>
#endif
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
	        <input class="link_bg" type="button" name="next" value="Next" onClick="window.location.href='rose_wizard_2.htm';">
		<input class="link_bg" type="button" name="close" value="Configure device manually" onClick="top.location.href='index.htm';" >
	</div>

</body>

</html>
