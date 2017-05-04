<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>广域网访问设置</TITLE>
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
var rules = new Array();
with(rules){<% rteMacFilterList(); %>}

function on_chkclick(index)
{
	if(index < 0 || index >= rules.length)
		return;
	rules[index].select = !rules[index].select;
}
/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	sji_docinit(document);

	if(rulelst.rows)
	{
		while(rulelst.rows.length > 1)
			rulelst.deleteRow(1);
	}

	for(var i = 0; i < rules.length; i++)
	{
		var row = rulelst.insertRow(i + 1);

		row.nowrap = true;
		row.vAlign = "top";

		var cell = row.insertCell(0);
		cell.innerHTML = rules[i].devname;
		cell = row.insertCell(1);
		cell.innerHTML = rules[i].mac;
		cell = row.insertCell(2);
		cell.innerHTML = "<input type=\"checkbox\" onClick=\"on_chkclick(" + i + ");\">";
	}

	if(rules.length == 0)
	{
		form.remove.disabled = true;
	}
}

function addClick()
{
   var loc = "secu_macfilter_router_add.asp";
   var code = "window.location.href=\"" + loc + "\"";
   eval(code);
}

function removeClick()
{
	with ( document.forms[0] )
	{
		form.bcdata.value = sji_encode(rules, "select");
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
		<form id="form" action=/boaform/admin/formRteMacFilter method=POST name="form">
			<b>路由模式下的MAC地址过滤 -- 最多可以配置 16 条规则.</b><br>
			<hr align="left" class="sep" size="1" width="90%">
			<table id="rulelst" class="flat" border="1" cellpadding="2" cellspacing="0">
			   <tr class="hd" align="center">
				  <td>局域网设备名</td>
				  <td>MAC</td>
				  <td>移除</td>
			   </tr>
			</table>
			<br>
			<hr align="left" class="sep" size="1" width="90%">
			<input type="button" name="add" class="button" onClick="addClick()" value="添加">
			<input type="button" name="remove" class="button" onClick="removeClick()" value="删除">
			<input type="hidden" name="action" value="rm">
			<input type="hidden" name="bcdata" value="le">
			<input type="hidden" name="submit-url" value="">
		</form>
		</div>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>
