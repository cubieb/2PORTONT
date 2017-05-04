<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<HTML>
<HEAD>
<TITLE>LED控制</TITLE>
<META http-equiv=pragma content=no-cache>
<META http-equiv=cache-control content="no-cache, must-revalidate">
<META http-equiv=content-type content="text/html; charset=gbk">
<META http-equiv=content-script-type content=text/javascript>
<STYLE type=text/css>
@import url(/style/default.css);
</STYLE>
<SCRIPT language="javascript" src="common.js"></SCRIPT>
<SCRIPT language="javascript" type="text/javascript">
var _enable = new Array();
var _controlCycle = new Array();
var _startTime = new Array();
var _endTime = new Array();
var ledsts;
function getObj(id)
{
	return(document.getElementById(id));
}

function setValue(id,value)
{
	document.getElementById(id).value=value;
}

function getValue(id)
{
	return(document.getElementById(id).value);
}

function getImage(src,strmethod,id)
{
	return ("<image id=\""+id+"\" onclick=\""+ strmethod +"\" src=\""+src+"\">");
}

function addline(index, enable, startTime, endTime, controlCycle)
{
	var newline;
	newline = document.getElementById('Schedule_Table').insertRow(-1);
	newline.nowrap = true;
	newline.vAlign = "top";
	newline.align = "center";
	newline.setAttribute("class","white");
	newline.setAttribute("className","white");
	newline.insertCell(-1).innerHTML = enable? "启用":"停用";
	newline.insertCell(-1).innerHTML = startTime;
	newline.insertCell(-1).innerHTML = endTime;
	newline.insertCell(-1).innerHTML = controlCycle;
	newline.insertCell(-1).innerHTML =getImage("image/edit.gif","Modify("+index+")","Btn_Modify"+index);
	newline.insertCell(-1).innerHTML =getImage("image/delete.gif","Delete("+index+")","Btn_Delete"+index);
}
function showTable()
{
	var num = _enable.length;
	if (num!=0) {
		for (var i=0; i<num; i++)
			addline(i, _enable[i], _startTime[i], _endTime[i], _controlCycle[i]);
	}
	else {
	}

	if(_ledsts=="1")
	{
		document.ledschedule.ledstatus[0].checked = true;
	}
	else
	{
		document.ledschedule.ledstatus[1].checked = true;
	}
		
	jslEnDisplay("Btn_Add");
	jslDiDisplay("Btn_Edit");
	jslDiDisplay("Btn_Back");
}

function onClickLEDSwitch()
{
	if (document.ledschedule.ledstatus[0].checked)
		document.ledschedule.ledSwitch.value = "enable";
	else
		document.ledschedule.ledSwitch.value = "disable";
	
	document.ledschedule.submit();
}

/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	showTable();
}

/********************************************************************
**          on document submit
********************************************************************/
function on_submit(edit) 
{
	if(pageCheckValue()){
		if(edit)
			getObj("action").value = 2;	//modify
		else
			getObj("action").value = 1;	//add
		getObj("ledchedule").submit();
	}
	else {}
}

function jslDiDisplay(id)
{
	var i;
	var num=arguments.length;
	if(num==0)return;
	for(i=0;i<num;i++){
		document.getElementById(arguments[i]).style.display="none";
	}
}

function jslEnDisplay(id)
{
	var i;
	var num=arguments.length;
	if(num==0)
		return;
	for(i=0;i<num;i++)
		document.getElementById(arguments[i]).style.display="";
}

function jslDisable(id)
{
	var i;
	var num=arguments.length;
	if(num==0)
		return;
	for(i=0;i<num;i++){
		document.getElementById(arguments[i]).disabled=true;
	}
}

function jslEnable(id)
{
	var i=0;
	var num=arguments.length;
	if(num==0)
		return;
	for(i=0;i<num;i++)
		document.getElementById(arguments[i]).disabled=false;
}

function Modify(i)
{
	var start,end;
	getObj("Frm_Active").checked = _enable[i];
	start = _startTime[i].split(":");
	getObj("startHour").value = start[0];
	getObj("startMin").value = start[1];
	end = _endTime[i].split(":");
	getObj("endHour").value = end[0];
	getObj("endMin").value = end[1];
	
	getObj("controlCycle").value = _controlCycle[i];

	getObj("if_index").value = i;

	jslDiDisplay("Btn_Add");
	jslEnDisplay("Btn_Edit");
	jslEnDisplay("Btn_Back");
	
}

function back4add()
{
	getObj("Frm_Active").checked = false;
	getObj("startHour").value = "";
	getObj("startMin").value = "";
	getObj("endHour").value = "";
	getObj("endMin").value = "";
	getObj("controlCycle").value = "";

	getObj("if_index").value = "";

	jslEnDisplay("Btn_Add");
	jslDiDisplay("Btn_Edit");
	jslDiDisplay("Btn_Back");
}

function Delete(i)
{
	if ( !confirm("您确定要删除?") ) {
		getObj("action").value = "";
		return false;
	}
	getObj("action").value=0;
	getObj("if_index").value = i;
	getObj("ledchedule").submit();
	return true;
}

function pageCheckValue()
{
	if(getObj("startHour").value==""){
		alert("时间不能为空");
		getObj("Frm_Start1").focus();
		return false;
	}
	if(!isNumber(getObj("startHour").value) || getObj("startHour").value<0 || getObj("startHour").value>23){
		alert("请输入0~23的数值");
		getObj("startHour").focus();
		return false;
	}
	if(getObj("startMin").value==""){
		alert("时间不能为空");
		getObj("startMin").focus();
		return false;
	}
	if(!isNumber(getObj("startMin").value) || getObj("startMin").value<0 || getObj("startMin").value>59){
		alert("请输入0~59的数值");
		getObj("startMin").focus();
		return false;
	}
	if(getObj("endHour").value==""){
		alert("时间不能为空");
		getObj("endHour").focus();
		return false;
	}
	if(!isNumber(getObj("endHour").value) || getObj("endHour").value<0 || getObj("endHour").value>23){
		alert("请输入0~23的数值");
		getObj("endHour").focus();
		return false;
	}
	if(getObj("endMin").value==""){
		alert("时间不能为空");
		getObj("endMin").focus();
		return false;
	}
	if(!isNumber(getObj("endMin").value) || getObj("endMin").value<0 || getObj("endMin").value>59){
		alert("请输入0~59的数值");
		getObj("endMin").focus();
		return false;
	}
	if(getObj("controlCycle").value==""){
		alert("时间不能为空");
		getObj("controlCycle").focus();
		return false;
	}
	if(!isNumber(getObj("controlCycle").value) || getObj("controlCycle").value<1 || getObj("controlCycle").value>31){
		alert("请输入1~31的数值");
		getObj("controlCycle").focus();
		return false;
	}
	return true;
}

</SCRIPT>
</HEAD>

<!-------------------------------------------------------------------------------------->
<!--主页代码-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
	<blockquote>
		<DIV align="left" style="padding-left:20px; padding-top:10px">
			<form id="ledchedule" action=/boaform/admin/formLedTimer method=POST name=ledschedule>
				<table border="0" cellpadding="3" cellspacing="0">
				<tr>
					<td>LED控制</td>
					<td >
					<input type="radio" value="1" name="ledstatus" onClick="onClickLEDSwitch()">打开&nbsp;&nbsp;
	    		 	<input type="radio" value="0" name="ledstatus" onClick="onClickLEDSwitch()">关闭
					</td>
				</tr>
				<tr>
					<td>
					<input type="hidden" id="ledSwitch" name="ledSwitch" value="">
					</td>
					<td >
					</td>
				</tr>
				<tr>
					<td>使能</td>
					<td ><input name="Fnt_Active" id="Frm_Active" type="checkbox" value="ON"/>
					</td>
				</tr>
				<tr>
					<td>LED打开时刻</td>
					<td>
					<input name="startHour" id="startHour" type="text"  size="2" maxlength="2" /> :
					<input name="startMin" id="startMin" type="text"  size="2" maxlength="2" />
					</td>
				</tr>
				<tr>
					<td>LED关闭时刻</td>
					<td>
					<input name="endHour" id="endHour" type="text"  size="2" maxlength="2" /> :
					<input name="endMin" id="endMin" type="text"  size="2" maxlength="2" />
					</td>
				</tr>
				<tr>
					<td>周期(天)</td>
					<td>
					<input name="controlCycle" id="controlCycle" type="text"  size="2" maxlength="2" />
					</td>
				</tr>
				</table>
				<table id="TestContent"  class="table" width="550px" border="0">
				<tr>
					<td class="td1"></td>
					<td class="td2">
					<input name="Btn_Add" type="button" id="Btn_Add" value="添加" onclick="on_submit(0)"/>
					<input name="Btn_Edit" type="button" id="Btn_Edit" value="修改" onclick="on_submit(1)" style="display:none" />
					<input name="Btn_Back" type="button" id="Btn_Back" value="取消"  onclick="back4add()" style="display:none"/>
					</td>
				</tr>
				</table>

				<table id="Schedule_Table" border="1" cellpadding="0" cellspacing="0" width="550px" >
				<tr>
				<td align="center" width="">使能</td>
				<td align="center" width="">LED打开时刻</td>
				<td align="center" width="">LED关闭时刻</td>
				<td align="center" width="">周期(天)</td>
				<td align="center" width="">修改</td>
				<td align="center" width="">删除</td>
				</tr>
				</table>
				<input type="hidden" name="if_index" id="if_index" value=''>
				<input type="hidden" name="action" id="action" value="">
				<input type="hidden" name="submit-url" value="/app_led_sched.asp">
			</form>
			<script>
			<% initPage("ledtimer"); %>
			</script>
		</DIV>
	<blockquote>
</body>
<%addHttpNoCache();%>
</html>
