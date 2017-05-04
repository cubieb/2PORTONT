<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>日常应用</TITLE>
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


/********************************************************************
**          on document load
********************************************************************/
function on_init()
{

}

/********************************************************************
**          on document submit
********************************************************************/
function on_submit() 
{

	if((form.proxy0_port.value == "") || !sji_checkdigitrange(form.proxy0_port.value, 1, 65535))
	{
		alert("主用SIP Proxy请输入合法的端口号！");
		return false;
	}

	if(!(form.proxy1_port.value == "") && !sji_checkdigitrange(form.proxy1_port.value, 1, 65535))
	{
		alert("备用SIP Proxy请输入合法的端口号！");
		return false;
	}

	//if(!(form.port1_number.value == "") && !sji_checkusername(form.port1_number.value, 1, 32))
	//{
		//alert("线路1 请输入合法的号码！");
		//return false;
	//}
	//if(!(form.port1_login_id.value == "") && !sji_checkusername(form.port1_login_id.value, 1, 32))
	//{
		//alert("线路1 请输入合法的用户名！");
		//return false;
	//}
	//if(!(form.port1_password.value == "") &&!sji_checkpswnor(form.port1_password.value, 1, 32))
	//{
		//alert("线路1 请输入合法的密码！");
		//return false;
	//}
	//if((form.max_voip_ports.value == 2)&&!(form.port2_number.value == "") && !sji_checkusername(form.port2_number.value, 1, 32))
	//{
		//alert("线路2 请输入合法的号码！");
		//return false;
	//}
	//if((form.max_voip_ports.value == 2)&&!(form.port2_login_id.value == "") && !sji_checkusername(form.port2_login_id.value, 1, 32))
	//{
		//alert("线路2 请输入合法的用户名！");
		//return false;
	//}
	//if((form.max_voip_ports.value == 2)&&!(form.port2_password.value == "") &&!sji_checkpswnor(form.port2_password.value, 1, 32))
	//{
		//alert("线路2 请输入合法的密码！");
		//return false;
	//}
	if((form.proxy0_reg_expire.value == "") || !sji_checkdigitrange(form.proxy0_reg_expire.value, 10, 86400))
	{
		alert("主用SIP Proxy Register Expire out of range [10-86400]！");
		return false;
	}
	if((form.proxy1_reg_expire.value == "") || !sji_checkdigitrange(form.proxy1_reg_expire.value, 10, 86400))
	{
		alert("备用SIP Proxy Register Expire out of range [10-86400]！");
		return false;
	}
	if((form.proxy1_enable.checked == true) && (form.port1_account_enable.checked != true)&&(form.port2_account_enable.checked != true))
	{
		alert("啟用備用sip proxy 需啟用线路账号");
		return false;
	}
	if((form.max_voip_ports.value == 2) && (form.port1_number.value!="")&&(form.port1_number.value == form.port2_number.value))
	{
		alert("线路1号码与线路2号码相同");
		return false;
	}
	form.submit();
	//submit();
}
</SCRIPT>
</HEAD>

<!-------------------------------------------------------------------------------------->
<!--主页代码-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000">
	<blockquote>
		<DIV align="left" style="padding-left:20px; padding-top:5px">
			<form id="form" action=/boaform/admin/voip_e8c_set method=POST>
			<input type=hidden name=max_voip_ports value="<%voip_e8c_get("max_voip_ports");%>">
				<td>
				<table border="0" cellpadding="0" cellspacing="0" width="700">
					<tr>
						<td width="200px">软交换版本</td><td><select name=servertype>"<%voip_e8c_get("servertype");%>"</select></td>
					</tr>
				</table>
				<b>主用SIP 代理</br>
				<table border="0" cellpadding="0" cellspacing="0" width="700">
					<tr>
						<td width="200px">地址:</td><td><input name="proxy0_addr" size="16" maxlength="32" type="text" style="width:140px" value="<% voip_e8c_get("proxy0_addr"); %>"></td>
					</tr>
					<tr>
						<td>端口号:</td><td><input name="proxy0_port" size="16" maxlength="32" style="width:140px" value="<% voip_e8c_get("proxy0_port"); %>"></td>
					</tr>
					<tr>
						<td>外发代理启用</td><td><input name="proxy0_obEnable" type="checkbox" value="enable" <% voip_e8c_get("proxy0_obEnable"); %>></td>
					</tr>
					<tr>
						<td>外发代理地址:</td><td><input name="proxy0_obAddr" size="16" maxlength="32" type="text" style="width:140px" value="<% voip_e8c_get("proxy0_obAddr"); %>"></td>
					</tr>
					<tr>
						<td>外发代理端口号:</td><td><input name="proxy0_obPort" size="16" maxlength="32" style="width:140px" value="<% voip_e8c_get("proxy0_obPort"); %>"></td>
					</tr>
					<tr>
						<td>SIP 域名:</td><td><input name="proxy0_domain_name" size="16" maxlength="32" type="text" style="width:140px" value="<% voip_e8c_get("proxy0_domain_name"); %>"></td>
					</tr>
					<tr>
						<td>注册有效期 (秒):</td><td><input name="proxy0_reg_expire" size="16" maxlength="32" style="width:140px" value="<% voip_e8c_get("proxy0_reg_expire"); %>"></td>
					</tr>
					<tr>
						<td>啟用會話更新</td><td><input name="proxy0_sessionEnable" type="checkbox" value="enable" <% voip_e8c_get("proxy0_sessionEnable"); %>></td>
					</tr>
					<tr>
						<td>会话更新周期 (秒):</td><td><input name="proxy0_sessionExpiry" size="16" maxlength="32" style="width:140px" value="<% voip_e8c_get("proxy0_sessionExpiry"); %>"></td>
					</tr>
				</table>
				<br>

				<b>备用SIP 代理</b>
				<table border="0" cellpadding="0" cellspacing="0" width="700">
					<tr>
						<td width="200px">备用SIP启用</td><td><input name="proxy1_enable" type="checkbox"  value="enable" <% voip_e8c_get("proxy1_enable"); %>></td>
					</tr>
					<tr>
					<tr>
						<td width="200px">地址:</td><td><input name="proxy1_addr" size="16" maxlength="32" type="text" style="width:140px" value="<% voip_e8c_get("proxy1_addr"); %>"></td>
					</tr>
					<tr>
						<td>端口号:</td><td><input name="proxy1_port" size="16" maxlength="32" style="width:140px" value="<% voip_e8c_get("proxy1_port"); %>"></td>
					</tr>
					<tr>
						<td>外发代理启用</td><td><input name="proxy1_obEnable" type="checkbox" value="enable" <% voip_e8c_get("proxy1_obEnable"); %>></td>
					</tr>
					<tr>
						<td>外发代理地址:</td><td><input name="proxy1_obAddr" size="16" maxlength="32" type="text" style="width:140px" value="<% voip_e8c_get("proxy1_obAddr"); %>"></td>
					</tr>
					<tr>
						<td>外发代理端口号:</td><td><input name="proxy1_obPort" size="16" maxlength="32" style="width:140px" value="<% voip_e8c_get("proxy1_obPort"); %>"></td>
					</tr>
					<tr>
						<td>SIP 域名:</td><td><input name="proxy1_domain_name" size="16" maxlength="32" type="text" style="width:140px" value="<% voip_e8c_get("proxy1_domain_name"); %>"></td>
					</tr>
					<tr>
						<td>注册有效期(秒):</td><td><input name="proxy1_reg_expire" size="16" maxlength="32" style="width:140px" value="<% voip_e8c_get("proxy1_reg_expire"); %>"></td>
					</tr>
					<tr>
						<td>啟用會話更新</td><td><input name="proxy1_sessionEnable" type="checkbox" value="enable" <% voip_e8c_get("proxy1_sessionEnable"); %>></td>
					</tr>
					<tr>
						<td>会话更新周期 (秒):</td><td><input name="proxy1_sessionExpiry" size="16" maxlength="32" style="width:140px" value="<% voip_e8c_get("proxy1_sessionExpiry"); %>"></td>
					</tr>
				</table>
				<br>
				
				<% voip_e8c_get("port_account"); %>
				

				<input type="button" class="button" name="btnDown" value="保存/应用" onClick="on_submit()">
				<input type="hidden" name="submit-url" value="">
			</form>
		</DIV>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>
