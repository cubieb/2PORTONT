<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>虚拟服务器添加</TITLE>
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

var protos = new Array("TCP/UDP", "TCP", "UDP");
var root = new Array( 
	new it_mlo("Aim Talk", new Array(1, 4099, 4099, 1, 5191, 5191)),
	new it_mlo("Asheron's Call", new Array(2, 9000, 9013, 2, 9000, 9013)),
	new it_mlo("Calista IP Phone", new Array(1, 5190, 5190, 2, 3000, 3000)),
	new it_mlo("Delta Force (Client/Server)", new Array(2, 3568, 3568, 0, 3100, 3999)),
	new it_mlo("ICQ", new Array(2, 4000, 4000, 1, 20000, 20059)),
	new it_mlo("Napster", new Array(1, 6699, 6699, 1, 6699, 6699),
		new Array(1, 6699, 6699, 1, 6697, 6697),
		new Array(1, 6699, 6699, 1, 4444, 4444),
		new Array(1, 6699, 6699, 1, 5555, 5555),
		new Array(1, 6699, 6699, 1, 6666, 6666),
		new Array(1, 6699, 6699, 1, 7777, 7777),
		new Array(1, 6699, 6699, 1, 8888, 8888)),
	new it_mlo("Net2Phone", new Array(2, 6801, 6801, 2, 6801, 6801)),
	new it_mlo("QuickTime 4 Client", new Array(1, 554, 554, 2, 6970, 32000),
		new Array(1, 554, 554, 0, 6970,7000)),
	new it_mlo("Rainbow Six/Rogue Spear", new Array(1, 2346, 2346, 0, 2436, 2438))
	);

/********************************************************************
**          on document load
********************************************************************/
var numToConfig =0;
function on_init()
{
	for(var i = 0; i < root.length; i++)
	{
		form.appName.options.add(new Option(root[i].name, root[i].name));
	}
	<%portTriggerLeft();%>
	on_mode();
}

/********************************************************************
**          on document submit
********************************************************************/
function on_submit() 
{
	var arrsubmit = new Array();
	var strname = "";
	var numReady = 0;
	/*ql:20080716 START: record the num of configured rule.*/
	var ruleNum = 0;
	/*ql:20080716 END*/
	
	with ( document.forms[0] ) 
	{
		if(radiosrv[0].checked == true && appName.selectedIndex == 0)
		{
			appName.focus();
			alert("请选择一个服务！");
			return;
		}
		if(radiosrv[1].checked == true && sji_checkstrnor(cusSrvName.value, 1, 60) == false)
		{
			cusSrvName.focus();
			alert("自定义服务器：" + cusSrvName.value + "为无效名称，请重新输入！");
			return;
		}
		strname = ((radiosrv[0].checked == true) ? appName.value : cusSrvName.value);
		for(var i = 0; i < trigStartPort.length; i++) 
		{
			if(trigStartPort[i].value == "")continue;
			/*ql:20080716 START: record the num of configured rule.*/
			ruleNum++;
			/*ql:20080716 END*/
			if(sji_checkdigitrange(trigStartPort[i].value, 1, 65535) == false)
			{
				trigStartPort[i].focus();
				alert("初始触发端口\"" + trigStartPort[i].value + "\"为无效端口，请重新输入！");
				return;
			}
			if(sji_checkdigitrange(trigEndPort[i].value, parseInt(trigStartPort[i].value, 10), 65535) == false)
			{
				trigEndPort[i].focus();
				alert("终止触发端口\"" + trigEndPort[i].value + "\"为无效端口，请重新输入！");
				return;
			}
			if(sji_checkdigitrange(openStartPort[i].value, 1, 65535) == false)
			{
				openStartPort[i].focus();
				alert("初始开放端口\"" + openStartPort[i].value + "\"为无效端口，请重新输入！");
				return;
			}
			if(sji_checkdigitrange(openEndPort[i].value, parseInt(openStartPort[i].value, 10), 65535) == false)
			{
				openEndPort[i].focus();
				alert("终止开放端口\"" + openEndPort[i].value + "\"为无效端口，请重新输入！");
				return;
			}
			numReady++;
            		if(numReady > numToConfig)
            		{
		                alert("超过最大可设规则数,请先删除多余规则!");
		                return;
		       }
			var itrc = new it_nr("", 
			new it("appName", strname),
			new it(trigStartPort[i].name, parseInt(trigStartPort[i].value, 10)),
			new it(trigEndPort[i].name, parseInt(trigEndPort[i].value, 10)),
			new it(trigProto[i].name, parseInt(trigProto[i].value, 10)),
			new it(openStartPort[i].name, parseInt(openStartPort[i].value, 10)),
			new it(openEndPort[i].name, parseInt(openEndPort[i].value, 10)),
			new it(openProto[i].name, parseInt(openProto[i].value, 10))
			);
			itrc.select = true;
			arrsubmit.push(itrc);
		}
		/*ql:20080716 START: if no rule configured, just do nothing, dont submit.*/
		if (ruleNum == 0)
		{
			alert("请至少设置一条规则!");
			return;
		}
		/*ql:20080716 END*/
		form.bcdata.value = sji_encode(arrsubmit, "select");
		submit();
	}
}

function on_change(index)
{
	if(index < 0 || index > root.length)return false;
	if(index == 0)
	{
		reset_all();
		return;
	}
	index--;
	
	reset_all(root[index].childs.length);
	
	with (form) 
	{
		for(var i = 0; i < root[index].childs.length; i++) 
		{
			trigProto[i].value = root[index].childs[i][0];
			trigStartPort[i].value = root[index].childs[i][1];
			trigEndPort[i].value = root[index].childs[i][2];
			openProto[i].value = root[index].childs[i][3];
			openStartPort[i].value = root[index].childs[i][4];
			openEndPort[i].value = root[index].childs[i][5];
		}
	}
}

function reset_all(start)
{
	if(typeof start == "undefined")start = 0;
	with (form) 
	{
		for(var i = start; i < trigStartPort.length; i++) 
		{
			trigStartPort[i].value = trigEndPort[i].value = openStartPort[i].value = openEndPort[i].value = "";
			trigProto[i].value = 0;
			openProto[i].value = 0;
		}
	}
}

function on_mode()
{
	if (form.radiosrv[0].checked == true)
	{
		form.appName.disabled = false;
		form.cusSrvName.disabled = true;
	}
	else
	{
		form.appName.disabled = true;
		form.cusSrvName.disabled = false;
	}
	//reset_all();
	form.appName.selectedIndex = 0;
}

</SCRIPT>
</HEAD>

<!-------------------------------------------------------------------------------------->
<!--主页代码-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
	<blockquote>
		<DIV align="left" style="padding-left:20px; padding-top:5px">
			<form id="form" action="/boaform/admin/formPortTrigger" method="post">
				<b>NAT -- 端口触发</b>
				<br><br>
				<div class="tip" style="width:90% ">
					&nbsp;&nbsp;&nbsp;&nbsp;一些应用，例如游戏，视频会
				  议，远程访问应用等，要求路由器防火墙的特定端口为应用程序的访问开通。在本页中,您可以通过选择已存在的应用程序或创建您自己的应用程序(客户端应用)来配置端口设置，点击"保存/应用"添加设置。
				</div>
				<br>
				<hr align="left" class="sep" size="1" width="90%">
				<br>
				<table border="0" cellpadding="0" cellspacing="0">
					<tr>
						<td><b>可配置的剩余数:</b></td><td><b><label id="numleft">4</label></b></td>
					</tr>
				</table>
				</br>
				<table border="0" cellpadding="0" cellspacing="0">
				   <tr>
					  <td colspan="2">服务器名:</td>
				   </tr>
				   <tr>
					  <td align="right"><input type="radio" name="radiosrv" onclick="on_mode()" checked>&nbsp;&nbsp;选择一项服务:&nbsp;&nbsp;</td>
					  <td><select name="appName" size="1" onChange="on_change(this.selectedIndex);" style="width:380px ">
							<option value="none">Select One</option>
						</select>
					  </td>
				   </tr>
				   <tr>
					  <td align="right"><input type="radio" name="radiosrv" onclick="on_mode()">&nbsp;&nbsp;自定义服务器:&nbsp;&nbsp;</td>
					  <td><input type="text" size="45" name="cusSrvName" style="width:380px "></td>
				   </tr>
				</table>
				</br>
				<table id="lstrule" class="flat" border="1" cellpadding="0" cellspacing="0">
				   <tr class="hdb" align="center">
					  <td>初始触发端口 </td>
					  <td>终止触发端口</td>
					  <td width="80px">触发协议</td>                  
					  <td>初始开放端口</td>
					  <td>终止开放端口</td>
					  <td width="80px">开放协议</td>
				   </tr>
				   <tr>
					  <td><input type="text" size="7" name="trigStartPort" style="width=100% "></td>
					  <td><input type="text" size="7" name="trigEndPort" style="width=100% "></td>
					  <td><select name="trigProto" style="width=100% ">
							<option value="0">
							TCP/UDP
							<option value="1" selected>
							TCP
							<option value="2">
							UDP
						 </select></td>
					  <td><input type="text" size="7" name="openStartPort" style="width=100% "></td>
					  <td><input type="text" size="7" name="openEndPort" style="width=100% "></td>
					  <td><select name="openProto" style="width=100% ">
							<option value="0">
							TCP/UDP
							<option value="1" selected>
							TCP
							<option value="2">
							UDP
						 </select></td>
					</tr>
				   <tr>
					  <td><input type="text" size="7" name="trigStartPort" style="width=100% "></td>
					  <td><input type="text" size="7" name="trigEndPort" style="width=100% "></td>
					  <td><select name="trigProto" style="width=100% ">
							<option value="0">
							TCP/UDP
							<option value="1" selected>
							TCP
							<option value="2">
							UDP
						 </select></td>
					  <td><input type="text" size="7" name="openStartPort" style="width=100% "></td>
					  <td><input type="text" size="7" name="openEndPort" style="width=100% "></td>
					  <td><select name="openProto" style="width=100% ">
							<option value="0">
							TCP/UDP
							<option value="1" selected>
							TCP
							<option value="2">
							UDP
						 </select></td>
					</tr>
				   <tr>
					  <td><input type="text" size="7" name="trigStartPort" style="width=100% "></td>
					  <td><input type="text" size="7" name="trigEndPort" style="width=100% "></td>
					  <td><select name="trigProto" style="width=100% ">
							<option value="0">
							TCP/UDP
							<option value="1" selected>
							TCP
							<option value="2">
							UDP
						 </select></td>
					  <td><input type="text" size="7" name="openStartPort" style="width=100% "></td>
					  <td><input type="text" size="7" name="openEndPort" style="width=100% "></td>
					  <td><select name="openProto" style="width=100% ">
							<option value="0">
							TCP/UDP
							<option value="1" selected>
							TCP
							<option value="2">
							UDP
						 </select></td>
					</tr>
				   <tr>
					  <td><input type="text" size="7" name="trigStartPort" style="width=100% "></td>
					  <td><input type="text" size="7" name="trigEndPort" style="width=100% "></td>
					  <td><select name="trigProto" style="width=100% ">
							<option value="0">
							TCP/UDP
							<option value="1" selected>
							TCP
							<option value="2">
							UDP
						 </select></td>
					  <td><input type="text" size="7" name="openStartPort" style="width=100% "></td>
					  <td><input type="text" size="7" name="openEndPort" style="width=100% "></td>
					  <td><select name="openProto" style="width=100% ">
							<option value="0">
							TCP/UDP
							<option value="1" selected>
							TCP
							<option value="2">
							UDP
						 </select></td>
					</tr>
				   <tr>
					  <td><input type="text" size="7" name="trigStartPort" style="width=100% "></td>
					  <td><input type="text" size="7" name="trigEndPort" style="width=100% "></td>
					  <td><select name="trigProto" style="width=100% ">
							<option value="0">
							TCP/UDP
							<option value="1" selected>
							TCP
							<option value="2">
							UDP
						 </select></td>
					  <td><input type="text" size="7" name="openStartPort" style="width=100% "></td>
					  <td><input type="text" size="7" name="openEndPort" style="width=100% "></td>
					  <td><select name="openProto" style="width=100% ">
							<option value="0">
							TCP/UDP
							<option value="1" selected>
							TCP
							<option value="2">
							UDP
						 </select></td>
					</tr>
				   <tr>
					  <td><input type="text" size="7" name="trigStartPort" style="width=100% "></td>
					  <td><input type="text" size="7" name="trigEndPort" style="width=100% "></td>
					  <td><select name="trigProto" style="width=100% ">
							<option value="0">
							TCP/UDP
							<option value="1" selected>
							TCP
							<option value="2">
							UDP
						 </select></td>
					  <td><input type="text" size="7" name="openStartPort" style="width=100% "></td>
					  <td><input type="text" size="7" name="openEndPort" style="width=100% "></td>
					  <td><select name="openProto" style="width=100% ">
							<option value="0">
							TCP/UDP
							<option value="1" selected>
							TCP
							<option value="2">
							UDP
						 </select></td>
					</tr>
				   <tr>
					  <td><input type="text" size="7" name="trigStartPort" style="width=100% "></td>
					  <td><input type="text" size="7" name="trigEndPort" style="width=100% "></td>
					  <td><select name="trigProto" style="width=100% ">
							<option value="0">
							TCP/UDP
							<option value="1" selected>
							TCP
							<option value="2">
							UDP
						 </select></td>
					  <td><input type="text" size="7" name="openStartPort" style="width=100% "></td>
					  <td><input type="text" size="7" name="openEndPort" style="width=100% "></td>
					  <td><select name="openProto" style="width=100% ">
							<option value="0">
							TCP/UDP
							<option value="1" selected>
							TCP
							<option value="2">
							UDP
						 </select></td>
					</tr>
				   <tr>
					  <td><input type="text" size="7" name="trigStartPort" style="width=100% "></td>
					  <td><input type="text" size="7" name="trigEndPort" style="width=100% "></td>
					  <td><select name="trigProto" style="width=100% ">
							<option value="0">
							TCP/UDP
							<option value="1" selected>
							TCP
							<option value="2">
							UDP
						 </select></td>
					  <td><input type="text" size="7" name="openStartPort" style="width=100% "></td>
					  <td><input type="text" size="7" name="openEndPort" style="width=100% "></td>
					  <td><select name="openProto" style="width=100% ">
							<option value="0">
							TCP/UDP
							<option value="1" selected>
							TCP
							<option value="2">
							UDP
						 </select></td>
					</tr>
				</table>
				<br>
				<hr align="left" class="sep" size="1" width="90%">
				<input type="button" class="button" onClick="on_submit()" value="保存/应用">
				<input type="hidden" id="action" name="action" value="ad">
				<input type="hidden" name="bcdata" value="le">
				<input type="hidden" name="submit-url" value="/app_nat_porttrig_show.asp">
			</form>
		</DIV>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>
