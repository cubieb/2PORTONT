<VWS_FUNCTION (void*)SendWebMetaStr();>
<VWS_FUNCTION (void*)SendWebCssStr();>
<title>Html Wizard</title>

<SCRIPT>
var wtime=6;
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
		id=setTimeout("start()",1000);
	}
	if (wtime == 0)
	{ 
		document.RoseHijackWizardWait2.submit();
	}
}
</SCRIPT>
</head>

<body onload="start();" onunload="stop();">
<form action="form2RoseHijackWizardWait2.cgi" method=POST name="RoseHijackWizardWait2">
	<div class="data_common data_common_notitle">
		<table>
			<tr class="data_wait_info">
				<td colspan="2"  style="padding: 25px;">Wait, is setting device ...
				</td>
			</tr>
		</table>
	</div>
	<input type="hidden" value="Send" name="submit.htm?rose_hijackwizard_wait2.htm">
</form>

</body>

</html>
