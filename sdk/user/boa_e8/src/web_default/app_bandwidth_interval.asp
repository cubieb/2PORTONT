<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>带宽计量周期配置</TITLE>
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

function getmsg(id)
{
	var str=new Array();
	str[0]=new Array(101,"不允许为空，请重新输入。");
	str[1]=new Array(102,"有误，请输入整数。");
	str[2]=new Array(103,"有误，请输入值为1～60之间的整数");
	return getMsgFormArray(str,arguments)
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

function getObj(id)
{
	return(document.getElementById(id));
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

function intervalClick(id)
{
	var interval_value=document.getElementById("bdw_interval").value;
	if(interval_value!=""){
		if(Check_IntegerRange(interval_value,1,60,"bdw_interval")!=true){
			return false;
		}
	}
	return true;
}

</SCRIPT>
</HEAD>

<!-------------------------------------------------------------------------------------->
<!--主页代码-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000">
	<blockquote>
		<DIV align="left" style="padding-left:20px; padding-top:5px">
			<form id="form" action=/boaform/formBandwidthInterval method=POST name="bandwidthinterval">
				<b>配置下挂设备带宽计量周期</b><br><br>
				这个页面允许你配置下挂设备上下行带宽计量周期<br><br>
			
				<hr align="left" class="sep" size="1" width="90%">				
				<table border=0 width="500" cellspacing=4 cellpadding=0>
					<tr><td width=150>带宽计量周期(seconds):&nbsp;</td>
						<td class="td2"><input name="bdw_interval" id="bdw_interval" type="text" class="inputId" value="" size="12" /></td>
					</tr>
				</table>
				<br>
				<input type="submit" class="button" name="apply_interval" onClick="return intervalClick();" value="保存/应用"> 
				<input type="hidden" name="submit-url" value="/app_bandwidth_interval.asp">
			<script>
				<% initPage("bdw_interval"); %>	
			</script>			
			</form>
		</DIV>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>
