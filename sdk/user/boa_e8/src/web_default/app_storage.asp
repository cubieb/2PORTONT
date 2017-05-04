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

var cgi = new Object();
<% initPageStorage(); %>
var rcs = new Array();
with(rcs){<% listUsbDevices(); %>}

/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	sji_docinit(document, cgi);
	with(form)
	{
		tdftpEnable.innerHTML = cgi.ftpEnable ? "启用" : "禁用";
	}
	
	while(form.saveDir.options.length >= 1)
		{form.saveDir.options.remove(0);}
	
	for(var i = 0; i < rcs.length; i++)
	{
		form.saveDir.options.add(new Option(rcs[i].path, rcs[i].path + "/xdown"));
	}
	
	if(rcs.length == 0)
	{
		form.saveDir.options.add(new Option("无USB存储设备", "0"));
		form.saveDir.disabled = true;
		form.btnDown.disabled = true;
	}
}

/********************************************************************
**          on document submit
********************************************************************/
function on_submit(act) 
{
	with ( document.forms[0] ) 
	{
		if(act == "rl") 
		{
			var loc = "app_storage.asp";
			var code = "window.location.href=\"" + loc + "\"";
			eval(code);
			return;
		}
		if(saveDir.value == "0") 
		{
			alert("请插入USB设备, 并刷新本页面！");
			return false;
		}
		if(user.value!=""&&!sji_checkusername(user.value, 1, 32))
		{
			alert("请输入合法的用户名！");
			return false;
		}
		if(passwd.value!=""&&!sji_checkpswnor(passwd.value, 1, 32))
		{
			alert("请输入合法的密码！");
			return false;
		}
		
		if(port.value!==""&&!sji_checkdigitrange(port.value, 1, 65535))
		{
			alert("请输入合法的端口号！");
			return false;
		}
			
		if(!sji_checkftpurl(rmtURL.value))
		{
			alert("请输入合法的远程 FTP URL！");
			return false;
		}
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
			<form id="form" action=/boaform/admin/formStorage method=POST>
				<b>服务状态<br><br>
				<table border="0" cellpadding="3" cellspacing="0">
					<tr>
						<td>FTP服务器:</td>
						<td id="tdftpEnable"></td>
					</tr>
				</table>
				<br>
				<input type="button" class="button" onClick ="on_submit('rl')" value="刷新本页">
				
				<p><HR align="left" width=500></p>
				
				<table border="0" cellpadding="0" cellspacing="0" width="500">
					<tr><td width="200px">USB远程下载文件存放目录:</td><td><select size="1" name="saveDir" style="width:140px "></select>/xdown</td></tr>
					<tr><td>用户名:</td><td><input name="user" size="16" maxlength="32" type="text" style="width:140px "></td></tr>
					<tr><td>密码:</td><td><input name="passwd" size="16" maxlength="32" type="password" style="width:140px "></td></tr>
					<tr><td>端口:</td><td><input name="port" size="6" maxlength="6" type="text" style="width:140px "></td></tr>
					<tr><td>远程URL:</td><td><input name="rmtURL" size="50" maxlength="128" type="text" style="width:280px "></td></tr>
					<!--<tr><td colspan="2"><div id="rstip" style="display:block;"><font color="red">提示: URL格式 :(协议)://(主机名):(端口号) / (文件路径)/(文件名),协议支持http与ftp</font><br></div></td></tr>-->

				</table>
				<br>
				<input type="button" class="button" name="btnDown" value="下载" onClick="on_submit('dl')">
				<input type="hidden" name="submit-url" value="">
			</form>
		</DIV>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>
