<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<html>
<head>
<title>动态DNS</title>
<meta http-equiv=pragma content=no-cache>
<meta http-equiv=cache-control content="no-cache, must-revalidate">
<meta http-equiv=content-type content="text/html; charset=gbk">
<meta http-equiv=content-script-type content=text/javascript>
<!--系统公共css-->
<style type=text/css>
@import url(/style/default.css);
</style>
<!--系统公共脚本-->
<script language="javascript" src="common.js"></script>
<script language="javascript" type="text/javascript">
var cgi = new Object();
var rules = new Array();
with(rules){<% showDNSTable(); %>}

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

	if(rulelst.rows)
	{
		while(rulelst.rows.length > 1){rulelst.deleteRow(1);}
	}
	for(var i = 0; i < rules.length; i++)
	{
		var row = rulelst.insertRow(i + 1);

		row.nowrap = true;
		row.vAlign = "top";

		var cell = row.insertCell(0);
		cell.innerHTML = rules[i].hostname;
		cell = row.insertCell(1);
		cell.innerHTML = rules[i].username;
		cell = row.insertCell(2);
		cell.innerHTML = rules[i].provider;
		cell = row.insertCell(3);
		cell.innerHTML = rules[i].ifname;
		cell = row.insertCell(4);
		cell.innerHTML = "<input type=\"checkbox\" onClick=\"on_chkclick(" + i + ");\">";
	}
	if(rules.length == 0)
	{
		form.remove.disabled = true;
	}
	if(cgi.ddnsEnable == false)
	{
		document.getElementById("ddnsConfig").style.display = "none";
		return true;
	}
}

function on_action(act)
{
	form.action.value = act;

	if (act == "sw" && !form.ddnsEnable.checked) {
		if (!confirm("您确定想要禁用动态DNS服务吗?")) {
			form.ddnsEnable.checked = true;
			return;
		}
	}

	if (act == "rm" && rules.length > 0) {
		form.bcdata.value = sji_encode(rules, "select");
	}

	with(form) {
		submit();
	}
}

</script>
</head>
<!-------------------------------------------------------------------------------------->
<!--主页代码-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
	<blockquote>
		<div align="left" style="padding-left:20px; padding-top:5px">
			<form id="form" action=/boaform/admin/formDDNS method=POST name="form">
				<b>动态DNS</b><br><br>
				动态DNS服务允许你将一个动态IP地址化名为任意多域中的一个静态主机名,允许您的路由器更容易被来自因特网的不同位置访问.<br><br>
				选择添加或移除来配置动态DNS.<br>
				<hr align="left" class="sep" size="1" width="90%">
				<input type="checkbox" name="ddnsEnable" onClick="on_action('sw')">启用动态DNS服务
				<br><br><br>
				<div id="ddnsConfig">
				<table id="rulelst" class="flat" border="1" cellpadding="4" cellspacing="0">
					<tr class="hd" align="center">
						<td>主机名</td>
						<td>用户名</td>
						<td>服务</td>
						<td>接口</td>
						<td>删除</td>
					</tr>
				</table>
				<hr align="left" class="sep" size="1" width="90%">
					<input type="button" class="button" name="add" onClick="window.location.assign('app_ddns_add.asp');" value="添加">
					<input type="button" class="button" name="remove" onClick="on_action('rm')" value="删除">
				</div>
				<input type="hidden" id="action" name="action" value="none">
				<input type="hidden" name="bcdata" value="le">
				<input type="hidden" name="submit-url" value="">
			</form>
		</div>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>
