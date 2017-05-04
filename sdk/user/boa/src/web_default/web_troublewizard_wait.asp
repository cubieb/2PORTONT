<VWS_FUNCTION (void*)SendWebMetaStr();>
<VWS_FUNCTION (void*)SendWebCssStr();>
<title>Html Wizard</title>

<SCRIPT>
var wtime=30;
var id;
function stop()
{
	clearTimeout(id); 
}

function start() 
{ 
	wtime--;
	if (wtime >= 0)
	{ 
		document.RoseTroubleWizardWait.time.value = wtime;
		id=setTimeout("start()",1000);
	}
	if (wtime == 0)
	{ 
		document.RoseTroubleWizardWait.submit();
	}
}
</SCRIPT>
</head>

<body onLoad="start();" onUnload="stop();">
<form action="form2RoseTroubleWizardWait.cgi" method=POST name="RoseTroubleWizardWait">
	<div class="data_common data_common_notitle">
		<table>
			<tr class="data_prompt_info">
				<th colspan="2"><font size="6px" color="green">Connection to ADSL. The device will be available through</font></th>
			</tr>
			<tr class="data_prompt_info">
				<th>
				    <input type="text" name="time" size=1 style="height:40px; font-size:25px;"><font size="6px" color="green">&nbsp;&nbsp;seconds</font>
				</th>
			</tr>
			<tr class="data_prompt_info">
				<th><font size="4px" color="red">Warning! Do not power off the device.</font></th>
			</tr>
		</table>
		<input type="hidden" value="Send" name="submit.htm?rose_troublewizard_wait.htm">
	</div>
</form>

</body>

</html>
