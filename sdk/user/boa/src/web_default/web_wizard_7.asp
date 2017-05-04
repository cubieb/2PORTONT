<VWS_FUNCTION (void*)SendWebMetaStr();>
<VWS_FUNCTION (void*)SendWebCssStr();>
<title>Html Wizard</title>

<SCRIPT>
#ifdef CONFIG_VENDOR_BAUDTEC
var wtime=50;
#elif defined(CONFIG_VENDOR_TPLINK) || defined(CONFIG_VENDOR_PROWARE)
var wtime=90;
#else
var wtime=20;
#endif
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
		document.RoseWizard7.time.value = wtime;
		id=setTimeout("start()",1000);
	}
	if (wtime == 0)
	{ 
		document.RoseWizard7.submit();
	}
}
</SCRIPT>
</head>

<body onLoad="start();" onUnload="stop();">
<form action="form2RoseWizard7.cgi" method=POST name="RoseWizard7">
	<div class="data_common data_common_notitle">
		<table>
			<tr class="data_prompt_info">
				<th colspan="2"><font size="6px" color="green">Applying new settings. Device will be available in</font></th>
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
		<input type="hidden" value="Send" name="submit.htm?rose_wizard_7.htm">
	</div>
</form>

</body>

</html>
