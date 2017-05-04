<VWS_FUNCTION (void*)SendWebMetaStr();>
<VWS_FUNCTION (void*)SendWebCssStr();>
<title>Html Wizard</title>

<SCRIPT>
#ifdef CONFIG_ROSETEL_TROUBLE_WIZARD
<VWS_FUNCTION  (void*)htmlWizardSetRedirect();>
#endif
<VWS_FUNCTION (void*)getCurrentLinkState();>
#if defined(CONFIG_ADSLUP) && defined(CONFIG_MULTI_ETHUP)
	<VWS_FUNCTION (void*) vmsGetPhytype(); >
#elif defined(CONFIG_ADSLUP)
	var wanphytype = 0;
#else
	var wanphytype = 1;
#endif

function onload()
{
	if(currentLinkState == 0)
	{
		adslAttempts++;
		document.getElementById("adslAttempts").value = adslAttempts;
	}
}

</SCRIPT>
</head>

<body onload="onload()">
<form action="form2RoseTroubleWizard2.cgi" method=POST name="RoseTroubleWizard2">
	<div class="data_common data_common_notitle">
		<table>
			<tr class="data_prompt_info">
				<td colspan="2">
<script>
#if defined(CONFIG_MULTI_ETHUP)
					if(wanphytype == 1)
					{
						document.write("Not connected to the line Ethernet. Please check the cable connection Ethernet as shown below. Wait until the Ethernet indicator on your device will be lit continuously. Then click \"Continue\".");
					}
#endif
#if defined(CONFIG_MULTI_ETHUP) && defined(CONFIG_ADSLUP)
					else
#endif
#if defined(CONFIG_ADSLUP)
					if(wanphytype == 0)
					{
						document.write("Not connected to the line ADSL. Please check the cable connection ADSL as shown below. Wait until the ADSL indicator on your device will be lit continuously. Then click \"Continue\".");
					}
#endif
</script>
					<input type="hidden" id="adslAttempts" name="adslAttempts">
				</td>
			</tr>
		</table>
		<table>
			<tr>
				<td colspan="2" style="text-align:center;">
#ifdef CONFIG_VENDOR_TPLINK				
					<VWS_FUNCTION (void*)getAdslNoConnJpg();>
#else
					<VWS_FUNCTION (void*)getAdslConnJpg();>
#endif
				</td>
			</tr>
		</table>
	</div>
	<br>
	<div class="adsl clearfix btn_center">
#ifdef CONFIG_VENDOR_BAUDTEC
		<input class="link_bg" type="button" value="Continue" onClick="window.location.href='rose_troublewizard_wait.htm';">
#else
		<input class="link_bg" type="submit" value="Continue">
#endif
		<input class="link_bg" type="button" value="Manual configuration" onClick="top.location.href='index.htm';">
		<input type="hidden" value="Send" name="submit.htm?rose_troublewizard_2.htm">
	</div>
</form>

</body>

</html>

