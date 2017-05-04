<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>中国电信-WLAN配置</TITLE>
<META http-equiv=pragma content=no-cache>
<META http-equiv=cache-control content="no-cache, must-revalidate">
<META http-equiv=content-type content="text/html; charset=gbk">
<META http-equiv=content-script-type content=text/javascript>
<!--系统公共css-->
<style>
BODY{font-family:"华文宋体";}
TR{height:16px;}
SELECT {width:120px;}
</style>
<!--系统公共脚本-->
<script language="javascript" src="/common.js"></script>
<SCRIPT language="javascript" type="text/javascript">
document.onmousedown = function(e)
{
	var tar = null;
	if (!e)e = window.event;
	if(e.button != 1)return true;
	if (e.target) tar = e.target;
	else if (e.srcElement) tar = e.srcElement;
	if(tar.tagName == "INPUT" || tar.tagName == "IMG" || tar.tagName == "SELECT") return true;
	document.location.href = "app:mouse:onmousedown";
}


//var ssidIdx = "0";//SSID索引号
var ssid = "China_kkk";
var wpa="tkip+aes";//用于判断后表列回显
var mode = "open";//网络认证方式
var bcntpsubcriber = "1";//网络认证方式开关：0表示不可编辑，1表示可编辑
var wlssidlist="China_kkk";//列表显示
var wep = "enabled";//sep加密分别为：enabled，disabled
var wlCorerev = "3";//WPA加密:大于等于3时,WPA 加密列表值分别为："TKIP", "AES", "TKIP+AES"；否则WPA 加密列表值"TKIP"
var bit = "0";//密钥长度:WEP128=0;WEP64=1
var wpapskValue = "00005544";//WPA预共享密钥
var wpapskUpInter = "55";//WPA更新会话密钥间隔
var keyIdx = "1";//网络密钥索引号分别对应：1,2,3,4
var keys = new Array( "key1", "key2","key3", "key4" ); //网络密钥值列表

<%init_wlanAdv_page();%>

/********************************************************************
**          on document load
********************************************************************/
function on_init() 
{
	with ( document.forms[0] )
	{
		//setSelect(wlSsidIdx, ssidIdx);
		ssideditor.value = ssid;
		setSelect(wlAuthMode, mode);
		wlWpaPsk.value = wpapskValue;
		wlwpapskUpInter.value = wpapskUpInter;
//		wlRadiusIPAddr.value = radiusServerIP;
//		wlRadiusPort.value = radiusPort;
//		wlRadiusKey.value = radiusKey;
		wlKeyBit.selectedIndex = parseInt(bit);
//		wlPreauth.selectedIndex = parseInt(varPreauth);
//		wlNetReauth.value = varNetReauth;
		for ( i = 0; i < 4; i++ ) 
		{
			wlKeys[i].value = keys[i];
		}
//		paWscDevPin.value = WscDevPin;
//		paWscStaPin.value = WscStaPin;

		authModeChange(1);

		if(bcntpsubcriber != '1')
			wlAuthMode.disabled = 1;  
	}   
}
function pin_window(pin)
{
	with(document.forms[0])
	{
		var w = window.open("", "", "toolbar=no,width=500,height=100");
		w.document.write('<font size=5><b><center>' + pin + '</center></b></font>');
		w.document.close();
	}
}

function help_window()
{
   with(document.forms[0])
   {
      var w = window.open("", "", "toolbar=no,width=500,height=100");
      w.document.write('If left blank, a new PIN will be generated when you click Configure.<br>');
      w.document.write('Use the link next to PIN textbox to show the generated PIN.');
      w.document.close();
    }
}

function wpapsk_window() 
{
	var psk_window = window.open("", "", "toolbar=no,width=500,height=100");
	psk_window.document.write(" WPA 的预共享密钥是 " + wpapskValue);
	psk_window.document.close();
}

/********************************************************************
**          on document update
********************************************************************/

function authModeChange(OnPageLoad) 
{
	with ( document.forms[0] ) 
	{
		var authMode = getSelect(wlAuthMode);
		var i, algos;
		var disableWepD = false;
		hideAllElements();
		switch ( authMode ) 
		{

			case 'open':	 
			case 'shared':
			showhide("wlWepD", 1);
			showhide("keyInfo", 1);
			break;

			case 'wpa':
			showhide("wlWpaD", 1);
	//		showhide("wlRadius", 1);
			showhide("wpapskUpInter", 1);
			disableWepD = true;
			break;      

			case 'psk2':      
			case 'psk2mix': 
			case 'psk':  
			showhide("wlWpaD", 1);
			showhide("wpapskUpInter", 1);
			showhide("wpaPreShareKey", 1);
			disableWepD = true;
			break;      	       
		}   

//		raApMethod[0].checked = true;
//		showhide("divWscDevPin", 0);

		if(OnPageLoad==1)
		{  

			if (wlCorerev >= 3)
			algos = new Array("TKIP", "AES", "TKIP+AES");
			else
			algos = new Array("TKIP");                  

			wlWpa.length = algos.length;

			for (var i in algos) 
			{          	

				wlWpa[i] = new Option(algos[i], algos[i].toLowerCase());
				wlWpa[i].value = algos[i].toLowerCase();

				if (algos[i].toLowerCase() == wpa)
				wlWpa[i].selected = true;

			}  
		}      

		if(OnPageLoad==0) 
		{
			if(authMode == "wpa" || authMode == "psk")
			wlWpa.selectedIndex=0;
			if(authMode == "wpa2" || authMode == "psk2")
			wlWpa.selectedIndex=1;  	
			if(authMode == "wpa2mix" || authMode == "psk2mix")  
			wlWpa.selectedIndex=2;    

			wpa=wlWpa[wlWpa.selectedIndex].value;  

		}  

		wlWep.length = 0;
		if (authMode == "shared" || authMode == "radius")  // shared and radius must have wep on
		{ 
			wlWep[0] = new Option("启用", "enabled");
			wlWep[0].selected = true;
		}
		else 
		{
			wlWep[0] = new Option("禁用", "disabled");
			wlWep[1] = new Option("启用", "enabled");
			if ((authMode.indexOf("wpa")!= -1 || authMode.indexOf("psk")!= -1) && (mode == "open" || mode == "shared")) // set wep off if switch to wpa modes
			{ 
				wlWep[0].selected = true;
			}
			else if (wep == "enabled")
			{
				wlWep[1].selected = true;
			}
			else 
			{
				wlWep[0].selected = true;
			}
		}
		/* -start: not coexist with WPA-*/
		if(disableWepD)
		{
			setSelect(wlWep, "disabled");
			wlWep.disabled = 1;
		}
		else 
		{
			wlWep.disabled = 0;
		}
		/* -end: not coexist with WPA-*/
	}   
	encrypChange();
//span chars for magic
}

function hideAllElements()
{
	showhide("wlWpaD", 0);
	showhide("wpaPreShareKey", 0);
	showhide("wpapskUpInter", 0);
//	showhide("wlRadius", 0);
//	showhide("Preauth", 0);
//	showhide("NetReauth", 0);
//	showhide("divWscAddClient", 0);
//	showhide("divWscStaPin", 0);
}

function encrypChange()
{
	with ( document.forms[0] ) 
	{
		var cwep = getSelect(wlWep);
		var authMode = getSelect(wlAuthMode)

		if (cwep == "enabled") 
		{
			showhide("keyInfo", 1);
			if (authMode != "open" && authMode != "shared") 
			{
				wlKeys[0].disabled = 1;
				wlKeys[1].disabled = 0;
				wlKeys[2].disabled = 0;
				wlKeys[3].disabled = 1;
				wlKeyIndex.length = 0;
				wlKeyIndex[0] = new Option("2", "2");
				wlKeyIndex[1] = new Option("3", "3");
				if (keyIdx != "2" && keyIdx != "3") 
				{
					keyIdx = "2";
				}
				wlKeyIndex.selectedIndex = parseInt(keyIdx) - 2;
			}
			else 
			{
				wlKeys[0].disabled = 0;
				wlKeys[1].disabled = 0;
				wlKeys[2].disabled = 0;
				wlKeys[3].disabled = 0;
				wlKeyIndex.length = 0;
				wlKeyIndex[0] = new Option("1", "1");
				wlKeyIndex[1] = new Option("2", "2");
				wlKeyIndex[2] = new Option("3", "3");
				wlKeyIndex[3] = new Option("4", "4");
				wlKeyIndex.selectedIndex = parseInt(keyIdx) - 1;
			}
		} 
		else
		{
			showhide("keyInfo", 0);
		}
	}
}  

//cathy
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

function isValidIpAddress(address) 
{
	var i = 0;

	if ( address == '0.0.0.0' ||address == '255.255.255.255' )
		return false;

	addrParts = address.split('.');
	if ( addrParts.length != 4 ) return false;
	for (i = 0; i < 4; i++) 
	{
		if (isNaN(addrParts[i]) || addrParts[i] =="")
			return false;
		num = parseInt(addrParts[i]);
		if ( num < 0 || num > 255 )
			return false;
	}
	return true;
}

//cathy
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

/********************************************************************
**          on document submit
********************************************************************/

function on_submit() 
{
   var swep = getSelect(document.forms[0].wlWep);
   var swpa = getSelect(document.forms[0].wlWpa);

/*   if ( enblWireless == '0' ) 
   {
         alert('因为当前无线被禁用，不能设置.');
         return;
   }*/

   with ( document.forms[0] )
   {
      var authMode = getSelect(wlAuthMode);
      var value;

     if ( ssideditor.value == '' ) 
     {
        alert('SSID不能为空.');
        return;
     }
     var str = new String();
     str = ssideditor.value;
     if ( str.length > 32 ) 
     {
        alert('SSID "' + ssideditor.value + '" 不能大于32个字符。');
        return;
     }
	 var place = str.indexOf("ChinaNet-");
	 if(place!=0)
	 {
	 	alert('SSID "' + ssideditor.value + '" 不是以ChinaNet-开头，请重新输入。');
		return;
	 }
	 if(isIncludeInvalidChar(ssideditor.value))
	 {
		alert("SSID 含有非法字符，请重新输入!");
		return;
     }	
	  
      if (authMode.indexOf("psk")!= -1)
	{
		value = wlWpaPsk.value;
		 
		if ( isValidwpapskValue(value) == false ) 
		{
			alert('WPA预共用的密钥应该在8个和63个ASCII字符或64个十六进制数字之间.');
			return;
		}
        }

      if (authMode.indexOf("wpa")!= -1 || authMode.indexOf("psk")!= -1)
	{ 
		value = parseInt(wlwpapskUpInter.value);
		
		if (  isNaN(value) == true || value < 0 || value > 0xffffffff )
		{
			alert('WPA 群组更新密钥间隔时间 "' + wlwpapskUpInter.value + '" 应该在 0 到 4294967295 之间.');
			return;
		}

/*		value = parseInt(wlNetReauth.value);*/
		
/*		if (  isNaN(value) == true || value < 0 || value > 0xffffffff ) 
		{
			alert('WPA 网络重新验证间隔时间"' + wlNetReauth.value + '" 应该在 0 到 4294967295 之间.');
			return;
		}*/
	}
      
/*	if (authMode.indexOf("wpa")!= -1 || authMode == 'radius') 
	{
	
		if ( isValidIpAddress(wlRadiusIPAddr.value) == false ) 
		{
			alert('RADIUS 服务器 IP 地址 "' + wlRadiusIPAddr.value + '" 是无效 IP 地址.');
			return;
		}
	}*/

	if (getSelect(wlWep) == "enabled") 
	{
		var i, val;
		var cbit = getSelect(wlKeyBit);
		var num = parseInt(getSelect(wlKeyIndex))-1;
		val = wlKeys[num].value;
		if ( val == '' && !(swep == 'enabled' && authMode == 'radius')) 
		{
			alert('不能选择空值密钥.');
			return;
		}
		if ( val != '' ) 
		{
			if ( cbit == '1' ) 
			{
				if ( isValidKey(val, 13) == false ) 
				{
				alert('Key "' + val + '" 无效. 请输入13个ASCII字符或26个十六进制数字为128位WEP密钥.');
				return;
				}
			} 
			else 
			{
				if ( isValidKey(val, 5) == false )
				{
					alert('Key "' + val + '" 无效. 请输入5个ASCII字符或10个十六进制数字为一个64位WEP密钥.');
					return;
				}
			}
	       }

	var tmplst = "";
	for (i=0; i<4; i++)
	{
		val = wlKeys[i].value;
	if ( val != '' ) 
	{
		if ( cbit == '1' ) 
		{
			if ( isValidKey(val, 13) == false ) 
			{
				alert('Key "' + val + '" 无效. 请输入13个ASCII字符或26个十六进制数字为128位WEP密钥.');
				return;
			}
		}
		else 
		{
			if ( isValidKey(val, 5) == false ) 
			{
				alert('Key "' + val + '" 无效. 请输入5个ASCII字符或10个十六进制数字为一个64位WEP密钥.');
				return;
			}
		}
	}
	if(tmplst.length != 0)
		tmplst += ";";
	 tmplst += val;
	} 
	lst.value = tmplst;
      }
   submit();
   }
}

</SCRIPT>
</HEAD>
<!-------------------------------------------------------------------------------------->
<!--主页代码-->
<body topmargin="0" bgcolor="E0E0E0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init()">
		<DIV align="center" >
			<form id="form" action=/boaform/formWlEncrypt method=POST>
			<table width="80%"><tr><td><p align="left"><font size="+2"><b>ADSL无线网络配置</b></font></p></td></tr></table><br>
			<table id="cfg" cellspacing="0" cellpadding="0" width="256" align="center" border="0">
			<tr valign="top">
			<td height="1">
				<table border="0" cellpadding="0" cellspacing="0">
					<tr>
<!--					
					<td> 选择 SSID: </td>
					<td><select name="wlSsidIdx"  disabled>
				               <script language="javascript">
						document.write('<option value="' + wlssidlist + '">' + wlssidlist + '</option>');
				               </script>
		                  	</select></td>
-->	
					<td> SSID: </td>
					<td><input type="text" name="ssideditor" size="30" maxlength=32 style="width:120px "></td>
					</tr>
					<tr> <td>&nbsp;</td></tr>
					<tr>
					<td width="180">网络认证方式:</td>
					<td><select name='wlAuthMode' size="1" onChange='authModeChange(0)'>
			                        <option value="open">开放</option>
			                        <option value="shared">共享</option>
			                        <option value="psk">WPA-PSK</option>
			                        <option value="psk2">WPA2-PSK</option>
			                     </select>
			                  </td>
					</tr>
				</table>
				<br>
				<div id="wpaPreShareKey">
				<table border="0" cellpadding="0" cellspacing="0">
					<tr>
					<td width="180">WPA 预共享密钥:</td>
					<td><input type='password' name='wlWpaPsk' size='20' maxlength='64' style="width:120px"></td>
					</tr>
				</table>
				</div>
				<div id="wpapskUpInter">
				<table border="0" cellpadding="0" cellspacing="0">
					<tr>
					<td width="180">WPA 更新会话密钥间隔:</td>
					<td><input type='text' name='wlwpapskUpInter' size='20' maxlength='10' style="width:120px"></td>
					</tr>
				</table>
				</div>
				<div id="wlWpaD">
				<table border="0" cellpadding="0" cellspacing="0">
		                  <tr>
		                     <td width='180'>WPA 加密:</td>
		                     <td><select name="wlWpa" onChange='encrypChange()'>
		                        </select>
		                     </td>
		                  </tr>
		               </table>
		            </div>
		            <div id="wlWepD">
		            <table border="0" cellpadding="0" cellspacing="0">
		                  <tr>
		                     <td width='180'>WEP 加密:</td>
		                     <td><select name="wlWep" onChange='encrypChange()'>
		                        </select>
		                     </td>
		                  </tr>
		               </table>
		            </div>								
				<div id='keyInfo'>
				<table border="0" cellpadding="0" cellspacing="0">
					<tr>
					<td width="180">密钥长度:</td>
		                     <td ><select name='wlKeyBit' size='1'>
		                           <option value="0">64-bit</option>
		                           <option value="1">128-bit</option>
		                        </select></td>
					</tr>
					<tr>
					<td width="180">密钥索引号:</td>
					<td><select name="wlKeyIndex" size="1">
					<option value="0">1</option>
					<option value="1">2</option>
					<option value="2">3</option>
					<option value="3">4</option>
					</select></td>
					</tr>
					<tr>
					<td width="180">网络密钥 1:</td>
					<td><input type="text" name="wlKeys" size="30" maxlength=26 style="width:120px "></td>
					</tr>
					<tr>
					<td width="180">网络密钥 2:</td>
					<td><input type="text" name="wlKeys" size="30" maxlength=26 style="width:120px "></td>
					</tr>
					<tr>
					<td width="180">网络密钥 3:</td>
					<td><input type="text" name="wlKeys" size="30" maxlength=26 style="width:120px "></td>
					</tr>
					<tr>
					<td width="180">网络密钥 4:</td>
					<td><input type="text" name="wlKeys" size="30" maxlength=26 style="width:120px "></td>
					</tr>
				</table>
				</div>
				<br>
				<div align="center">
				<table border="0" cellpadding="0" cellspacing="0" width="80%">
					<tr nowrap>
					<td align="center"><input type="button" value="后退" onClick="window.location.href='/autorun/e8cfg.asp';" style="width:80px; border-style:groove; font-weight:bold "></td>&nbsp;&nbsp;
					<td align="center"><input type="button" value="保存配置" onClick="on_submit();" style="width:80px; border-style:groove; font-weight:bold "></td>
					</tr>
				</table>
				</div>
				<input type="hidden" name="lst" >
				<input type="hidden" name="submit-url" value="/autorun/wlancfg.asp">
				<input type="hidden" name="showOkPage" value="yes">
				</td></tr>
			</table>
			</form>
		</DIV>
	</td></tr>
</body>
<%addHttpNoCache();%>
</html>
