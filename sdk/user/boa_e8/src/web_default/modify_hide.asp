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
<%initHidePage();%>
/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
//	form.devmodel.disabled=true;
	sji_docinit(document, cgi);
}

function btnApply(act)
{
	with(form)
	{
		if(act=="modifyhard"){
			if ( sji_checkhex(MACAddr.value,12,12) == false ) 
			{
				msg = "MAC地址 \"" + MACAddr.value + "\" 非法.";
				alert(msg);
				return false;
			}
			if ( sji_checknum(Serialno.value) == false || Serialno.value.length!=8) 
			{
				msg = "Serial Number 非法,请输入8位十进制数.";
				alert(msg);
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
		<form id="form" action=/boaform/admin/formModify method=POST>	
			<b><font size=3>隐藏 配置:</font></b><br><br>
			<td><b>    用于临时配置telnet和ftp的使能，重启后即失效:</b></td><br><br>
			<table border="0" cellpadding="0" cellspacing="0">
			<tr>
				<td><input type="checkbox" name="telnetEnable">&nbsp;&nbsp;启用telnet</td>
			</tr>
			<tr>
				<td><input type="checkbox" name="ftpEnable">&nbsp;&nbsp;启用ftp</td>
			</tr>
			<tr>
				<td><input type="checkbox" name="webEnable">&nbsp;&nbsp;启用web</td>
			</tr>
			</table>
			<br>
			<input type="submit" class="button" value="保存" name="save" onClick="btnApply('access')"><br><br>

			
			<td><b>    用于临时开启关闭tr069 debug信息,重启后即失效:</b></td><br><br>
			<table border="0" cellpadding="0" cellspacing="0">
			<tr>
			<td><input type="submit" class="button" value="开启tr069 debug" name="debugopen" onClick="btnApply('debugopen')">&nbsp;&nbsp;</td> 
			<td><input type="submit" class="button" value="关闭tr069 debug" name="debugclose" onClick="btnApply('debugclose')"></td>
			</tr>
			</table>
			<br><br>
<!--
			<td><b>    用于更改MAC地址和设备序列号,需重启后才能生效:</b></td><br><br>
			<table border="0" cellpadding="0" cellspacing="0">
			<tr>
				<td align="left" width="90">MAC地址:</td>
                  	<td><input type='text' name='MACAddr' size="25" maxlength="12" value="<% getInfo("lanmac"); %>"></td>
              </tr>
              <tr>
                  <td align="left" width="90">设备序列号:</td>
                  	<td><input type='text' name='Serialno' size="25" maxlength="8" value="<% getInfo("serialno"); %>"></td>
			</tr>
              </table> 
              <br>
			<input type="button" class="button" value="保存" name="savemac" onClick="btnApply('modifyhard')">
			<br><br>  -->

			
			<input type="hidden" name="submit-url" value="/modify_hide.asp">
			<input type="hidden" id="action" name="action" value="none">
			
		</form>
	</DIV>
  </blockquote>
</body>
<%addHttpNoCache();%>
</html>
