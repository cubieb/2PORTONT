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

var protos = new Array("TCP/UDP", "TCP", "UDP");
var rcs = new Array();
with(rcs){<% PortTriggerList(); %>}

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
	
	if(lstrc.rows){while(lstrc.rows.length > 3) lstrc.deleteRow(3);}
	for(var i = 0; i < rcs.length; i++)
	{
		var row = lstrc.insertRow(i + 3);
		
		row.nowrap = true;
		row.vAlign = "top";
		row.align = "center";
		var cell = row.insertCell(0);
		cell.innerHTML = rcs[i].appName;
		cell = row.insertCell(1);
		cell.innerHTML = protos[rcs[i].trigProto];
		cell = row.insertCell(2);
		cell.innerHTML = rcs[i].trigStartPort;
		cell = row.insertCell(3);
		cell.innerHTML = rcs[i].trigEndPort;
		cell = row.insertCell(4);
		cell.innerHTML = protos[rcs[i].openProto];
		cell = row.insertCell(5);
		cell.innerHTML = rcs[i].openStartPort;
		cell = row.insertCell(6);
		cell.innerHTML = rcs[i].openEndPort;
		cell = row.insertCell(7);
		cell.align = "center";
		cell.innerHTML = "<input type=\"checkbox\" onClick=\"on_chkclick(" + i + ");\">";
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
			<form id="form" action="/boaform/admin/formPortTrigger" method="post">
				<b>NAT -- 端口触发设置</b>
				<br><br>
				<div class="tip" style="width:90% ">
					&nbsp;&nbsp;&nbsp;&nbsp;一些应用要求路由器防火墙的指定端口开放,以供远程访问.
					当局域网的一个应用通过使用"<b>触发端口</b>",将TCP/UDP初始连接到一个远程用户时,端口触发动态的打开防火墙的"<b>开放端口</b>".
					路由器允许来自广域网的远程用户建立一个新的连接,通过使用"<b>开放端口</b>"返回局域网的应用程序.
				</div>
				<br>
				<b>注：最多可以输入32个配置.</b><br>
				<hr align="left" class="sep" size="1" width="90%">
				<br>				
				<table class="flat" id="lstrc" border="1" cellpadding="0" cellspacing="1">
				   <tr class="hdb" align="center" nowrap>
						<td>应用程序</td>
						<td colspan=3>触发</td>
						<td colspan=3>打开</td>
						<td>删除</td>
					</tr>
				   <tr class="hdb" align="center" nowrap>
						<td>名字</td>
						<td>协议</td>
						<td colspan=2>端口范围</td>
						<td>协议</td>
						<td colspan=2>端口范围</td>
						<td>&nbsp;</td>
					</tr>
				   <tr class="hdb" align="center" nowrap>
						<td>&nbsp;</td>
						<td width="60px">&nbsp;</td>
						<td width="60px">初始</td>
						<td width="60px">终止</td>
						<td width="60px">&nbsp;</td>
						<td width="60px">初始</td>
						<td width="60px">终止</td>
						<td>&nbsp;</td>
					</tr>
				</table>
				<br>
				<input type="button" class="button" onClick="location.href='app_nat_porttrig_add.asp';" value="添加">
				<input type="button" class="button" onClick="on_submit();" value="删除">
				<input type="hidden" id="action" name="action" value="rm">
				<input type="hidden" name="bcdata" value="le">
				<input type="hidden" name="submit-url" value="">
			</form>
		</DIV>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>
