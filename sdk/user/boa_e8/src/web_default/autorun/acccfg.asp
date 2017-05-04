<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>中国电信-ADSL宽带帐号配置</TITLE>
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
var curlink = null;
var cgi = new Object();
var links = new Array();
with(links){<% initPageAdsl(); %>}

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
	var selind = -1;
	sji_docinit(document, cgi);	
	
	with ( document.forms[0] ) 
	{
		for(var k in links)
		{
			var lk = links[k];
			var sname = lk.name;
			if(sname.indexOf("Internet") >= 0 && selind == -1) selind = lkname.options.length;
			lkname.options.add(new Option(sname, k));
		}
		if(selind >= 0)
		{
			lkname.value = selind;
			on_ctrlupdate();
			lkname.disabled = true;
			lkname.style.display = "none";
		}
		else
		{
			lkname.disabled = true;
			lkname.style.display = "none";
			var ins = document.getElementsByTagName("input");
			for(var i in ins){if(ins[i].value != "后退")ins[i].disabled = true;}
			ins = document.getElementsByTagName("img");
			for(var i in ins)ins[i].disabled = true;
		}
	}
}

function on_linkchange(itlk)
{
	with ( document.forms[0] ) 
	{
		sji_onchanged(document, itlk);
	}
}

function on_ctrlupdate()
{
	with ( document.forms[0] ) 
	{
		curlink = links[lkname.value];
		vpi.value=curlink.vpi;
		vci.value=curlink.vci;
		pppUsername.value=curlink.pppUsername;
		pppPassword.value=curlink.pppPassword;
		on_linkchange(curlink);
	}
}

function on_submit() 
{
	with ( document.forms[0] ) 
	{
		//input text
		if(sji_checkpppacc(pppUsername.value, 1, 25) == false)
		{
			pppUsername.focus();
			external.ShowDlg("用户名\"" + pppUsername.value + "\"为无效用户名或长度不在1-25字节之间，请重新输入！");
			return;
		}

		if(oldpppPassword.value != curlink.pppPassword)
		{
			oldpppPassword.focus();
			external.ShowDlg("旧密码不匹配，请重新输入！");
			return;
		} 

		if(sji_checkpppacc(pppPassword.value, 1, 25) == false)
		{
			pppPassword.focus();
			external.ShowDlg("密码存在无效字符或长度不在1-25字节之间，请重新输入！");
			return;
		}
		if(pppPassword.value != pppPassword_cfm.value)
		{
			pppPassword_cfm.focus();
			external.ShowDlg("密码确认和密码不一致，请重新输入！");
			return;
		}
		lkname.style.display = "block";
		lkname.disabled = false;
		submit();
	}
}
*/

/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	sji_docinit(document, cgi);	
	
	with ( document.forms[0] ) 
	{
		var i=0;
		var first=-1;
		for(var k in links)
		{
			var lk = links[k];
			if(lk.cmode==2&&lk.PPPoEProxyEnable==0&&(lk.applicationtype==0 || lk.applicationtype==1)){
				i=i+1;
				if(first==-1)
					first=k;
				lkname.options.add(new Option(lk.name, lk.name));
			}
		}
		if(i> 0)
		{
			lkname.value = links[first].name;
			vpi.value=links[first].vpi;
			vci.value=links[first].vci;
			pppUsername.value=links[first].pppUsername;
			on_ctrlupdate();
		}
		else
		{
			lkname.disabled = true;
			var ins = document.getElementsByTagName("input");
			for(var i in ins)ins[i].disabled = true;
			ins = document.getElementsByTagName("img");
			for(var i in ins)ins[i].disabled = true;
			save.disabled = true;
			back.disabled = false;
		}
	}
}

function on_linkchange(itlk)
{
	with ( document.forms[0] ) 
	{
		sji_onchanged(document, itlk);
	}
}

function on_ctrlupdate()
{
	with ( document.forms[0] ) 
	{
		for(var i=0;i<links.length;i++){
			if(links[i].name==lkname.value){
				curlink=links[i];
				break;
			}
		}
		vpi.value=curlink.vpi;
		vci.value=curlink.vci;
		pppUsername.value=curlink.pppUsername;
		on_linkchange(curlink);
	}
}

/********************************************************************
**          on document submit
********************************************************************/
function on_submit() 
{
	with ( document.forms[0] ) 
	{
		//input text
/*star:20090302 START ppp username and password can be empty*/
		if(pppUsername.value!="" && sji_checkpppacc(pppUsername.value, 1, 25) == false)
		{
			pppUsername.focus();
			alert("用户名\"" + pppUsername.value + "\"为无效用户名或长度不在1-25字节之间，请重新输入！");
			return;
		}
		if(pppPassword.value!="" && sji_checkpppacc(pppPassword.value, 1, 25) == false)
		{
			pppPassword.focus();
			alert("密码存在无效字符或长度不在1-25字节之间，请重新输入！");
			return;
		}
		if(pppPassword.value != pppPassword_cfm.value)
		{
			pppPassword_cfm.focus();
			alert("密码确认和密码不一致，请重新输入！");
			return;
		}
		submit();
	}
}

function on_cancel()
{
	with ( document.forms[0] ) 
	{
		pppUsername.value = curlink.pppUsername;
		pppPassword.value = "";
		pppPassword_cfm.value = "";		
	}
}


</script>

</HEAD>
<!-------------------------------------------------------------------------------------->
<!--主页代码-->
<body topmargin="0" bgcolor="E0E0E0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init()">
	<DIV align="center" style="padding-left:5px; padding-top:5px">
		<form id="form" action="/boaform/admin/formAdslClt" method="post">
			<table width="80%"><tr><td><p align="left"><font size="+2"><b>ADSL宽带帐号配置</b></font></p></td></tr></table><br><br>
			<table id="cfg" cellspacing="0" cellpadding="0" width="256" align="center" border="0">
			  <tr valign="top">
				<td height="1">
				  <table height="200" cellspacing="0" cellpadding="0" width="100%" align="center" border="0">
					<tr>
					  <td vAlign=top>
						<table cellspacing="0" cellpadding="0" border="0" width="100%">
							<tr nowrap><td>连接名称：</td><td align="right"><select id="lkname" name="lkname" onChange="on_ctrlupdate();" size="1"></select></td></tr>
							<tr nowrap><td>帐号：</td><td align="right"><input type="text" name="pppUsername" maxlength="26" size="26" style="width:150px " readonly="1"></td></tr>
							<!--<tr nowrap><td>旧密码：</td><td align="right"><input type="password"name="oldpppPassword" maxlength="16" size="16" style="width:150px "></td></tr>-->
							<tr nowrap><td>新密码：</td><td align="right"><input type="password"name="pppPassword" maxlength="26" size="26" style="width:150px "></td></tr>
							<tr nowrap><td>确认新密码：</td><td align="right"><input type="password"name="pppPassword_cfm" maxlength="26" size="26" style="width:150px "></td></tr>
						</table>
					  </td>
					</tr>
				  </table>
				<table border="0" cellpadding="1" cellspacing="0" width="80%">
					<tr>
					<td align="center"><input type="button" name="back" value="后退" onClick="window.location.href='/autorun/e8cfg.asp';" style="width:80px; border-style:groove; font-weight:bold "></td>&nbsp;&nbsp;
					<td align="center"><input type="button" name="save" value="保存配置" onClick="on_submit();" style="width:80px; border-style:groove; font-weight:bold "></td>
					</tr>
				</table>
				</td>
			  </tr>
			</table>
			<input type="hidden" name="vpi" value="0">
			<input type="hidden" name="vci" value="0">
			<br>
			<input type="hidden" name="submit-url" value="/autorun/acccfg.asp">
		</form>
	</DIV>
</body>
</html>

