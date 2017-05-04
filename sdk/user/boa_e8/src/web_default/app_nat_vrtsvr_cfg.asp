<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>虚拟服务器配置</TITLE>
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

var protos = new Array( "TCP/UDP", "TCP", "UDP");
var rcs = new Array();
with(rcs){<% virtualSvrList(); %>}

function on_chkclick(index)
{
	if(index < 0 || index >= rcs.length)
		return; 
	rcs[index].select = !rcs[index].select;
}

/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	sji_docinit(document);
	
	if(lstrc.rows){while(lstrc.rows.length > 1) lstrc.deleteRow(1);}
	for(var i = 0; i < rcs.length; i++)
	{
		var row = lstrc.insertRow(i + 1);
		
		row.nowrap = true;
		row.vAlign = "top";
		row.align = "center";
		var cell = row.insertCell(0);
		cell.innerHTML = rcs[i].svrName;
		cell = row.insertCell(1);
		cell.innerHTML = rcs[i].wanStartPort;
		cell = row.insertCell(2);
		cell.innerHTML = rcs[i].wanEndPort;
		cell = row.insertCell(3);
		cell.innerHTML = protos[rcs[i].protoType];
		cell = row.insertCell(4);
		cell.innerHTML = rcs[i].serverIp;
		cell = row.insertCell(5);
		cell.innerHTML = rcs[i].lanPort;
		cell = row.insertCell(6);
		cell.align = "center";
		cell.innerHTML = "<input type=\"checkbox\" name=\"rml"
				+ i + "\" value=\"ON\" onClick=\"on_chkclick(" + i + ");\">";
	}
}

/********************************************************************
**          on document submit
********************************************************************/
function on_submit() 
{
	with ( document.forms[0] ) 
	{
		form.bcdata.value = sji_encode(rcs, "select");
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
			<form id="form" action="/boaform/admin/formVrtsrv" method="post">
				<b>NAT --虚拟服务器设置</b>
				<div class="tip" style="width:90% ">
					<br><br>虚拟服务器是使用lan侧的一台设备作为服务器，远程的设备从wan侧流入直接访问该服务器，最大只能输入32个配置.<br><br>
				</div>
				<hr align="left" class="sep" size="1" width="90%">
				<br>				
				<table class="flat" id="lstrc" border="1" cellpadding="0" cellspacing="1">
				   <tr class="hdb" align="center" nowrap>
					  <td>服务器名</td>
					  <td width="90px">外部初始端口</td>
					  <td width="90px">外部终止端口</td>
					  <td width="60px">协议</td>
					  <td width="120px">服务器IP地址</td>
					  <td width="90px">源端口</td>
					  <td width="40px">删除</td>
				   </tr>
				</table>
				<br>
				<input type="button" class="button" onClick="location.href='app_nat_vrtsvr_add.asp';" value="添加">
				<input type="button" class="button" onClick="on_submit();" value="删除">
				<input type="hidden" id="action" name="action" value="delete">
				<input type="hidden" name="bcdata" value="le">
				<input type="hidden" name="submit-url" value="">
			</form>
		</DIV>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>
