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
var rules = new Array();
var act_idx = -1;
with(rules){<% initPageSleepModeRule(); %>}

var day_display = new Array("立即休眠/唤醒","星期日","星期一","星期二","星期三","星期四","星期五","星期六");

function on_chkclick(index)
{
	if(index < 0 || index >= rules.length)
		return;
	act_idx=index;
	//alert(act_idx);
}

/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	sji_docinit(document, cgi);

	if(cgi.bandcontrolEnable == false)
	{
		document.getElementById("table_list").style.display="none";
		document.getElementById("act_btn").style.display="none";
	}
	
	document.getElementById("input_div").style.display="none";

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
		cell.innerHTML = day_display[rules[i].day];

		cell = row.insertCell(1);
		
		if(rules[i].day == 0)
			cell.innerHTML = "--";
		else
			cell.innerHTML = rules[i].hour + ":" + rules[i].minute;
		
		cell = row.insertCell(2);
		if(rules[i].enable==1)
			cell.innerHTML = "使能";
		else
			cell.innerHTML = "禁止";

		cell = row.insertCell(3);
		if(rules[i].onoff==1)
			cell.innerHTML = "休眠";
		else
			cell.innerHTML = "唤醒";
		
		cell = row.insertCell(4);
		cell.innerHTML = "<input type=\"radio\" name=\"act_select\" onClick=\"on_chkclick(" + i + ");\">";
	}
}

function addClick()
{
   var loc = "app_sleepmode_rule_add.asp";
   var code = "window.location.href=\"" + loc + "\"";
   eval(code);
}

function setValue(id,value)
{
	document.getElementById(id).value=value;
}

function modifyClick()
{
	if(act_idx == -1)
	{
		alert("请先选择要修改的规则!");
		return false;
	}
	document.getElementById("input_div").style.display="";

	var i=0;
	for(i=0; i<24; i++)
		document.form.hour.options.add(new Option(i, i));

	for(i=0; i<60; i++)
		document.form.minute.options.add(new Option(i, i));
	
	document.getElementById("day").value=rules[act_idx].day;
	document.getElementById("hour").value=rules[act_idx].hour;
	document.getElementById("minute").value=rules[act_idx].minute;

	if(rules[act_idx].enable==1){
		document.form.timerEnable[1].checked = true;
	}else{
		document.form.timerEnable[0].checked = true;
	}

	if(rules[act_idx].onoff==1){
		document.form.onoffEnable[1].checked = true;
	}else{
		document.form.onoffEnable[0].checked = true;
	}

	timeDisplay()
	return true;
}

function ModifyApply()
{
	setValue("idx", act_idx);
	setValue("action", "modify");
	return true;
}

function back2add()
{
	/*mean user cancel modify, refresh web page again!*/
	document.getElementById("input_div").style.display="none";
	
	setValue("idx",-1);
	getObj("form").submit();
}

function on_action(act)
{
	form.action.value = act;

	if(act == "rm")
	{
		//alert(act_idx);
		if(act_idx == -1)
		{
			alert("请先选择要删除的规则!");
			return false;
		}
		setValue("idx",act_idx);
		setValue("action", "del");
	}

	with(form)
	{
		submit();
	}
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
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
	<blockquote>
		<DIV align="left" style="padding-left:20px; padding-top:5px">
			<form id="form" action=/boaform/admin/formSleepMode method=POST name="form">
				<b>设置家庭网关定时休眠功能-- 最多允许支持 100条规则.</b><br><br>
				<div id="table_list">
				<hr align="left" class="sep" size="1" width="90%">
				<div align="left" style="padding-left:20px;"><br>
					<table id="rulelst" class="flat" border="1" cellpadding="2" cellspacing="0">
						<tr align="center" class="hd">
							<td width="120px">重复周期</td>
							<td width="120px">时间</td>
							<td width="120px">规则/禁止</td>
							<td width="120px">动作</td>
							<td>选择</td>
						</tr>
					</table>
				</div>
				</div>
				<br>
				<div id="input_div">
					<hr align="left" class="sep" size="1" width="90%">
					<tr>
						<td width="120">设置休眠时间:&nbsp;</td>
						<td>
							<select id="day" name="day" onchange="timeDisplay()">
  								<option value=0>立即休眠/唤醒</option>
 								<option value=1>星期一</option>
								<option value=2>星期二</option>
								<option value=3>星期三</option>
								<option value=4>星期四</option>
								<option value=5>星期五</option>
								<option value=6>星期六</option>
								<option value=7>星期日</option>
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
						<td><input type="radio" name="timerEnable" value="off" >&nbsp;&nbsp;禁止</td>
						<td><input type="radio" name="timerEnable" value="on" >&nbsp;&nbsp;启用</td>
					</tr>
					<br><br>
					<tr>
						<td width="120">动作:&nbsp;</td>
						<td><input type="radio" name="onoffEnable" value="off" >&nbsp;&nbsp;唤醒</td>
						<td><input type="radio" name="onoffEnable" value="on" >&nbsp;&nbsp;休眠</td>
					</tr>
					
					<br><br>
					<tr>
						<td  class="td2">
							<input type="submit" class="button2" value="保存" id="modify" onclick="return ModifyApply();" />
							<input name="back" type="button" id="back" value="取 消"  class="button2" onclick="back2add()"/>
						</td>
					</tr>
				</div>
				<br>
				<div id="act_btn">
				<hr align="left" class="sep" size="1" width="90%">
				<input type="button" class="button" name="add" onClick="addClick()" value="添加">
				<input type="button" class="button" name="modify" onClick="return modifyClick();" value="修改">
				<input type="button" class="button" name="remove" onClick="on_action('rm')" value="删除">
				</div>
				<input type="hidden" id="action" name="action" value="none">
				<input type="hidden" name="idx" id="idx" value="">
				<input type="hidden" name="submit-url" value="/app_sleepmode_rule.asp" >
			</form>
		</div>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>
