<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title>Html Wizard</title>
<link href="reset.css" rel="stylesheet" type="text/css" />
<link href="base.css" rel="stylesheet" type="text/css" />
<link href="style.css" rel="stylesheet" type="text/css" />
<script type="text/javascript" src="share.js"></script>

<SCRIPT>

var userpassFlag = 0;

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
	if(userpassFlag == 1)
		return true;
	if (document.WebWizard1.newpass.value.length == 0) 
	{
		alert('Password cannot be empty. Please try it again.');
		document.WebWizard1.newpass.focus();
		return false;
	}
	if (document.WebWizard1.confirmpass.value.length == 0) 
	{
		alert('Confirm Password cannot be empty. Please try it again.');
		document.WebWizard1.confirmpass.focus();
		return false;
	}
	if (document.WebWizard1.newpass.value != document.WebWizard1.confirmpass.value)
	{
		alert('Password is not matched. Please type the same password between \'New password\' and \'Confirm\' box.');   
		document.WebWizard1.newpass.focus();
		return false;
	}	
	if (includeSpecialKey(document.WebWizard1.newpass.value)) 
	{
		alert('Invalid password,Please try it again.');                
		document.WebWizard1.newpass.focus();               
		return false;            
	}
	if (includeCyrillicKey(document.WebWizard1.newpass.value))
	{
		alert('Invalid password,Please try it again.');                
		document.WebWizard1.newpass.focus();               
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
				<td colspan="2">
				This router is ideal for home and small office networks. Built-in Quick Setup Wizard will help you prepare your router to connect to the Rostelecom. Please follow the instructions in the Quick Setup wizard.
				</td>
			</tr>
			<form action=/boaform/form2WebWizard1 method=POST name="WebWizard1">
			<tr id="userpassInfo" class="data_prompt_info" style="display:none;">
				<td colspan="2">For your safety please change default CPE password. Enter new password confirm it in the form below and press "Ok".
				</td>
			</tr>
			<tr id="usernewpass" style="display:none;">
				<th width="25%">New password</th>
				<th>
				<input name="newpass" type="text"size="20" maxlength="30">
				</th>
			</tr>
			<tr id="userconfirmpass" style="display:none;">
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
		<input type="hidden" value="/web_wizard_4.asp" name="submit-url">
	</div>
</form>
<script>
	if(userpassFlag == 0)
	{
		document.getElementById("userpassInfo").style.display = "";
		document.getElementById("usernewpass").style.display = "";
		document.getElementById("userconfirmpass").style.display = "";
	}
</script>
</body>

</html>
