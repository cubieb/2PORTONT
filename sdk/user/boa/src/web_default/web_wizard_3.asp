<VWS_FUNCTION (void*)SendWebMetaStr();>
<VWS_FUNCTION (void*)SendWebCssStr();>
<title>Html Wizard</title>
#ifdef CONFIG_ROSETEL_DEFAULT_REGION
<script language="javascript" src="region_list.js"></script>
<SCRIPT>
#if defined(CONFIG_ADSLUP) && defined(CONFIG_MULTI_ETHUP)
	<VWS_FUNCTION (void*) vmsGetPhytype(); >
#elif defined(CONFIG_ADSLUP)
	var wanphytype = 0;
#else
	var wanphytype = 1;
#endif

function onload()
{
	if(wanphytype == 0)
	{
#if defined(CONFIG_ADSLUP)
		createRegion(document.getElementById("branch"));
#endif
	}
#if defined(CONFIG_MULTI_ETHUP)
	else
	{
		createEthRegion(document.getElementById("branch"));
	}
#endif
	onChangeBranch();
}

function onChangeBranch()
{
	if(wanphytype == 0)
	{
#if defined(CONFIG_ADSLUP)
		changeBranch(document.getElementById("branch"), document.getElementById("city"));
#endif
	}
#if defined(CONFIG_MULTI_ETHUP)
	else
	{
		changeEthBranch(document.getElementById("branch"), document.getElementById("city"));
	}
#endif
	onChangeCity();
}

function onChangeCity()
{
	if(wanphytype == 0)
	{
#if defined(CONFIG_ADSLUP)
		changeProfile(document.getElementById("city"), document.getElementById("serviceType"));
#endif
	}
#if defined(CONFIG_MULTI_ETHUP)
	else
	{
		changeEthProfile(document.getElementById("city"), document.getElementById("serviceType"));
	}
#endif
	onChangeService();
}

function onChangeService()
{
	if(wanphytype == 0)
	{
#if defined(CONFIG_ADSLUP)
		getServiceDetails(document.getElementById("serviceType"), document.RoseWizard3);
#endif
	}
#if defined(CONFIG_MULTI_ETHUP)
	else
	{
		getEthServiceDetails(document.getElementById("serviceType"), document.RoseWizard3);
	}
#endif
}

</SCRIPT>
</head>

<body onload = "onload();">
<form action="form2RoseWizard3.cgi" method=POST name="RoseWizard3">
	<div class="data_common data_common_notitle">
		<table>
			<tr>
				<th width="25%">Choose branch</th>
				<th>
				<select name="branch" id="branch" size=1 onchange = "onChangeBranch();">
				</select>
				</th>
				<td rowspan="3" class="data_prompt_td_info">
					Please select the macro regional branch and the branch where you are.
				</td>
			</tr>
			<tr>
				<th width="25%">Choose city</th>
				<th>
				<select name="city" id="city" size=1 onchange="onChangeCity();">
				</select>
				</th>
			</tr>
			<tr>
				<th width="25%">Choose service</th>
				<th>
				<select name="serviceType" id="serviceType" size=1 onchange="onChangeService();">
				<input type="hidden" name="service">
				</select>
				</th>
			</tr>
		</table>
	</div>
	
	<input type="hidden" name="PVC1Enable">
#if defined(CONFIG_ADSLUP) 
	<input type="hidden" name="PVC1VPI">
	<input type="hidden" name="PVC1VCI">
	<input type="hidden" name="PVC1QoS">
	<input type="hidden" name="PVC1PCR">
	<input type="hidden" name="PVC1SCR">
	<input type="hidden" name="PVC1MBS">
#endif
	<input type="hidden" name="PVC1Proto">
	<input type="hidden" name="PVC1ConnType">
#if defined(CONFIG_ADSLUP) 
	<input type="hidden" name="PVC1IP">
	<input type="hidden" name="PVC1Mask">
	<input type="hidden" name="PVC1Gateway">
#endif
#if defined(CONFIG_MULTI_ETHUP)
	<input type="hidden" name="PVC1VlanId">
	<input type="hidden" name="PVC1VlanPriority">
#endif
	<input type="hidden" name="PVC1NAT">
	<input type="hidden" name="PVC1Droute">
	<input type="hidden" name="PVC1IGMP">

	<input type="hidden" name="PVC2Enable">
#if defined(CONFIG_ADSLUP) 
	<input type="hidden" name="PVC2VPI">
	<input type="hidden" name="PVC2VCI">
	<input type="hidden" name="PVC2QoS">
	<input type="hidden" name="PVC2PCR">
	<input type="hidden" name="PVC2SCR">
	<input type="hidden" name="PVC2MBS">
#endif
	<input type="hidden" name="PVC2Proto">
	<input type="hidden" name="PVC2ConnType">
#if defined(CONFIG_ADSLUP) 
	<input type="hidden" name="PVC2IP">
	<input type="hidden" name="PVC2Mask">
	<input type="hidden" name="PVC2Gateway">
#endif
#if defined(CONFIG_MULTI_ETHUP)
	<input type="hidden" name="PVC2VlanId">
	<input type="hidden" name="PVC2VlanPriority">
#endif
	<input type="hidden" name="PVC2NAT">
	<input type="hidden" name="PVC2Droute">
	<input type="hidden" name="PVC2IGMP">

	<input type="hidden" name="PVC3Enable">
#if defined(CONFIG_ADSLUP) 
	<input type="hidden" name="PVC3VPI">
	<input type="hidden" name="PVC3VCI">
	<input type="hidden" name="PVC3QoS">
	<input type="hidden" name="PVC3PCR">
	<input type="hidden" name="PVC3SCR">
	<input type="hidden" name="PVC3MBS">
#endif
	<input type="hidden" name="PVC3Proto">
	<input type="hidden" name="PVC3ConnType">
#if defined(CONFIG_ADSLUP) 
	<input type="hidden" name="PVC3IP">
	<input type="hidden" name="PVC3Mask">
	<input type="hidden" name="PVC3Gateway">
#endif
#if defined(CONFIG_MULTI_ETHUP)
	<input type="hidden" name="PVC3VlanId">
	<input type="hidden" name="PVC3VlanPriority">
#endif
	<input type="hidden" name="PVC3NAT">
	<input type="hidden" name="PVC3Droute">
	<input type="hidden" name="PVC3IGMP">

	<input type="hidden" name="PVC4Enable">
#if defined(CONFIG_ADSLUP) 
	<input type="hidden" name="PVC4VPI">
	<input type="hidden" name="PVC4VCI">
	<input type="hidden" name="PVC4QoS">
	<input type="hidden" name="PVC4PCR">
	<input type="hidden" name="PVC4SCR">
	<input type="hidden" name="PVC4MBS">
#endif
	<input type="hidden" name="PVC4Proto">
	<input type="hidden" name="PVC4ConnType">
#if defined(CONFIG_ADSLUP) 
	<input type="hidden" name="PVC4IP">
	<input type="hidden" name="PVC4Mask">
	<input type="hidden" name="PVC4Gateway">
#endif
#if defined(CONFIG_MULTI_ETHUP)
	<input type="hidden" name="PVC4VlanId">
	<input type="hidden" name="PVC4VlanPriority">
#endif
	<input type="hidden" name="PVC4NAT">
	<input type="hidden" name="PVC4Droute">
	<input type="hidden" name="PVC4IGMP">
	
	<input type="hidden" name="PVC5Enable">
#if defined(CONFIG_ADSLUP) 
	<input type="hidden" name="PVC5VPI">
	<input type="hidden" name="PVC5VCI">
	<input type="hidden" name="PVC5QoS">
	<input type="hidden" name="PVC5PCR">
	<input type="hidden" name="PVC5SCR">
	<input type="hidden" name="PVC5MBS">
#endif
	<input type="hidden" name="PVC5Proto">
	<input type="hidden" name="PVC5ConnType">
#if defined(CONFIG_ADSLUP) 
	<input type="hidden" name="PVC5IP">
	<input type="hidden" name="PVC5Mask">
	<input type="hidden" name="PVC5Gateway">
#endif
#if defined(CONFIG_MULTI_ETHUP)
	<input type="hidden" name="PVC5VlanId">
	<input type="hidden" name="PVC5VlanPriority">
#endif
	<input type="hidden" name="PVC5NAT">
	<input type="hidden" name="PVC5Droute">
	<input type="hidden" name="PVC5IGMP">

	<input type="hidden" name="interPortmap">
	<input type="hidden" name="STBPortmap">
	<input type="hidden" name="VoIPPortmap">
	<input type="hidden" name="dhcpStatus">
	<input type="hidden" name="aclStatus">
	<input type="hidden" name="wanport">
	<input type="hidden" name="tr069">
	<input type="hidden" name="ondemandSec">
	
	<br>
	<div class="adsl clearfix btn_center">
		<input class="link_bg" type="submit" name="continue" value="Continue">
		<input type="hidden" value="Send" name="submit.htm?rose_wizard_3.htm">
	</div>
</form>

</body>
#else
<SCRIPT>

</SCRIPT>
</head>

<body>
<form action="form2RoseWizard3.cgi" method=POST name="RoseWizard3">
	<div class="data_common data_common_notitle">
		<table>
			<tr>
				<th width="25%">Choose branch</th>
				<th>
				<select name="branch" size=1>
				    <option value="0">North-west</option>
				    <option value="1">Volga</option>
				    <option value="2">Ural</option>
				</select>
				</th>
				<td rowspan="3" class="data_prompt_td_info">
					Please select the macro regional branch and the branch where you are.
				</td>
			</tr>
			<tr>
				<th width="25%">Choose city</th>
				<th>
				<select name="city" size=1>
				    <option value="0">Nizhny Novgorod</option>
				    <option value="1">Yoshkar Ola</option>
				    <option value="2">Cheboksary</option>
				</select>
				</th>
			</tr>
			<tr>
				<th width="25%">Choose service</th>
				<th>
				<select name="service" size=1>
				    <option value="0">Internet</option>
				    <option value="1">Internet + TB</option>
				    <option value="2">Internet + VOIP</option>
				    <option value="3">Internet + VOIP+TB</option>
				</select>
				</th>
			</tr>
		</table>
	</div>
	<br>
	<div class="adsl clearfix btn_center">
		<input class="link_bg" type="submit" name="continue" value="Continue">
		<input type="hidden" value="Send" name="submit.htm?rose_wizard_3.htm">
	</div>
</form>

</body>

#endif
</html>

