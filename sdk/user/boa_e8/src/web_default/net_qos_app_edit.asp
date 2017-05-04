<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>增加网络流量控制业务规则</TITLE>
<META http-equiv=pragma content=no-cache>
<META http-equiv=cache-control content="no-cache, must-revalidate">
<META http-equiv=content-type content="text/html; charset=gbk">
<META http-equiv=content-script-type content=text/javascript>
<!--系统公共css-->
<STYLE type=text/css>
@import url(/style/default.css);
</STYLE>
<style>
SELECT {width:200px;}
</style>
<!--系统公共脚本-->
<SCRIPT language="javascript" src="common.js"></SCRIPT>
<SCRIPT language="javascript" type="text/javascript">

var quekeys = new it_nr("queuekey");

//default
quekeys.add(new it("", ""));
<% initRulePriority(); %>

<% initPageQoSAPP(); %>
var opts = new Array(new Array("prio", quekeys), new Array("appName", appNames) );

var rule = new it_nr("app_");
var paramrl = sji_queryparam("rule");
rule.dec(paramrl);


// 检查字符串，只能包含数字，字符及下划线
function checkstr(str)
{
	var ch="";
	if(typeof str != "string") return 0;
	for(var i =0;i< str.length; i++) {
		ch = str.charAt(i);
		if(!(ch =="_"||(ch<="9"&&ch>="0")||(ch<="z"&&ch>="a")||(ch<="Z"&&ch>="A"))) return 0;
	}
	return 1;
}


/********************************************************************
**          on document load
********************************************************************/
function on_init()
{	
	for(var i in opts)
	{
		var slit = document.getElementById(opts[i][0]);
		if(typeof slit == "undefined")continue;
		for(var j in opts[i][1])
		{
			if(j == "name" || (typeof opts[i][1][j] != "string" && typeof opts[i][1][j] != "number"))continue;
			slit.options.add(new Option(opts[i][1][j], j));
		}
		slit.selectedIndex = 0;
	}
	
	for(var key in rule)
	{
		if((typeof rule[key] != "string" && typeof rule[key] != "number"))continue;
		if(typeof form[key] == "undefined")continue;
		form[key].value =  rule[key];
	}
		
}

/********************************************************************
**          on document submit
********************************************************************/
function on_submit() 
{
	with ( document.forms[0] )	{
		var sbmtstr = "addrule";
		if(paramrl != null)
			sbmtstr = "editrule";
		sbmtstr = sbmtstr+"&index="+index.value;		
		sbmtstr = sbmtstr+"&appName="+appName.value;		
		
		if(prio.value == "") {
			prio.focus();
			alert("请选择业务阵列！");
			return;
		} else {
			sbmtstr = sbmtstr+"&prio="+prio.value;
		}		
				
		lst.value = sbmtstr;
		submit();
	}
}

</SCRIPT>
</HEAD>

<!-------------------------------------------------------------------------------------->
<!--主页代码-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
	<blockquote>
		<DIV align="left" style="padding-left:20px; padding-top:5px;">
			<form id="form" action="/boaform/admin/formQosAppRuleEdit" method="post">
				<b>增加网络流量控制业务规则</b>
				<br><br>				
				<br>
				点击"<b>保存</b>"按钮保存规则。<br>
				<hr class="sep" size="1" width="90%">
				<br>		
				
				<table cellSpacing="1" cellPadding="0" border="0">				
										
					<tr><td colspan="2">&nbsp; </td></tr>
					<tr><td>业务名称: </td><td><select id="appName" size="1"></select></td></tr>
					<tr><td>业务阵列: </td><td><select id="prio" size="1"></select></td></tr>
				</table>				
				
				<hr class="sep" size="1" width="90%">
				<br>
				<input type="button" class="button" onClick="on_submit();" value="保存">
				<input type="hidden" name="submit-url" value="/net_qos_app.asp">
				<input type="hidden" name="lst" value="">
				<input type="hidden" name="index" value="0">
			</form>
		</DIV>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>
