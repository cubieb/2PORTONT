<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<HTML>
<HEAD>
<TITLE>绑定模式</TITLE>
<META http-equiv=pragma content=no-cache>
<META http-equiv=cache-control content="no-cache, must-revalidate">
<META http-equiv=content-type content="text/html; charset=gbk">
<META http-equiv=content-script-type content=text/javascript>
<STYLE type=text/css>
@import url(/style/default.css);
</STYLE>
<SCRIPT language="javascript" src="common.js"></SCRIPT>
<SCRIPT language="javascript" type="text/javascript">
var vlan_mapping_interface = <% checkWrite("vlan_mapping_interface"); %>;
<% initVlanRange(); %>
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

function convertDisplay(name,col)
{
	//var port=["LAN1","LAN2","LAN3","LAN4","SSID1","SSID2","SSID3","SSID4","SSID5"];
	var port = vlan_mapping_interface;
	var mode=["端口绑定","VLAN绑定"];
	if(col==0){
		return port[name]||"";
	}
	else if(col==1){
		return mode[name]||mode[0];
	}
	else if(col==2){
		return getValue('VLAN'+name);
	}
	return value;
}

function getImage(src,strmethod,id)
{
	//return ("<input type=\"button\" id=\""+id+"\"  onclick=\""+ strmethod +"\" style=\"width:20px;height:20px;border:0px;padding:2px;cursor:pointer;background:url("+src+");\">");
	return ("<image id=\""+id+"\" onclick=\""+ strmethod +"\" src=\"image/edit.gif\">");
}

function addline(index)
{
	var newline;
	var mode= getValue('Mode'+index);
	newline = document.getElementById('Special_Table').insertRow(-1);
	newline.nowrap = true;
	newline.vAlign = "top";
	newline.align = "center";
	newline.setAttribute("class","white");
	newline.setAttribute("className","white");
	newline.insertCell(-1).innerHTML = convertDisplay(index, 0);
	newline.insertCell(-1).innerHTML = convertDisplay(mode,1);
	newline.insertCell(-1).innerHTML = (mode==0)?"":convertDisplay(index, 2);
	newline.insertCell(-1).innerHTML =getImage("image/edit.gif","Modify("+index+")","Btn_Modify"+index);
}

function showTable()
{
	//var num = getValue('if_instnum');
	var num = vlan_mapping_interface.length;

	if (num!=0) {
		for (var i=0; i<num; i++)
			addline(i);
	}
	else {
	}
}

/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	//sji_docinit(document, cgi);
	jslDisable("modify","Frm_Mode");
	jslDiDisplay("vlan_binding_config");
	showTable();
}

/********************************************************************
**          on document submit
********************************************************************/
function on_submit() 
{
	//var number=getValue("if_instnum");
	var num = vlan_mapping_interface.length;
	if(pageCheckValue()){
		jslDisable("modify");
		pageSetValue();
		/*
		setValue("IF_ACTION","apply");
		setValue("IF_INDEX",index);
		*/
		getObj("vmap").submit();
		jslDisable("Frm_Mode");
		for(var i=0;i<4;i++){
			jslDisable("Frm_VLAN"+i+"a","Frm_VLAN"+i+"b");
		}	
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

function RmZero(str)
{
	while(str.indexOf("0")==0&&str.length>1){
		str=str.substr(1);
	}
	return str;
}

function ReSetValueRmZero(ID)
{
	var num=arguments.length;
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

function trimLRSpaces(str)
{
	return str.replace(/(^\s*)|(\s*$)/g,"");
}

function ModifyGetValue(i)
{
	var mode=getValue("Mode"+i)||0;
	getObj("Frm_Mode").value=mode;
	var conf_arr=getValue("VLAN"+i).split(";",4);
	var i=0,max=4;
	for(var j=0;j<conf_arr.length;j++){
		var conf=conf_arr[j];
		var pair=conf.split("/",2);
		if(pair.length==2){
			getObj("Frm_VLAN"+i+"a").value=pair[0];
			getObj("Frm_VLAN"+i+"b").value=pair[1];
			i++;
		}
	}
	for(;i<max;i++){
		getObj("Frm_VLAN"+i+"a").value="";
		getObj("Frm_VLAN"+i+"b").value="";
	}
}

function Modify(i)
{
	document.getElementById("target_lan").innerHTML = convertDisplay(i, 0)+':';
	jslEnable("modify","Frm_Mode");
	ModifyGetValue(i);
	var ele=getObj("Frm_Mode");
	ele.onchange&&ele.onchange();
	setValue("if_index", i);
	//index=i;
}

function checkVLANRange(vlan)
{
	var num = reservedVlanA.length;
	for(var i = 0; i<num; i++){
		if(vlan == reservedVlanA[i])
			return false;
	}
	if(sji_checkdigitrange(vlan, otherVlanStart, otherVlanEnd) == true)
		return false;
	//return vlan==parseInt(vlan)&&0<vlan&&vlan<4095;
	return true;
}

function pageSetValue()
{
	var k;
	if(getValue("Frm_Mode")==1){
		var conf=[];
		k=0;
		for(var i=0;i<4;i++){
			var vlan_a=parseInt(getValue("Frm_VLAN"+i+"a"));
			var vlan_b=parseInt(getValue("Frm_VLAN"+i+"b"));
			if(!isNaN(vlan_a)&&!isNaN(vlan_b)){
				//conf.push(vlan_a+"/"+vlan_b);
				setValue("Frm_VLAN"+k+"a", vlan_a);
				setValue("Frm_VLAN"+k+"b", vlan_b);
				k++;
			}
		}
		for (i=k;i<4;i++) {
			setValue("Frm_VLAN"+i+"a", "");
			setValue("Frm_VLAN"+i+"b", "");
		}
	}
	else {
		for(var i=0;i<4;i++){
			setValue("Frm_VLAN"+i+"a", "");
			setValue("Frm_VLAN"+i+"b", "");
		}
	}
}

function pageCheckValue()
{
	var msg=["VLAN ID必须成对填写，请重新输入。","VLAN ID为系统保留值，请输入\""+ alertVlanStr + "\"之外的整数。", "VLAN ID对不允许重复，请重新输入。"];
	if(getValue("Frm_Mode")!=1){
		return true;
	}
	var vlan_str=";";
	for(var i=0;i<4;i++){
		getObj("Frm_VLAN"+i+"a").value=trimLRSpaces(getValue("Frm_VLAN"+i+"a"));
		getObj("Frm_VLAN"+i+"b").value=trimLRSpaces(getValue("Frm_VLAN"+i+"b"));
		ReSetValueRmZero("Frm_VLAN"+i+"a","Frm_VLAN"+i+"b");
		var vlan_a=getValue("Frm_VLAN"+i+"a");
		var vlan_b=getValue("Frm_VLAN"+i+"b");
		if(vlan_a.length==0&&vlan_b.length==0){
			continue;
		}
		if(vlan_a.length==0||vlan_b.length==0){
			var frm_id="Frm_VLAN"+i+"a";
			if(vlan_a.length){
				frm_id="Frm_VLAN"+i+"b"
			}
			alert(msg[0]);
			getObj(frm_id).focus();
			return false;
		}
		if(!checkVLANRange(vlan_a)){
			alert(msg[1]);
			getObj("Frm_VLAN"+i+"a").focus();
			return false;
		}
		else if(!checkVLANRange(vlan_b)){
			alert(msg[1]);
			getObj("Frm_VLAN"+i+"b").focus();
			return false;
		}
		var pair_str=trimLRSpaces(getObj("Frm_VLAN"+i+"a").value)+"/"+trimLRSpaces(getObj("Frm_VLAN"+i+"b").value);
		if(vlan_str&&vlan_str.indexOf(pair_str) !=-1){
			alert(msg[2]);
			getObj("Frm_VLAN"+i+"a").focus();
			return false;
		}
		vlan_str=vlan_str+pair_str+";";
	}
	return true;
}

function Frm_Mode_OnChange(ele)
{
	if(ele.value==1){
		jslEnDisplay("vlan_binding_config");
	}
	else {
		jslDiDisplay("vlan_binding_config");
	}
}

</SCRIPT>
</HEAD>

<!-------------------------------------------------------------------------------------->
<!--主页代码-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
	<blockquote>
		<DIV align="left" style="padding-left:20px; padding-top:5px">
			<form id="vmap" action=/boaform/admin/formVlanMapping method=POST name=vmap>
				<b>绑定配置<br><br>
				<div id="note" style="text-align:center;width:480px;">
				<div class="space_0">
				<ul>
				<li style="padding-top:8px;text-align:left; margin-left:20px;">注：当绑定模式为端口绑定时，请到WAN连接页面查看详情。</li>
				</ul>
				</div>
				</div>

				<table border="0" cellpadding="3" cellspacing="0">
				<tr>
					<td id=target_lan></td>
					<td class="td1"><span>绑定模式</span></td>
					<td class="td2">
						<select name="Frm_Mode" id="Frm_Mode" onchange="Frm_Mode_OnChange(this);">
							<option value="0">端口绑定</option>
							<option value="1">VLAN绑定</option>
						</select>
					</td>
				</tr>
				<tr id="vlan_binding_config">
					<td colspan="2">
					<table width="100%">
					<tr>
						<td class="td1"><span>1</span></td>
						<td class="td2">
						<input id="Frm_VLAN0a" name="Frm_VLAN0a" size="4" type="text" value="">
						<span style="padding: 0pt 5px;">/</span>
						<input id="Frm_VLAN0b" name="Frm_VLAN0b" size="4" type="text" value="">
						</td>
					</tr>
					<tr>
						<td class="td1"><span>2</span></td>
						<td class="td2">
						<input id="Frm_VLAN1a" name="Frm_VLAN1a" size="4" type="text" value="">
						<span style="padding: 0pt 5px;">/</span>
						<input id="Frm_VLAN1b" name="Frm_VLAN1b" size="4" type="text" value="">
						</td>
					</tr>
					<tr>
						<td class="td1"><span>3</span></td>
						<td class="td2">
						<input id="Frm_VLAN2a" name="Frm_VLAN2a" size="4" type="text" value="">
						<span style="padding: 0pt 5px;">/</span>
						<input id="Frm_VLAN2b" name="Frm_VLAN2b" size="4" type="text" value="">
						</td>
					</tr>
					<tr>
						<td class="td1"><span>4</span></td>
						<td class="td2">
						<input id="Frm_VLAN3a" name="Frm_VLAN3a" size="4" type="text" value="">
						<span style="padding: 0pt 5px;">/</span>
						<input id="Frm_VLAN3b" name="Frm_VLAN3b" size="4" type="text" value="">
						</td>
					</tr>
					</table>
					</td>
				</tr>
				<tr>
					<td style="height: 24px; text-align: center;" colspan="2">
					<input class="button2" id="modify" value="应用" onclick="on_submit()" type="button">
					</td>
				</tr>
				<tr>
					<td class="td1"></td>
					<td class="td2"></td>
				</tr>
				</table>
				<table id="Special_Table" class="flat" border="1" cellpadding="1" cellspacing="1" width="30%">
					<tr class="hdb" align="center" nowrap>
						<td>端口</td>
						<td>绑定模式</td>
						<td>VLAN绑定</td>
						<td>修 改</td>
					</tr>
				</table>
				<input type='hidden' name=if_index ID=if_index value=''>
				<input type="hidden" name="submit-url" value="/net_vlan_mapping.asp">
			</form>
			<input type='hidden' name=if_instnum ID=if_instnum value=9>
			<input type='hidden' name=Mode0   ID=Mode0 value='0'>
			<input type='hidden' name=VLAN0   ID=VLAN0 value=''>
			<input type='hidden' name=Mode1   ID=Mode1 value='0'>
			<input type='hidden' name=VLAN1   ID=VLAN1 value=''>
			<input type='hidden' name=Mode2   ID=Mode2 value='0'>
			<input type='hidden' name=VLAN2   ID=VLAN2 value=''>
			<input type='hidden' name=Mode3   ID=Mode3 value='0'>
			<input type='hidden' name=VLAN3   ID=VLAN3 value=''>
			<input type='hidden' name=Mode4   ID=Mode4 value='0'>
			<input type='hidden' name=VLAN4   ID=VLAN4 value=''>
			<input type='hidden' name=Mode5   ID=Mode5 value='0'>
			<input type='hidden' name=VLAN5   ID=VLAN5 value=''>
			<input type='hidden' name=Mode6   ID=Mode6 value='0'>
			<input type='hidden' name=VLAN6   ID=VLAN6 value=''>
			<input type='hidden' name=Mode7   ID=Mode7 value='0'>
			<input type='hidden' name=VLAN7   ID=VLAN7 value=''>
			<input type='hidden' name=Mode8   ID=Mode8 value='0'>
			<input type='hidden' name=VLAN8   ID=VLAN8 value=''>
			<script>
			<% initPagePBind(); %>
			</script>
		</DIV>
	<blockquote>
</body>
<%addHttpNoCache();%>
</html>
