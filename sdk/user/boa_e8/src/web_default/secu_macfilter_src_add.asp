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

/********************************************************************
**          on document apply
********************************************************************/
function btnApply()
{
	/*
	if(form.devname.value == "")
	{
		alert("局域网设备名不能为空！");
		return false;
	}
	*/
	if(form.mac.value == "")
	{
		alert("mac 地址不能为空！");
		return false;
	}
	if(!sji_checkmac(form.mac.value))
	{
		alert("mac 地址错误！");
		return false;
	}
	for(var i = 0; i < rules.length; i++)
	{
		if(/*rules[i].name == form.devname.value ||*/ rules[i].mac == form.mac.value)
		{
			alert( "该规则已存在");//alert("That rule already exists");
			return false;
		}
	}
	form.submit();
}
</SCRIPT>
</HEAD>

<!-------------------------------------------------------------------------------------->
<!--主页代码-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000">
  <blockquote>
	<DIV align="left" style="padding-left:20px; padding-top:5px">
		<form id="form" action=/boaform/admin/formRteMacFilter method=POST name="form">
			<b>添加拒绝存取的 MAC 地址过滤规则</b><br><br><br>
			<!--本页将添加需过滤的已连接路由器的指定局域网设备的MAC地址。<br>
			在"局域网设备名"一栏中输入需限制的局域网设备名，在"MAC地址"一栏输入该设备的MAC地址。<br>
			进入命令窗口输入命令"ipconfig /all"来查看基于PC的MAC地址。<br> -->
			<hr align="left" class="sep" size="1" width="90%">
			<table border="0" cellpadding="0" cellspacing="0">
				<tr style="display:none">
					<td width="180">局域网设备名</td>
					<td><input type="text" name="devname" size="18" maxlength="31"></td>
				</tr>
				<tr>
					<td width="180">MAC地址(xx-xx-xx-xx-xx-xx)&nbsp;</td>
					<td><input type="text" name="mac" size="18"></td>
				</tr>
			</table>
			<hr align="left" class="sep" size="1" width="90%">
			<input type="button" class="button" value="保存/应用" onClick="btnApply()">
			<input type="hidden" name="action" value="ad">
			<input type="hidden" name="submit-url" value="/secu_macfilter_src.asp">
		</form>
	</div>
  </blockquote>
</body>
<%addHttpNoCache();%></html>
