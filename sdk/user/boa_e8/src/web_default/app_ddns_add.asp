<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>添加动态DNS</TITLE>
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
var links = new Array();
with(links){<% listWanif("rt"); %>}

/********************************************************************
**          on document load
********************************************************************/
function ddnsChange()
{
	with(form)
	{
		switch(provider.value)
		{
			case "oray":
			document.getElementById("orayInfo").style.display = "block";
			document.getElementById("dyndnsInfo").style.display = "none";
			document.getElementById("gnudipInfo").style.display = "none";
			break;
			case "dyndns":
			document.getElementById("orayInfo").style.display = "none";
			document.getElementById("dyndnsInfo").style.display = "block";
			document.getElementById("gnudipInfo").style.display = "none";
			break;
			case "gnudip":
			document.getElementById("orayInfo").style.display = "none";
			document.getElementById("dyndnsInfo").style.display = "none";
			document.getElementById("gnudipInfo").style.display = "block";
			break;
		}
	}
}

function on_init()
{
	with ( document.forms[0] )
	{
		for(var i = 0; i < links.length; i++)
		{
			ifname.options.add(new Option(links[i].name, links[i].ifIndex));
		}
	}

	ddnsChange();
}

function btnApply()
{
	if(!sji_checkhostname(form.hostname.value, 1, 32))
	{
		alert("请输入合法的主机名！");
		return false;
	}

	var ddns = form.provider.value;

	if(ddns == "oray")
	{
		if(!sji_checkusername(form.orayusername.value, 0, 32))
		{
			alert("请输入合法的用户名！");
			return false;
		}
		if(!sji_checkpswnor(form.oraypassword.value, 0, 32))
		{
			alert("请输入合法的密码！");
			return false;
		}
	}
	else if(ddns == "dyndns")
	{
		if(!sji_checkusername(form.dynusername.value, 0, 32))
		{
			alert("请输入合法的用户名！");
			return false;
		}
		if(!sji_checkpswnor(form.dynpassword.value, 0, 32))
		{
			alert("请输入合法的密码！");
			return false;
		}
	}
	else
	{
		if(!sji_checkusername(form.gnudipusername.value, 0, 32))
		{
			alert("请输入合法的用户名！");
			return false;
		}
		if(!sji_checkpswnor(form.gnudippassword.value, 0, 32))
		{
			alert("请输入合法的密码！");
			return false;
		}
	}

	form.submit();
}
</SCRIPT>
</HEAD>

<!-------------------------------------------------------------------------------------->
<!--主页代码-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
	<blockquote>
		<DIV align="left" style="padding-left:20px; padding-top:5px">
			<form id="form" action=/boaform/admin/formDDNS method=POST>
				<b>添加动态DNS</b><br><br><br>
				本页允许你添加一个动态DNS地址。<br><br>
				<hr align="left" class="sep" size="1" width="90%">
				<table border="0" cellpadding="0" cellspacing="0">
					<tr>
						<td width="180">D-DNS提供者:</td>
						<td>
							<select name="provider" size="1" onChange="ddnsChange()" style="width:200px ">
								<option value="oray">花生壳(oray.com)</option>
								<option value="dyndns">DynDNS.org</option>
								<!--<option value="gnudip">GnuDIP</option>-->
							</select>
						</td>
					</tr>
				</table>
				<br>
				<table border="0" cellpadding="0" cellspacing="0">
				<tr>
					<td width="180">主机名:</td>
					<td><input type="text" name="hostname" style="width:200px "></td>
				</tr>
				<tr>
					<td>接口:</td>
					<td>
						<select name="ifname" style="width:200px ">
							<option value="LAN">LAN/br0</option>
						</select>
					</td>
				</tr>
				</table>
				<br>
				<div id="orayInfo">
					<table border="0" cellpadding="0" cellspacing="0">
						<tr>
							<td><b>花生壳设置<b></td>
						</tr>
						<tr>
							<td width="180">用户名:</td>
							<td><input type="text" name="orayusername" size="20" maxlen="64" style="width:200px "></td>
						</tr>
						<tr>
							<td>密码:</td>
							<td><input type="password" name="oraypassword" style="width:200px "></td>
						</tr>
					</table>
				</div>
				<div id="dyndnsInfo">
					<table border="0" cellpadding="0" cellspacing="0">
						<tr>
							<td><b>DynDNS设置<b></td>
						</tr>
						<tr>
							<td width="180">用户名:</td>
							<td><input type="text" name="dynusername" size="20" maxlen="64" style="width:200px "></td>
						</tr>
						<tr>
							<td>密码:</td>
							<td><input type="password" name="dynpassword" style="width:200px "></td>
						</tr>
					</table>
				</div>
				<div id="gnudipInfo">
					<table border="0" cellpadding="0" cellspacing="0">
						<tr>
							<td><b>GnuDIP设置<b></td>
						</tr>
						<tr>
							<td width="180">用户名:</td>
							<td><input type="text" name="gnudipusername" size="20" maxlen="64" style="width:200px "></td>
						</tr>
						<tr>
							<td>密码:</td>
							<td><input type="password" name="gnudippassword" style="width:200px "></td>
						</tr>
					</table>
				</div>
				<br>
				<hr align="left" class="sep" size="1" width="90%">
				<input type="button" class="button" value="保存/应用" onClick="btnApply()">
				<input type="hidden" id="action" name="action" value="ad">
				<input type="hidden" value="/app_ddns_show.asp" name="submit-url">
			</form>
		</div>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>
