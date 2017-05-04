

function disableTextField (field) {
	if (document.all || document.getElementById)
		field.disabled = true;
	else {
		field.oldOnFocus = field.onfocus;
		field.onfocus = skip;
	}
}
function enableTextField (field) {
	if (document.all || document.getElementById)
		field.disabled = false;
	else {
		field.onfocus = field.oldOnFocus;
	}
}

function disableRadioGroup (radioArrOrButton)
{
  if (radioArrOrButton.type && radioArrOrButton.type == "radio") {
        var radioButton = radioArrOrButton;
        var radioArray = radioButton.form[radioButton.name];
  }
  else
        var radioArray = radioArrOrButton;
        radioArray.disabled = true;
        for (var b = 0; b < radioArray.length; b++) {
        if (radioArray[b].checked) {
                radioArray.checkedElement = radioArray[b];
                break;
        }
  }
  for (var b = 0; b < radioArray.length; b++) {
        radioArray[b].disabled = true;
        radioArray[b].checkedElement = radioArray.checkedElement;
  }
}

function enableRadioGroup (radioArrOrButton)
{
  if (radioArrOrButton.type && radioArrOrButton.type == "radio") {
        var radioButton = radioArrOrButton;
        var radioArray = radioButton.form[radioButton.name];
  }
  else
        var radioArray = radioArrOrButton;

  radioArray.disabled = false;
  radioArray.checkedElement = null;
  for (var b = 0; b < radioArray.length; b++) {
        radioArray[b].disabled = false;
        radioArray[b].checkedElement = null;
  }
}

function disableCheckBox (checkBox) {
  if (!checkBox.disabled) {
    checkBox.disabled = true;
    if (!document.all && !document.getElementById) {
      checkBox.storeChecked = checkBox.checked;
      checkBox.oldOnClick = checkBox.onclick;
      checkBox.onclick = preserve;
    }
  }
}

function enableCheckBox (checkBox)
{
  if (checkBox.disabled) {
    checkBox.disabled = false;
    if (!document.all && !document.getElementById)
      checkBox.onclick = checkBox.oldOnClick;
  }
}


function check_wps_enc(enc, radius, auth)
{
        if (enc == 0 || enc == 1) {
                if (radius != 0)
                        return 2;
        }
        else {
                if (auth & 1)
                        return 2;
        }
        return 0;
}

function check_wps_wlanmode(mo, type)
{
        if (mo == 2) {
                return 1;
        }
        if (mo == 1 && type != 0) {
                return 1;
        }
        return 0;
}


function getDigit(str, num)
{
  i=1;
  if ( num != 1 ) {
  	while (i!=num && str.length!=0) {
		if ( str.charAt(0) == '.' ) {
			i++;
		}
		str = str.substring(1);
  	}
  	if ( i!=num )
  		return -1;
  }
  for (i=0; i<str.length; i++) {
  	if ( str.charAt(i) == '.' ) {
		str = str.substring(0, i);
		break;
	}
  }
  if ( str.length == 0)
  	return -1;
  d = parseInt(str, 10);
  return d;
}

function getDigitforMac(str, num)
{
  i=1;
  if ( num != 1 ) {
  	while (i!=num && str.length!=0) {
		if ( str.charAt(0) == '-' ) {
			i++;
		}
		str = str.substring(1);
  	}
  	if ( i!=num )
  		return -1;
  }
  for (i=0; i<str.length; i++) {
  	if ( str.charAt(i) == '-' ) {
		str = str.substring(0, i);
		break;
	}
  }
  if ( str.length == 0)
  	return -1;
  d = parseInt(str, 10);
  return d;
}

function validateKey(str)
{
   for (var i=0; i<str.length; i++) {
    if ( (str.charAt(i) >= '0' && str.charAt(i) <= '9') ||
    		(str.charAt(i) == '.' ) )
			continue;
	return 0;
  }
  return 1;
}

function validateKey2(str)
{
   for (var i=0; i<str.length; i++) {
    if ( (str.charAt(i) >= '0' && str.charAt(i) <= '9') ||
    		(str.charAt(i) == '-' ) || 
    		(str.charAt(i) >= 'A' && str.charAt(i) <= 'F')||
    		(str.charAt(i) >= 'a' && str.charAt(i) <= 'f') )
			continue;
	return 0;
  }
  return 1;
}

function IsInvalidIP(str)
{
//	if ((str == "127.100.100.100") || (str == "127.255.255.254"))
	d = getDigit(str, 1);
	if (d == 127)
		return 1;
	return 0;
}

function IsLoopBackIP(str)
{
	if(str=="127.0.0.1")
		return 1;
	return 0;
}

function checkDigit(str)
{
	for (var i=0; i<str.length; i++) {
		if ((str.charAt(i) >= '0' && str.charAt(i) <= '9'))
			continue;
		return 0;
	}
	return 1;
}

function checkDigitRange(str, num, min, max)
{
  d = getDigit(str,num);
  if ( d > max || d < min )
      	return false;
  return true;
}

function checkDigitRangeforMac(str, num, min, max)
{  
  d = getDigitforMac(str,num);
  if ( d > max || d < min )
      	return false;
  return true;
}

function checkLan1andLan2(ip1, mask1, ip2, mask2)
{
	d11 = getDigit(ip1.value,1);
	d12 = getDigit(mask1.value, 1);
	d21 = getDigit(ip2.value,1);
	d22 = getDigit(mask2.value,1);
	d1 = d11 & d12;
	d2 = d21 & d22;
	if (d1 != d2)
		return true;
	
	d11 = getDigit(ip1.value,2);
	d12 = getDigit(mask1.value, 2);
	d21 = getDigit(ip2.value,2);
	d22 = getDigit(mask2.value,2);
	d1 = d11 & d12;
	d2 = d21 & d22;
	if (d1 != d2)
		return true;

	d11 = getDigit(ip1.value,3);
	d12 = getDigit(mask1.value, 3);
	d21 = getDigit(ip2.value,3);
	d22 = getDigit(mask2.value,3);
	d1 = d11 & d12;
	d2 = d21 & d22;
	if (d1 != d2)
		return true;

	d11 = getDigit(ip1.value,4);
	d12 = getDigit(mask1.value, 4);
	d21 = getDigit(ip2.value,4);
	d22 = getDigit(mask2.value,4);
	d1 = d11 & d12;
	d2 = d21 & d22;
	if (d1 != d2)
		return true;

	return false;
}

function checkIP(ip)
{
	if (ip.value=="") {
		alert("IP address cannot be empty! It should be filled with 4 digit numbers as xxx.xxx.xxx.xxx.");
		ip.value = ip.defaultValue;
		ip.focus();
		return false;
	}
	if ( validateKey( ip.value ) == 0 ) {
		alert("Invalid IP address value. It should be the decimal number (0-9).");
		ip.value = ip.defaultValue;
		ip.focus();
		return false;
	}
	
	if( IsLoopBackIP( ip.value)==1 ) {
		alert("Invalid IP address value.");
		ip.value = ip.defaultValue;	// Jenny,  Buglist B058, backward default value
		ip.focus();
		return false;
	}
	
	if ( !checkDigitRange(ip.value,1,0,255) ) {
		alert('Invalid IP address range in 1st digit. It should be 0-255.');
		ip.value = ip.defaultValue;
		ip.focus();
		return false;
	}
	if ( !checkDigitRange(ip.value,2,0,255) ) {
		alert('Invalid IP address range in 2nd digit. It should be 0-255.');
		ip.value = ip.defaultValue;
		ip.focus();
		return false;
	}
	if ( !checkDigitRange(ip.value,3,0,255) ) {
		alert('Invalid IP address range in 3rd digit. It should be 0-255.');
		ip.value = ip.defaultValue;
		ip.focus();
		return false;
	}
	if ( !checkDigitRange(ip.value,4,1,254) ) {
		alert('Invalid IP address range in 4th digit. It should be 1-254.');
		ip.value = ip.defaultValue;
		ip.focus();
		return false;
	}
	
	return true;
}

function checkHostIP(ip, checkEmpty)
{
	if (checkEmpty == 1 && ip.value=="") {
		alert("IP地址不能是空的! 格式为 xxx.xxx.xxx.xxx.");
		ip.value = ip.defaultValue;
		ip.focus();
		return false;
	}
	if (validateKey(ip.value) == 0) {
		alert("IP地址无效. 必须是(0-9)的数字.");
		ip.value = ip.defaultValue;
		ip.focus();
		return false;
	}
	
	if (IsLoopBackIP(ip.value)==1 || IsInvalidIP(ip.value)==1) {
		alert("IP地址无效.");
		ip.value = ip.defaultValue;
		ip.focus();
		return false;
	}
	
	if (!checkDigitRange(ip.value, 1, 1, 223)) {
		alert('IP地址的第一个位元无效. 必须是 1-223的数值.');
		ip.value = ip.defaultValue;
		ip.focus();
		return false;
	}
	if (!checkDigitRange(ip.value, 2, 0, 255)) {
		alert('IP地址的第二个位元无效. I必须是 0-255的数值.');
		ip.value = ip.defaultValue;
		ip.focus();
		return false;
	}
	if (!checkDigitRange(ip.value, 3, 0, 255)) {
		alert('IP地址的第三个位元无效. 必须是 0-255的数值.');
		ip.value = ip.defaultValue;
		ip.focus();
		return false;
	}
	if (!checkDigitRange(ip.value, 4, 0, 254)) {
		alert('IP地址的第四个位元无效. 必须是 0-254的数值.');
		ip.value = ip.defaultValue;
		ip.focus();
		return false;
	}
	
	return true;
}

function checkNetIP(ip, checkEmpty)
{
	if (checkEmpty == 1 && ip.value=="") {
		alert("IP address cannot be empty! It should be filled with 4 digit numbers as xxx.xxx.xxx.xxx.");
		ip.value = ip.defaultValue;
		ip.focus();
		return false;
	}
	if (validateKey(ip.value) == 0) {
		alert("Invalid IP address value. It should be the decimal number (0-9).");
		ip.value = ip.defaultValue;
		ip.focus();
		return false;
	}
	
	if (IsLoopBackIP(ip.value)==1 || IsInvalidIP(ip.value)==1) {
		alert("Invalid IP address value.");
		ip.value = ip.defaultValue;
		ip.focus();
		return false;
	}
	
	if (!checkDigitRange(ip.value, 1, 0, 255)) {
		alert('Invalid IP address range in 1st digit. It should be 0-255.');
		ip.value = ip.defaultValue;
		ip.focus();
		return false;
	}
	if (!checkDigitRange(ip.value, 2, 0, 255)) {
		alert('Invalid IP address range in 2nd digit. It should be 0-255.');
		ip.value = ip.defaultValue;
		ip.focus();
		return false;
	}
	if (!checkDigitRange(ip.value, 3, 0, 255)) {
		alert('Invalid IP address range in 3rd digit. It should be 0-255.');
		ip.value = ip.defaultValue;
		ip.focus();
		return false;
	}
	if (!checkDigitRange(ip.value, 4, 0, 255)) {
		alert('Invalid IP address range in 4th digit. It should be 0-255.');
		ip.value = ip.defaultValue;
		ip.focus();
		return false;
	}
	
	return true;
}

function checkNetmask(netmask, checkEmpty)
{
	var i, d;

	if (checkEmpty == 1 && netmask.value == "") {
		alert("Subnet mask cannot be empty! It should be filled with 4 digit numbers as xxx.xxx.xxx.xxx.");
		netmask.value = netmask.defaultValue;
		netmask.focus();
		return false;
	}

	if (validateKey(netmask.value) == 0) {
		alert("Invalid subnet mask value. It should be the decimal number (0-9).");
		netmask.value = netmask.defaultValue;
		netmask.focus();
		return false;
	}

	for (i=1; i<=4; i++) {
		d = getDigit(netmask.value, i);
		if( !(d==0 || d==128 || d==192 || d==224 || d==240 || d==248 || d==252 || d==254 || d==255 )) {
			alert('Invalid subnet mask digit.\nIt should be the number of 0, 128, 192, 224, 240, 248, 252 or 254');
			netmask.focus();
			return false;
		}
	}

	return true;
}

function checkMask(netmask)
{
	var i, d;

	if (netmask.value == "") {
		alert("Subnet mask cannot be empty! It should be filled with 4 digit numbers as xxx.xxx.xxx.xxx.");
		netmask.value = netmask.defaultValue;
		netmask.focus();
		return false;
	}

	if (validateKey(netmask.value) == 0) {
		alert("Invalid subnet mask value. It should be the decimal number (0-9).");
		netmask.value = netmask.defaultValue;
		netmask.focus();
		return false;
	}

	for (i=1; i<=4; i++) {
		d = getDigit(netmask.value, i);
		if (!(d==0 || d==128 || d==192 || d==224 || d==240 || d==248 || d==252 || d==254 || d==255)) {
			alert('Invalid subnet mask digit.\nIt should be the number of 0, 128, 192, 224, 240, 248, 252 or 254');
			netmask.focus();
			return false;
		}
	}

	return true;
}

function checkMac(macAddr, checkEmpty)
{
	var i, macdigit = 0;

	if (checkEmpty == 1 && macAddr.value.length == 0) {
		alert("MAC address cannot be empty");
		return false;
	}

	if (macAddr.value.length > 0 && macAddr.value.length < 12) {
		alert("Input MAC address is not complete. It should be 12 digits in hex.");
		macAddr.focus();
		return false;
	}

	if (macAddr.value.length == 0)
		macdigit = -1;
	for (i=0; i<macAddr.value.length; i++) {
		if ((macAddr.value.charAt(i) == 'f') || (macAddr.value.charAt(i) == 'F'))
			macdigit ++;
		else
			continue;
	}

	if (macdigit == macAddr.value.length || macAddr.value == "000000000000") {
		alert("Invalid MAC address.");
		macAddr.focus();
		return false;
	}

	for (i=0; i<macAddr.value.length; i++) {
		if ((macAddr.value.charAt(i) >= '0' && macAddr.value.charAt(i) <= '9') ||
			(macAddr.value.charAt(i) >= 'a' && macAddr.value.charAt(i) <= 'f') ||
			(macAddr.value.charAt(i) >= 'A' && macAddr.value.charAt(i) <= 'F') )
			continue;
		alert("Invalid MAC address. It should be in hex number (0-9 or a-f).");
		macAddr.focus();
		return false;
	}
	return true;
}

function includeSpace(str)
{
  for (var i=0; i<str.length; i++) {
  	if ( str.charAt(i) == ' ' ) {
	  return true;
	}
  }
  return false;
}

function checkPrintableString(str)
{
	for (var i=0; i<str.length; i++) {
		//if ((str.charAt(i) >= '0' && str.charAt(i) <= '9') || (str.charAt(i) >= 'A' && str.charAt(i) <= 'Z') || (str.charAt(i) >= 'a' && str.charAt(i) <= 'z') ||
		//   (str.charAt(i) == '.') || (str.charAt(i) == ':') || (str.charAt(i) == '-') || (str.charAt(i) == '_') || (str.charAt(i) == ' ') || (str.charAt(i) == '/') || (str.charAt(i) == '@'))
		if((str.charAt(i) >= ' ') && (str.charAt(i) <= '~'))
			continue;
		return 0;
	}
	return 1;
}

function checkString(str)
{
	for (var i=0; i<str.length; i++) {
		if ((str.charAt(i) >= '0' && str.charAt(i) <= '9') || (str.charAt(i) >= 'A' && str.charAt(i) <= 'Z') || (str.charAt(i) >= 'a' && str.charAt(i) <= 'z') ||
		   (str.charAt(i) == '.') || (str.charAt(i) == ':') || (str.charAt(i) == '-') || (str.charAt(i) == '_') || (str.charAt(i) == ' ') || (str.charAt(i) == '/') || (str.charAt(i) == '@'))
			continue;
		return 0;
	}
	return 1;
}

function deleteClick()
{
	if ( !confirm('删除这项设定?') ) {
		return false;
	}
	else
		return true;
}
        
function deleteAllClick()
{
	if ( !confirm('删除本表所有设定?') ) {
		return false;
	}
	else
		return true;
}

function delClick(index)
{
	if ( !confirm('Are you sure you want to delete?') ) {
		return false;
	}
	
	document.actionForm.action.value=0;
	document.actionForm.idx.value=index;
	document.actionForm.submit();
	return true;
}

function editClick(index)
{
	document.actionForm.action.value=1;
	document.actionForm.idx.value=index;
	document.actionForm.submit();
	return true;
}

function verifyBrowser() {
	var ms = navigator.appVersion.indexOf("MSIE");
	ie4 = (ms>0) && (parseInt(navigator.appVersion.substring(ms+5, ms+6)) >= 4);
	var ns = navigator.appName.indexOf("Netscape");
	ns= (ns>=0) && (parseInt(navigator.appVersion.substring(0,1))>=4);
	if (ie4)
		return "ie4";
	else
		if(ns)
			return "ns";
		else
			return false;
}

function isBrowser(b,v) {
	browserOk = false;
	versionOk = false;
	
	browserOk = (navigator.appName.indexOf(b) != -1);
	if (v == 0) versionOk = true;
	else  versionOk = (v <= parseInt(navigator.appVersion));
	return browserOk && versionOk;
}

function disableButton (button) {
  if (document.all || document.getElementById)
    button.disabled = true;
  else if (button) {
    button.oldOnClick = button.onclick;
    button.onclick = null;
    button.oldValue = button.value;
    button.value = 'DISABLED';
  }
}

function disableButtonIB (button) {
	if (isBrowser('Netscape', 0))
		return;
	if (document.all || document.getElementById)
		button.disabled = true;
	else if (button) {
		button.oldOnClick = button.onclick;
		button.onclick = null;
		button.oldValue = button.value;
		button.value = 'DISABLED';
	}
}

function disableButtonVB (button) {
  if (verifyBrowser() == "ns")
  	return;
  if (document.all || document.getElementById)
    button.disabled = true;
  else if (button) {
    button.oldOnClick = button.onclick;
    button.onclick = null;
    button.oldValue = button.value;
    button.value = 'DISABLED';
  }
}

function enableButton (button) {
  if (document.all || document.getElementById)
    button.disabled = false;
  else if (button) {
    button.onclick = button.oldOnClick;
    button.value = button.oldValue;
  }
}

function enableButtonVB (button) {
  if (verifyBrowser() == "ns")
  	return;
  if (document.all || document.getElementById)
    button.disabled = false;
  else if (button) {
    button.onclick = button.oldOnClick;
    button.value = button.oldValue;
  }
}

function enableButtonIB (button) {
	if (isBrowser('Netscape', 4))
		return;
	if (document.all || document.getElementById)
		button.disabled = false;
	else if (button) {
		button.onclick = button.oldOnClick;
		button.value = button.oldValue;
	}
}

//Add is invalid message
//Function Name: alertInvalid(fieldname, fieldvalue [,additional])
//Description: alerts invalid message containing fieldname and value
//Parameters: fieldname, fieldvalue, additional - Any additional comments to be added
//Output: MessageBox(invalid message)
function alertInvalid(fieldname, fieldvalue, additional)
{
	if (additional == undefined)
		alert (fieldname + " " + fieldvalue + " is invalid.");
	else
		alert (fieldname + " " + fieldvalue + " is invalid, " + additional + ".");
}

//Function Name: isValidIpAddress(address[,fieldname][,type])
//Description: Check that address entered is valid ip address
//Parameters: address, 	fieldname(optional): entering will show error message when encountered
//			  type: TYPE_NETWORK_ADDRESS:check network address | TYPE_IP_ADDRESS (default) check of type IP address
//output: true:no error		false:has error
function isValidIpAddress(address,fieldname,type) {
	var i = 0;
	var c = '';
	var hasfield = false;

	if (fieldname != undefined)	hasfield = true;

	if (address == "") {
		if (hasfield) alertInvalid(fieldname,address);
		return false;
	}

	for (i = 0; i < address.length; i++) {
		c = address.charAt(i);
		if ((c>='0'&&c<='9')||(c=='.'))
			continue;
		else {
			if (hasfield) alertInvalid(fieldname,address);
			return false;
		}
	}
	if (address == '0.0.0.0' ||address == '255.255.255.255') {
		if (hasfield) alertInvalid(fieldname,address);
		return false;
	}

	addrParts = address.split('.');

	// Make sure that everything is in decimal place
	for (i=0; i<addrParts.length; i++) {
		addrParts[i] = parseInt(addrParts[i], 10);
		addrParts[i] += "";
	}

	if (addrParts.length != 4) {
		if (hasfield) alertInvalid(fieldname,address);
		return false;
	}

	for (i = 0; i<4; i++) {
		if (isNaN(addrParts[i]) || addrParts[i] =="") {
			if (hasfield) alertInvalid(fieldname,address);
			return false;
		}
		num = parseInt(addrParts[i], 10);
		if (num < 0 || num > 255) {
			if (hasfield) alertInvalid(fieldname,address);
			return false;
		}
		if (addrParts[i].length > 3) {
			if (hasfield) alertInvalid(fieldname,address);
			return false;
		}
	}

	if ((type == undefined) || (type==TYPE_IP_ADDRESS)) {
		if (parseInt(addrParts[0],10)==0||parseInt(addrParts[3],10)==0||parseInt(addrParts[0],10)==127||parseInt(addrParts[0],10)>223) {
			if (hasfield) alertInvalid(fieldname,address);
			return false;
		}
	} else {
		if (type == TYPE_NETWORK_ADDRESS) {
			if ((parseInt(addrParts[0],10)==0) || (parseInt(addrParts[0],10)==127)||parseInt(addrParts[0],10)>223) {
				if (hasfield) alertInvalid(fieldname,address);
				return false;
			}
		}
	}

	return true;
}

//Function name:changeBlockState(idname,status)
//Description: This function changes the disabled and color property of elements given under id
//	Input: idname : the id of the tag or DIV, must have id property
//		   status: ENABLED | DISABLED
function changeBlockState(idname, status) {
	var i;
	var tempelems = document.getElementById(idname).getElementsByTagName("*");
	for (i = 0; i < tempelems.length; i++) {
		if (tempelems[i].disabled != undefined)
			tempelems[i].disabled = status;
	}

	// disable the element itself
	var tempelems = document.getElementById(idname);
	if (tempelems.disabled != undefined)
		tempelems.disabled = status;
}

function validateKeyV6IP(str)
{   
   find_flag=0;  
   account=0;   
   
   for (var i=0; i<str.length; i++) {
    if ( (str.charAt(i) >= '0' && str.charAt(i) <= '9') ||
    		(str.charAt(i) == ':' ) || 
    		(str.charAt(i) >= 'A' && str.charAt(i) <= 'F')||
    		(str.charAt(i) >= 'a' && str.charAt(i) <= 'f') ) {    			
    			if ((str.charAt(i) == ':' && str.charAt(i-1) == ':') || str.charAt(i) == ':' ) {
    				//find_flag = find_flag + 1;
    				account = account + 1;				
    			}
    			
    			if ((str.charAt(i) == ':' && str.charAt(i-1) == ':')) {
    				find_flag = find_flag + 1;
    			}
    			
    			if (str.charAt(i) == ':' && str.charAt(i-1) == ':' && str.charAt(i-2) == ':') {
    				return 0;    				
    			}    			
    			 				
			continue;
    }
    return 0;
  }
  
  // alert("find_flag="+find_flag);
  
  if ( find_flag > 1 || account == 1)
  	return 0;
  	
  return 1;
}


function validateKeyV6Prefix(str)
{   
   find_flag=0;    
   var prefix_len=0;
   var prefix_len_tmp1=0;
   var prefix_len_tmp2=0;   
  
   for (var i=0; i<str.length; i++) {
    if ( (str.charAt(i) >= '0' && str.charAt(i) <= '9') ||
    		(str.charAt(i) == ':' ) || (str.charAt(i) == '/' ) ||
    		(str.charAt(i) >= 'A' && str.charAt(i) <= 'F')||
    		(str.charAt(i) >= 'a' && str.charAt(i) <= 'f') ) {    			
    			if ((str.charAt(i) == ':' && str.charAt(i-1) == ':')) {
    				if ( str.charAt(i+1) != '/' ) {    					
    					return 0;
    				}
    				find_flag = find_flag + 1;
    			}    			
    			
    			if ( str.charAt(i) == '/' ) {
    				
    				 if ( str.charAt(i+1) < '0' || str.charAt(i+1) > '9') {    				 	
    				 	return 0;
    				 }
    				 
    				if ( (i+2) < str.length ) {
    				 	if ( str.charAt(i+2) < '0' || str.charAt(i+2) > '9') {    				 		
    				 		return 0;
    					}
    					    					
    					prefix_len_tmp1 = parseInt(str.charAt(i+1), 10); 					
    					prefix_len_tmp1 = prefix_len_tmp1*10;
    					prefix_len_tmp2 = parseInt(str.charAt(i+2), 10);    					
    					prefix_len = prefix_len_tmp1 + prefix_len_tmp2;       					 
    				} else {
    					prefix_len = parseInt(str.charAt(i+1), 10);    										
    				}    				
    				
    				if ( prefix_len > 64 || prefix_len < 3 ) {    					
    					return 0;
    				}
    			}    			
    				
    			if (str.charAt(i) == ':' && str.charAt(i-1) == ':' && str.charAt(i-2) == ':') {    				
    				return 0;    				
    			}    			
    			 				
			continue;
    }
    
    return 0;
  }
  
  //alert("find_flag="+find_flag);
  
  if ( find_flag != 1 ) {
  	//alert("validateKeyV6Prefix(6)");
  	return 0;
  }
  	
  return 1;
}

//star:20100825 IPV6_RELATED
function isIPv6(str)  
{  
return str.match(/:/g).length<=7  
&&/::/.test(str)  
?/^([\da-f]{1,4}(:|::)){1,6}[\da-f]{1,4}$/i.test(str)  
:/^([\da-f]{1,4}:){7}[\da-f]{1,4}$/i.test(str);  
} 

function $(a) {
        return document.getElementById(a);
 }
 
 function isNumber(value)
{
	return /^\d+$/.test(value);
}

function ParseIpv6Array(str)
{
    var Num;
    var i,j;
    var finalAddrArray = new Array();
    var falseAddrArray = new Array();
    
    var addrArray = str.split(':');
    Num = addrArray.length;
    if (Num > 8)
    {
        return falseAddrArray;
    }

    for (i = 0; i < Num; i++)
    {
        if ((addrArray[i].length > 4) 
            || (addrArray[i].length < 1))
        {
            return falseAddrArray;
        }
        for (j = 0; j < addrArray[i].length; j++)
        {
            if ((addrArray[i].charAt(j) < '0')
                || (addrArray[i].charAt(j) > 'f')
                || ((addrArray[i].charAt(j) > '9') && 
                (addrArray[i].charAt(j) < 'a')))
            {
                return falseAddrArray;
            }
        }

        finalAddrArray[i] = '';
        for (j = 0; j < (4 - addrArray[i].length); j++)
        {
            finalAddrArray[i] += '0';
        }
        finalAddrArray[i] += addrArray[i];
    }

    return finalAddrArray;
}

function getFullIpv6Address(address)
{
    var c = '';
    var i = 0, j = 0, k = 0, n = 0;
    var startAddress = new Array();
    var endAddress = new Array();
    var finalAddress = '';
    var startNum = 0;
    var endNum = 0;
    var lowerAddress;
    var totalNum = 0;

    lowerAddress = address.toLowerCase();
 
    var addrParts = lowerAddress.split('::');
    if (addrParts.length == 2)
    {
        if (addrParts[0] != '')
        {
            startAddress = ParseIpv6Array(addrParts[0]);
            if (startAddress.length == 0)
            {
                return '';
            }
        }
        if (addrParts[1] != '')
        {
            endAddress = ParseIpv6Array(addrParts[1]);
            if (endAddress.length == 0)
            {
               return '';
            }
        }

        if (startAddress.length +  endAddress.length >= 8)
        {
            return '';
        }
    }
    else if (addrParts.length == 1)
    {
        startAddress = ParseIpv6Array(addrParts[0]);
        if (startAddress.length != 8)
        {
            return '';
        }
    }
    else
    {
        return '';
    }

    for (i = 0; i < startAddress.length; i++)
    {
        finalAddress += startAddress[i];
        if (i != 7)
        {
            finalAddress += ':';
        }
    }
    for (; i < 8 - endAddress.length; i++)
    {
        finalAddress += '0000';
        if (i != 7)
        {
            finalAddress += ':';
        }
    }
    for (; i < 8; i++)
    {
        finalAddress += endAddress[i - (8 - endAddress.length)];
        if (i != 7)
        {
            finalAddress += ':';
        }
    }

    return finalAddress;
    
}

function isIpv6Address(address)
{
    if (getFullIpv6Address(address) == '')
    {
        return false;
    }
    
    return true;
}

function isUnicastIpv6AddressForDHCPv6(address)
{
    var tempAddress = getFullIpv6Address(address);
    
    if ((tempAddress == '')
        || (tempAddress.substring(0, 2) == 'ff'))
    {
        return false;
    }
    
    return true;
}

function isUnicastIpv6Address(address)
{
    var tempAddress = getFullIpv6Address(address);
    
    if ((tempAddress == '')
        || (tempAddress == '0000:0000:0000:0000:0000:0000:0000:0000')
        || (tempAddress == '0000:0000:0000:0000:0000:0000:0000:0001')
        || (tempAddress.substring(0, 2) == 'ff'))
    {
        return false;
    }
    
    return true;
}

function isGlobalIpv6Address(address)
{
    var tempAddress = getFullIpv6Address(address);
    
    if ((tempAddress == '')
        || (tempAddress == '0000:0000:0000:0000:0000:0000:0000:0000')
        || (tempAddress == '0000:0000:0000:0000:0000:0000:0000:0001')
        || (tempAddress.substring(0, 3) == 'fe8')
        || (tempAddress.substring(0, 3) == 'fe9')
        || (tempAddress.substring(0, 3) == 'fea')
        || (tempAddress.substring(0, 3) == 'feb')
        || (tempAddress.substring(0, 2) == 'ff'))
    {
        return false;
    }
    
    return true;
}

function isLinkLocalIpv6Address(address)
{
    var tempAddress = getFullIpv6Address(address);
    
    if ( (tempAddress.substring(0, 3) == 'fe8')
        || (tempAddress.substring(0, 3) == 'fe9')
        || (tempAddress.substring(0, 3) == 'fea')
        || (tempAddress.substring(0, 3) == 'feb'))
    {
        return true;
    }
    
    return false;
}

//star:20100825 IPV6_RELATED END

