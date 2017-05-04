<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>中国电信</TITLE>
<META http-equiv=pragma content=no-cache>
<META http-equiv=cache-control content="no-cache, must-revalidate">
<META http-equiv=content-type content="text/html; charset=gbk">
<META http-equiv=content-script-type content=text/javascript>
<!--系统公共css-->
<STYLE type=text/css>
@import url(/style/default.css);
</STYLE>
<!--系统公共脚本-->
<SCRIPT language="javascript" type="text/javascript">
var loginFlag = 0;

function myKeyDown(e) 
{
	var code;

	if (!e) {
		e = window.event;
	}

	if (e.keyCode) {
		code = e.keyCode;
	} else if (e.which) {
		code = e.which;
	}

	if (code == 13) {
		on_submit();
	}

	return true;
}

document.onkeydown = myKeyDown;
if (document.captureEvents) document.captureEvents(Event.KEYDOWN);

/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	document.forms[0].username.focus();

	if(document.referrer.search("usereg.asp") != -1)
		return ;

	var regStatus = <% getInfo("cwmp_UserInfo_Status"); %>;
	if (regStatus != 0 // not registered OK
		&& regStatus != 5) // not already registered
	{
		var win = window.open('/usereg.asp');
		win.focus();
	}
}

/********************************************************************
**          on document submit
********************************************************************/
function on_submit() 
{
	if (loginFlag)
		return 1;
	
	with ( document.forms[0] ) {
		/*
		if(username.value.length <= 0) {
			alert("用户名为空，请输入用户名!");
			return;
		}
		*/
		if(psd.value.length <= 0) {
			alert("密码为空，请输入密码!");
			return;
		}
		loginFlag = 1;
		
		submit();
	}
}
function on_Diag()
{
	window.location.href="/diag_index.html";
}
</SCRIPT>

</head>

<body leftmargin="0" topmargin="0"  bgcolor="white" onLoad="on_init();">
<form action=/boaform/admin/formLogin method=POST name="cmlogin">
<table width="100%" height="100%" align="center" valign="middle" >
<tr>
	<td width="25%" height="25%"></td>
	<td width="50%" ></td>
	<td width="25%"></td>
</tr>
<tr  align="left" valign="top">
	<td height="50%"></td>
	<td>
		<table width="100%" cellspacing="0"  style="font-size:10pt">
			<tr bgcolor="#427594" height="10%">
				<td  bgcolor="#427594" width="1"></td>
				<td>&nbsp;&nbsp;<font color="white">用户登录</font></td>
				<td  bgcolor="#427594" width="1"></td>
			</tr>
			<tr  align="center" valign="middle" >
				<td  bgcolor="#427594" width="1"></td>
				<td><table  cellspacing="8"  style="font-size:10pt">
					<tr align="center" valign="middle">
						<td colspan="3"><IMG height="78" src="/image/logo.gif" width="225" border=0></td>
					</tr>
					<tr><td width="21">&nbsp;&nbsp;&nbsp;</td><td>用户:</td><td><input type="text" name="username" id="username" style="width:150;" value="<% getInfo("normal-user"); %>"/></td></tr>
					<tr><td width="21">&nbsp;&nbsp;&nbsp;</td><td>密码:</td><td><input type="password" name="psd" id="psd" style="width:150;"/></td></tr>
				</table></td>
				<td  bgcolor="#427594" width="1"></td>
			</tr>
			<tr bgcolor="#427594" height="10%"  align="center" valign="middle" >
				<td  bgcolor="#427594" width="1"></td>
				<td>&nbsp;&nbsp;<input type="button" class="button" onClick="on_submit();" value="登录"/>
				&nbsp;&nbsp;<input type="reset" value="重写" />&nbsp;&nbsp;<!--<input type="button" value="快速诊断" onClick="on_Diag();" />&nbsp;&nbsp;-->
				<!--input type="button" value="逻辑ID注册" onclick="location.href='/usereg.asp';" /-->
				</td>
				<td  bgcolor="#427594" width="1"></td>
			</tr>
		</table>
	</td>
	<td></td>
</tr>
<tr>
	<td height="25%"></td>
	<td></td>
	<td></td>
</tr>
</table>
</form>
</BODY>
<%addHttpNoCache();%>
</html>
