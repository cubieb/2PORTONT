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

var cgi = new Object();
var rules = new Array();
with(rules){<% initPageURL(); %>}

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
	sji_docinit(document, cgi);

	if(cgi.urlfilterEnble == false)
	{
		form.urlFilterMode[0].disabled = true;
		form.urlFilterMode[1].disabled = true;
	}

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
		row.align = "center";

		var cell = row.insertCell(0);
		cell.innerHTML = rules[i].url;
		cell.align = "left";
		cell = row.insertCell(1);
		//cell.innerHTML = rules[i].port;
		//cell = row.insertCell(2);
		cell.innerHTML = "<input type=\"checkbox\" onClick=\"on_chkclick(" + i + ");\">";
	}
}

function addClick()
{
   var loc = "secu_urlfilter_add.asp";
   var code = "window.location.href=\"" + loc + "\"";
   eval(code);
}

function on_action(act)
{
	form.action.value = act;

	if(act == "rm" && rules.length > 0)
	{
		//form.bcdata.value = sji_encode(rules, "select");
		form.bcdata.value = sji_idxencode(rules, "select", "idx");
	}

	with(form)
	{
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
			<form id="form" action=/boaform/admin/formURL method=POST name="form">
				<b>URL过滤 -- 最多允许您添加 100条规则.</b><br><br>
				<div id="rstip" style="display:none;"><font color="red">提示：本页面的设置，需要重启路由器才能生效！</font><br></div>
				<hr align="left" class="sep" size="1" width="90%">
				<table border="0" cellpadding="2" cellspacing="0">
					<tr>
						<td>URL过滤:</td>
						<td><input type="radio" name="urlfilterEnble" value="off" onClick="on_action('sw')">&nbsp;&nbsp;禁用</td>
						<td><input type="radio" name="urlfilterEnble" value="on" onClick="on_action('sw')">&nbsp;&nbsp;启用</td>
					</tr>
					<tr>
						<td>过滤模式:</td>
						<td><input type="radio" name="urlFilterMode" value="off" onClick="on_action('md')">&nbsp;&nbsp;黑名单</td>
						<td><input type="radio" name="urlFilterMode" value="on" onClick="on_action('md')">&nbsp;&nbsp;白名单</td>
					</tr>
				</table>
				<br><br><br>
				<table id="rulelst" class="flat" border="1" cellpadding="2" cellspacing="0">
					<tr align="center" class="hd">
						<td width="240px">URL地址</td>
						<!--<td>端口</td>-->
						<td>删除</td>
					</tr>
				</table>
				<br>
				<hr align="left" class="sep" size="1" width="90%">
				<input type="button" class="button" name="add" onClick="addClick()" value="添加">
				<input type="button" class="button" name="remove" onClick="on_action('rm')" value="删除">
				<input type="hidden" id="action" name="action" value="none">
				<input type="hidden" name="bcdata" value="le">
				<input type="hidden" name="submit-url" value="" >
			</form>
		</div>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>
