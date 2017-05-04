<!-- add by liuxiao 2008-02-15 -->
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
<head>
<title>中国电信</title>
<meta http-equiv=pragma content=no-cache>
<meta http-equiv=cache-control content="no-cache, must-revalidate">
<meta http-equiv=content-type content="text/html; charset=gbk">
<meta http-equiv=content-script-type content=text/javascript>
<!--系统公共css-->
<style type=text/css>
@import url(/style/default.css);
</style>
<style>
SELECT {width:150px;}
</style>
<!--系统公共脚本-->
<script language="javascript" src="/common.js"></script>
<script language="javascript" type="text/javascript">
var curlink = null;
var cgi = new Object();
var links = new Array();
with(links){<% initPageAdsl(); %>}

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
</head>
<!--主页代码-->
<body onLoad="on_init();">
<form id="form" action="/boaform/admin/formAdslClt" method="post" style="border:0; padding:0; ">
	<br>
	<table id="cfg" cellspacing="0" cellpadding="0" width="256" align="center" border="0">
	  <tr valign="top">
		<td height="1">
		  <table height="200" cellspacing="0" cellpadding="0" width="100%" align="center" border="0">
			<tr>
			  <td vAlign=top>
				<table cellspacing="0" cellpadding="0" border="0" width="100%">
					<tr nowrap><td>连接名称：</td><td align="right"><select id="lkname" name="lkname" onChange="on_ctrlupdate();" size="1"></select></td></tr>
					<tr nowrap><td>账号：</td><td align="right"><input type="text" name="pppUsername" maxlength="26" size="26" style="width:150px "></td></tr>
					<tr nowrap><td>新密码：</td><td align="right"><input type="password"name="pppPassword" maxlength="26" size="26" style="width:150px "></td></tr>
					<tr nowrap><td>确认新密码：</td><td align="right"><input type="password"name="pppPassword_cfm" maxlength="26" size="26" style="width:150px "></td></tr>
				</table>
			  </td>
			</tr>
			<tr>
			  <td height="1">
				<table height="35" cellspacing="0" cellpadding="0" width="100%" border="0">
				  <tr>
					<td bgcolor="#427594" align="center"><img width="80" height="23" src="/image/apply.gif" border="0" onClick="on_submit();" style="cursor:pointer "></td>
					<td bgcolor="#427594" align="center"><img width="80" height="23" src="/image/cancel.gif" border="0" onClick="on_cancel();" style="cursor:pointer "></td>
				  </tr>
				</table>
			  </td>
			</tr>
		  </table>
		</td>
	  </tr>
	</table>
	<input type="hidden" name="vpi" value="0">
	<input type="hidden" name="vci" value="0">
	<input type="hidden" name="submit-url" value="/ehomeclient/e8clientusermgm.asp">
</form>
</body>
<%addHttpNoCache();%>
</html>
