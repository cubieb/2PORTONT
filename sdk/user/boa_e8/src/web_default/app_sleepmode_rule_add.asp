<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>定时网关休眠</TITLE>
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

function btnApply()
{
	document.form.action.value="add";
	form.submit();
}

function on_init()
{
	var i=0;
	for(i=0; i<24; i++)
		document.form.hour.options.add(new Option(i, i));

	for(i=0; i<60; i++)
		document.form.minute.options.add(new Option(i, i));
}

function timeDisplay()
{
	var selc = document.getElementById("day");
	var index = selc.selectedIndex;

	if( selc.options[index].value==0 )
	{
		document.getElementById("hour").disabled=true;
		document.getElementById("minute").disabled=true;
	}
	else
	{
		document.getElementById("hour").disabled=false;
		document.getElementById("minute").disabled=false;
	}
}
</SCRIPT>
</HEAD>


<!-------------------------------------------------------------------------------------->
<!--主页代码-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init()">
  <blockquote>
	<DIV align="left" style="padding-left:20px; padding-top:5px">
		<form id="form" action=/boaform/admin/formSleepMode method=POST name="form">
			<b>添加家庭网关定时休眠规则-- 最多允许您添加 100条规则.</b><br><br><br>
			<hr align="left" class="sep" size="1" width="90%">
				<tr>
					<td width="120">设置休眠时间:&nbsp;</td>
					<td>
						<select id="day" name="day" onchange="timeDisplay()">
  							<option value=0>立即休眠/唤醒</option>
  							<option value=1>星期日</option>
 							<option value=2>星期一</option>
							<option value=3>星期二</option>
							<option value=4>星期三</option>
							<option value=5>星期四</option>
							<option value=6>星期五</option>
							<option value=7>星期六</option>
						</select>
					</td>
					<td>&nbsp;</td>
					<td>
						<select id="hour" name="hour">
						</select>
					</td>
					<td>时&nbsp;</td>
					<td>
						<select id="minute" name="minute">
						</select>
					</td>
					<td>分&nbsp;</td>
				</tr>
				<br><br>
				<tr>
					<td width="120">使能/禁止:&nbsp;</td>
					<td><input type="radio" name="timerEnable" value="off" checked>&nbsp;&nbsp;禁止</td>
					<td><input type="radio" name="timerEnable" value="on" >&nbsp;&nbsp;使能</td>
				</tr>
				<br><br>
				<tr>
					<td width="120">动作:&nbsp;</td>
					<td><input type="radio" name="onoffEnable" value="off" checked>&nbsp;&nbsp;唤醒</td>
					<td><input type="radio" name="onoffEnable" value="on" >&nbsp;&nbsp;休眠</td>
				</tr>
			<br><br>
			<hr align="left" class="sep" size="1" width="90%">
			<input type="button" class="button" value="保存/应用" onClick="btnApply()">
			<input type="hidden" name="action" value="add">
			<input type="hidden" name="submit-url" value="/app_sleepmode_rule.asp">

			<script>
				timeDisplay();
			</script>
		</form>
	</div>
  </blockquote>
</body>
<%addHttpNoCache();%></html>
