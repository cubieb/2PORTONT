<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>下挂设备上下行带宽限制</TITLE>
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
with(rules){<% initPageBandwidthControl(); %>}

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
		cell.innerHTML = rules[i].mac;

		cell = row.insertCell(1);
		cell.innerHTML = rules[i].usBandwidth;
		
		cell = row.insertCell(2);
		cell.innerHTML = rules[i].dsBandwidth;
		
		cell = row.insertCell(3);
		cell.innerHTML = "<input type=\"radio\" name=\"act_select\" onClick=\"on_chkclick(" + i + ");\">";
	}
}

function addClick()
{
   var loc = "app_bandwidth_control_add.asp";
   var code = "window.location.href=\"" + loc + "\"";
   eval(code);
}

function getmsg(id)
{
	var str=new Array();
	str[0]=new Array(101,"不允许为空，请重新输入。");
	str[1]=new Array(102,"有误，请输入整数。");
	str[2]=new Array(103,"有误，请输入值为0～1024*1024之间的整数");
	return getMsgFormArray(str,arguments)
}

function getMsgFormArray(str,arg){
	var errid=0;
	var min =0;
	var max=0;
	var param=-1;
	var msg="";
	var num=arg.length;
	if(num==1){
		errid=arg[0];
	}else if(num==3){
		errid=arg[0];
		min =arg[1];
		max=arg[2];
	}else if(num==2){
		errid=arg[0];
		param=arg[1];
	}else return null;
	for(var i=0;i<str.length;i++){
		if(typeof(str[i])=="undefined"){
			alert("数组初始化有误，请检查代码！i = "+i);
			return null;
		}if(errid==str[i][0]){
			if(min ==max&&min ==0){
				if(param==-1){
					msg=str[i][1];
				}else {
					msg=str[i][1]+param+str[i+1][1];
				}
			}else 
				msg=str[i][1]+min +"~"+max+str[i+1][1];
			return msg;
		}
	}
	return null;
}

function setValue(id,value)
{
	document.getElementById(id).value=value;
}

function getValue(id)
{
	return(document.getElementById(id).value);
}

function modifyClick()
{
	if(act_idx == -1)
	{
		alert("请先选择要修改的规则!");
		return false;
	}
	document.getElementById("input_div").style.display="";
	document.getElementById("modify_us").value=rules[act_idx].usBandwidth;
	document.getElementById("modify_ds").value=rules[act_idx].dsBandwidth;
}

function RmZero(str)
{
	while(str.indexOf("0")==0&&str.length>1){
		str=str.substr(1);
	}
	return str;
}

function ReSetValueRmZero(ID){
	var num=ReSetValueRmZero.arguments.length;
	var obj;
	if(num==0)
		return;
	for(i=0;i<num;i++){
		obj=document.getElementById(arguments[i]);
		if(obj!=null &&obj.value!=null &&obj.value!=""){
			if(obj.value.length>10){
				return false;
			}
			obj.value=RmZero(obj.value);
		}
	}
}

function getObj(id)
{
	return(document.getElementById(id));
}
function checkNull(value)
{
	if(value==""||value==null)
		return false;
	else 
		return true;
}

function checkInteger(str){
	if(str.charAt(0)=='-')
		str=str.substr(1);
	if(str.match("^[0-9]+\$"))
		return true;
	return false;
}


function checkIntegerRange(value,minValue, maxValue)
{
	if(checkNull(value)==false){
		return -1;
	}
	if(checkInteger(value)!=true)
		return -2;
	if(value<minValue ||value>maxValue){
		return -3;
	}
	return true;
}
function Check_IntegerRange(value,min,max,Frm)
{
	var msg;
	var tem=checkIntegerRange(value,min,max);
	if(tem==-1){
		//delete mVid!!
		msg=getmsg(101);
		alert(msg);
		getObj(Frm).focus();
		return false;
	}else if(tem==-2){
		msg=getmsg(102);
		alert(msg);
		getObj(Frm).focus();
		return false;
	}else if(tem==-3){
		msg=getmsg(103);
		alert(msg);
		getObj(Frm).focus();
		return false;
	}
	return true;
}

function pageCheckValue(id)
{
	var bandwidth=document.getElementById(id).value;
	if(bandwidth!=""){
		if(Check_IntegerRange(bandwidth,0,1024*1024,id)!=true){
			return false;
		}
	}
	return true;
}

function pageSetValue(id, targetId)
{
	var bandwidth;
	if((getValue(id)!="")&&(getValue(id)!=null )){
		bandwidth = getValue(id);
		setValue(targetId,bandwidth);
	}
}

function ModifyApply()
{
	ReSetValueRmZero("modify_us");
	ReSetValueRmZero("modify_ds");
	if( (pageCheckValue("modify_us")==true)&&(pageCheckValue("modify_ds")==true) )
	{
		pageSetValue("modify_us", "usBandwidth");
		pageSetValue("modify_ds", "dsBandwidth");
		setValue("idx", act_idx);
		setValue("action", "modify");
		return true;
	}
	return false;
}

function back2add()
{
	/*mean user cancel modify, refresh web page again!*/
	document.getElementById("modify").style.display="none";
	document.getElementById("back").style.display="none";
	document.getElementById("modify_us").disabled=true;
	document.getElementById("modify_ds").disabled=true;
	
	setValue("usBandwidth", -1);
	setValue("modify_ds",-1);
	setValue("idx",-1);
	getObj("form").submit();
}

function on_action(act)
{
	form.action.value = act;

	if(act == "rm" && rules.length > 0)
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

</SCRIPT>
</HEAD>
<!-------------------------------------------------------------------------------------->
<!--主页代码-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
	<blockquote>
		<DIV align="left" style="padding-left:20px; padding-top:5px">
			<form id="form" action=/boaform/admin/formBandWidth method=POST name="form">
				<b>下挂设备上下行带宽限制 -- 最多允许您添加 16条规则.</b><br><br>
				<div id="rstip" style="display:none;"><font color="red">提示：本页面的设置，需要重启路由器才能生效！</font><br></div>
				<hr align="left" class="sep" size="1" width="90%">
				<table border="0" cellpadding="2" cellspacing="0">
					<tr>
						<td>上下行带宽限制功能:</td>
						<td><input type="radio" name="bandcontrolEnable" value="off" onClick="on_action('sw')">&nbsp;&nbsp;禁用</td>
						<td><input type="radio" name="bandcontrolEnable" value="on" onClick="on_action('sw')">&nbsp;&nbsp;启用</td>
					</tr>
				</table>
				<br>
				<div id="table_list">
				<hr align="left" class="sep" size="1" width="90%">
				<div align="left" style="padding-left:20px;"><br>
					<table id="rulelst" class="flat" border="1" cellpadding="2" cellspacing="0">
						<tr align="center" class="hd">
							<td width="120px">MAC地址</td>
							<td width="120px">上行带宽限制</td>
							<td width="120px">下行带宽限制</td>
							<td>选择</td>
						</tr>
					</table>
				</div>
				</div>
				<br>
				<div id="input_div">
					<hr align="left" class="sep" size="1" width="90%">
					<tr>
						<td id="Mod_us" class="td1"><b>上行(kbps): </b></td>
						<td class="td2"><input name="modify_us" id="modify_us" type="text" class="inputId"  value="" size="15" /></td>
						<td id="Mod_ds" class="td1"><b>&nbsp;&nbsp;下行(kbps): </b></td>
						<td class="td2"><input name="modify_ds" id="modify_ds" type="text" class="inputId"  value="" size="15" /></td>
	
					</tr>
					<br><br>
					<tr>
					<td class="td1"></td>
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
				<input type="hidden" name="usBandwidth" id="usBandwidth" value="">
				<input type="hidden" name="dsBandwidth" id="dsBandwidth" value="">
				<input type="hidden" name="submit-url" value="/app_bandwidth_control.asp" >
			</form>
		</div>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>
