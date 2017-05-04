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
			<tr class="data_prompt_info">
				<th colspan="2" style="color:red; font-size:20px;">
<script>
#if defined(CONFIG_MULTI_ETHUP)
				if(wanphytype == 1)
					document.write("Ethernet line is not detected.");
#endif
#if defined(CONFIG_ADSLUP) && defined(CONFIG_MULTI_ETHUP)
				else
#endif
#if defined(CONFIG_ADSLUP)
				if(wanphytype == 0)
					document.write("ADSL line is not detected.");
#endif
</script>
				</th>
			</tr>
			<tr>
				<td colspan="2" class="data_prompt_td_info">
<script>
#if defined(CONFIG_MULTI_ETHUP)
				if(wanphytype == 1)
					document.write("Please check the cable connection Ethernet, as shown below. If the problem persists, contact technical support of JSC \"Rostelecom\".");
#endif
#if defined(CONFIG_ADSLUP) && defined(CONFIG_MULTI_ETHUP)
				else
#endif
#if defined(CONFIG_ADSLUP)
				if(wanphytype == 0)
					document.write("Please check the cable connection ADSL, as shown below. If the problem persists, contact technical support of JSC \"Rostelecom\".");
#endif
</script>
				</th>
			</tr>
		</table>
		<table>
			<tr>
				<th width="15%">Device type</th>
				<td>
				<VWS_SCREEN (char*)xscrnRoseModelName[];>
				</td>
				<td rowspan="5" style="text-align:center;">
#ifdef CONFIG_VENDOR_TPLINK				
				    <VWS_FUNCTION (void*)getAdslNoConnJpg();>
#else
				    <VWS_FUNCTION (void*)getAdslConnJpg();>
#endif  
				    <!--<img src="ADSL_connection.jpg">-->	
				</td>
			</tr>
			<tr>
				<th>Firmware version</th>
				<td>
				<VWS_SCREEN (char*)xscrnHwVersion[];>
				</td>
			</tr>
			<tr>
				<th>Software version</th>
				<td>
				<VWS_SCREEN (char*)xscrnAppVersion[];>
				</td>
			</tr>
			<tr>
				<th>Mac address</th>
				<td>
#ifdef CONFIG_RESERVE_DEFAULT_MAC
				<VWS_SCREEN (char*)xscrnRoseReserveMAC[];>
#else
				<VWS_SCREEN (char*)xscrnRoseMAC[];>
#endif
				</td>
			</tr>
			<tr>
				<th>Serial number</th>
				<td>
				<VWS_SCREEN (char*)xscrnRoseSerial[];>
				</td>
			</tr>
		</table>
	</div>
	<br>
	<form action="form2RoseWizardNoLink.cgi" method=POST name="RoseWizardNoLink">
	<div class="adsl clearfix btn_center">
		<input class="link_bg" type="submit" name="next" value="Continue">
#ifdef CONFIG_RTC_LAB_TEST_SPECIAL
                <VWS_FUNCTION (void *)getManualConfigurationBtn();>
#else
		<input class="link_bg" type="button" name="close" value="Manual configuration" onClick="top.location.href='login.htm';" >
#endif
		<input type="hidden" value="Send" name="submit.htm?rose_wizard_nolink.htm">
	</div>
	</form>

</body>

</html>
