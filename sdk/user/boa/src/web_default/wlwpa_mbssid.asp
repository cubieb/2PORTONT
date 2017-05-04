<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang(LANG_WLAN_SECURITY_SETTINGS); %></title>
<script type="text/javascript" src="share.js">
</script>
<style>
.on {display:on}
.off {display:none}	
.bggrey {
	BACKGROUND: #FFFFFF
}
</style>
<script>
var defPskLen, defPskFormat;

var ap_mode=0;

function get_by_id(id){
	with(document){
	return getElementById(id);
	}
}

function show_8021x_settings()
{
	var wlan_encmode = get_by_id("method");
	var enable_1x = get_by_id("use1x");
	var form1 = document.formEncrypt ;
	var dF=document.forms[0];
	if (enable_1x.checked) {		
		if (wlan_encmode.selectedIndex == 1)
			get_by_id("show_1x_wep").style.display = "";
		else 
			get_by_id("show_1x_wep").style.display = "none";
		get_by_id("setting_wep").style.display = "none";		
		get_by_id("show_8021x_eap").style.display = "";
		dF.auth_type[2].checked = true;
		dF.auth_type[0].disabled = true;
		dF.auth_type[1].disabled = true;
		dF.auth_type[2].disabled = true;
	}
	else {	
		if (wlan_encmode.selectedIndex == 1)
			get_by_id("setting_wep").style.display = "";	
		else	
			get_by_id("setting_wep").style.display = "none";	

		get_by_id("show_1x_wep").style.display = "none";			
		get_by_id("show_8021x_eap").style.display = "none";

		if(ap_mode!=1){//AP
			if (wlan_encmode.selectedIndex == 2 || wlan_encmode.selectedIndex == 3 || wlan_encmode.selectedIndex == 4){
				if(dF.wpaAuth[1].checked==true)
					get_by_id("show_8021x_eap").style.display = "none";
				else
					get_by_id("show_8021x_eap").style.display = "";
			}
		}else{//Router
			if (wlan_encmode.selectedIndex == 2 || wlan_encmode.selectedIndex == 3 || wlan_encmode.selectedIndex == 4 ){
				if(dF.wpaAuth[1].checked==true)
		get_by_id("show_8021x_eap").style.display = "none";
				else
					get_by_id("show_8021x_eap").style.display = "";
			}
		}		
		//get_by_id("show_8021x_eap").style.display = "none";
		//dF.auth_type[2].checked = true;
		dF.auth_type[0].disabled = false;
		dF.auth_type[1].disabled = false;
		dF.auth_type[2].disabled = false;
	}		
}

function show_wpa_settings()
{
	var dF=document.forms[0];
	var wep_type = get_by_id("method");
	var allow_tkip=0;
	if(wep_type.selectedIndex==2 || wep_type.selectedIndex==3)
		allow_tkip=0;
	else
		allow_tkip=1;
	get_by_id("show_wpa_psk1").style.display = "none";
	get_by_id("show_wpa_psk2").style.display = "none";	
	get_by_id("show_8021x_eap").style.display = "none";
//	get_by_id("show_pre_auth").style.display = "none";
	
	if (dF.wpaAuth[1].checked)
	{
		get_by_id("show_wpa_psk1").style.display = "";
		get_by_id("show_wpa_psk2").style.display = "";		
	}
	else{
		if (ap_mode != 1)
		get_by_id("show_8021x_eap").style.display = "";
//		if (wep_type.selectedIndex > 2) 
//			get_by_id("show_pre_auth").style.display = "";		
	}	
}

function show_wapi_settings()
{
        var dF=document.forms[0];
        var wep_type = get_by_id("method");
        
        get_by_id("show_wapi_psk1").style.display = "none";
        get_by_id("show_wapi_psk2").style.display = "none";
        get_by_id("show_8021x_wapi").style.display = "none";
//      get_by_id("show_pre_auth").style.display = "none";
        
        if (dF.wapiAuth[1].checked){
                get_by_id("show_wapi_psk1").style.display = "";
                get_by_id("show_wapi_psk2").style.display = "";
        }
        else{
                if (ap_mode != 1)
                {
                	get_by_id("show_8021x_wapi").style.display = "";
			if(''=='true')
			{
				get_by_id("show_8021x_wapi_local_as").style.display = "";
			}
			else
			{
				get_by_id("show_8021x_wapi_local_as").style.display = "none";
				dF.uselocalAS.checked=false;
			}
                }
		if (dF.wapiASIP.value == "192.168.1.1")
		{
			dF.uselocalAS.checked=true;
		}
//              if (wep_type.selectedIndex > 2) 
//                      get_by_id("show_pre_auth").style.display = "";          
        }
}

function show_wapi_ASip()
{
	var dF=document.forms[0];
	if (dF.uselocalAS.checked)
	{
		dF.wapiASIP.value = "192.168.1.1";
        }
	else
	{
		dF.wapiASIP.value = "";
	}
}

function show_authentication()
{	
	var wep_type = get_by_id("method");
	var enable_1x = get_by_id("use1x");	
	var form1 = document.formEncrypt ;

	get_by_id("show_wep_auth").style.display = "none";	
	get_by_id("setting_wep").style.display = "none";
	get_by_id("setting_wpa").style.display = "none";
	get_by_id("setting_wapi").style.display = "none";
//	get_by_id("show_pre_auth").style.display = "none";
	get_by_id("show_wpa_cipher").style.display = "none";
	get_by_id("show_wpa2_cipher").style.display = "none";
	get_by_id("enable_8021x").style.display = "none";
	get_by_id("show_8021x_eap").style.display = "none";
	get_by_id("show_8021x_wapi").style.display = "none";
	get_by_id("show_1x_wep").style.display = "none";
        get_by_id("show_wapi_psk1").style.display = "none";
        get_by_id("show_wapi_psk2").style.display = "none";
        get_by_id("show_8021x_wapi").style.display = "none";
	
	if (wep_type.selectedIndex == 1){
		get_by_id("show_wep_auth").style.display = "";		
		if (ap_mode == 1) 
			get_by_id("setting_wep").style.display = "";		
		else {
			get_by_id("enable_8021x").style.display = "";
			if(enable_1x.checked){		
				get_by_id("show_8021x_eap").style.display = "";
				get_by_id("show_1x_wep").style.display = "";
				get_by_id("setting_wep").style.display = "none";
			}else{		
				get_by_id("setting_wep").style.display = "";
			}
		}
	
	}else if (wep_type.selectedIndex > 1 && wep_type.selectedIndex < 5){
		get_by_id("setting_wpa").style.display = "";
		if (wep_type.selectedIndex == 2) {			
			get_by_id("show_wpa_cipher").style.display = "";
			if ( form1.isNmode.value == 1 ) {
				//alert("Select wpa");
				form1.ciphersuite_t.disabled = true;
				form1.wpa2ciphersuite_t.disabled = true;
			}
		}
		if(wep_type.selectedIndex ==3) {
			get_by_id("show_wpa2_cipher").style.display = "";
			if ( form1.isNmode.value == 1 ) {
				//alert("Select wpa2 and is Nmode");
				form1.ciphersuite_t.disabled = true;
				form1.wpa2ciphersuite_t.disabled = true;
			}
		}
		if(wep_type.selectedIndex ==4){
			get_by_id("show_wpa_cipher").style.display = "";
			get_by_id("show_wpa2_cipher").style.display = "";
			form1.ciphersuite_t.disabled = false;
			form1.wpa2ciphersuite_t.disabled = false;
		}		
		show_wpa_settings();
	}else if(wep_type.selectedIndex == 5 )
	{
		get_by_id("setting_wapi").style.display = "";
		show_wapi_settings();
	}
	
	if (wep_type.selectedIndex == 0) {
		if (ap_mode != 1) {
			get_by_id("enable_8021x").style.display = "";
			if(enable_1x.checked){		
				get_by_id("show_8021x_eap").style.display = "";
			}
			else {
				get_by_id("show_8021x_eap").style.display = "none";			
			}
		}
	}	
}

function setDefaultKeyValue(form, wlan_id)
{
  if (form.elements["length"+wlan_id].selectedIndex == 0) {
	if ( form.elements["format"+wlan_id].selectedIndex == 0) {
		form.elements["key"+wlan_id].maxLength = 5;
		form.elements["key"+wlan_id].value = "*****";
		
/*		
		form.elements["key1"+wlan_id].maxLength = 5;
		form.elements["key2"+wlan_id].maxLength = 5;
		form.elements["key3"+wlan_id].maxLength = 5;
		form.elements["key4"+wlan_id].maxLength = 5;
		form.elements["key1"+wlan_id].value = "*****";
		form.elements["key2"+wlan_id].value = "*****";
		form.elements["key3"+wlan_id].value = "*****";
		form.elements["key4"+wlan_id].value = "*****";
*/		
	}
	else {
		form.elements["key"+wlan_id].maxLength = 10;
		form.elements["key"+wlan_id].value = "**********";
		
/*		
		form.elements["key1"+wlan_id].maxLength = 10;
		form.elements["key2"+wlan_id].maxLength = 10;
		form.elements["key3"+wlan_id].maxLength = 10;
		form.elements["key4"+wlan_id].maxLength = 10;
		form.elements["key1"+wlan_id].value = "**********";
		form.elements["key2"+wlan_id].value = "**********";
		form.elements["key3"+wlan_id].value = "**********";
		form.elements["key4"+wlan_id].value = "**********";
*/
	}
  }
  else {
  	if ( form.elements["format"+wlan_id].selectedIndex == 0) {
		form.elements["key"+wlan_id].maxLength = 13;		
		form.elements["key"+wlan_id].value = "*************";		
/*		
		form.elements["key1"+wlan_id].maxLength = 13;
		form.elements["key2"+wlan_id].maxLength = 13;
		form.elements["key3"+wlan_id].maxLength = 13;
		form.elements["key4"+wlan_id].maxLength = 13;
		form.elements["key1"+wlan_id].value = "*************";
		form.elements["key2"+wlan_id].value = "*************";
		form.elements["key3"+wlan_id].value = "*************";
		form.elements["key4"+wlan_id].value = "*************";
*/		

	}
	else {
		form.elements["key"+wlan_id].maxLength = 26;
		form.elements["key"+wlan_id].value ="**************************";		
/*		
		form.elements["key1"+wlan_id].maxLength = 26;
		form.elements["key2"+wlan_id].maxLength = 26;
		form.elements["key3"+wlan_id].maxLength = 26;
		form.elements["key4"+wlan_id].maxLength = 26;
		form.elements["key1"+wlan_id].value ="**************************";
		form.elements["key2"+wlan_id].value ="**************************";
		form.elements["key3"+wlan_id].value ="**************************";
		form.elements["key4"+wlan_id].value ="**************************";
*/		
	}
  }
}
  
function updateWepFormat(form, wlan_id)
{
	if (form.elements["length" + wlan_id].selectedIndex == 0) {
		form.elements["format" + wlan_id].options[0].text = 'ASCII (5 characters)';
		form.elements["format" + wlan_id].options[1].text = 'Hex (10 characters)';
	}
	else {
		form.elements["format" + wlan_id].options[0].text = 'ASCII (13 characters)';
		form.elements["format" + wlan_id].options[1].text = 'Hex (26 characters)';
	}
	//form.elements["format" + wlan_id].selectedIndex =  wep_key_fmt;
	// Mason Yu. TBD
	//form.elements["format" + wlan_id].selectedIndex =  0;
	
	setDefaultKeyValue(form, wlan_id);
}

function saveChanges()
{
  form = document.formEncrypt;
  wpaAuth = form.wpaAuth;
  if (document.formEncrypt.method.selectedIndex>=2) {
        // Mason Yu. 201009_new_security. Start
        if (document.formEncrypt.method.selectedIndex ==2 || document.formEncrypt.method.selectedIndex ==4) {
        	if(form.ciphersuite_t.checked == false && form.ciphersuite_a.checked == false )
		{
			alert("<% multilang(LANG_WPA_CIPHER_SUITE_CAN_NOT_BE_EMPTY); %>");
			return false;
		}
        }
        
        if (document.formEncrypt.method.selectedIndex ==3 || document.formEncrypt.method.selectedIndex ==4) {
        	if(form.wpa2ciphersuite_t.checked == false && form.wpa2ciphersuite_a.checked == false )
		{
			alert("<% multilang(LANG_WPA2_CIPHER_SUITE_CAN_NOT_BE_EMPTY); %>");
			return false;
		}
        }
        // Mason Yu. 201009_new_security. End
         
	var str = document.formEncrypt.pskValue.value;
	if (document.formEncrypt.pskFormat.selectedIndex==1) {
		if (str.length != 64) {
			alert("<% multilang(LANG_PRE_SHARED_KEY_VALUE_SHOULD_BE_64_CHARACTERS); %>");
			document.formEncrypt.pskValue.focus();
			return false;
		}
		takedef = 0;
		if (defPskFormat == 1 && defPskLen == 64) {
			for (var i=0; i<64; i++) {
    				if ( str.charAt(i) != '*')
					break;
			}
			if (i == 64 )
				takedef = 1;
  		}
		if (takedef == 0) {
			for (var i=0; i<str.length; i++) {
    				if ( (str.charAt(i) >= '0' && str.charAt(i) <= '9') ||
					(str.charAt(i) >= 'a' && str.charAt(i) <= 'f') ||
					(str.charAt(i) >= 'A' && str.charAt(i) <= 'F') )
					continue;
				alert("<% multilang(LANG_INVALID_PRE_SHARED_KEY_VALUE_IT_SHOULD_BE_IN_HEX_NUMBER_0_9_OR_A_F); %>");
				document.formEncrypt.pskValue.focus();
				return false;
  			}
		}
	}
	else {
		if ( (document.formEncrypt.method.selectedIndex>=2 && wpaAuth[1].checked) ) {
		if (str.length < 8) {
			alert("<% multilang(LANG_PRE_SHARED_KEY_VALUE_SHOULD_BE_SET_AT_LEAST_8_CHARACTERS); %>");
			document.formEncrypt.pskValue.focus();
			return false;
		}
		if (str.length > 64) {
			alert("<% multilang(LANG_PRE_SHARED_KEY_VALUE_SHOULD_BE_LESS_THAN_64_CHARACTERS); %>");
			document.formEncrypt.pskValue.focus();
			return false;
		}
		if (checkString(document.formEncrypt.pskValue.value) == 0) {
			alert("<% multilang(LANG_INVALID_PRE_SHARED_KEY); %>");
			document.formEncrypt.pskValue.focus();
			return false;
		}
		}
	}
  }

   return true;
}


function postSecurity(encrypt, enable1X, wep, wpaAuth, wpaPSKFormat, wpaPSK, rsPort, rsIpAddr, rsPassword, ap, uCipher, wpa2uCipher, wepAuth, wepLen, wepKeyFormat) 
{	
	document.formEncrypt.method.value = encrypt;
	document.formEncrypt.pskFormat.value = wpaPSKFormat;
	document.formEncrypt.pskValue.value = wpaPSK;				
	document.formEncrypt.radiusIP.value = rsIpAddr;
	document.formEncrypt.radiusPort.value = rsPort;
	document.formEncrypt.radiusPass.value = rsPassword;
		
	if ( wep != 0 )
		document.formEncrypt.wepKeyLen[wep-1].checked = true;
	
	if (enable1X==1)
		document.formEncrypt.use1x.checked = true;		
	document.formEncrypt.wpaAuth[wpaAuth-1].checked = true;	
	
	// Mason Yu. 201009_new_security. Start
	document.formEncrypt.ciphersuite_t.checked = false;
	document.formEncrypt.ciphersuite_a.checked = false;
	if ( uCipher == 1 )
		document.formEncrypt.ciphersuite_t.checked = true;
	if ( uCipher == 2 )
		document.formEncrypt.ciphersuite_a.checked = true;
	if ( uCipher == 3 ) {
		document.formEncrypt.ciphersuite_t.checked = true;
		document.formEncrypt.ciphersuite_a.checked = true;
	}
	
	document.formEncrypt.wpa2ciphersuite_t.checked = false;
	document.formEncrypt.wpa2ciphersuite_a.checked = false;	
	if ( wpa2uCipher == 1 )
		document.formEncrypt.wpa2ciphersuite_t.checked = true;
	if ( wpa2uCipher == 2 )
		document.formEncrypt.wpa2ciphersuite_a.checked = true;
	if ( wpa2uCipher == 3 ) {
		document.formEncrypt.wpa2ciphersuite_t.checked = true;
		document.formEncrypt.wpa2ciphersuite_a.checked = true;
	}	

	document.formEncrypt.auth_type[wepAuth].checked = true;
	
	if ( wepLen == 0 )
		document.formEncrypt.length0.value = 1;
	else
		document.formEncrypt.length0.value = wepLen;
	
	document.formEncrypt.format0.value = wepKeyFormat+1;			
	show_authentication();
	// Mason Yu. 201009_new_security. End

	
        defPskLen = document.formEncrypt.pskValue.value.length;
	defPskFormat = document.formEncrypt.pskFormat.selectedIndex;
	updateWepFormat(document.formEncrypt, 0);
}

</script>

</head>

<body>
<blockquote>
<h2><font color="#0000FF"><% multilang(LANG_WLAN_SECURITY_SETTINGS); %></font></h2>

<form action=/boaform/admin/formWlEncrypt method=POST name="formEncrypt">
<table border=0 width="500" cellspacing=4 cellpadding=0>
    <tr><td><font size=2>
    <% multilang(LANG_PAGE_DESC_WLAN_SECURITY_SETTING); %>
    </font></td></tr>
    <tr><td><hr size=1 noshade align=top></td></tr>
</table>
<table border=0 width="500" cellspacing=4 cellpadding=0>
	<input type=hidden name="wlanDisabled" value=<% wlanStatus(); %>>
	<input type=hidden name="isNmode" value=0 >    
    
    <tr>
      <td width="35%"><font size="2"><b><% multilang(LANG_SSID); %> <% multilang(LANG_TYPE); %>:</b></font></td>
	<td width="65%"><font size="2">	
	  <% SSID_select(); %>
	  <!--
	  <input type="radio" name="wpaSSID" value="root"  <% postSSID("root"); %>>Root&nbsp;
	  <input type="radio" name="wpaSSID" value="vap0"  <% postSSID("vap0"); %>>VAP0&nbsp;
	  <input type="radio" name="wpaSSID" value="vap1"  <% postSSID("vap1"); %>>VAP1&nbsp;
	  <input type="radio" name="wpaSSID" value="vap2"  <% postSSID("vap2"); %>>VAP2&nbsp;
	  <input type="radio" name="wpaSSID" value="vap3"  <% postSSID("vap3"); %>>VAP3
	  -->
	</font></td>
    </tr>    
</table>

<table border=0 width="500" cellspacing=4 cellpadding=0>
    <tr><td><hr size=1 noshade align=top></td></tr>
     
    <tr>
      <td width="35%"><font size="2"><b><% multilang(LANG_ENCRYPTION); %>:&nbsp;</b>        
        <select size="1" name="method" onChange="show_authentication()">
	  	<% checkWrite("wpaEncrypt"); %> 
        </select></font></td>
    </tr>   
    <!-- Mason Yu -->
 
    <tr id="enable_8021x" style="display:none">
  	<td colspan="2" width="100%">			
  		<table width="100%" border="0" cellpadding="0" cellspacing="4">
  		 	<tr>
  				 <td width="30%" class="bgblue"><font size="2"><b>802.1x <% multilang(LANG_AUTHENTICATION); %>:</b></font></td>
  				 <td width="70%" class="bggrey"><font size="2">
  					<input type="checkbox" id="use1x" name="use1x" value="ON" onClick="show_8021x_settings()">
  						</font>
  			</td></tr>
  		</table>
    </td></tr>
    
    <tr id="show_wep_auth" style="display:none">
    	<td colspan="2" width="100%">			
    		<table width="100%" border="0" cellpadding="0" cellspacing="4">
    		 	<tr>	
    				<td width="30%"bgcolor="#FFFFFF" class="bgblue"><font size="2"><b><% multilang(LANG_AUTHENTICATION); %>:</b></font></td>
    				<td width="70%" class="bggrey"><font size="2">
    					<input name="auth_type" type=radio value="open"><% multilang(LANG_OPEN_SYSTEM); %>
    					<input name="auth_type" type=radio value="shared"><% multilang(LANG_SHARED_KEY); %>
    					<input name="auth_type" type=radio value="both"><% multilang(LANG_AUTO); %>
    						</font>
    			</td></tr>
    		</table>
    </td></tr>	
  
    <tr id="setting_wep" style="display:none">	
		<td colspan="2" width="100%">			
			<table width="100%" border="0" cellpadding="0" cellspacing="4">
			      <input type="hidden" name="wepEnabled" value="ON" checked>			
			 	<tr bgcolor="#FFFFFF">
					<td width="30%" bgcolor="#FFFFFF" class="bgblue"><font size="2"><b><% multilang(LANG_KEY_LENGTH); %>:</b></font></td>
					<td width="70%" class="bggrey"><font size="2">
					<select size="1" name="length0" id="length" onChange="updateWepFormat(document.formEncrypt, 0)">	
						 <option value=1> 64-bit</option>
						 <option value=2>128-bit</option>
					</select></font>
				</td></tr>
				<tr bgcolor="#FFFFFF">
					<td width="30%" bgcolor="#FFFFFF" class="bgblue"><font size="2"><b><% multilang(LANG_KEY_FORMAT); %>:</b></font></td>
					<td width="70%" class="bggrey">
					<select id="format" name="format0" onChange="setDefaultKeyValue(document.formEncrypt, 0)">
					     	<option value="1">ASCII</option>
						<option value="2">Hex</option>					
					</select>
				</td></tr>
				<tr bgcolor="#FFFFFF">
					<td width="30%" bgcolor="#FFFFFF" class="bgblue"><font size="2"><b><% multilang(LANG_ENCRYPTION_KEY); %>:</b></font></td>
					<td width="70%" class="bggrey">
						<input type="text" id="key" name="key0" maxlength="26" size="26" value="">
				</td></tr> 
			</table>				
	</td></tr>     

	<tr id="setting_wpa" style="display:none">
		<td colspan="2">	
			<table width="100%" border="0" cellpadding="0" cellspacing="4">			
				<tr>
					<td width="30%" class="bgblue"><font size="2"><b><% multilang(LANG_AUTHENTICATION_MODE); %>:</b></font></td>
					<td width="70%" class="bggrey"><font size="2">
						<input name="wpaAuth" type="radio" value="eap" onClick="show_wpa_settings()">Enterprise (RADIUS)
						<input name="wpaAuth" type="radio" value="psk" onClick="show_wpa_settings()">Personal (Pre-Shared Key)
							</font>
				</td></tr>  

				<tr id="show_wpa_cipher" style="display:none">
					<td width="30%" class="bgblue"><font size="2"><b>WPA <% multilang(LANG_CIPHER_SUITE); %>:</b></font></td>
					<td width="70%" class="bggrey"><font size="2">
						<input type="checkbox" name="ciphersuite_t" value=1>TKIP&nbsp;
						<input type="checkbox" name="ciphersuite_a" value=1>AES
							</font>
				</td></tr> 
					
				<tr id="show_wpa2_cipher" style="display:none">
					<td width="30%"class="bgblue"><font size="2"><b>WPA2 <% multilang(LANG_CIPHER_SUITE); %>:</b></font></td>
					<td width="70%" class="bggrey"><font size="2">
						<input type="checkbox" name="wpa2ciphersuite_t" value=1>TKIP&nbsp;
						<input type="checkbox" name="wpa2ciphersuite_a" value=1>AES
							</font>
				</td></tr> 
 
				<tr id="show_wpa_psk1" style="display:none">				
					<td width="30%" bgcolor="#FFFFFF" class="bgblue"><font size="2"><b><% multilang(LANG_PRE_SHARED_KEY_FORMAT); %>:</b></font></td>
					<td width="70%" class="bggrey">
					<select id="psk_fmt" name="pskFormat" onChange="">
						<option value="0">Passphrase</option>
						<option value="1">HEX (64 characters)</option>
						</select>
				</td></tr>
				<tr id="show_wpa_psk2" style="display:none">
					<td width="30%" bgcolor="#FFFFFF" class="bgblue"><font size="2"><b><% multilang(LANG_PRE_SHARED_KEY); %>:</b></font></td>
					<td width="70%" class="bggrey"><input type="password" name="pskValue" id="wpapsk" size="32" maxlength="64" value="">
				</td></tr>
			</table>		
	</td></tr>	 
        <tr id="setting_wapi" style="display:none">
                <td colspan="2">
                        <table width="100%" border="0" cellpadding="0" cellspacing="4">    
                                <tr>
                                        <td width="30%" class="bgblue"><font size="2"><b><% multilang(LANG_AUTHENTICATION_MODE); %>:</b></font></td>
                                        <td width="70%" class="bggrey"><font size="2">
                                                <input name="wapiAuth" type="radio" value="eap" onClick="show_wapi_settings()">Enterprise (AS Server)
                                                <input name="wapiAuth" type="radio" value="psk" onClick="show_wapi_settings()">Personal (Pre-Shared Key)
                                                        </font>
                                </td></tr>
				<tr id="show_wapi_psk1" style="display:none">
				<td width="30%" bgcolor="#FFFFFF" class="bgblue"><font size="2"><b><% multilang(LANG_PRE_SHARED_KEY_FORMAT); %>:</b></font></td>
                                        <td width="70%" class="bggrey">
                                        <select id="wapi_psk_fmt" name="wapiPskFormat" onChange="">
                                                <option value="0">Passphrase</option>
                                                <option value="1">HEX (64 characters)</option>
                                                </select>
                                </td></tr>
                                <tr id="show_wapi_psk2" style="display:none">
                                        <td width="30%" bgcolor="#FFFFFF" class="bgblue"><font size="2"><b><% multilang(LANG_PRE_SHARED_KEY); %>:</b></font></td>
                                        <td width="70%" class="bggrey"><input type="password" name="wapiPskValue" id="wapipsk" size="32" maxlength="64" value="">
                                </td></tr>
                        </table>
	</td></tr>
	<tr id="show_1x_wep" style="display:none">
		<td colspan="2">	
			<table width="100%" border="0" cellpadding="0" cellspacing="4">			
				<tr>	
					<td width="30%" class="bgblue"><font size="2"><b><% multilang(LANG_KEY_LENGTH); %>:</b></font></td>
					<td width="70%" class="bggrey"><font size="2">
						<input name="wepKeyLen" type="radio" value="wep64">64 Bits
						<input name="wepKeyLen" type="radio" value="wep128">128 Bits
							</font>
				</td></tr>
		 	</table>
	</td></tr> 

	<tr id="show_8021x_eap" style="display:none">
		<td colspan="2">			
			<table width="100%" border="0" cellpadding="0" cellspacing="4"> 
				<tr>
					 <td width="30%" bgcolor="#FFFFFF" class="bgblue"><font size="2"><b>RADIUS <% multilang(LANG_SERVER); %> <% multilang(LANG_IP_ADDRESS); %>:</b></font></td>
					 <td width="70%" bgcolor="#FFFFFF" class="bggrey"><input id="radius_ip" name="radiusIP" size="16" maxlength="15" value="0.0.0.0"></td>
				 </tr>
				<tr>
					<td width="30%" bgcolor="#FFFFFF" class="bgblue"><font size="2"><b>RADIUS <% multilang(LANG_SERVER); %> <% multilang(LANG_PORT); %>:</b></font></td>
					<td width="70%" class="bggrey"><input type="text" id="radius_port" name="radiusPort" size="5" maxlength="5" value="1812"></td>
				 </tr>
				<tr>
					<td width="30%" class="bgblue"><font size="2"><b>RADIUS <% multilang(LANG_SERVER); %> <% multilang(LANG_PASSWORD); %>:</b></font></td>
					<td width="70%" class="bggrey"><input type="password" id="radius_pass" name="radiusPass" size="32" maxlength="64" value="12345"></td>
				</tr>
			</table>								
	</td></tr>
			
        <tr id="show_8021x_wapi" style="display:none">
                <td colspan="2">
                        <table width="100%" border="0" cellpadding="0" cellspacing="4">
                                <tr id="show_8021x_wapi_local_as" style="">
                                <td width="30%" class="bgblue"><font size="2"><b><% multilang(LANG_USE_LOCAL_AS_SERVER); %>:</b></font></td>
                                <td width="70%" class="bggrey"><font size="2">
                                <input type="checkbox" id="uselocalAS" name="uselocalAS" value="ON" onClick="show_wapi_ASip()">
                                </font>
                                </td></tr>
				<tr>
                                         <td width="30%" bgcolor="#FFFFFF" class="bgblue"><font size="2"><b>AS <% multilang(LANG_SERVER); %> <% multilang(LANG_IP_ADDRESS); %>:</b></font></td>
                                         <td width="70%" bgcolor="#FFFFFF" class="bggrey"><input id="wapiAS_ip" name="wapiASIP" size="16" maxlength="15" value="0.0.0.0"></td></tr>
                        </table>
        </td></tr>        
        
        <tr>
        	<input type="hidden" name="wlan_idx" value=<% checkWrite("wlan_idx"); %>>
            <input type=hidden value="/admin/wlwpa_mbssid.asp" name="submit-url">
           <td width="100%" colspan="2"><input type=submit value="Apply Changes" name=save onClick="return saveChanges()">&nbsp;
           </td>
        </tr>
  
    <script>
        
	<% initPage("wlwpa_mbssid"); %>
	show_authentication();
	defPskLen = document.formEncrypt.pskValue.value.length;
	defPskFormat = document.formEncrypt.pskFormat.selectedIndex;
	updateWepFormat(document.formEncrypt, 0);
    </script>


  </table>  	

</form>
</blockquote>
</body>

</html>
