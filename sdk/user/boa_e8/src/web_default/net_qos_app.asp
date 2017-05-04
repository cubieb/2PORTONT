<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>QoS 业务</TITLE>
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

var appNames = new Array("", "VOIP", "TR069");
var rules = new Array();
<% initQosAppRulePage(); %>

/*
function on_chkstate(index)
{
	if(index < 0 || index >= rules.length)
		return; 
	rules[index].state = !rules[index].state;
//	rules[index].dirty = true;
}
*/

function on_chkclick(index)
{
	if(index < 0 || index >= rules.length)
		return; 
	rules[index].select = !rules[index].select;
}

function on_onedit(index)
{
	if(index < 0 || index >= rules.length)
		return;
	window.location.href = "net_qos_app_edit.asp?rule=" + rules[index].enc();
}

/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	if(lstrc.rows){while(lstrc.rows.length > 1) lstrc.deleteRow(1);}
	for(var i = 0; i < rules.length; i++)
	{

		//var bcheck = " ";
		var row = lstrc.insertRow(i + 1);
		var strprio = "";
		row.nowrap = true;
		row.vAlign = "center";
		row.align = "left";

		var cell = row.insertCell(0);
		cell.innerHTML = rules[i].index;
		cell = row.insertCell(1);
		cell.innerHTML = appNames[rules[i].appName];
		cell = row.insertCell(2);		
		cell.innerHTML = rules[i].prio;
		
		cell = row.insertCell(3);
		cell.align = "center";
		cell.innerHTML = "<input type=\"checkbox\" onClick=\"on_chkclick(" + i + ");\">";
		cell = row.insertCell(4);
		cell.align = "center";
		cell.innerHTML = "<input type=\"button\" onClick=\"on_onedit(" + i + ");\" value=\"编辑\">";		
		
	}
}

function rc2string(it)
{
	//return it.index + "," + Number(it.state) + "," + Number(it.select);	// + "|" + it.tporte + "|" + it.oprotocol + "|" + it.oportb + "|" + it.oporte; 
	return it.index + "," + Number(it.select);
}

/********************************************************************
**          on document submit
********************************************************************/
function on_submit() 
{
	var tmplst = "";
	var first = true;

	if (rules.length == 0)
		return;
	
	with ( document.forms[0] ) 
	{
		for(var i = 0; i < rules.length; i++)
		{
			if(first)
			{
				first = false;
			}
			else 
			{
				tmplst += "&";
			}
			tmplst += rc2string(rules[i]);
		}
		lst.value = tmplst;
		submit();
	}
}

</SCRIPT>
</HEAD>

<!-------------------------------------------------------------------------------------->
<!--主页代码-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
	<blockquote>
		<DIV align="left" style="padding-left:20px; padding-top:5px;">
			<form id="form" action="/boaform/admin/formQosAppRule" method="post">
				<b>QoS 业务</b>
				<br><br>
				<div class="tip" style="width:90% ">
					<font color="red">(在添加配置以后请点击"保存/应用"使配置生效)</font>
				</div>
				<br>
				<hr class="sep" size="1" width="90%">
				<br>
				<table class="flat" id="lstrc" border="1" cellpadding="0" cellspacing="1">				   
				   <tr class="hdb" align="center" nowrap>
						<td>序号</td>
						<td>业务名称</td>
						<td>业务阵列</td>
						<td>删除</td>
						<td>编辑</td>						
					</tr>
				</table>
				<br>
				<input type="button" class="button" onClick="location.href='net_qos_app_edit.asp';" value="添加">
				<input type="button" class="button" onClick="on_submit();" value="保存/应用">
				<input type="hidden" id="lst" name="lst" value="">
				<input type="hidden" name="submit-url" value="/net_qos_app.asp">
			</form>
		</DIV>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>
