<!-- add by liuxiao 2008-01-16 -->
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
<head>
<title>L2TP配置</title>
<meta http-equiv=pragma content=no-cache>
<meta http-equiv=cache-control content="no-cache, must-revalidate">
<meta http-equiv=content-type content="text/html; charset=gbk">
<meta http-equiv=content-script-type content=text/javascript>
<style type=text/css>
@import url(/style/default.css);
</style>
<script language="javascript" src="common.js"></script>
<SCRIPT language="javascript" type="text/javascript">
function checkTextStr(str)
{
	for (var i=0; i<str.length; i++) 
	{
		if ( str.charAt(i) == '%' || str.charAt(i) =='&' ||str.charAt(i) =='\\' || str.charAt(i) =='?' || str.charAt(i)=='"') 
			return false;			
	}
	return true;
}

function pppAuthChange()
{
	if (document.l2tp.auth.selectedIndex==3)
	{
		document.l2tp.enctype.disabled = false;
	}
	else
	{
		document.l2tp.enctype.disabled = true;
	}
}

function tunnelAuthChange()
{
	if (document.l2tp.tunnel_auth.checked)
		document.l2tp.tunnel_secret.disabled = false;
	else
		document.l2tp.tunnel_secret.disabled = true;
}

function connTypeChange()
{	
	document.l2tp.defaultgw.checked = false;
	document.l2tp.defaultgw.disabled = false;	
	if(document.l2tp.pppconntype.selectedIndex==1)
	{
		document.l2tp.idletime.disabled=false;		
		document.l2tp.defaultgw.checked = true;
		document.l2tp.defaultgw.disabled = true;
	}else{
		document.l2tp.idletime.disabled=true;		
	}
}

function l2tpSelection()
{
	if (document.l2tp.l2tpen[0].checked) {
		document.l2tp.server.disabled = true;
		document.l2tp.tunnel_auth.disabled = true;
		document.l2tp.tunnel_secret.disabled = true;
		document.l2tp.auth.disabled = true;
		document.l2tp.username.disabled = true;
		document.l2tp.password.disabled = true;
		document.l2tp.pppconntype.disabled = true;
		document.l2tp.idletime.disabled = true;
		document.l2tp.mtu.disabled = true;
		document.l2tp.defaultgw.disabled = true;
		document.l2tp.addL2TP.disabled = true;
		document.l2tp.enctype.disabled = true;
	}
	else {
		document.l2tp.server.disabled = false;
		document.l2tp.tunnel_auth.disabled = false;
		document.l2tp.tunnel_secret.disabled = false;
		document.l2tp.auth.disabled = false;
		document.l2tp.username.disabled = false;
		document.l2tp.password.disabled = false;
		document.l2tp.pppconntype.disabled = false;
		document.l2tp.idletime.disabled = false;
		document.l2tp.mtu.disabled = false;
		document.l2tp.defaultgw.disabled = false;
		document.l2tp.addL2TP.disabled = false;
		document.l2tp.enctype.disabled = false;
	}
	tunnelAuthChange();
	pppAuthChange();
	connTypeChange()
}

function onClickL2TPEnable()
{
	l2tpSelection();
	
	if (document.l2tp.l2tpen[0].checked)
		document.l2tp.lst.value = "disable";
	else
		document.l2tp.lst.value = "enable";
	
	document.l2tp.submit();
}

function addL2TPItf()
{
	if(document.l2tp.l2tpen[0].checked)
		return false;
	
	if (document.l2tp.server.value=="") {		
		alert("please_enter_l2tp_server_address");
		document.l2tp.server.focus();
		return false;
	}
	
	if(!checkTextStr(document.l2tp.server.value))
	{		
		alert("invalid value in server address");
		document.l2tp.server.focus();
		return false;		
	}

	if (document.l2tp.tunnel_auth.checked)
	{
		if (document.l2tp.tunnel_secret.value=="")
		{			
			alert("please enter l2tp tunnel authentication secret");
			document.l2tp.tunnel_secret.focus();
			return false;
		}
		if(!checkTextStr(document.l2tp.tunnel_secret.value))
		{			
			alert("invalid value in tunnel authentication secret");
			document.l2tp.tunnel_secret.focus();
			return false;		
		}
	}
	
	if (document.l2tp.auth.selectedIndex!=3)
	{
		if (document.l2tp.username.value=="")
		{			
			alert("please enter l2tp client username");
			document.l2tp.username.focus();
			return false;
		}
		if(!checkTextStr(document.l2tp.username.value))
		{			
			alert("invalid value in username");
			document.l2tp.username.focus();
			return false;		
		}
		if (document.l2tp.password.value=="") {			
			alert("please enter l2tp client password");
			document.l2tp.password.focus();
			return false;
		}
		if(!checkTextStr(document.l2tp.password.value))
		{			
			alert("invalid value in password");
			document.l2tp.password.focus();
			return false;		
		}
	}

	if (document.l2tp.pppconntype.selectedIndex==1)
	{
		if (document.l2tp.idletime.value=="")
		{			
			alert("please enter l2tp tunnel idle time");
			document.l2tp.idletime.focus();
			return false;
		}
		
		if (document.l2tp.defaultgw.checked==false)
		{			
			alert("please select default gateway for dial on demand");
			document.l2tp.defaultgw.focus();
			return false;
		}
	}

	if (document.l2tp.mtu.value=="")
	{		
		alert("please enter l2tp tunnel mtu");
		document.l2tp.mtu.focus();
		return false;
	}

	return true;
}

</SCRIPT>
</head>

<body>
<blockquote>
<form action=/boaform/formL2TP method=POST name="l2tp">
<table border=0 width="500" cellspacing=0 cellpadding=0>
  <tr><hr size=1 noshade align=top></tr>

  <tr>
      <td width="40%"><font size=2><b>L2TP VPN:</b></td>
      <td width="60%"><font size=2>
      	<input type="radio" value="0" name="l2tpen" <% checkWrite("l2tpenable0"); %> onClick="onClickL2TPEnable()">禁用&nbsp;&nbsp;
     	<input type="radio" value="1" name="l2tpen" <% checkWrite("l2tpenable1"); %> onClick="onClickL2TPEnable()">启用
      </td>
  </tr>
</table>
<input type="hidden" id="lst" name="lst" value="">
<br>

<table border=0 width="500" cellspacing=0 cellpadding=0>
  <tr>
    <td width="40%"><font size=2><b>服务器IP地址:</b></td>
    <td width="60%"><input type="text" name="server" size="32" maxlength="256"></td>
  </tr>
  <tr>
    <td width="40%"><font size=2><b>通道验证:</b></td>
    <td width="60%"><input type=checkbox name="tunnel_auth" value=1 onClick=tunnelAuthChange()></td>
  </tr>
  <tr>
    <td width="40%"><font size=2><b>通道验证密码:</b></td>
    <td width="60%"><input type="text" name="tunnel_secret" size="15" maxlength="35"></td>
  </tr>
  <tr>
    <td width="40%"><font size=2><b>PPP验证协议:</b></td>
    <td width="60%"><select name="auth" onChange="pppAuthChange()">
      <option value="0">自动</option>
      <option value="1">PAP</option>
      <option value="2">CHAP</option>
      <option value="3">CHAPMSV2</option>
      </select>
    </td>
  </tr>
  <tr>
    <td width="30%"><font size=2></b>PPP加密:</b></td>
    <td width="70%"><select name="enctype" >
      <option value="0">无</option>
      <option value="1">MPPE</option>
      <option value="2">MPPC</option>
      <option value="3">MPPE&MPPC</option>
      </select>
    </td>
  </tr>
  <tr>
    <td width="40%"><font size=2><b>拨号用户名:</b></td>
    <td width="60%"><input type="text" name="username" size="15" maxlength="35"></td>
  </tr>
  <tr>
    <td width="40%"><font size=2><b>拨号密码:</b></td>
    <td width="60%"><input type="text" name="password" size="15" maxlength="35"></td>
  </tr>
  <tr>
    <td width="40%"><font size=2><b>PPP拨号方式:</b></td>
    <td width="60%"><select name="pppconntype" onChange="connTypeChange()">
      <option value="0">连续不中断</option>
      <option value="1">按需拨号</option>
      <option value="2">手动</option>
      <option value="3">无</option>
      </select>
    </td>
  </tr>
  <tr>
    <td width="40%"><font size=2><b>闲置超时时间:</b></td>
    <td width="60%"><input type="text" name="idletime" size="32" maxlength="256"></td>
  </tr>
  <tr>
    <td width="40%"><font size=2><b>MTU:</b></td>
    <td width="60%"><input type="text" name="mtu" size="32" maxlength="256" value="1458"></td>
  </tr>
  <tr>
    <td width="40%"><font size=2><b>设定为缺省网关:</b></td>
    <td width="60%"><input type="checkbox" name="defaultgw"></td>
  </tr>
</table>

<table border=0 width="500" cellspacing=0 cellpadding=0>
  </tr>
      <td><input type="submit" value="保存/应用" name="addL2TP" onClick="return addL2TPItf()">&nbsp;&nbsp;</td>
  </tr>
</table>
<br><br>

<table border=0 width="600" cellspacing=4 cellpadding=0>
  <tr><font size=2><b>L2TP 配置表:</b></font></tr>
  <tr>
    <td align=center width="3%" bgcolor="#808080"><font size=2>选择</font></td>
    <td align=center width="5%" bgcolor="#808080"><font size=2>服务接口</font></td>
    <td align=center width="5%" bgcolor="#808080"><font size=2>服务器IP地址</font></td>
    <td align=center width="5%" bgcolor="#808080"><font size=2>通道验证协议</font></td>
    <td align=center width="5%" bgcolor="#808080"><font size=2>PPP验证协议</font></td>
    <td align=center width="5%" bgcolor="#808080"><font size=2>MTU</font></td>
    <td align=center width="5%" bgcolor="#808080"><font size=2>缺省网关</font></td>
    <td align=center width="8%" bgcolor="#808080"><font size=2>连接状态</font></td>
  </tr>
	<% l2tpList(); %>
</table>
<br>
<input type="submit" value="删除" name="delSel" onClick="return deleteClick()">&nbsp;&nbsp;
<input type="hidden" value="/l2tp.asp" name="submit-url">
<script>
	l2tpSelection();
</script>
</form>
</blockquote>
</body>
</html>
