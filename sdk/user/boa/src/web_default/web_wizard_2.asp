<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title>Html Wizard</title>

<SCRIPT>
<VWS_FUNCTION  (void*)htmlWizardSetRedirect();>
<VWS_FUNCTION (void*)getCurrentLinkState();>
	var wanphytype = 1;

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
<form action=/boaform/form2RoseWizard2 method=POST name="RoseWizard2">
	<div class="data_common data_common_notitle">
		<table>
			<tr class="data_prompt_info">
				<td colspan="2">
<script>
					if(wanphytype == 1)
					{
						if(currentLinkState == 0 && (adslAttempts == 1 ||adslAttempts == 2 ||adslAttempts == 3))
							document.write("Not connected to the line Ethernet. Please check the cable connection Ethernet as shown below. Wait until the Ethernet indicator on your device will be lit continuously. Then click \"Continue\".");
						else
							document.write("You can configure the router by yourself or with the help of the built-in Quick Setup wizard. Connect the Ethernet cable as shown below.")
					}
</script>
					<input type="hidden" id="adslAttempts" name="adslAttempts">
				</td>
			</tr>
		</table>
		<table>
			<tr>
				<td colspan="2" style="text-align:center;">
					<VWS_FUNCTION (void*)getAdslConnJpg();>
					<!--<img src="ADSL_connection.jpg">-->	
				</td>
			</tr>
		</table>
	</div>
	<br>
	<div class="adsl clearfix btn_center">
<script>
		if(adslAttempts == 0)
			document.write("<input class=\"link_bg\" type=\"submit\" value=\"Wizard Start\">");
		else
			document.write("<input class=\"link_bg\" type=\"submit\" value=\"Continue\">");
</script>
		<input class="link_bg" type="button" value="Manual configuration" onClick="top.location.href='index.html';">
		<input type="hidden" value="/rose_wizard_2.asp" name="submit-url">
	</div>
</form>

</body>

</html>

