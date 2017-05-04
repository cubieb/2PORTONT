<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>增加网络流量控制类型规则</TITLE>
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

var dscps = new it_nr("dscplst", 
 new it(0, ""),
 new it(1, "缺省(000000)"), 
 new it(57, "AF13(001110)"), 
 new it(49, "AF12(001100)"), 
 new it(41, "AF11(001010)"),
 new it(33, "CS1(001000)"),
 new it(89, "AF23(010110)"),
 new it(81, "AF22(010100)"),
 new it(73, "AF21(010010)"),
 new it(65, "CS2(010000)"),
 new it(121, "AF33(011110)"),
 new it(113, "AF32(011100)"),
 new it(105, "AF31(011010)"),
 new it(97, "CS3(011000)"),
 new it(153, "AF43(100110)"),
 new it(145, "AF42(100100)"),
 new it(137, "AF41(100010)"),
 new it(129, "CS4(100000)"),
 new it(185, "EF(101110)"),
 new it(161, "CS5(101000)"),
 new it(193, "CS6(110000)"),
 new it(225, "CS7(111000)"));

var protos = new Array("", "TCP", "UDP", "ICMP", "TCP/UDP", "RTP");

var states = new Array("禁用", "启用");
var md802ps = new Array("", "0", "1", "2", "3", "4", "5", "6", "7");
var iffs = new Array(""<% checkWrite("qos_interface"); %>);
var quekeys = new it_nr("queuekey");
var oifkeys = new it_nr("outifkey");

var rules = new Array();
<% initQosRulePage(); %>

var sel1 = new Array("phypt", "proto", "dscp", "sip", "smsk", "spts", "spte", "dip", "dmsk", "dpts", "dpte", "smac", "dmac",
					 "sip6", "sip6PrefixLen", "dip6", "dip6PrefixLen");
var sel2 = new Array("vlan1p");

var ipversions = new Array("请指定IP协议版本", "IPv4", "IPv6");
//default
quekeys.add(new it("", ""));
<% initRulePriority(); %>

<% initOutif(); %>

var opts = new Array(new Array("prio", quekeys), new Array("outif", oifkeys), new Array("proto", protos),
	new Array("mdscp", dscps), new Array("m1p", md802ps), new Array("phypt", iffs),
	new Array("dscp", dscps), new Array("vlan1p", md802ps), new Array("IpProtocolType", ipversions) );

var rule = new it_nr("rule_");
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

function onChange_proto()
{
/*	for(var n2 in sel2)
	{
		var oin = document.getElementById(sel2[n2]);
		if(typeof oin == "undefined")continue;
		if(oin.tagName == "SELECT")oin.selectedIndex = 0;
		else oin.value = "";
	}
*/
	with ( document.forms[0] )
	{
		if (proto.value == 3) {
			spts.disabled = true;
			spte.disabled = true;
			dpts.disabled = true;
			dpte.disabled = true;
		} else {
			spts.disabled = false;
			spte.disabled = false;
			dpts.disabled = false;
			dpte.disabled = false;
		}
	}
}

function onchange_sel1()
{
/*
	for(var n2 in sel2)
	{
		var oin = document.getElementById(sel2[n2]);
		if(typeof oin == "undefined")continue;
		if(oin.tagName == "SELECT")oin.selectedIndex = 0;
		else oin.value = "";
	}
	*/
}

function onchange_sel2()
{
/*
	for(var n1 in sel1)
	{
		var oin = document.getElementById(sel1[n1]);
		if(typeof oin == "undefined")continue;
		if(oin.tagName == "SELECT")oin.selectedIndex = 0;
		else oin.value = "";
	}
	*/
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
	if ( <%checkWrite("IPv6Show");%> )
	{
		if (document.getElementById)  // DOM3 = IE5, NS6
		{
			document.getElementById('ipprotbl').style.display = 'block';						
		}
		else {
			if (document.layers == false) // IE4
			{
				document.all.ipprotbl.style.display = 'block';						
			}
		}
		
		protocolChange();
	}
//	on_iptypechange();
//	on_m1pchange();
}

// Mason Yu:20110524 ipv6 setting. START


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
		if(!checkstr(name.value)) {
			name.value="";
			name.focus();
			alert("流量控制类型名包含非法字符，请重新填写！");
			return;
		} else {
			sbmtstr = sbmtstr+"&name="+name.value;
		}
		//sbmtstr = sbmtstr+"&state="+state.value;
		
		if(prio.value == "") {
			prio.focus();
			alert("请选择IP优先次序标记！");
			return;
		} else {
			sbmtstr = sbmtstr+"&prio="+prio.value;
		}

		//condition check
		if (phypt.value==0 && proto.value==0 && dscp.value==0 && (sip.value=="" || 
			sip.value=="0.0.0.0") && (spts.value==0 || spts.value=="") && (dip.value=="" || 
			dip.value=="0.0.0.0") && (dpts.value==0 || dpts.value=="") && vlan1p.value==0 && 
			smac.value=="" && dmac.value=="" && sip6.value=="" && dip6.value=="")
		{
			alert("必须配置至少一个流量控制条件!");
			return;
		}


	//	if (proto.value==5) {//RTP
	//		if (dpts.value!=0 && dpts.value!="")
	//		{
	//			alert("已经选择RTP，不能再指定目的端口!");
	//			return;
	//		}
	//	}
		
		sbmtstr = sbmtstr+"&outif="+outif.value;
		sbmtstr = sbmtstr+"&markdscp="+mdscp.value;
		sbmtstr = sbmtstr+"&mark1p="+m1p.value;
		sbmtstr = sbmtstr+"&phyport="+phypt.value;
		sbmtstr = sbmtstr+"&proto="+proto.value;
		sbmtstr = sbmtstr+"&matchdscp="+dscp.value;		
		
		if ( <%checkWrite("IPv6Show");%> ) {
			sbmtstr = sbmtstr+"&IPversion="+IpProtocolType.value;
			if(document.forms[0].IpProtocolType.value == 0) {
				alert("请指定IP协议版本！");
				return;
			}
			
			// If this is a IPv4 rule
			if(document.forms[0].IpProtocolType.value == 1) {
				if(sip.value!=""&&!sji_checkip(sip.value)) {
					sip.value = "";
					sip.focus();
					alert("源IP地址输入非法，请重新输入！");
					return;
				} else {
					sbmtstr = sbmtstr+"&sip="+sip.value;
				}
				
				if(smsk.value!=""&&!sji_checkmask(smsk.value)) {
					smsk.value="";
					smsk.focus();
					alert("源子网掩码格式非法！");
					return;
				} else {
					sbmtstr = sbmtstr+"&smask="+smsk.value;
				}
				
				if(dip.value!=""&&!sji_checkip(dip.value)) {
					dip.value = "";
					dip.focus();
					alert("目的IP地址输入非法，请重新输入！");
					return;
				} else {
					sbmtstr = sbmtstr+"&dip="+dip.value;
				}
			}
			else {
				//If this is IPv6 rule.
				if(document.forms[0].IpProtocolType.value == 2){				
					if(sip6.value != ""){
						if (! isGlobalIpv6Address(sip6.value) ){
							alert("Invalid Source IPv6 address!");					
							return;
						}
						if ( sip6PrefixLen.value != "" ) {
							var prefixlen= getDigit(sip6PrefixLen.value, 1);
							if (prefixlen > 128 || prefixlen <= 0) {
								alert("不正确的IPv6前缀长度!");							
								return;
							}
						}
					}
					
					if(dip6.value != ""){
						if (! isGlobalIpv6Address(dip6.value) ){
							alert("不正确的IPv6目的IP地址!");					
							return;
						}
						if ( dip6PrefixLen.value != "" ) {
							var prefixlen= getDigit(dip6PrefixLen.value, 1);
							if (prefixlen > 128 || prefixlen <= 0) {
								alert("不正确的IPv6目的前辍长度!");							
								return;
							}
						}
					}
					sbmtstr = sbmtstr+"&sip6="+sip6.value;
					sbmtstr = sbmtstr+"&dip6="+dip6.value;
					sbmtstr = sbmtstr+"&sip6PrefixLen="+sip6PrefixLen.value;
					sbmtstr = sbmtstr+"&dip6PrefixLen="+dip6PrefixLen.value;					
				}
			}
		}
		else {
			if(sip.value!=""&&!sji_checkip(sip.value)) {
				sip.value = "";
				sip.focus();
				alert("源IP地址输入非法，请重新输入！");
				return;
			} else {
				sbmtstr = sbmtstr+"&sip="+sip.value;
			}
			
			if(smsk.value!=""&&!sji_checkmask(smsk.value)) {
				smsk.value="";
				smsk.focus();
				alert("源子网掩码格式非法！");
				return;
			} else {
				sbmtstr = sbmtstr+"&smask="+smsk.value;
			}
			
			if(dip.value!=""&&!sji_checkip(dip.value)) {
				dip.value = "";
				dip.focus();
				alert("目的IP地址输入非法，请重新输入！");
				return;
			} else {
				sbmtstr = sbmtstr+"&dip="+dip.value;
			}
		}
		
		if(dmsk.value!=""&&!sji_checkmask(dmsk.value)) {
			dmsk.value="";
			dmsk.focus();
			alert("目的子网掩码格式非法！");
			return;
		} else {
			sbmtstr = sbmtstr+"&dmask="+dmsk.value;
		}
		
		if(spts.value<0||spts.value>65536) {
			spts.focus();
			alert("源起始端口非法！");
			return;
		} else {
			if (spts.value!=0 && ( (proto.value==0) || (proto.value==3) ) ) { //NONE or ICMP
				alert("协议不是TCP/UDP,无法指定端口 !");
				return;
			}
			sbmtstr = sbmtstr+"&spts="+spts.value;
		}
		
		if(spte.value<0||spte.value>65536) {
			spte.focus();
			alert("源结束端口非法！");
			return;
		} else {
			if (spte.value!=0 && ( (proto.value==0) || (proto.value==3) ) ) { //NONE or ICMP
				alert("协议不是TCP/UDP,无法指定端口!");
				return;
			}
			sbmtstr = sbmtstr+"&spte="+spte.value;
		}
		
		if(dpts.value<0||dpts.value>65536) {
			dpts.focus();
			alert("目的起始端口非法！");
			return;
		} else {
			if (dpts.value!=0 && ( (proto.value==0) || (proto.value==3) ) ) { //NONE or ICMP	
				alert("协议不是TCP/UDP,无法指定端口!");
				return;
			}
			sbmtstr = sbmtstr+"&dpts="+dpts.value;
		}
		
		if(dpte.value<0||dpte.value>65536) {
			dpte.focus();
			alert("目的结束端口非法！");
			return;
		} else {
			if (dpte.value!=0 && ( (proto.value==0) || (proto.value==3) ) ) { //NONE or ICMP	
				alert("协议不是TCP/UDP,无法指定端口 !");
				return;
			}
			sbmtstr = sbmtstr+"&dpte="+dpte.value;
		}

		if(smac.value!=""&&!sji_checkmac2(smac.value)) {
			smac.value = "";
			smac.focus();
			alert("源MAC地址输入非法，请重新输入！");
			return;
		} else {
			sbmtstr = sbmtstr+"&smac="+smac.value;
		}

		if(dmac.value!=""&&!sji_checkmac2(dmac.value)) {
			dmac.value = "";
			dmac.focus();
			alert("目的MAC地址输入非法，请重新输入！");
			return;
		} else {
			sbmtstr = sbmtstr+"&dmac="+dmac.value;
		}	
		
		sbmtstr = sbmtstr+"&vlan1p="+vlan1p.value;		
		lst.value = sbmtstr;
		submit();
	}
}

function protocolChange()
{	
	// If protocol is IPv4 only.
	if(document.forms[0].IpProtocolType.value == 1){			
		if (document.getElementById)  // DOM3 = IE5, NS6
		{
			document.getElementById('ip4tbl').style.display = 'block';
			document.getElementById('ip6tbl').style.display = 'none';						
		}
		else {
			if (document.layers == false) // IE4
			{
				document.all.ip4tbl.style.display = 'block';
				document.all.ip6tbl.style.display = 'none';						
			}
		}			
		if(sip.value=="0.0.0.0")
			sip.value = "";
		if(dip.value=="0.0.0.0")
			dip.value = "";

	}
	// If protocol is IPv6 only.
	else if(document.forms[0].IpProtocolType.value == 2){			
		if (document.getElementById)  // DOM3 = IE5, NS6
		{
			document.getElementById('ip4tbl').style.display = 'none';
			document.getElementById('ip6tbl').style.display = 'block';			
		}
		else {
			if (document.layers == false) // IE4
			{
				document.all.ip4tbl.style.display = 'none';
				document.all.ip6tbl.style.display = 'block';						
			}
		}
		if(sip6.value=="::")
			sip6.value = "";
		if(dip6.value=="::")
			dip6.value = "";

		if(sip6PrefixLen.value==0)
			sip6PrefixLen.value="";
		if(dip6PrefixLen.value==0)
			dip6PrefixLen.value="";

	}	
	if(spts.value==0)
		spts.value="";
	if(spte.value==0)
		spte.value="";
	if(dpts.value==0)
		dpts.value="";
	if(dpte.value==0)
		dpte.value="";


	if(smac.value=="00:00:00:00:00:00")
		smac.value="";
	if(dmac.value=="00:00:00:00:00:00")
		dmac.value="";

}
</SCRIPT>
</HEAD>

<!-------------------------------------------------------------------------------------->
<!--主页代码-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
	<blockquote>
		<DIV align="left" style="padding-left:20px; padding-top:5px;">
			<form id="form" action="/boaform/admin/formQosRuleEdit" method="post">
				<b>增加网络流量控制类型规则</b>
				<br><br>
				<div class="tip" style="width:90% ">
					&nbsp;&nbsp;本页列出了一个待编辑的流量控制类型规则，实现对上行通信流量进行分类、指定队列优先级和给出IP报头上TOS位段的选项值。
					每条规则由一个类型名和配置条件（至少下列一种）组成。本规则中所有指定的条件在规则生效后必须全部满足。<br>
				</div>
				<br>
				点击"<b>保存</b>"按钮保存规则。<br>
				<hr class="sep" size="1" width="100%">
				<br>				
		
				<table cellSpacing="1" cellPadding="0" border="0">	
				<div id='ipprotbl' style="display:none">						
					<tr><td>IP协议版本:</td>
						<td><select id="IpProtocolType" size="1" style="width:200px " onChange="protocolChange()" name="IpProtocolType">							
							</select>
						</td>
					</tr>					
				</div>
					<tr><td>流量控制类型名:</td><td><input type="text" id="name" size="22" style="width:200px "></td></tr>
					<!--<tr><td>规则状态:</td><td><select id="state" size="1"></select></td></tr>-->
					
					<tr><td colspan="2">&nbsp; </td></tr>
					<tr><td colspan="2"><b>指定ATM传送优先权和DSCP标记类型</b></td></tr>
					<tr><td colspan="2">如果指定的差分服务代码点(DSCP)标记不为空值,上行包IP报头的相应TOS位段将使用该指定的值填充.</td></tr>
					
					<tr><td colspan="2">&nbsp; </td></tr>
					<tr><td>指定IP优先次序标记: </td><td><select id="prio" size="1"></select></td></tr>
					<tr><td>指定差分服务代码点(DSCP)标记:</td><td><select id="mdscp" size="1"></select></td></tr>
					<tr><td>如果WAN口802.1p被启用,则设置802.1p值: </td><td><select id="m1p" size="1"></select></td></tr>
					
					<tr><td colspan="2">&nbsp; </td></tr>
					<tr><td colspan="2"><b>流量控制类型规则配置</b></td></tr>
								</table>		<br>			
				<table cellSpacing="1" cellPadding="0" border="0">
					<tr><td width="260px">物理局域网端口:</td><td><select id="phypt" size="1" style="width:200px " onChange="onchange_sel1();"></select></td></tr>
					<tr><td width="260px">协议: </td><td><select id="proto" size="1" style="width:200px " onChange="onChange_proto();"></select></td></tr>
					</tr>
					<tr><td width="260px">差分服务代码点(DSCP)核对:</td><td><select id="dscp" size="1" style="width:200px " onChange="onchange_sel1();"></select></td></tr>
					<tr><td width="260px">802.1p 优先级:</td><td><select id="vlan1p" size="1" style="width:200px "  onChange="onchange_sel2();"></select></td></tr>
				</table>
				<div id='ip4tbl' style="display:block;">
				<table cellSpacing="1" cellPadding="0" border="0">
					<tr><td width="260px">源IP地址: </td><td><input type="text" id="sip" size="15" maxlength="15" style="width:200px " onChange="onchange_sel1();"></td></tr>
					<tr><td width="260px">源子网掩码: </td><td><input type="text" id="smsk" size="15" maxlength="15" style="width:200px " onChange="onchange_sel1();"></td></tr>					
					<tr><td width="260px">目的IP地址: </td><td><input type="text" id="dip" size="15" maxlength="15" style="width:200px " onChange="onchange_sel1();"></td></tr>
					<tr><td width="260px">目的子网掩码: </td><td><input type="text" id="dmsk" size="15" maxlength="15" style="width:200px " onChange="onchange_sel1();"></td></tr>					
				</table>				
				</div>
				
				<div id='ip6tbl' style="display:none">
				<table cellSpacing="1" cellPadding="0" border="0">
					<tr><td width="260px">源IP地址: </td><td><input type="text" id="sip6" size="36" maxlength="39" style="width:200px " onChange="onchange_sel1();"></td></tr>
					<tr><td width="260px">源前辍长度: </td><td><input type="text" id="sip6PrefixLen" size="15" maxlength="15" style="width:200px " onChange="onchange_sel1();"></td></tr>					
					<tr><td width="260px">目的IP地址: </td><td><input type="text" id="dip6" size="36" maxlength="39" style="width:200px " onChange="onchange_sel1();"></td></tr>
					<tr><td width="260px">目的前辍长度: </td><td><input type="text" id="dip6PrefixLen" size="15" maxlength="15" style="width:200px " onChange="onchange_sel1();"></td></tr>					
				</table>
				</div>
				
				<table cellSpacing="1" cellPadding="0" border="0">
					<tr><td width="260px">源端口(端口或端口:端口): </td><td><input type="text" id="spts" size="5" maxlength="5" style="width:97px" onChange="onchange_sel1();">:<input type="text" id="spte" size="5" maxlength="5" style="width:97px" onChange="onchange_sel1();"></td></tr>
					<tr><td width="260px">目的端口(端口或端口:端口): </td><td><input type="text" id="dpts" size="5" maxlength="5" style="width:97px" onChange="onchange_sel1();">:<input type="text" id="dpte" size="5" maxlength="5" style="width:97px" onChange="onchange_sel1();"></td></tr>
					<tr><td width="260px">源MAC地址(xx:xx:xx:xx:xx:xx):</td><td><input type="text" id="smac" size="17" maxlength="17" style="width:200px " onChange="onchange_sel1();"></td></tr>
					<tr><td width="260px">目的MAC地址(xx:xx:xx:xx:xx:xx):</td><td><input type="text" id="dmac" size="17" maxlength="17" style="width:200px " onChange="onchange_sel1();"></td></tr>
				</table>
				
				<div id='wan_interface' style="display:none">							
					<tr><td>WAN接口: </td><td><select id="outif" size="1"></select></td></tr>
				</div>	
				
				<hr class="sep" size="1" width="100%">
				<br>
				<input type="button" class="button" onClick="on_submit();" value="保存">
				<input type="hidden" name="submit-url" value="/net_qos_cls.asp">
				<input type="hidden" name="lst" value="">
				<input type="hidden" name="index" value="0">
			</form>
		</DIV>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>
