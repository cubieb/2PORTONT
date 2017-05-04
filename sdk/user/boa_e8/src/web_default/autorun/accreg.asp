<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>中国电信-ADSL宽带帐号身份认证</TITLE>
<META http-equiv=pragma content=no-cache>
<META http-equiv=cache-control content="no-cache, must-revalidate">
<META http-equiv=content-type content="text/html; charset=gbk">
<META http-equiv=content-script-type content=text/javascript>
<!--系统公共css-->
<style>
BODY{font-family:"华文宋体";}
TR{height:16px;}
SELECT {width:150px;}
</style>
<!--系统公共脚本-->
<script language="javascript" src="/common.js"></script>
<SCRIPT language="javascript" type="text/javascript">

var pvcPreSet;
var pppUsername;
<% initE8clientUserRegPage(); %>

document.onmousedown = function(e)
{
	var tar = null;
	if (!e)e = window.event;
	if(e.button != 1)return true;
	if (e.target) tar = e.target;
	else if (e.srcElement) tar = e.srcElement;
	if(tar.tagName == "INPUT" || tar.tagName == "IMG") return true;
	document.location.href = "app:mouse:onmousedown";
}
/*

function on_init()
{
	
	with ( document.forms[0] ) 
	{

		if (pvcPreSet == "1") {
			user.disabled = false;
			auth.disabled = false;
			if (pppUsername != "")
				user.value = pppUsername;
		} else {
			user.disabled = true;
			auth.disabled = true;
		}		
		
		if(user.value == "")
		{
			var ins = document.getElementsByTagName("input");
			for(var i in ins){if(ins[i].value != "后退")ins[i].disabled = true;}
			ins = document.getElementsByTagName("img");
			for(var i in ins)ins[i].disabled = true;
		}
	}
}

function on_submit() 
{
	with ( document.forms[0] ) 
	{
		//input text
		if(sji_checkpppacc(auth.value, 1, 25) == false)
		{
			auth.focus();
			external.ShowDlg("身份认证信息\"" + auth.value + "\"存在无效字符或长度不在1-25字节之间，请重新输入！");
			return;
		}
		if(sji_checkpppacc(user.value, 1, 25) == false)
		{
			user.focus();
			external.ShowDlg("用户名\"" + user.value + "\"为无效用户名或长度不在1-25字节之间，请重新输入！");
			return;
		}
		
		submit();
	}
}
*/
/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	if (pvcPreSet == "1") {
		document.getElementById("normaldisplay").style.display = "block";
		document.getElementById("errordisplay").style.display = "none";
		if (pppUsername != "")
			document.forms[0].broadbandusername.value = pppUsername;
/*
		document.forms[0].broadbandusername.disabled = false;
		document.getElementById("customer ID").disabled = false;
		if (pppUsername != "")
			document.forms[0].broadbandusername.value = pppUsername;
*/
	} else {
		document.getElementById("normaldisplay").style.display = "none";
		document.getElementById("errordisplay").style.display = "block";
/*
		document.forms[0].broadbandusername.disabled = true;
		document.getElementById("customer ID").disabled = true;
*/
	}
}

/********************************************************************
**          on document submit
********************************************************************/
function on_submit() 
{
	//input text
	if(sji_checkpppacc(document.getElementById("customer ID").value, 1, 25) == false)
	{
		document.getElementById("customer ID").focus();
		alert("身份认证信息\"" + document.getElementById("customer ID").value + "\"存在无效字符或长度不在1-25字节之间，请重新输入！");
		return;
	}
	if(sji_checkpppacc(document.forms[0].broadbandusername.value, 1, 25) == false)
	{
		document.forms[0].broadbandusername.focus();
		alert("用户名存在无效字符或长度不在1-25字节之间，请重新输入！");
		return;
	}
	
	document.forms[0].submit();
}

</script>

</HEAD>
<!-------------------------------------------------------------------------------------->
<!--主页代码-->
<body topmargin="0" bgcolor="E0E0E0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init()">
	<DIV align="center" style="padding-left:5px; padding-top:5px">
		<form id="form" action="/boaform/admin/formAccountReg" method="post">
			<table width="80%"><tr><td><p align="left"><font size="+2"><b>ADSL宽带帐号身份认证</b></font></p></td></tr></table><br><br>
			<table id="cfg" cellspacing="0" cellpadding="0" width="256" align="center" border="0">
			  <tr valign="top">
				<td height="1">
				<div align="center" id="errordisplay">
				  <table height="200" cellspacing="0" cellpadding="0" width="100%" align="center" border="0">
						<tr>
							<td><font color="#FF0000">终端未配置带“INTERNET”且为“Router”属性的WAN连接，请联系10000!</td>
						</tr>
				   </table>
				   <table border="0" cellpadding="1" cellspacing="0" width="80%">
					<tr>
					<td align="center"><input type="button" value="后退" onClick="window.location.href='/autorun/e8cfg.asp';" style="width:80px; border-style:groove; font-weight:bold "></td>&nbsp;&nbsp;
					</tr>
				</table>
				  </div>
				  <div align="center" id="normaldisplay">
				  <table height="200" cellspacing="0" cellpadding="0" width="100%" align="center" border="0">
					<tr>
					  <td vAlign=top>
						<table cellspacing="0" cellpadding="0" border="0" width="100%">
							<tr nowrap><td>身份证号：</td><td align="right"><input type="text" id="customer ID"  name="customer ID" maxlength="20" size="20" style="width:150px "></td></tr>
							<tr nowrap><td>宽带帐号：</td><td align="right"><input type="text" name="broadbandusername" maxlength="26" size="26" style="width:150px "></td></tr>
						</table>
					  </td>
					</tr>
				  </table>
				  <table border="0" cellpadding="1" cellspacing="0" width="80%">
					<tr>
					<td align="right"><input type="button" value="后退" onClick="window.location.href='/autorun/e8cfg.asp';" style="width:80px; border-style:groove; font-weight:bold "></td>&nbsp;&nbsp;
					<td align="right"><input type="button" name="regbutton" value="注册" onClick="on_submit();" style="width:80px; border-style:groove; font-weight:bold "></td>
					</tr>
				</table>
				  </div>
				</td>
			  </tr>
			</table>
			<br>
			
			<input type="hidden" name="submit-url" value="/autorun/accregresult.asp">
		</form>
	</DIV>
</body>
</html>

