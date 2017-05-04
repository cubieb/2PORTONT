<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>隐藏页面配置</TITLE>
<META http-equiv=pragma content=no-cache>
<META http-equiv=cache-control content="no-cache, must-revalidate">
<META http-equiv=content-type content="text/html; charset=gbk">
<META http-equiv=content-script-type content=text/javascript>
<!--系统公共css-->
<STYLE type=text/css>
@import url(/style/default.css);
</STYLE>
<!--系统公共脚本-->
<SCRIPT language="javascript" src="common.js"></SCRIPT>
<SCRIPT language="javascript" type="text/javascript">
var cgi = new Object();
<%initFactory();%>
/********************************************************************
**          on document load
********************************************************************/
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
function on_init()
{
	//form.devmodel.disabled=true;
	sji_docinit(document, cgi);
/*
	with(form){
		if(rootssid.value=="")
			rootssid.disabled=true;
		if(userpassword.value=="")
			userpassword.disabled=true;
	}
*/
}

function btnApply(act)
{
	var modemmac=cgi.macaddr;
	var modemssid=cgi.rootssid;
	var modemuserpass=cgi.userpassword;
	with(form)
	{
			if ( sji_checkhex(macaddr.value,12,12) == false ) 
			{
				msg = "MAC地址 \"" + macaddr.value + "\" 非法.";
				alert(msg);
				return false;
			}
			if(isIncludeInvalidChar(serialnum.value))
			{
					alert("设备标识含有非法字符，请重新输入!");
					return false;
			}	
			/*
			if ( devmodel.value == '' ) 
		         {
		         	devmodel.focus();
		            alert('设备型号不能为空.');
		            return false;
		         }*/
///		if(modemssid!="")
		/*
			if ( rootssid.value == '' ) 
		         {
		            alert('SSID不能为空.');
		            return false;
		         }
		         */
				 	
		if(rootssid.value!=""){
		         var str = new String();
		         str = rootssid.value;
		         if ( str.length > 32 ) 
		         {
		            alert('SSID "' + rootssid.value + '" 不能大于32个字符。');
		            return false;
		         }
			   var place = str.indexOf("ChinaNet-");
			   if(place!=0)
			   {
				 	alert('SSID "' + rootssid.value + '" 不是以ChinaNet-开头，请重新输入。');
					return false;
			   }
			   if(isIncludeInvalidChar(rootssid.value))
			   {
					alert("SSID 含有非法字符，请重新输入!");
					return false;
			   }	
		}

		if ( isValidwpapskValue(rootpassword.value) == false ) 
		{
			alert('WPA预共用的密钥应该在8个和63个ASCII字符或64个十六进制数字之间.');
			return false;
		}

//		if(modemuserpass!="")
		/*
			if(userpassword.value.length <= 0) 
			{
				userpassword.focus();
				alert("新密码不能为空，请输入新密码!");
				return false;
			}*/
		if(userpassword.value!=""){
			if(sji_checkpswnor(userpassword.value, 1, 30) == false)
			{
				userpassword.focus();
				alert("新密码错误，请重新输入新密码!");
				return false;
			}
		}

		action.value=act;
		submit();
	}
}

</SCRIPT>
</HEAD>

<!-------------------------------------------------------------------------------------->
<!--主页代码-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
  <blockquote>
	<DIV align="left" style="padding-left:20px; padding-top:5px">
		<form id="form" action=/boaform/admin/formFactory method=POST>	
			<td><b>本页的配置需重启才能生效:</b></td><br><br>
			<table border="0" cellpadding="0" cellspacing="0">
			<!--
			<tr>
				<td align="left" width="110">设备型号:</td>
                  	<td><input type='text' name='devmodel' size="25" maxlength="30" value="<% getInfo("devModel"); %>"></td>
              </tr> -->
              <tr>
                  <td align="left" width="110">MAC码:</td>
                  	<td><input type='text' name='macaddr' size="25" maxlength="12"></td>
			</tr>
			<tr>
                  <td align="left" width="110">设备序列号:</td>
                  	<td><input type='text' name='serialnum' size="25" maxlength="128" value="<% getInfo("serialno"); %>"></td>
			</tr>
			<tr>
                  <td align="left" width="110">默认无线网络名称:</td>
                  	<td><input type='text' name='rootssid' size="25" maxlength="32" ></td>
			</tr>
			<tr>
                  <td align="left" width="110">默认无线网络密钥:</td>
                  	<td><input type='text' name='rootpassword' size="25" maxlength="65" ></td>
			</tr>
			<tr>
                  <td align="left" width="110">默认终端配置密码:</td>
                  	<td><input type='text' name='userpassword' size="25" maxlength="30" ></td>
			</tr>
              </table>   
              <br>
			<input type="button" class="button" value="保存" name="save" onClick="btnApply('modify')">
		
			<input type="hidden" id="action" name="action" value="none">
			<input type="hidden" name="submit-url" value="/factorymode.asp">
			
		</form>
	</DIV>
  </blockquote>
</body>
<%addHttpNoCache();%>
</html>
