<VWS_FUNCTION (void*)SendWebMetaNoRefreshStr();>
<VWS_FUNCTION (void*)SendWebCssStr();>
<title>Html Wizard</title>

<SCRIPT>
function includeSpecialKey(str)
{
	for (var i = 0; i < str.length; i++)
	{
		if ((str.charAt(i) == ' ') || (str.charAt(i) == '%')
			|| ( str.charAt(i)== '\\' ) || ( str.charAt(i)== '\'' )
			|| (str.charAt(i) == '?') || (str.charAt(i) == '&') || (str.charAt(i) == '"')) 
		{
			return true;
		}
	}
	return false;
}

function saveCheck()
{
	if (document.RoseSmallWizard.newpass.value.length == 0) 
	{
		alert('Password cannot be empty. Please try it again.');
		document.RoseSmallWizard.newpass.focus();
		return false;
	}
	if (document.RoseSmallWizard.confirmpass.value.length == 0) 
	{
		alert('Confirm Password cannot be empty. Please try it again.');
		document.RoseSmallWizard.confirmpass.focus();
		return false;
	}
	if(!checkDigitLetterExclude(document.RoseSmallWizard.newpass.value))
	{
		alert('Admin paswword must contain at least one digit (0...9) and one letter (a...z A...Z) and be not less than 5 symbols.');
		document.RoseSmallWizard.newpass.focus();
		return false;
	}
	if (document.RoseSmallWizard.newpass.value != document.RoseSmallWizard.confirmpass.value)
	{
		alert('Password is not matched. Please type the same password between \'New password\' and \'Confirm\' box.');   
		document.RoseSmallWizard.newpass.focus();
		return false;
	}	
	if (includeSpecialKey(document.RoseSmallWizard.newpass.value)) 
	{
		alert('Invalid password,Please try it again.');                
		document.RoseSmallWizard.newpass.focus();               
		return false;            
	}            
	return true;
}

</SCRIPT>
</head>

<body>
	<div class="data_common data_common_notitle">
		<table>
			<tr class="data_prompt_info">
				<td colspan="2">This router is ideal for home and small office networks. Your device is already set up. Built-in quick setup wizard will help you complete your personal settings. Please follow the instructions in the Quick Setup wizard step by step. 
				<br>
				For your security, please change the password of the router, the default. Enter the new password and confirm in the box below and click "OK" to continue.
				</td>
			</tr>
	        <form action="form2RoseSmallWizard.cgi" method=POST name="RoseSmallWizard">
			<tr>
				<th width="25%">New password</th>
				<th>
				<input name="newpass" type="text"size="20" maxlength="30">
				</th>
			</tr>
			<tr>
				<th width="25%">Confirm password</th>
				<th>
				<input name="confirmpass" type="text" size="20" maxlength="30">
				</th>
			</tr>
		</table>
	</div>
	<br>
	<div class="adsl clearfix btn_center">
		<input class="link_bg" type="submit" name="ok" value="OK" onClick="return saveCheck()" >
		<input type="hidden" value="Send" name="submit.htm?rose_smallwizard.htm">
	</div>
</form>

</body>

</html>
