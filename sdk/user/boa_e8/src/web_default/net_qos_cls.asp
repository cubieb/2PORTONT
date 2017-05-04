<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>QoS 设置</TITLE>
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

var dscps = new it_nr("dscplst", 
 new it(0, ""),
 new it(1, "缺省(000000)"), 
 new it(57,  "AF13(001110)"), 
 new it(49,  "AF12(001100)"), 
 new it(41,  "AF11(001010)"),
 new it(33,  "CS1(001000)"),
 new it(89,  "AF23(010110)"),
 new it(81,  "AF22(010100)"),
 new it(73,  "AF21(010010)"),
 new it(65,  "CS2(010000)"),
 new it(121, "AF33(011110)"),
 new it(113, "AF32(011100)"),
 new it(105, "AF31(011010)"),
 new it(97,  "CS3(011000)"),
 new it(153, "AF43(100110)"),
 new it(145, "AF42(100100)"),
 new it(137, "AF41(100010)"),
 new it(129, "CS4(100000)"),
 new it(185, "EF(101110)"),
 new it(161, "CS5(101000)"),
 new it(193, "CS6(110000)"),
 new it(225, "CS7(111000)")
);

var iffs = new Array(""<% checkWrite("qos_interface"); %>);
var protos = new Array("", "TCP", "UDP", "ICMP", "TCP/UDP", "RTP");
//var protos = new Array("", "TCP", "UDP", "ICMP"); //Remove TCP/UDP, since e8 not require this, and in RG, this need to acl rules.
												  //So if user need this, just use web to add two QoS rules.
var rules = new Array();
<% initQosRulePage(); %>

/*
function on_chkstate(index)
{
	if(index < 0 || index >= rules.length)
		return; 
	rules[index].state = !rules[index].state;
//	rules[index].dirty = true;
}
*/

function on_chkclick(index)
{
	if(index < 0 || index >= rules.length)
		return; 
	rules[index].select = !rules[index].select;
}

function on_onedit(index)
{
	if(index < 0 || index >= rules.length)
		return;
	window.location.href = "net_qos_cls_edit.asp?rule=" + rules[index].enc();
}

/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	if(lstrc.rows){while(lstrc.rows.length > 2) lstrc.deleteRow(2);}
	for(var i = 0; i < rules.length; i++)
	{

		//var bcheck = " ";
		var row = lstrc.insertRow(i + 2);
		var strprio = "";
		row.nowrap = true;
		row.vAlign = "center";
		row.align = "left";

		var cell = row.insertCell(0);
		cell.innerHTML = rules[i].index;
		cell = row.insertCell(1);
		cell.innerHTML = rules[i].name;
		cell = row.insertCell(2);
		if (rules[i].mdscp != "0") {
			cell.innerHTML = rules[i].mdscp-1;
		} else
			cell.innerHTML = "";
		cell = row.insertCell(3);
		//strprio = rules[i].prio.split("|");
        //cell.innerHTML = strprio[1];
		cell.innerHTML = rules[i].prio;
		cell = row.insertCell(4);
		if (rules[i].m1p != "0") {
			cell.innerHTML = rules[i].m1p-1;
		} else
			cell.innerHTML = "";
		cell = row.insertCell(5);
		cell.innerHTML = iffs[rules[i].phypt];
		cell = row.insertCell(6);
		cell.innerHTML = protos[rules[i].proto];
		cell = row.insertCell(7);
		//cell.innerHTML = dscps[rules[i].dscp];
		if (rules[i].dscp != "0") {
			cell.innerHTML = rules[i].dscp-1;
		} else 
			cell.innerHTML = "";
			
		cell = row.insertCell(8);
		// For IPv4 and IPv6 
		if ( <%checkWrite("IPv6Show");%> ) {		
			// For IPv4
			if ( rules[i].IpProtocolType == "1 ") {
				if (rules[i].sip == "0.0.0.0")
					cell.innerHTML = "";
				else {
					if(rules[i].smsk == "")
						cell.innerHTML = rules[i].sip;
					else
						cell.innerHTML = rules[i].sip + "/" + rules[i].smsk;
				}
			}
			// For IPv6
			else if ( rules[i].IpProtocolType == "2" ) {
				if (rules[i].sip6 == "::")
					cell.innerHTML = "";
				else {	
					if(rules[i].sip6PrefixLen == "")
						cell.innerHTML = rules[i].sip6;
					else
						cell.innerHTML = rules[i].sip6 + "/" + rules[i].sip6PrefixLen;					
				}
			}
		}
		// For IPv4 only
		else {
			if (rules[i].sip == "0.0.0.0")
				cell.innerHTML = "";
			else {
				if(rules[i].smsk == "")
					cell.innerHTML = rules[i].sip;
				else
					cell.innerHTML = rules[i].sip + "/" + rules[i].smsk;
			}
		}
		
		cell = row.insertCell(9);
		if (rules[i].spts == "0")
			cell.innerHTML = "";
		else if (rules[i].spte == "0")
			cell.innerHTML = rules[i].spts;
		else
			cell.innerHTML = ((typeof rules[i].spte == "undefined") ? rules[i].spts : rules[i].spts + ":" + rules[i].spte);
		
		cell = row.insertCell(10);
		// For IPv4 and IPv6 
		if ( <%checkWrite("IPv6Show");%> ) {		
			// For IPv4
			if ( rules[i].IpProtocolType == "1" ) {
				if (rules[i].dip == "0.0.0.0")
					cell.innerHTML = "";
				else {
					if(rules[i].dmsk == "")
						cell.innerHTML = rules[i].dip;
					else
						cell.innerHTML = rules[i].dip + "/" + rules[i].dmsk;
				}
			}
			// For IPv6
			else if ( rules[i].IpProtocolType == "2" ) {
				if (rules[i].dip6 == "::")
					cell.innerHTML = "";
				else {					
					if(rules[i].dip6PrefixLen == "")
						cell.innerHTML = rules[i].dip6;
					else
						cell.innerHTML = rules[i].dip6 + "/" + rules[i].dip6PrefixLen;
				}
			} 
		}
		// For IPv4 only
		else {
			if (rules[i].dip == "0.0.0.0")
				cell.innerHTML = "";
			else {
				if(rules[i].dmsk == "")
					cell.innerHTML = rules[i].dip;
				else
					cell.innerHTML = rules[i].dip + "/" + rules[i].dmsk;
			}
		}
		
		cell = row.insertCell(11);
		if (rules[i].dpts == "0")
			cell.innerHTML = "";
		else if (rules[i].dpte == "0")
			cell.innerHTML = rules[i].dpts;
		else
			cell.innerHTML = rules[i].dpts + ":" + rules[i].dpte;
		cell = row.insertCell(12);
		cell.innerHTML = ((rules[i].smac=="00:00:00:00:00:00")?"":rules[i].smac);
		cell = row.insertCell(13);
		cell.innerHTML = ((rules[i].dmac=="00:00:00:00:00:00")?"":rules[i].dmac);
		cell = row.insertCell(14);
		if (rules[i].vlan1p != "0") {
			cell.innerHTML = rules[i].vlan1p-1;
		} else
			cell.innerHTML = "";
		/*
		cell = row.insertCell(15);
		cell.align = "center";
		if(rules[i].state)
			bcheck = "checked";
		cell.innerHTML = "<input type=\"checkbox\" onClick=\"on_chkstate(" + i + ");\" "+bcheck+">";
		*/
		cell = row.insertCell(15);
		cell.align = "center";
		cell.innerHTML = "<input type=\"checkbox\" onClick=\"on_chkclick(" + i + ");\">";
		cell = row.insertCell(16);
		cell.align = "center";
		cell.innerHTML = "<input type=\"button\" onClick=\"on_onedit(" + i + ");\" value=\"编辑\">";
		
		cell = row.insertCell(17);
		if ( <%checkWrite("IPv6Show");%> ) {			
			if (rules[i].IpProtocolType == "1")
				cell.innerHTML = "IPv4";
			else if (rules[i].IpProtocolType == "2")
				cell.innerHTML = "IPv6";
		}
		else {				
			cell.innerHTML = "IPv4";		
		}
	}
}

function rc2string(it)
{
	return it.index + "," + Number(it.state) + "," + Number(it.select);// + "|" + it.tporte + "|" + it.oprotocol + "|" + it.oportb + "|" + it.oporte; 
}

/********************************************************************
**          on document submit
********************************************************************/
function on_submit() 
{
	var tmplst = "";
	var first = true;

	if (rules.length == 0)
		return;
	
	with ( document.forms[0] ) 
	{
		for(var i = 0; i < rules.length; i++)
		{
			if(first)
			{
				first = false;
			}
			else 
			{
				tmplst += "&";
			}
			tmplst += rc2string(rules[i]);
		}
		lst.value = tmplst;
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
			<form id="form" action="/boaform/admin/formQosRule" method="post">
				<b>QoS 设置</b>
				<br><br>
				<div class="tip" style="width:90% ">
					选择增加或删除网络信息流通量配置<font color="red">(在添加配置以后请点击"保存/应用"使配置生效)</font>
				</div>
				<br>
				<hr class="sep" size="1" width="100%">
				<br>
				<table class="flat" id="lstrc" border="1" cellpadding="0" cellspacing="1">
				   <tr class="hdb" align="center" nowrap bgcolor="#CCCCCC">
						<td colspan="2">&nbsp;</td>
						<td colspan="3">标志</td>
						<td colspan="10">流量分类规则</td>
						<td colspan="3">&nbsp;</td>
					</tr>
				   <tr class="hdb" align="center" nowrap>
						<td>序号</td>
						<td>组名</td>
						<td>DSCP标记</td>
						<td>IP优先级</td>
						<td>802.1P标记</td>
						<td>LAN端口</td>
						<td>协议</td>
						<td>DSCP</td>
						<td>源IP地址/子网掩码</td>
						<td>源端口</td>
						<td>目的IP地址/子网掩码</td>
						<td>目的端口</td>
						<td>源MAC地址</td>
						<td>目的MAC地址</td>
						<td>802.1P</td>
						<!--<td>启用/禁用</td>-->
						<td>删除</td>
						<td>编辑</td>
						<td>IP协议版本</td>
					</tr>
				</table>
				<br>
				<input type="button" class="button" onClick="location.href='net_qos_cls_edit.asp';" value="添加">
				<input type="button" class="button" onClick="on_submit();" value="保存/应用">
				<input type="hidden" id="lst" name="lst" value="">
				<input type="hidden" name="submit-url" value="/net_qos_cls.asp">
			</form>
		</DIV>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>
