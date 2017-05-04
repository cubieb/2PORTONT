<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<HTML>
<HEAD>
<TITLE>开关定时</TITLE>
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
var _onoff = new Array();
var _time = new Array();
var _day = new Array();

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

function convertDay(day)
{
	var day_name = ["", "一", "二", "三", "四", "五", "六", "日"];
	var value="";
	for(i=1;i<=7;i++){
		if(day & (1<<i))
			value+=day_name[i];
		else
			value+="**";
		value+=" ";
	}
	return value;
}

function getImage(src,strmethod,id)
{
	//return ("<input type=\"button\" id=\""+id+"\"  onclick=\""+ strmethod +"\" style=\"width:20px;height:20px;border:0px;padding:2px;cursor:pointer;background:url("+src+");\">");
	return ("<image id=\""+id+"\" onclick=\""+ strmethod +"\" src=\""+src+"\">");
}

function addline(index, enable, onoff, time, day)
{
	var newline;
	newline = document.getElementById('Schedule_Table').insertRow(-1);
	newline.nowrap = true;
	newline.vAlign = "top";
	newline.align = "center";
	newline.setAttribute("class","white");
	newline.setAttribute("className","white");
	newline.insertCell(-1).innerHTML = enable? "启用":"停用";
	newline.insertCell(-1).innerHTML = onoff? "开启":"关闭";
	newline.insertCell(-1).innerHTML = time;
	newline.insertCell(-1).innerHTML = convertDay(day);
	newline.insertCell(-1).innerHTML =getImage("image/edit.gif","Modify("+index+")","Btn_Modify"+index);
	newline.insertCell(-1).innerHTML =getImage("image/delete.gif","Delete("+index+")","Btn_Delete"+index);
}

function showTable()
{
	var num = _enable.length;
	if (num!=0) {
		for (var i=0; i<num; i++)
			addline(i, _enable[i], _onoff[i], _time[i], _day[i]);
	}
	else {
	}

	jslEnDisplay("Btn_Add");
	jslDiDisplay("Btn_Edit");
	jslDiDisplay("back");
}

/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	//sji_docinit(document, cgi);
	showTable();
}

/********************************************************************
**          on document submit
********************************************************************/
function on_submit(edit) 
{
	if(pageCheckValue()){
		if(edit)
			getObj("action").value = 2;
		else //add
			getObj("action").value = 1;
		getObj("wlanschedule").submit();
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
	var time;
	time = _time[i].split(":");
	getObj("Frm_Active").checked = _enable[i];
	getObj("Frm_Enable").value = _onoff[i];
	getObj("Frm_Start1").value = time[0];
	getObj("Frm_Start2").value = time[1];

	for(k=1; k<=7;k++){
		if(_day[i] & (1<<k))
			getObj("Frm_S_"+k).checked = true;
		else
			getObj("Frm_S_"+k).checked = false;
	}
	getObj("if_index").value = i;

	jslDiDisplay("Btn_Add");
	jslEnDisplay("Btn_Edit");
	jslEnDisplay("back");
	
}

function back4add()
{
	getObj("Frm_Active").checked = false;
	getObj("Frm_Enable").value = 1;
	getObj("Frm_Start1").value = "";
	getObj("Frm_Start2").value = "";

	for(k=0; k<7;k++)
			getObj("Frm_S_"+k).checked = false;

	getObj("if_index").value = "";

	jslEnDisplay("Btn_Add");
	jslDiDisplay("Btn_Edit");
	jslDiDisplay("back");
}

function Delete(i)
{
	if ( !confirm("您确定要删除?") ) {
		getObj("action").value = "";
		return false;
	}
	getObj("action").value=0;
	getObj("if_index").value = i;
	getObj("wlanschedule").submit();
	return true;
}

function pageCheckValue()
{

	if(getObj("Frm_Start1").value==""){
		alert("时间不能为空");
		getObj("Frm_Start1").focus();
		return false;
	}
	if(!isNumber(getObj("Frm_Start1").value) || getObj("Frm_Start1").value<0 || getObj("Frm_Start1").value>23){
		alert("请输入0~23的数值");
		getObj("Frm_Start1").focus();
		return false;
	}
	if(getObj("Frm_Start2").value==""){
		alert("时间不能为空");
		getObj("Frm_Start2").focus();
		return false;
	}
	if(!isNumber(getObj("Frm_Start2").value) || getObj("Frm_Start2").value<0 || getObj("Frm_Start2").value>59){
		alert("请输入0~59的数值");
		getObj("Frm_Start2").focus();
		return false;
	}
	if(getObj("Frm_S_1").checked==false && getObj("Frm_S_2").checked==false && getObj("Frm_S_3").checked==false
		&& getObj("Frm_S_4").checked==false && getObj("Frm_S_5").checked==false && getObj("Frm_S_6").checked==false 
		&& getObj("Frm_S_7").checked==false){
		alert("请指定星期");
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
			<form id="wlanschedule" action=/boaform/admin/formWifiTimerEx method=POST name=wlanschedule>
				<table border="0" cellpadding="3" cellspacing="0">
				<tr>
					<td>使能</td>
					<td ><input name="Fnt_Active" id="Frm_Active" type="checkbox" value="ON"/>
				</td>
				</tr>
				<tr>
					<td>开关wifi</td>
					<td><select name="Fnt_Enable" id="Frm_Enable" />
					<option value="1">开启</option>
					<option value="0">关闭</option>
					</td>
				</tr>
				<tr >
					<td>时刻</td>
					<td>
					<input name="Frm_Start1" id="Frm_Start1" type="text"  size="2" maxlength="2" /> :
					<input name="Frm_Start2" id="Frm_Start2" type="text"  size="2" maxlength="2" />
					</td>
				</tr>
				<tr>
					<td>周期</td>
				</tr>
				<tr>
					<td>星期一</td>
					<td>
					<input name="Frm_Monday_S" id="Frm_S_1" type="checkbox" value="ON" />
					</td>
				</tr>
				<tr>
					<td>星期二</td>
					<td>
					<input name="Frm_Tuesday_S" id="Frm_S_2" type="checkbox" value="ON" />
					</td>
				</tr>
				<tr>
					<td>星期三</td>
					<td>
					<input name="Frm_Wednesday_S" id="Frm_S_3" type="checkbox" value="ON" />
					</td>
				</tr>
				<tr>
					<td>星期四</td>
					<td>
					<input name="Frm_Thursday_S" id="Frm_S_4" type="checkbox" value="ON" />
					</td>
				</tr>
				<tr>
					<td>星期五</td>
					<td>
					<input name="Frm_Friday_S" id="Frm_S_5" type="checkbox" value="ON" />
					</td>
				</tr>
				<tr>
					<td>星期六</td>
					<td>
					<input name="Frm_Saturday_S" id="Frm_S_6" type="checkbox" value="ON" />
					</td>
				</tr>
				<tr>
					<td>星期日</td>
					<td>
					<input name="Frm_Sunday_S" id="Frm_S_7" type="checkbox" value="ON" />
					</td>
				</tr>
				</table>
				<table id="TestContent"  class="table" width="550px" border="0">
				<tr>
					<td class="td1"></td>
					<td class="td2">
					<input name="Btn_Add" type="button" id="Btn_Add" value="添加" onclick="on_submit(0)"/>
					<input name="Btn_Edit" type="button" id="Btn_Edit" value="修改" onclick="on_submit(1)" style="display:none" />
					<input name="back" type="button" id="back" value="取 消"  onclick="back4add()" style="display:none"/>
					</td>
				</tr>
				</table>

				<table id="Schedule_Table" border="1" cellpadding="0" cellspacing="0" width="550px" >
				<tr>
				<td align="center" width="">使能</td>
				<td align="center" width="">开关wifi</td>
				<td align="center" width="">时刻</td>
				<td align="center" width="">星期</td>
				<td align="center" width="">修改</td>
				<td align="center" width="">删除</td>
				</tr>
				</table>
				<input type="hidden" name="if_index" id="if_index" value=''>
				<input type="hidden" name="action" id="action" value="">
				<input type="hidden" name="submit-url" value="/net_wlan_sched.asp">
			</form>
			<script>
			<% initPage("wltimerEx"); %>
			</script>
		</DIV>
	<blockquote>
</body>
<%addHttpNoCache();%>
</html>
