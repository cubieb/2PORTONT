<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<HTML>
<HEAD>
<TITLE>中国电信-MBSSID配置</TITLE>
<META http-equiv=pragma content=no-cache>
<META http-equiv=cache-control content="no-cache, must-revalidate">
<META http-equiv=content-type content="text/html; charset=gbk">
<META http-equiv=content-script-type content=text/javascript>
<STYLE type=text/css>
@import url(/style/default.css);
</STYLE>
<SCRIPT language="javascript" src="common.js"></SCRIPT>
<SCRIPT language="javascript" type="text/javascript">


/*****************************************************************************/
var wlCorerev = "3";//WPA加密:大于等于3时,WPA 加密列表值分别为："TKIP", "AES", "TKIP+AES"；否则WPA 加密列表值"TKIP"

/*vap0 variables*/
var var_vap0_wpa="tkip+aes";//用于判断后表列回显
var var_vap0_mode = "open";//网络认证方式
var var_vap0_wep = "disabled";//sep加密分别为：enabled，disabled
var var_vap0_keyIdx = "1";//网络密钥索引号分别对应：1,2,3,4
var var_vap0_enable = "1"; // 1 表示启用该vap, 0则不启用。
var var_vap0_ssid = "ChinaNet-vap0";//SSID值
var var_vap0_hiddenSSID = "0";//取消广播的开关,1表示启用，0表示不启用
var var_vap0_format = "0"; //密钥格式。 0: ascii, 1: 16 hex
var var_vap0_bit = "0";//密钥长度:WEP128=1;WEP64=0
var var_vap0_wpapskValue = "00005544";//WPA预共享密钥
//var var_vap0_wpapskUpInter = "55";//WPA更新会话密钥间隔
var var_vap0_keyIdx = "1";//网络密钥索引号分别对应：1,2,3,4
var var_vap0_keys = new Array( "key1", "key2","key3", "key4" ); //网络密钥值列表
var var_vap0_pskformat = "0"; //wpa 密钥格式: 0: passphrase; 1: hex

/*vap1 variables*/
var var_vap1_wpa="tkip+aes";
var var_vap1_mode = "open";
var var_vap1_wep = "disabled";
var var_vap1_keyIdx = "1";
var var_vap1_enable = "1"; 
var var_vap1_ssid = "ChinaNet-vap1";
var var_vap1_hiddenSSID = "0";
var var_vap1_format = "0"; //密钥格式。 0: ascii, 1: 16 hex
var var_vap1_bit = "0";
var var_vap1_wpapskValue = "00005544";
//var var_vap1_wpapskUpInter = "55";
var var_vap1_keyIdx = "1";
var var_vap1_keys = new Array( "key1", "key2","key3", "key4" ); 
var var_vap1_pskformat = "0"; //wpa 密钥格式: 0: passphrase; 1: hex

/*vap2 variables*/
var var_vap2_wpa="tkip+aes";
var var_vap2_mode = "open";
var var_vap2_wep = "disabled";
var var_vap2_keyIdx = "1";
var var_vap2_enable = "1"; 
var var_vap2_ssid = "ChinaNet-vap2";
var var_vap2_hiddenSSID = "0";
var var_vap2_bit = "0";
var var_vap2_format = "0"; //密钥格式。 0: ascii, 1: 16 hex
var var_vap2_wpapskValue = "00005544";
//var var_vap2_wpapskUpInter = "55";
var var_vap2_keyIdx = "1";
var var_vap2_keys = new Array( "key1", "key2","key3", "key4" ); 
var var_vap2_pskformat = "0"; //wpa 密钥格式: 0: passphrase; 1: hex

/*vap3 variables*/
var var_vap3_wpa="tkip+aes";
var var_vap3_mode = "open";
var var_vap3_wep = "disabled";
var var_vap3_keyIdx = "1";
var var_vap3_enable = "1"; 
var var_vap3_ssid = "ChinaNet-vap3";
var var_vap3_hiddenSSID = "0";
var var_vap3_bit = "0";
var var_vap3_format = "0"; //密钥格式。 0: ascii, 1: 16 hex
var var_vap3_wpapskValue = "00005544";
//var var_vap3_wpapskUpInter = "55";
var var_vap3_keyIdx = "1";
var var_vap3_keys = new Array( "key1", "key2","key3", "key4" ); 
var var_vap3_pskformat = "0"; //wpa 密钥格式: 0: passphrase; 1: hex

<%init_mbssid_page();%>

function setCheck(item, value)
{
    if ( value == '1' ) 
	{
         item.checked = true;
    }
	else 
	{
         item.checked = false;
    }
}

function isHexaDigit(val)
{
	if(val>='a' && 'f'>=val)
		return true;
	else if(val>='A' && 'F'>=val)
		return true;
	else if(val>='0' && '9'>=val)
		return true;
	else 
		return false;
}

function isValidwpapskValue(val) 
{
	var ret = false;
	var len = val.length;
	var maxSize = 64;
	var minSize = 8;

	if ( len >= minSize && len < maxSize )
		ret = true;
	else if ( len == maxSize ) 
	{
		for ( i = 0; i < maxSize; i++ )
		if ( isHexaDigit(val.charAt(i)) == false )
			break;
		if ( i == maxSize )
			ret = true;
	}
	else
		ret = false;

	return ret;
}

function isAllZero(val)
{
	var len = val.length;
	for( i=0; i<len; i++) 
		if( i != '0' ) 
			break;
	if( i == len )
		return true;
	else 
		return false;
}

function isValidKey(val, size) 
{
	var ret = false;
	var len = val.length;
	var dbSize = size * 2;

	if( isAllZero(val) )	//cathy
		return false;
	
	if ( len == size )
		ret = true;
	else if ( len == dbSize ) 
	{
		for ( i = 0; i < dbSize; i++ )
		if ( isHexaDigit(val.charAt(i)) == false )
			break;
		if ( i == dbSize )
			ret = true;
	} else
	ret = false;

	return ret;
}

function vap0_hideAllEncryptElements()
{
	showhide("vap0_WpaD", 0);
	showhide("vap0_WepD", 0);
	showhide("vap0_keyInfo", 0);
}

function vap1_hideAllEncryptElements()
{
	showhide("vap1_WpaD", 0);
	showhide("vap1_WepD", 0);
	showhide("vap1_keyInfo", 0);
}

function vap2_hideAllEncryptElements()
{
	showhide("vap2_WpaD", 0);
	showhide("vap2_WepD", 0);
	showhide("vap2_keyInfo", 0);
}

function vap3_hideAllEncryptElements()
{
	showhide("vap3_WpaD", 0);
	showhide("vap3_WepD", 0);
	showhide("vap3_keyInfo", 0);
}

function vap0_encryptChange()
{
	with ( document.forms[0] ) 
	{
		var cwep = getSelect(vap0_wlWep);
		var authMode = getSelect(vap0_AuthMode)

		if (cwep == "enabled") 
		{
			showhide("vap0_keyInfo", 1);
			if (authMode != "open" && authMode != "shared") 
			{
				vap0_Keys[0].disabled = 1;
				vap0_Keys[1].disabled = 0;
				vap0_Keys[2].disabled = 0;
				vap0_Keys[3].disabled = 1;
				vap0_KeyIndex.length = 0;
				vap0_KeyIndex[0] = new Option("2", "2");
				vap0_KeyIndex[1] = new Option("3", "3");
				if (var_vap0_keyIdx != "2" && var_vap0_keyIdx != "3") 
				{
					var_vap0_keyIdx = "2";
				}
				vap0_KeyIndex.selectedIndex = parseInt(var_vap0_keyIdx) - 2;
			}
			else 
			{
				vap0_Keys[0].disabled = 0;
				vap0_Keys[1].disabled = 0;
				vap0_Keys[2].disabled = 0;
				vap0_Keys[3].disabled = 0;
				vap0_KeyIndex.length = 0;
				vap0_KeyIndex[0] = new Option("1", "1");
				vap0_KeyIndex[1] = new Option("2", "2");
				vap0_KeyIndex[2] = new Option("3", "3");
				vap0_KeyIndex[3] = new Option("4", "4");
				vap0_KeyIndex.selectedIndex = parseInt(var_vap0_keyIdx) - 1;
			}
		} 
		else
		{
			showhide("vap0_keyInfo", 0);
		}
	}
}  

function vap1_encryptChange()
{
	with ( document.forms[0] ) 
	{
		var cwep = getSelect(vap1_wlWep);
		var authMode = getSelect(vap1_AuthMode)

		if (cwep == "enabled") 
		{
			showhide("vap1_keyInfo", 1);
			if (authMode != "open" && authMode != "shared") 
			{
				vap1_Keys[0].disabled = 1;
				vap1_Keys[1].disabled = 0;
				vap1_Keys[2].disabled = 0;
				vap1_Keys[3].disabled = 1;
				vap1_KeyIndex.length = 0;
				vap1_KeyIndex[0] = new Option("2", "2");
				vap1_KeyIndex[1] = new Option("3", "3");
				if (var_vap1_keyIdx != "2" && var_vap1_keyIdx != "3") 
				{
					var_vap1_keyIdx = "2";
				}
				vap1_KeyIndex.selectedIndex = parseInt(var_vap1_keyIdx) - 2;
			}
			else 
			{
				vap1_Keys[0].disabled = 0;
				vap1_Keys[1].disabled = 0;
				vap1_Keys[2].disabled = 0;
				vap1_Keys[3].disabled = 0;
				vap1_KeyIndex.length = 0;
				vap1_KeyIndex[0] = new Option("1", "1");
				vap1_KeyIndex[1] = new Option("2", "2");
				vap1_KeyIndex[2] = new Option("3", "3");
				vap1_KeyIndex[3] = new Option("4", "4");
				vap1_KeyIndex.selectedIndex = parseInt(var_vap1_keyIdx) - 1;
			}
		} 
		else
		{
			showhide("vap1_keyInfo", 0);
		}
	}
}  

function vap2_encryptChange()
{
	with ( document.forms[0] ) 
	{
		var cwep = getSelect(vap2_wlWep);
		var authMode = getSelect(vap2_AuthMode)

		if (cwep == "enabled") 
		{
			showhide("vap2_keyInfo", 1);
			if (authMode != "open" && authMode != "shared") 
			{
				vap2_Keys[0].disabled = 1;
				vap2_Keys[1].disabled = 0;
				vap2_Keys[2].disabled = 0;
				vap2_Keys[3].disabled = 1;
				vap2_KeyIndex.length = 0;
				vap2_KeyIndex[0] = new Option("2", "2");
				vap2_KeyIndex[1] = new Option("3", "3");
				if (var_vap2_keyIdx != "2" && var_vap2_keyIdx != "3") 
				{
					var_vap2_keyIdx = "2";
				}
				vap2_KeyIndex.selectedIndex = parseInt(var_vap2_keyIdx) - 2;
			}
			else 
			{
				vap2_Keys[0].disabled = 0;
				vap2_Keys[1].disabled = 0;
				vap2_Keys[2].disabled = 0;
				vap2_Keys[3].disabled = 0;
				vap2_KeyIndex.length = 0;
				vap2_KeyIndex[0] = new Option("1", "1");
				vap2_KeyIndex[1] = new Option("2", "2");
				vap2_KeyIndex[2] = new Option("3", "3");
				vap2_KeyIndex[3] = new Option("4", "4");
				vap2_KeyIndex.selectedIndex = parseInt(var_vap2_keyIdx) - 1;
			}
		} 
		else
		{
			showhide("vap2_keyInfo", 0);
		}
	}
}  

function vap3_encryptChange()
{
	with ( document.forms[0] ) 
	{
		var cwep = getSelect(vap3_wlWep);
		var authMode = getSelect(vap3_AuthMode)

		if (cwep == "enabled") 
		{
			showhide("vap3_keyInfo", 1);
			if (authMode != "open" && authMode != "shared") 
			{
				vap3_Keys[0].disabled = 1;
				vap3_Keys[1].disabled = 0;
				vap3_Keys[2].disabled = 0;
				vap3_Keys[3].disabled = 1;
				vap3_KeyIndex.length = 0;
				vap3_KeyIndex[0] = new Option("2", "2");
				vap3_KeyIndex[1] = new Option("3", "3");
				if (var_vap3_keyIdx != "2" && var_vap3_keyIdx != "3") 
				{
					var_vap3_keyIdx = "2";
				}
				vap3_KeyIndex.selectedIndex = parseInt(var_vap3_keyIdx) - 2;
			}
			else 
			{
				vap3_Keys[0].disabled = 0;
				vap3_Keys[1].disabled = 0;
				vap3_Keys[2].disabled = 0;
				vap3_Keys[3].disabled = 0;
				vap3_KeyIndex.length = 0;
				vap3_KeyIndex[0] = new Option("1", "1");
				vap3_KeyIndex[1] = new Option("2", "2");
				vap3_KeyIndex[2] = new Option("3", "3");
				vap3_KeyIndex[3] = new Option("4", "4");
				vap3_KeyIndex.selectedIndex = parseInt(var_vap3_keyIdx) - 1;
			}
		} 
		else
		{
			showhide("vap3_keyInfo", 0);
		}
	}
}  

function vap0_authModeChange(OnPageLoad) 
{
	with ( document.forms[0] ) 
	{
		var authMode = getSelect(vap0_AuthMode);
		var i, algos;
		var disableWepD = false;
		vap0_hideAllEncryptElements();
		switch ( authMode ) 
		{

			case 'open':	 
			case 'shared':
			showhide("vap0_WepD", 1);
			showhide("vap0_keyInfo", 1);
			break;

			case 'wpa':
			showhide("vap0_WpaD", 1);
			disableWepD = true;
			break;      

			case 'psk2':      
			case 'psk2mix': 
			case 'psk':  
			showhide("vap0_WpaD", 1);
			disableWepD = true;
			break;      	       
		}   

		if(OnPageLoad==1)
		{  

			if (wlCorerev >= 3)
			algos = new Array("TKIP", "AES", "TKIP+AES");
			else
			algos = new Array("TKIP");                  

			vap0_wlWpa.length = algos.length;

			for (var i in algos) 
			{          	

				vap0_wlWpa[i] = new Option(algos[i], algos[i].toLowerCase());
				vap0_wlWpa[i].value = algos[i].toLowerCase();

				if (algos[i].toLowerCase() == var_vap0_wpa)
				vap0_wlWpa[i].selected = true;

			}  

			if(var_vap0_wpa == "tkip")
				vap0_wlWpa.selectedIndex=0;
			else if(var_vap0_wpa == "aes")
				vap0_wlWpa.selectedIndex=1; 
			else
				vap0_wlWpa.selectedIndex=2; 
			
		}      

		if(OnPageLoad==0) 
		{
/*			if(authMode == "wpa" || authMode == "psk")
			vap0_wlWpa.selectedIndex=0;
			if(authMode == "wpa2" || authMode == "psk2")
			vap0_wlWpa.selectedIndex=1;  	
			if(authMode == "wpa2mix" || authMode == "psk2mix")  
			vap0_wlWpa.selectedIndex=2;    
*/
			var_vap0_wpa=vap0_wlWpa[vap0_wlWpa.selectedIndex].value;  
		}  

		vap0_wlWep.length = 0;
		if (authMode == "shared" || authMode == "radius")  // shared and radius must have wep on
		{ 
			vap0_wlWep[0] = new Option("启用", "enabled");
			vap0_wlWep[0].selected = true;
		}
		else 
		{
			vap0_wlWep[0] = new Option("禁用", "disabled");
			vap0_wlWep[1] = new Option("启用", "enabled");
/*
			if ((authMode.indexOf("wpa")!= -1 || authMode.indexOf("psk")!= -1) && (var_vap0_mode == "open" || var_vap0_mode == "shared")) // set wep off if switch to wpa modes
			{ 
				vap0_wlWep[0].selected = true;
			}
			if (var_vap0_wep == "enabled")
			{
				vap0_wlWep[1].selected = true;
			}
*/			
			if(authMode == "open" && var_vap0_wep == "enabled")
			{
				vap0_wlWep[1].selected = true;
			}
			else 
			{
				vap0_wlWep[0].selected = true;
			}
		}
		/* -start: not coexist with WPA-*/
		if(disableWepD)
		{
			setSelect(vap0_wlWep, "disabled");
			vap0_wlWep.disabled = 1;
		}
		else 
		{
			vap0_wlWep.disabled = 0;
		}
		/* -end: not coexist with WPA-*/
	}   
	vap0_encryptChange();
}

function vap1_authModeChange(OnPageLoad) 
{
	with ( document.forms[0] ) 
	{
		var authMode = getSelect(vap1_AuthMode);
		var i, algos;
		var disableWepD = false;
		vap1_hideAllEncryptElements();
		switch ( authMode ) 
		{

			case 'open':	 
			case 'shared':
			showhide("vap1_WepD", 1);
			showhide("vap1_keyInfo", 1);
			break;

			case 'wpa':
			showhide("vap1_WpaD", 1);
			disableWepD = true;
			break;      

			case 'psk2':      
			case 'psk2mix': 
			case 'psk':  
			showhide("vap1_WpaD", 1);
			disableWepD = true;
			break;      	       
		}   

		if(OnPageLoad==1)
		{  

			if (wlCorerev >= 3)
			algos = new Array("TKIP", "AES", "TKIP+AES");
			else
			algos = new Array("TKIP");                  

			vap1_wlWpa.length = algos.length;

			for (var i in algos) 
			{          	

				vap1_wlWpa[i] = new Option(algos[i], algos[i].toLowerCase());
				vap1_wlWpa[i].value = algos[i].toLowerCase();

				if (algos[i].toLowerCase() == var_vap1_wpa)
				vap1_wlWpa[i].selected = true;

			}  

			if(var_vap1_wpa == "tkip")
				vap1_wlWpa.selectedIndex=0;
			else if(var_vap1_wpa == "aes")
				vap1_wlWpa.selectedIndex=1; 
			else
				vap1_wlWpa.selectedIndex=2; 

		}      

		if(OnPageLoad==0) 
		{
/*
			if(authMode == "wpa" || authMode == "psk")
			vap1_wlWpa.selectedIndex=0;
			if(authMode == "wpa2" || authMode == "psk2")
			vap1_wlWpa.selectedIndex=1;  	
			if(authMode == "wpa2mix" || authMode == "psk2mix")  
			vap1_wlWpa.selectedIndex=2;    
*/
			var_vap1_wpa=vap1_wlWpa[vap1_wlWpa.selectedIndex].value;  
		}  

		vap1_wlWep.length = 0;
		if (authMode == "shared" || authMode == "radius")  // shared and radius must have wep on
		{ 
			vap1_wlWep[0] = new Option("启用", "enabled");
			vap1_wlWep[0].selected = true;
		}
		else 
		{
			vap1_wlWep[0] = new Option("禁用", "disabled");
			vap1_wlWep[1] = new Option("启用", "enabled");
/*
			if ((authMode.indexOf("wpa")!= -1 || authMode.indexOf("psk")!= -1) && (var_vap1_mode == "open" || var_vap1_mode == "shared")) // set wep off if switch to wpa modes
			{ 
				vap1_wlWep[0].selected = true;
			}
			if (var_vap1_wep == "enabled")
			{
				vap1_wlWep[1].selected = true;
			}
*/			
			if(authMode == "open" && var_vap1_wep == "enabled")
			{
				vap1_wlWep[1].selected = true;
			}
			else 
			{
				vap1_wlWep[0].selected = true;
			}
		}
		/* -start: not coexist with WPA-*/
		if(disableWepD)
		{
			setSelect(vap1_wlWep, "disabled");
			vap1_wlWep.disabled = 1;
		}
		else 
		{
			vap1_wlWep.disabled = 0;
		}
		/* -end: not coexist with WPA-*/
	}   
	vap1_encryptChange();
}

function vap2_authModeChange(OnPageLoad) 
{
	with ( document.forms[0] ) 
	{
		var authMode = getSelect(vap2_AuthMode);
		var i, algos;
		var disableWepD = false;
		vap2_hideAllEncryptElements();
		switch ( authMode ) 
		{

			case 'open':	 
			case 'shared':
			showhide("vap2_WepD", 1);
			showhide("vap2_keyInfo", 1);
			break;

			case 'wpa':
			showhide("vap2_WpaD", 1);
			disableWepD = true;
			break;      

			case 'psk2':      
			case 'psk2mix': 
			case 'psk':  
			showhide("vap2_WpaD", 1);
			disableWepD = true;
			break;      	       
		}   

		if(OnPageLoad==1)
		{  

			if (wlCorerev >= 3)
			algos = new Array("TKIP", "AES", "TKIP+AES");
			else
			algos = new Array("TKIP");                  

			vap2_wlWpa.length = algos.length;

			for (var i in algos) 
			{          	
				vap2_wlWpa[i] = new Option(algos[i], algos[i].toLowerCase());
				vap2_wlWpa[i].value = algos[i].toLowerCase();

				if (algos[i].toLowerCase() == var_vap2_wpa)
				vap2_wlWpa[i].selected = true;

			}  

			if(var_vap2_wpa == "tkip")
				vap2_wlWpa.selectedIndex=0;
			else if(var_vap2_wpa == "aes")
				vap2_wlWpa.selectedIndex=1; 
			else
				vap2_wlWpa.selectedIndex=2; 

		}      

		if(OnPageLoad==0) 
		{
/*
			if(authMode == "wpa" || authMode == "psk")
			vap2_wlWpa.selectedIndex=0;
			if(authMode == "wpa2" || authMode == "psk2")
			vap2_wlWpa.selectedIndex=1;  	
			if(authMode == "wpa2mix" || authMode == "psk2mix")  
			vap2_wlWpa.selectedIndex=2;    
*/
			var_vap2_wpa=vap2_wlWpa[vap2_wlWpa.selectedIndex].value;  
		}  

		vap2_wlWep.length = 0;
		if (authMode == "shared" || authMode == "radius")  // shared and radius must have wep on
		{ 
			vap2_wlWep[0] = new Option("启用", "enabled");
			vap2_wlWep[0].selected = true;
		}
		else 
		{
			vap2_wlWep[0] = new Option("禁用", "disabled");
			vap2_wlWep[1] = new Option("启用", "enabled");
/*
			if ((authMode.indexOf("wpa")!= -1 || authMode.indexOf("psk")!= -1) && (var_vap2_mode == "open" || var_vap2_mode == "shared")) // set wep off if switch to wpa modes
			{ 
				vap2_wlWep[0].selected = true;
			}
			if (var_vap2_wep == "enabled")
			{
				vap2_wlWep[1].selected = true;
			}
*/			
			if(authMode == "open" && var_vap2_wep == "enabled")
			{
				vap2_wlWep[1].selected = true;
			}
			else 
			{
				vap2_wlWep[0].selected = true;
			}
		}
		/* -start: not coexist with WPA-*/
		if(disableWepD)
		{
			setSelect(vap2_wlWep, "disabled");
			vap2_wlWep.disabled = 1;
		}
		else 
		{
			vap2_wlWep.disabled = 0;
		}
		/* -end: not coexist with WPA-*/
	}   
	vap2_encryptChange();
}

function vap3_authModeChange(OnPageLoad) 
{
	with ( document.forms[0] ) 
	{
		var authMode = getSelect(vap3_AuthMode);
		var i, algos;
		var disableWepD = false;
		vap3_hideAllEncryptElements();
		switch ( authMode ) 
		{

			case 'open':	 
			case 'shared':
			showhide("vap3_WepD", 1);
			showhide("vap3_keyInfo", 1);
			break;

			case 'wpa':
			showhide("vap3_WpaD", 1);
			disableWepD = true;
			break;      

			case 'psk2':      
			case 'psk2mix': 
			case 'psk':  
			showhide("vap3_WpaD", 1);
			disableWepD = true;
			break;      	       
		}   

		if(OnPageLoad==1)
		{  

			if (wlCorerev >= 3)
			algos = new Array("TKIP", "AES", "TKIP+AES");
			else
			algos = new Array("TKIP");                  

			vap3_wlWpa.length = algos.length;

			for (var i in algos) 
			{          	
				vap3_wlWpa[i] = new Option(algos[i], algos[i].toLowerCase());
				vap3_wlWpa[i].value = algos[i].toLowerCase();

				if (algos[i].toLowerCase() == var_vap3_wpa)
				vap3_wlWpa[i].selected = true;
			}  

			if(var_vap3_wpa == "tkip")
				vap3_wlWpa.selectedIndex=0;
			else if(var_vap3_wpa == "aes")
				vap3_wlWpa.selectedIndex=1; 
			else
				vap3_wlWpa.selectedIndex=2; 
		}      

		if(OnPageLoad==0) 
		{
/*			if(authMode == "wpa" || authMode == "psk")
			vap3_wlWpa.selectedIndex=0;
			if(authMode == "wpa2" || authMode == "psk2")
			vap3_wlWpa.selectedIndex=1;  	
			if(authMode == "wpa2mix" || authMode == "psk2mix")  
			vap3_wlWpa.selectedIndex=2;    
*/
			var_vap3_wpa=vap3_wlWpa[vap3_wlWpa.selectedIndex].value;  
		}  

		vap3_wlWep.length = 0;
		if (authMode == "shared" || authMode == "radius")  // shared and radius must have wep on
		{ 
			vap3_wlWep[0] = new Option("启用", "enabled");
			vap3_wlWep[0].selected = true;
		}
		else 
		{
			vap3_wlWep[0] = new Option("禁用", "disabled");
			vap3_wlWep[1] = new Option("启用", "enabled");
/*
			if ((authMode.indexOf("wpa")!= -1 || authMode.indexOf("psk")!= -1) && (var_vap3_mode == "open" || var_vap3_mode == "shared")) // set wep off if switch to wpa modes
			{ 
				vap3_wlWep[0].selected = true;
			}
			if (var_vap3_wep == "enabled")
			{
				vap3_wlWep[1].selected = true;
			}
*/			
			if(authMode == "open" && var_vap3_wep == "enabled")
			{
				vap3_wlWep[1].selected = true;
			}
			else 
			{
				vap3_wlWep[0].selected = true;
			}
		}
		/* -start: not coexist with WPA-*/
		if(disableWepD)
		{
			setSelect(vap3_wlWep, "disabled");
			vap3_wlWep.disabled = 1;
		}
		else 
		{
			vap3_wlWep.disabled = 0;
		}
		/* -end: not coexist with WPA-*/
	}   
	vap3_encryptChange();
}

function vap0_showElements(show)
{
	with ( document.forms[0] ) 
	{
   		if ( show == 1 )
		{
			showhide("vap0_SecInfo", 1);
			showhide("vap0_Encrypt", 1);
			vap0_authModeChange(0);
  		}
   		else 
		{
			showhide("vap0_SecInfo", 0);
			showhide("vap0_Encrypt", 0);
			vap0_hideAllEncryptElements();
  		}
	}
}

function vap1_showElements(show)
{
	with ( document.forms[0] ) 
	{
   		if ( show == 1 )
		{
			showhide("vap1_SecInfo", 1);
			showhide("vap1_Encrypt", 1);
			vap1_authModeChange(0);
  		}
   		else 
		{
			showhide("vap1_SecInfo", 0);
			showhide("vap1_Encrypt", 0);
			vap1_hideAllEncryptElements();
  		}
	}
}

function vap2_showElements(show)
{
	with ( document.forms[0] ) 
	{
   		if ( show == 1 )
		{
			showhide("vap2_SecInfo", 1);
			showhide("vap2_Encrypt", 1);
			vap2_authModeChange(0);
  		}
   		else 
		{
			showhide("vap2_SecInfo", 0);
			showhide("vap2_Encrypt", 0);
			vap2_hideAllEncryptElements();
  		}
	}
}

function vap3_showElements(show)
{
	with ( document.forms[0] ) 
	{
   		if ( show == 1 )
		{
			showhide("vap3_SecInfo", 1);
			showhide("vap3_Encrypt", 1);
			vap3_authModeChange(0);
  		}
   		else 
		{
			showhide("vap3_SecInfo", 0);
			showhide("vap3_Encrypt", 0);
			vap3_hideAllEncryptElements();
  		}
	}
}

function vap0_CbClick(cb) 
{
	with( document.forms[0])
	{
   		if ( cb.checked == true )
		{
     			vap0_showElements(1);
  		}
   		else 
		{
     		 	vap0_showElements(0);
  		}
  	}
}

function vap1_CbClick(cb) 
{
	with( document.forms[0])
	{
   		if ( cb.checked == true )
		{
     			vap1_showElements(1);
  		}
   		else 
		{
     		 	vap1_showElements(0);
  		}
  	}
}

function vap2_CbClick(cb) 
{
	with( document.forms[0])
	{
   		if ( cb.checked == true )
		{
     			vap2_showElements(1);
  		}
   		else 
		{
     		 	vap2_showElements(0);
  		}
  	}
}

function vap3_CbClick(cb) 
{
	with( document.forms[0])
	{
   		if ( cb.checked == true )
		{
     			vap3_showElements(1);
  		}
   		else 
		{
     		 	vap3_showElements(0);
  		}
  	}
}


function on_init()
{
	with ( document.forms[0] ) 
	{
		/**************vap0**************/
		setCheck(vap0_enable, var_vap0_enable);
		
		if(var_vap0_hiddenSSID=="1")
			vap0_hide.checked=true;
		else
			vap0_hide.checked=false;

		vap0_ssid.value = var_vap0_ssid;

		setSelect(vap0_AuthMode, var_vap0_mode);
		vap0_PskFormat.selectedIndex = parseInt(var_vap0_pskformat);		
		vap0_WpaPsk.value = var_vap0_wpapskValue;
		//vap0_wpaInter.value = var_vap0_wpapskUpInter;
		vap0_KeyBit.selectedIndex = parseInt(var_vap0_bit);
		vap0_KeyFormat.selectedIndex = parseInt(var_vap0_format);
		for ( i = 0; i < 4; i++ ) 
		{
			vap0_Keys[i].value = var_vap0_keys[i];
		}

		vap0_authModeChange(1);

		if ( var_vap0_enable == '1' ) 
		{
			vap0_showElements(1);
		}else{
			vap0_showElements(0);
		}

		/**************vap1**************/
		setCheck(vap1_enable, var_vap1_enable);
		
		if(var_vap1_hiddenSSID=="1")
			vap1_hide.checked=true;
		else
			vap1_hide.checked=false;

		vap1_ssid.value = var_vap1_ssid;

		setSelect(vap1_AuthMode, var_vap1_mode);
		vap1_PskFormat.selectedIndex = parseInt(var_vap1_pskformat);		
		vap1_WpaPsk.value = var_vap1_wpapskValue;
		//vap1_wpaInter.value = var_vap1_wpapskUpInter;
		vap1_KeyBit.selectedIndex = parseInt(var_vap1_bit);
		vap1_KeyFormat.selectedIndex = parseInt(var_vap1_format);
		for ( i = 0; i < 4; i++ ) 
		{
			vap1_Keys[i].value = var_vap1_keys[i];
		}

		vap1_authModeChange(1);
		
		if ( var_vap1_enable == '1' ) 
		{
			vap1_showElements(1);
		}else{
			vap1_showElements(0);
		}

		/**************vap2**************/
		setCheck(vap2_enable, var_vap2_enable);
		
		if(var_vap2_hiddenSSID=="1")
			vap2_hide.checked=true;
		else
			vap2_hide.checked=false;
		
		vap2_ssid.value = var_vap2_ssid;

		setSelect(vap2_AuthMode, var_vap2_mode);
		vap2_PskFormat.selectedIndex = parseInt(var_vap2_pskformat);		
		vap2_WpaPsk.value = var_vap2_wpapskValue;
		//vap2_wpaInter.value = var_vap2_wpapskUpInter;
		vap2_KeyBit.selectedIndex = parseInt(var_vap2_bit);
		vap2_KeyFormat.selectedIndex = parseInt(var_vap2_format);
		for ( i = 0; i < 4; i++ ) 
		{
			vap2_Keys[i].value = var_vap2_keys[i];
		}

		vap2_authModeChange(1);
		
		if ( var_vap2_enable == '1' ) 
		{
			vap2_showElements(1);
		}else{
			vap2_showElements(0);
		}

		/**************vap3**************/
		setCheck(vap3_enable, var_vap3_enable);

		if(var_vap3_hiddenSSID=="1")
			vap3_hide.checked=true;
		else
			vap3_hide.checked=false;

		vap3_ssid.value = var_vap3_ssid;
		
		setSelect(vap3_AuthMode, var_vap3_mode);
		vap3_PskFormat.selectedIndex = parseInt(var_vap3_pskformat);
		vap3_WpaPsk.value = var_vap3_wpapskValue;
		//vap3_wpaInter.value = var_vap3_wpapskUpInter;
		vap3_KeyBit.selectedIndex = parseInt(var_vap3_bit);
		vap3_KeyFormat.selectedIndex = parseInt(var_vap3_format);
		for ( i = 0; i < 4; i++ ) 
		{
			vap3_Keys[i].value = var_vap3_keys[i];
		}

		vap3_authModeChange(1);
		
		if ( var_vap3_enable == '1' ) 
		{
			vap3_showElements(1);
		}else{
			vap3_showElements(0);
		}
		
	}
}


function on_submit() 
{
	with ( document.forms[0] ) 
	{
		/********vap0********/
		if ( vap0_enable.checked == true ) 
		{
			if ( vap0_ssid.value == '' ) 
			{
				alert('vap0 SSID不能为空.');
				return;
			}
			var str = new String();
			str = vap0_ssid.value;
			if ( str.length > 32 ) 
			{
				alert('vap0 SSID "' + vap0_ssid.value + '" 不能大于32个字符。');
				return;
			}
			if(isIncludeInvalidChar(vap0_ssid.value))
			{
				alert(" vap0 SSID 含有非法字符，请重新输入!");
				return;
			}	

			var vap0_swep = getSelect(vap0_wlWep);
			var vap0_authMode = getSelect(vap0_AuthMode);
			var value;
		  
			if (vap0_authMode == 'psk' || vap0_authMode == 'psk2')
			{
				var pskformat = getSelect(vap0_PskFormat);
				value = vap0_WpaPsk.value;

			 	if((pskformat == '0' && (value.length > 63 || value.length < 8)) || (pskformat == '1' && value.length != 64) || (isValidwpapskValue(value) == false))
				{
					alert('WPA预共用的密钥应该是8至63个ASCII字符或64个十六进制数字.');
					return;
				}
			}
	/*
			if (vap0_authMode == 'psk' || vap0_authMode == 'psk2')
			{ 
				value = parseInt(vap0_wpaInter.value);
			
				if (  isNaN(value) == true || value < 0 || value > 0xffffffff )
				{
					alert('WPA 群组更新密钥间隔时间 "' + vap0_wpaInter.value + '" 应该在 0 到 4294967295 之间.');
					return;
				}
			}
	*/	  
			if ((vap0_authMode == 'open' || vap0_authMode == 'shared') && getSelect(vap0_wlWep) == "enabled") 
			{
				var i, val;
				var vap0_cbit = getSelect(vap0_KeyBit);
				var vap0_cformat = getSelect(vap0_KeyFormat);
				var vap0_num = parseInt(getSelect(vap0_KeyIndex))-1;
				val = vap0_Keys[vap0_num].value;
				if ( val == '' && !(vap0_swep == 'enabled' && vap0_authMode == 'radius')) 
				{
					alert('不能选择空值密钥.');
					return;
				}
				var tmplst = "";
				for (i=0; i<4; i++)
				{
					val = vap0_Keys[i].value;
					if ( val != '' ) 
					{
						if(vap0_cbit == '0')
						{
							if(!((vap0_cformat == '0' && val.length == 5) ||(vap0_cformat == '1' && val.length == 10)) || isValidKey(val, 5) == false)
							{
								alert('Key "' + val + '" 无效. 请输入5个ASCII码或10个16进制数! ');
								return;
							}
						}else{
							if(!((vap0_cformat == '0' && val.length == 13) ||(vap0_cformat == '1' && val.length == 26)) ||  isValidKey(val, 13) == false)
							{
								alert('Key "' + val + '" 无效. 请输入13个ASCII码或26个16进制数! ');
								return;
							}
						}
					}
					if(i>0)
						tmplst += ";";
					tmplst += val;
				} 
				vap0_lst.value = tmplst;
			}
		}

		/********vap1********/
		if ( vap1_enable.checked == true ) 
		{
			if ( vap1_ssid.value == '' ) 
			{
				alert('vap1 SSID不能为空.');
				return;
			}
			var str = new String();
			str = vap1_ssid.value;
			if ( str.length > 32 ) 
			{
				alert('vap1 SSID "' + vap1_ssid.value + '" 不能大于32个字符。');
				return;
			}
			if(isIncludeInvalidChar(vap1_ssid.value))
			{
				alert(" vap1 SSID 含有非法字符，请重新输入!");
				return;
			}	

			var vap1_swep = getSelect(vap1_wlWep);
			var vap1_authMode = getSelect(vap1_AuthMode);
			var value;
		  
			if (vap1_authMode == 'psk' || vap1_authMode == 'psk2')
			{
				var pskformat = getSelect(vap1_PskFormat);
				value = vap1_WpaPsk.value;
			 
			 	if((pskformat == '0' && (value.length > 63 || value.length < 8)) || (pskformat == '1' && value.length != 64) || (isValidwpapskValue(value) == false))
				{
					alert('WPA预共用的密钥应该是8至63个ASCII字符或64个十六进制数字.');
					return;
				}
			}
	/*
			if (vap1_authMode == 'psk' || vap1_authMode == 'psk2')
			{ 
				value = parseInt(vap1_wpaInter.value);
			
				if (  isNaN(value) == true || value < 0 || value > 0xffffffff )
				{
					alert('WPA 群组更新密钥间隔时间 "' + vap1_wpaInter.value + '" 应该在 0 到 4294967295 之间.');
					return;
				}
			}
	*/	  
			if ((vap1_authMode == 'open' || vap1_authMode == 'shared') && getSelect(vap1_wlWep) == "enabled") 
			{
				var i, val;
				var vap1_cbit = getSelect(vap1_KeyBit);
				var vap1_cformat = getSelect(vap1_KeyFormat);
				var vap1_num = parseInt(getSelect(vap1_KeyIndex))-1;
				val = vap1_Keys[vap1_num].value;
				if ( val == '' && !(vap1_swep == 'enabled' && vap1_authMode == 'radius')) 
				{
					alert('不能选择空值密钥.');
					return;
				}
				var tmplst = "";
				for (i=0; i<4; i++)
				{
					val = vap1_Keys[i].value;
					if ( val != '' ) 
					{
						if(vap1_cbit == '0')
						{
							if(!((vap1_cformat == '0' && val.length == 5) ||(vap1_cformat == '1' && val.length == 10)) || isValidKey(val, 5) == false)
							{
								alert('Key "' + val + '" 无效. 请输入5个ASCII码或10个16进制数! ');
								return;
							}
						}else{
							if(!((vap1_cformat == '0' && val.length == 13) ||(vap1_cformat == '1' && val.length == 26)) ||  isValidKey(val, 13) == false)
							{
								alert('Key "' + val + '" 无效. 请输入13个ASCII码或26个16进制数! ');
								return;
							}
						}
					}
					if(i>0)
						tmplst += ";";
					tmplst += val;
				} 
				vap1_lst.value = tmplst;
			}
		}
		
		/********vap2********/
		if ( vap2_enable.checked == true ) 
		{
			if ( vap2_ssid.value == '' ) 
			{
				alert('vap2 SSID不能为空.');
				return;
			}
			var str = new String();
			str = vap2_ssid.value;
			if ( str.length > 32 ) 
			{
				alert('vap2 SSID "' + vap2_ssid.value + '" 不能大于32个字符。');
				return;
			}
			if(isIncludeInvalidChar(vap2_ssid.value))
			{
				alert(" vap2 SSID 含有非法字符，请重新输入!");
				return;
			}	

			var vap2_swep = getSelect(vap2_wlWep);
			var vap2_authMode = getSelect(vap2_AuthMode);
			var value;
		  
			if (vap2_authMode == 'psk' || vap2_authMode == 'psk2')
			{
				var pskformat = getSelect(vap2_PskFormat);		
				value = vap2_WpaPsk.value;
			 
			 	if((pskformat == '0' && (value.length > 63 || value.length < 8)) || (pskformat == '1' && value.length != 64) || (isValidwpapskValue(value) == false))
				{
					alert('WPA预共用的密钥应该是8至63个ASCII字符或64个十六进制数字.');
					return;
				}
			}
	/*
			if (vap2_authMode == 'psk' || vap2_authMode == 'psk2')
			{ 
				value = parseInt(vap2_wpaInter.value);
			
				if (  isNaN(value) == true || value < 0 || value > 0xffffffff )
				{
					alert('WPA 群组更新密钥间隔时间 "' + vap2_wpaInter.value + '" 应该在 0 到 4294967295 之间.');
					return;
				}
			}
	*/	  
			if ((vap2_authMode == 'open' || vap2_authMode == 'shared') && getSelect(vap2_wlWep) == "enabled") 
			{
				var i, val;
				var vap2_cbit = getSelect(vap2_KeyBit);
				var vap2_cformat = getSelect(vap2_KeyFormat);
				var vap2_num = parseInt(getSelect(vap2_KeyIndex))-1;
				val = vap2_Keys[vap2_num].value;
				if ( val == '' && !(vap2_swep == 'enabled' && vap2_authMode == 'radius')) 
				{
					alert('不能选择空值密钥.');
					return;
				}
				var tmplst = "";
				for (i=0; i<4; i++)
				{
					val = vap2_Keys[i].value;
					if ( val != '' ) 
					{
						if(vap2_cbit == '0')
						{
							if(!((vap2_cformat == '0' && val.length == 5) ||(vap2_cformat == '1' && val.length == 10)) || isValidKey(val, 5) == false)
							{
								alert('Key "' + val + '" 无效. 请输入5个ASCII码或10个16进制数! ');
								return;
							}
						}else{
							if(!((vap2_cformat == '0' && val.length == 13) ||(vap2_cformat == '1' && val.length == 26)) ||  isValidKey(val, 13) == false)
							{
								alert('Key "' + val + '" 无效. 请输入13个ASCII码或26个16进制数! ');
								return;
							}
						}
					}
					if(i>0)
						tmplst += ";";
					tmplst += val;
				} 
				vap2_lst.value = tmplst;
			}
		}


		/********vap3********/
		if ( vap3_enable.checked == true ) 
		{
			if ( vap3_ssid.value == '' ) 
			{
				alert('vap3 SSID不能为空.');
				return;
			}
			var str = new String();
			str = vap3_ssid.value;
			if ( str.length > 32 ) 
			{
				alert('vap3 SSID "' + vap3_ssid.value + '" 不能大于32个字符。');
				return;
			}
			if(isIncludeInvalidChar(vap3_ssid.value))
			{
				alert(" vap3 SSID 含有非法字符，请重新输入!");
				return;
			}	

			var vap3_swep = getSelect(vap3_wlWep);
			var vap3_authMode = getSelect(vap3_AuthMode);
			var value;
		  
			if (vap3_authMode == 'psk' || vap3_authMode == 'psk2')
			{
				var pskformat = getSelect(vap3_PskFormat);		
				value = vap3_WpaPsk.value;
			 
			 	if((pskformat == '0' && (value.length > 63 || value.length < 8)) || (pskformat == '1' && value.length != 64) || (isValidwpapskValue(value) == false))
				{
					alert('WPA预共用的密钥应该是8至63个ASCII字符或64个十六进制数字.');
					return;
				}
			}
	/*
			if (vap3_authMode == 'psk' || vap3_authMode == 'psk2')
			{ 
				value = parseInt(vap3_wpaInter.value);
			
				if (  isNaN(value) == true || value < 0 || value > 0xffffffff )
				{
					alert('WPA 群组更新密钥间隔时间 "' + vap3_wpaInter.value + '" 应该在 0 到 4294967295 之间.');
					return;
				}
			}
	*/	  
			if ((vap3_authMode == 'open' || vap3_authMode == 'shared') && getSelect(vap3_wlWep) == "enabled") 
			{
				var i, val;
				var vap3_cbit = getSelect(vap3_KeyBit);
				var vap3_cformat = getSelect(vap3_KeyFormat);
				var vap3_num = parseInt(getSelect(vap3_KeyIndex))-1;
				val = vap3_Keys[vap3_num].value;
				if ( val == '' && !(vap3_swep == 'enabled' && vap3_authMode == 'radius')) 
				{
					alert('不能选择空值密钥.');
					return;
				}
				var tmplst = "";
				for (i=0; i<4; i++)
				{
					val = vap3_Keys[i].value;
					if ( val != '' ) 
					{
						if(vap3_cbit == '0')
						{
							if(!((vap3_cformat == '0' && val.length == 5) ||(vap3_cformat == '1' && val.length == 10)) || isValidKey(val, 5) == false)
							{
								alert('Key "' + val + '" 无效. 请输入5个ASCII码或10个16进制数! ');
								return;
							}
						}else{
							if(!((vap3_cformat == '0' && val.length == 13) ||(vap3_cformat == '1' && val.length == 26)) ||  isValidKey(val, 13) == false)
							{
								alert('Key "' + val + '" 无效. 请输入13个ASCII码或26个16进制数! ');
								return;
							}
						}
					}
					if(i>0)
						tmplst += ";";
					tmplst += val;
				} 
				vap3_lst.value = tmplst;
			}
		}

		
		submit();
	}
}


  </script>
</head>
   
<!--主页代码-->

   <body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad='on_init()'>
      <blockquote>
	    <DIV align="left" style="padding-left:20px; padding-top:10px">
         <form action=/boaform/formMBSSID method="post">
    <b>无线设置 -- 虚拟SSID</b><br>
		<br>
    本页用于配置无线虚拟SSID。点击"保存/应用"，无线虚拟SSID的配置生效。<br>
 		<br><br>
 		
		<table border="0" cellpadding="4" cellspacing="0">
            	<tr>
				<td bgColor=#aaddff>虚拟SSID0</td>
				<td bgColor=#aaddff  width='380'><input type='checkbox' name='vap0_enable' onClick='vap0_CbClick(this)' value="ON" >启用</td>         
			</tr>
		</table>
  		<div id='vap0_SecInfo'>
  		<br>
			<table border="0" cellpadding="0" cellspacing="0">
				<tr>
					<td valign="middle" align="left" width="30" height="30"><input type='checkbox' name='vap0_hide' value="ON"></td>
					<td>取消广播</td>
				</tr>
			</table>
			<br>
			<table border="0" cellpadding="0" cellspacing="0">
				<tr>
					<td width="150">SSID:</td>
					<td><input type='text' name='vap0_ssid' maxlength="32" size="32"></td>
				</tr>
			</table>
		</div>
  		<div id='vap0_Encrypt'>
		<br>
			<table border="0" cellpadding="0" cellspacing="0">
				<tr>
					<td width='150'>网络认证方式:</td>
					<td>
						<select name='vap0_AuthMode' size="1" onChange='vap0_authModeChange(0)'>
						<option value="open">开放</option>
						<option value="shared">共享</option>
						<option value="psk">WPA-PSK</option>
						<option value="psk2">WPA2-PSK</option>
						</select>
					</td>
				</tr>
			</table>
		</div>
		<div id="vap0_WpaD">
			<br>
			<table border="0" cellpadding="0" cellspacing="0">
				<tr>
					<td width='150'>WPA密钥格式:</td>
					<td><select name='vap0_PskFormat' size='1'>
						<option value="0">passphrase</option>
						<option value="1">64位16进制码</option>
						</select>
					</td>
				</tr>
				<tr>
					<td width='150'>WPA 预共享密钥:</td>
					<td><input type='text' name='vap0_WpaPsk' size='20' maxlength='64'></td>
				</tr>
<!--
				<tr>
					<td width='150'>WPA 更新会话密钥间隔:</td>
					<td><input type='text' name='vap0_wpaInter' size='20' maxlength='10'></td>
				</tr>
-->
				<tr>
					<td width='150'>WPA 加密:</td>
					<td><select name="vap0_wlWpa" onChange='vap0_encryptChange()'></select></td>
				</tr>
			</table>
		</div>
		<div id="vap0_WepD">
			<br>
			<table border="0" cellpadding="0" cellspacing="0">
				<tr>
					<td width='150'>WEP 加密:</td>
					<td><select name="vap0_wlWep" onChange='vap0_encryptChange()'></select></td>
				</tr>
			</table>
		</div>
		<div id='vap0_keyInfo'>
			<table border="0" cellpadding="0" cellspacing="0">
				<tr>
					<td width='150'>密钥长度:</td>
					<td><select name='vap0_KeyBit' size='1'>
						<option value="0">64-bit</option>
						<option value="1">128-bit</option>
						</select>
					</td>
				</tr>
				<tr>
					<td width='150'>密钥格式:</td>
					<td><select name='vap0_KeyFormat' size='1'>
						<option value="0">ASCII</option>
						<option value="1">16HEX</option>
						</select>
					</td>
				</tr>
				<tr>
					<td >当前网络密钥索引号:</td>
					<td><select name='vap0_KeyIndex' size='1'></select></td>
				</tr>
				<tr>
					<td>网络密钥 1:</td>
					<td><input type='text' name='vap0_Keys' size='30' maxlength=26></td>
				</tr>
				<tr>
                     	<td>网络密钥 2:</td>
					<td><input type='text' name='vap0_Keys' size='30' maxlength=26></td>
				</tr>
				<tr>
					<td>网络密钥 3:</td>
					<td><input type='text' name='vap0_Keys' size='30' maxlength=26></td>
				</tr>
				<tr>
					<td>网络密钥 4:</td>
					<td><input type='text' name='vap0_Keys' size='30' maxlength=26></td>
				</tr>
				<tr> 
					<td>&nbsp;</td>
					<td>64位密钥要求输入5个ASCII字符或10个十六进制数字</td>
				</tr>
				<tr> 
					<td>&nbsp;</td>
					<td>128位密钥要求输入13个ASCII字符或26个十六进制数字</td>
				</tr>
			</table>
		</div>

		<br><br>
		
 		<table border="0" cellpadding="4" cellspacing="0">
            	<tr>
				<td bgColor=#aaddff>虚拟SSID1</td>
				<td bgColor=#aaddff  width='380'><input type='checkbox' name='vap1_enable' onClick='vap1_CbClick(this)' value="ON" >启用</td>         
			</tr>
		</table>
  		<div id='vap1_SecInfo'>
		<br>
			<table border="0" cellpadding="0" cellspacing="0">
				<tr>
					<td valign="middle" align="left" width="30" height="30"><input type='checkbox' name='vap1_hide' value="ON"></td>
					<td>取消广播</td>
				</tr>
			</table>
			<br>
			<table border="0" cellpadding="0" cellspacing="0">
				<tr>
					<td width="150">SSID:</td>
					<td><input type='text' name='vap1_ssid' maxlength="32" size="32"></td>
				</tr>
			</table>
		</div>
  		<div id='vap1_Encrypt'>
		<br>
			<table border="0" cellpadding="0" cellspacing="0">
				<tr>
					<td width='150'>网络认证方式:</td>
					<td>
						<select name='vap1_AuthMode' size="1" onChange='vap1_authModeChange(0)'>
						<option value="open">开放</option>
						<option value="shared">共享</option>
						<option value="psk">WPA-PSK</option>
						<option value="psk2">WPA2-PSK</option>
						</select>
					</td>
				</tr>
			</table>
		</div>
		<div id="vap1_WpaD">
			<br>
			<table border="0" cellpadding="0" cellspacing="0">
				<tr>
					<td width='150'>WPA密钥格式:</td>
					<td><select name='vap1_PskFormat' size='1'>
						<option value="0">passphrase</option>
						<option value="1">64位16进制码</option>
						</select>
					</td>
				</tr>
				<tr>
					<td width='150'>WPA 预共享密钥:</td>
					<td><input type='text' name='vap1_WpaPsk' size='20' maxlength='64'></td>
				</tr>
<!--
				<tr>
					<td width='150'>WPA 更新会话密钥间隔:</td>
					<td><input type='text' name='vap1_wpaInter' size='20' maxlength='10'></td>
				</tr>
-->
				<tr>
					<td width='150'>WPA 加密:</td>
					<td><select name="vap1_wlWpa" onChange='vap1_encryptChange()'></select></td>
				</tr>
			</table>
		</div>
		<div id="vap1_WepD">
			<br>
			<table border="0" cellpadding="0" cellspacing="0">
				<tr>
					<td width='150'>WEP 加密:</td>
					<td><select name="vap1_wlWep" onChange='vap1_encryptChange()'></select></td>
				</tr>
			</table>
		</div>
		<div id='vap1_keyInfo'>
			<table border="0" cellpadding="0" cellspacing="0">
				<tr>
					<td width='150'>密钥长度:</td>
					<td><select name='vap1_KeyBit' size='1'>
						<option value="0">64-bit</option>
						<option value="1">128-bit</option>
						</select>
					</td>
				</tr>
				<tr>
					<td width='150'>密钥格式:</td>
					<td><select name='vap1_KeyFormat' size='1'>
						<option value="0">ASCII</option>
						<option value="1">16HEX</option>
						</select>
					</td>
				</tr>
				<tr>
					<td >当前网络密钥索引号:</td>
					<td><select name='vap1_KeyIndex' size='1'></select></td>
				</tr>
				<tr>
					<td>网络密钥 1:</td>
					<td><input type='text' name='vap1_Keys' size='30' maxlength=26></td>
				</tr>
				<tr>
                     	<td>网络密钥 2:</td>
					<td><input type='text' name='vap1_Keys' size='30' maxlength=26></td>
				</tr>
				<tr>
					<td>网络密钥 3:</td>
					<td><input type='text' name='vap1_Keys' size='30' maxlength=26></td>
				</tr>
				<tr>
					<td>网络密钥 4:</td>
					<td><input type='text' name='vap1_Keys' size='30' maxlength=26></td>
				</tr>
				<tr> 
					<td>&nbsp;</td>
					<td>64位密钥要求输入5个ASCII字符或10个十六进制数字</td>
				</tr>
				<tr> 
					<td>&nbsp;</td>
					<td>128位密钥要求输入13个ASCII字符或26个十六进制数字</td>
				</tr>
			</table>
		</div>

		<br><br>
		
 		<table border="0" cellpadding="4" cellspacing="0">
            	<tr>
				<td bgColor=#aaddff>虚拟SSID2</td>
				<td bgColor=#aaddff  width='380'><input type='checkbox' name='vap2_enable' onClick='vap2_CbClick(this)' value="ON" >启用</td>         
			</tr>
		</table>
  		<div id='vap2_SecInfo'>
		<br>
			<table border="0" cellpadding="0" cellspacing="0">
				<tr>
					<td valign="middle" align="left" width="30" height="30"><input type='checkbox' name='vap2_hide' value="ON"></td>
					<td>取消广播</td>
				</tr>
			</table>
			<br>
			<table border="0" cellpadding="0" cellspacing="0">
				<tr>
					<td width="150">SSID:</td>
					<td><input type='text' name='vap2_ssid' maxlength="32" size="32"></td>
				</tr>
			</table>
		</div>
  		<div id='vap2_Encrypt'>
		<br>
			<table border="0" cellpadding="0" cellspacing="0">
				<tr>
					<td width='150'>网络认证方式:</td>
					<td>
						<select name='vap2_AuthMode' size="1" onChange='vap2_authModeChange(0)'>
						<option value="open">开放</option>
						<option value="shared">共享</option>
						<option value="psk">WPA-PSK</option>
						<option value="psk2">WPA2-PSK</option>
						</select>
					</td>
				</tr>
			</table>
		</div>
		<div id="vap2_WpaD">
			<br>
			<table border="0" cellpadding="0" cellspacing="0">
				<tr>
					<td width='150'>WPA密钥格式:</td>
					<td><select name='vap2_PskFormat' size='1'>
						<option value="0">passphrase</option>
						<option value="1">64位16进制码</option>
						</select>
					</td>
				</tr>
				<tr>
					<td width='150'>WPA 预共享密钥:</td>
					<td><input type='text' name='vap2_WpaPsk' size='20' maxlength='64'></td>
				</tr>
<!--
				<tr>
					<td width='150'>WPA 更新会话密钥间隔:</td>
					<td><input type='text' name='vap2_wpaInter' size='20' maxlength='10'></td>
				</tr>
-->
				<tr>
					<td width='150'>WPA 加密:</td>
					<td><select name="vap2_wlWpa" onChange='vap2_encryptChange()'></select></td>
				</tr>
			</table>
		</div>
		<div id="vap2_WepD">
			<br>
			<table border="0" cellpadding="0" cellspacing="0">
				<tr>
					<td width='150'>WEP 加密:</td>
					<td><select name="vap2_wlWep" onChange='vap2_encryptChange()'></select></td>
				</tr>
			</table>
		</div>
		<div id='vap2_keyInfo'>
			<table border="0" cellpadding="0" cellspacing="0">
				<tr>
					<td width='150'>密钥长度:</td>
					<td><select name='vap2_KeyBit' size='1'>
						<option value="0">64-bit</option>
						<option value="1">128-bit</option>
						</select>
					</td>
				</tr>
				<tr>
					<td width='150'>密钥格式:</td>
					<td><select name='vap2_KeyFormat' size='1'>
						<option value="0">ASCII</option>
						<option value="1">16HEX</option>
						</select>
					</td>
				</tr>
				<tr>
					<td >当前网络密钥索引号:</td>
					<td><select name='vap2_KeyIndex' size='1'></select></td>
				</tr>
				<tr>
					<td>网络密钥 1:</td>
					<td><input type='text' name='vap2_Keys' size='30' maxlength=26></td>
				</tr>
				<tr>
                     	<td>网络密钥 2:</td>
					<td><input type='text' name='vap2_Keys' size='30' maxlength=26></td>
				</tr>
				<tr>
					<td>网络密钥 3:</td>
					<td><input type='text' name='vap2_Keys' size='30' maxlength=26></td>
				</tr>
				<tr>
					<td>网络密钥 4:</td>
					<td><input type='text' name='vap2_Keys' size='30' maxlength=26></td>
				</tr>
				<tr> 
					<td>&nbsp;</td>
					<td>64位密钥要求输入5个ASCII字符或10个十六进制数字</td>
				</tr>
				<tr> 
					<td>&nbsp;</td>
					<td>128位密钥要求输入13个ASCII字符或26个十六进制数字</td>
				</tr>
			</table>
		</div>

		<br><br>
		
 		<table border="0" cellpadding="4" cellspacing="0">
            	<tr>
				<td bgColor=#aaddff>虚拟SSID3</td>
				<td bgColor=#aaddff  width='380'><input type='checkbox' name='vap3_enable' onClick='vap3_CbClick(this)' value="ON" >启用</td>         
			</tr>
		</table>
  		<div id='vap3_SecInfo'>
		<br>
			<table border="0" cellpadding="0" cellspacing="0">
				<tr>
					<td valign="middle" align="left" width="30" height="30"><input type='checkbox' name='vap3_hide' value="ON"></td>
					<td>取消广播</td>
				</tr>
			</table>
			<br>
			<table border="0" cellpadding="0" cellspacing="0">
				<tr>
					<td width="150">SSID:</td>
					<td><input type='text' name='vap3_ssid' maxlength="32" size="32"></td>
				</tr>
			</table>
		</div>
  		<div id='vap3_Encrypt'>
		<br>
			<table border="0" cellpadding="0" cellspacing="0">
				<tr>
					<td width='150'>网络认证方式:</td>
					<td>
						<select name='vap3_AuthMode' size="1" onChange='vap3_authModeChange(0)'>
						<option value="open">开放</option>
						<option value="shared">共享</option>
						<option value="psk">WPA-PSK</option>
						<option value="psk2">WPA2-PSK</option>
						</select>
					</td>
				</tr>
			</table>
		</div>
		<div id="vap3_WpaD">
			<br>
			<table border="0" cellpadding="0" cellspacing="0">
				<tr>
					<td width='150'>WPA密钥格式:</td>
					<td><select name='vap3_PskFormat' size='1'>
						<option value="0">passphrase</option>
						<option value="1">64位16进制码</option>
						</select>
					</td>
				</tr>
				<tr>
					<td width='150'>WPA 预共享密钥:</td>
					<td><input type='text' name='vap3_WpaPsk' size='20' maxlength='64'></td>
				</tr>
<!--
				<tr>
					<td width='150'>WPA 更新会话密钥间隔:</td>
					<td><input type='text' name='vap3_wpaInter' size='20' maxlength='10'></td>
				</tr>
-->
				<tr>
					<td width='150'>WPA 加密:</td>
					<td><select name="vap3_wlWpa" onChange='vap3_encryptChange()'></select></td>
				</tr>
			</table>
		</div>
		<div id="vap3_WepD">
			<br>
			<table border="0" cellpadding="0" cellspacing="0">
				<tr>
					<td width='150'>WEP 加密:</td>
					<td><select name="vap3_wlWep" onChange='vap3_encryptChange()'></select></td>
				</tr>
			</table>
		</div>
		<div id='vap3_keyInfo'>
			<table border="0" cellpadding="0" cellspacing="0">
				<tr>
					<td width='150'>密钥长度:</td>
					<td><select name='vap3_KeyBit' size='1'>
						<option value="0">64-bit</option>
						<option value="1">128-bit</option>
						</select>
					</td>
				</tr>
				<tr>
					<td width='150'>密钥格式:</td>
					<td><select name='vap3_KeyFormat' size='1'>
						<option value="0">ASCII</option>
						<option value="1">16HEX</option>
						</select>
					</td>
				</tr>
				<tr>
					<td >当前网络密钥索引号:</td>
					<td><select name='vap3_KeyIndex' size='1'></select></td>
				</tr>
				<tr>
					<td>网络密钥 1:</td>
					<td><input type='text' name='vap3_Keys' size='30' maxlength=26></td>
				</tr>
				<tr>
                     	<td>网络密钥 2:</td>
					<td><input type='text' name='vap3_Keys' size='30' maxlength=26></td>
				</tr>
				<tr>
					<td>网络密钥 3:</td>
					<td><input type='text' name='vap3_Keys' size='30' maxlength=26></td>
				</tr>
				<tr>
					<td>网络密钥 4:</td>
					<td><input type='text' name='vap3_Keys' size='30' maxlength=26></td>
				</tr>
				<tr> 
					<td>&nbsp;</td>
					<td>64位密钥要求输入5个ASCII字符或10个十六进制数字</td>
				</tr>
				<tr> 
					<td>&nbsp;</td>
					<td>128位密钥要求输入13个ASCII字符或26个十六进制数字</td>
				</tr>
			</table>
		</div>

		<br><br>
		<br><br>
		
		<table border="0" cellpadding="0" cellspacing="0">
			<tr>
				<td  width="150"><input type="hidden" value="/mbssid.asp" name="submit-url"></td>  
				<td><input type='button' onClick='on_submit()' value='保存/应用'></td>
			</tr> 
		</table>

		<table border="0" cellpadding="0" cellspacing="0">
			<tr>
				<td><input type='hidden' name="vap0_lst" ></td>
				<td><input type='hidden' name="vap1_lst" ></td>
				<td><input type='hidden' name="vap2_lst" ></td>
				<td><input type='hidden' name="vap3_lst" ></td>
			</tr>
		</table>
		
		<br><br>

	    </form>
      </blockquote>
   </body>
<%addHttpNoCache();%>
</html>


