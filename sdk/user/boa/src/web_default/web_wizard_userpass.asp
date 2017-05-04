<VWS_FUNCTION (void*)SendWebMetaStr();>
<VWS_FUNCTION (void*)SendWebCssStr();>
<title>Html Wizard</title>
<SCRIPT>

#if defined(CONFIG_ADSLUP) && defined(CONFIG_MULTI_ETHUP)
	<VWS_FUNCTION (void*) vmsGetPhytype();>
#elif defined(CONFIG_ADSLUP)
	var wanphytype = 0;
#else
	var wanphytype = 1;
#endif
#ifdef CONFIG_MULTI_ETHUP
<VWS_FUNCTION (void*)getPPPVLANType();>
#endif

function checkPPPSetting()
{
	if(document.RoseWizardUserPass.pppusername.value == "")
	{
		alert("User name cannot be empty!");
		document.RoseWizardUserPass.pppusername.focus();
		return false;
	}
#ifdef CONFIG_MULTI_ETHUP
	if(wanphytype == 1)
	{
		if(document.RoseWizardUserPass.ppp_vlan.value != "")
		{
			if(!validateDecimalDigit(document.RoseWizardUserPass.ppp_vlan.value) || !checkDigitRange(document.RoseWizardUserPass.ppp_vlan.value,1,1,4095))
			{
				alert("Invalid VlanID value. It should be 1-4095.");
				document.RoseWizardUserPass.ppp_vlan.focus();
				return false;
			}
		}
	}
#endif
	return true;
}

function onload()
{
#ifdef CONFIG_MULTI_ETHUP
	if(wanphytype == 1 && show_ppp_vlanid == 1)
		document.getElementById("vlanShow").style.display = "";
#endif
}
</SCRIPT>
</head>

<body onload="onload();">
<form action="form2RoseWizardUserPass.cgi" method=POST name="RoseWizardUserPass">
        <div id="pppoesetting">
	<div class="data_common data_common_notitle">
		<table>
			<tr>
				<th width="25%">Enter username</th>
				<th>
				<input type="text" name="pppusername">
				</th>
#ifdef CONFIG_MULTI_ETHUP
				<script>
					if(wanphytype == 1)
					{
						document.write("<td rowspan=\"3\" class=\"data_prompt_td_info\">");
					}
					else
					{
						document.write("<td rowspan=\"2\" class=\"data_prompt_td_info\">");
					}
				</script>
#else
				<td rowspan="2" class="data_prompt_td_info">
#endif
#ifdef CONFIG_VENDOR_BAUDTEC
					Error 691! 
					<br>
					Please enter login and password from your ISP.
#elif defined(CONFIG_RTC_LAB_TEST_SPECIAL)
					PPPoE login and/or password is wrong. Please check the language. Please check CapsLock key. Check PPPoE login and password and try again.
#else
					Please input username and password received from your ISP.
#endif
				</td>
			</tr>
			<tr>
				<th width="25%">Enter password</th>
				<th>
				<input type="text" name="ppppassword">
				</th>
			</tr>
#ifdef CONFIG_MULTI_ETHUP
			<tr id="vlanShow" style="display:none;">
				<th width="25%">VLAN ID</th>
				<th>
				<input type="text" name="vlanid">
				</th>
			</tr>
#endif			
		</table>
	</div>
	<br>
	<div class="adsl clearfix btn_center">
		<input class="link_bg" type="submit" name="continue" value="Next" onClick="return checkPPPSetting();">
	</div>
	</div>

	<input type="hidden" value="Send" name="submit.htm?rose_wizard_userpass.htm">
</form>
<SCRIPT>
</SCRIPT>

</body>

</html>

