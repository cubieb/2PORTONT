<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>隐藏页面配置-供应商/版本信息</TITLE>
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
<%initVendorVersion();%>
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

	with(form){
/*
		if(rootssid.value=="")
			rootssid.disabled=true;
		if(userpassword.value=="")
			userpassword.disabled=true;
*/
		softwareversion.disabled=true;
		oui.disabled=true;
	}
}

function btnApply(act)
{
	var modemmac=cgi.macaddr;
	var modemssid=cgi.rootssid;
	var modemuserpass=cgi.userpassword;
	with(form)
	{
			if( manufacture.value == '' )
			{
				manufacture.focus();
		            alert('设备供应商不能为空.');
		            return false;
			}
			if(isIncludeInvalidChar(manufacture.value))
			{
					alert("设备供应商含有非法字符，请重新输入!");
					return false;
			}	
			if ( devmodel.value == '' ) 
		         {
		         	devmodel.focus();
		            alert('设备型号不能为空.');
		            return false;
		         }
			if(isIncludeInvalidChar(devmodel.value))
			{
					alert("设备型号含有非法字符，请重新输入!");
					return false;
			}	
			/*
			if ( oui.value == '' ) 
		         {
		         	oui.focus();
		            alert('供应商OUI不能为空.');
		            return false;
		         }
			if(isIncludeInvalidChar(oui.value))
			{
					alert("供应商OUI含有非法字符，请重新输入!");
					return false;
			}	
			if ( softwareversion.value == '' ) 
		         {
		         	softwareversion.focus();
		            alert('设备软件版本不能为空.');
		            return false;
		         }
			if(isIncludeInvalidChar(softwareversion.value))
			{
					alert("设备软件版本含有非法字符，请重新输入!");
					return false;
			}
			*/
			if ( hardwareversion.value == '' ) 
		         {
		         	hardwareversion.focus();
		            alert('设备硬件版本不能为空.');
		            return false;
		         }
			if(isIncludeInvalidChar(hardwareversion.value))
			{
					alert("设备硬件版本含有非法字符，请重新输入!");
					return false;
			}
/*
			if ( productclass.value == '' ) 
		         {
		         	productclass.focus();
		            alert('TR069 ProductClass不能为空.');
		            return false;
		         }
			if(isIncludeInvalidChar(productclass.value))
			{
					alert("TR069 ProductClass含有非法字符，请重新输入!");
					return false;
			}

			if ( specversion.value == '' ) 
		         {
		         	specversion.focus();
		            alert('TR069 Specversion不能为空.');
		            return false;
		         }
			if(isIncludeInvalidChar(specversion.value))
			{
					alert("TR069 Specversion含有非法字符，请重新输入!");
					return false;
			}
*/
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
		<form id="form" action=/boaform/admin/formVendorVersion method=POST>	
			<br><br>
			<table border="0" cellpadding="0" cellspacing="0">
			<tr>
				<td align="left" width="120">设备供应商:</td>
                  		<td><input type='text' name='manufacture' size="25" maxlength="30" value="<% getInfo("manufacture"); %>"></td>
              	</tr>
			<tr>
				<td align="left" width="120">设备型号:</td>
                  		<td><input type='text' name='devmodel' size="25" maxlength="30" value="<% getInfo("devModel"); %>"></td>
              	</tr> 
              	<tr>
				<td align="left" width="120">供应商OUI:</td>
                  		<td><input type='text' name='oui' size="25" maxlength="30" value="<% getInfo("oui"); %>"></td>
              	</tr>

              	<tr>
				<td align="left" width="120">设备软件版本:</td>
                  		<td><input type='text' name='softwareversion' size="25" maxlength="30" value="<% getInfo("stVer"); %>"></td>
              	</tr>

              	<tr>
				<td align="left" width="120">设备硬件版本:</td>
                  		<td><input type='text' name='hardwareversion' size="25" maxlength="30" value="<% getInfo("hdVer"); %>"></td>
              	</tr>
              	<!--
              	<tr>
				<td align="left" width="120">TR069 ProductClass:</td>
                  		<td><input type='text' name='productclass' size="25" maxlength="30" value="<% getInfo("ProductClass"); %>"></td>
              	</tr>
              	-->
              	<!--
              	<tr>
				<td align="left" width="120">TR069 Specversion:</td>
                  		<td><input type='text' name='specversion' size="25" maxlength="30" value="<% getInfo("SpecVer"); %>"></td>
              	</tr>	-->
			
              </table>   
              <br>
			<input type="button" class="button" value="保存" name="save" onClick="btnApply('modify')">
		
			<input type="hidden" id="action" name="action" value="none">
			<input type="hidden" name="submit-url" value="/vendorversion.asp">
			
		</form>
	</DIV>
  </blockquote>
</body>
<%addHttpNoCache();%>
</html>
