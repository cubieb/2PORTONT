<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<html>
<head>
<TITLE>QoS 流量控制设置</TITLE>
<META http-equiv=pragma content=no-cache>
<META http-equiv=cache-control content="no-cache, must-revalidate">
<META http-equiv=content-type content="text/html; charset=gbk">
<META http-equiv=content-script-type content=text/javascript>
<!--系统公共css-->
<STYLE type=text/css>
@import url(/style/default.css);
.STYLE1 {color: #FF0000}
</STYLE>
<!--系统公共脚本-->
<SCRIPT language="javascript" src="common.js"></SCRIPT>
<SCRIPT language="javascript" type="text/javascript">

<!--var protos = new Array("", "TCP", "UDP", "ICMP", "TCP/UDP", "RTP");-->
var protos = new Array("",  "TCP", "UDP", "ICMP");
var traffictlRules = new Array();
var totalBandwidth = 1000;
var totalBandWidthEn = 0;
<% initTraffictlPage(); %>

function on_chkdel(index) {
	if(index<0 || index>=traffictlRules.length)
		return;
	traffictlRules[index].select = !traffictlRules[index].select;
}

/********************************************************************
**          on document load
********************************************************************/
function on_init_page(){
	if (totalBandWidthEn == 0) {//to apply
		if (document.getElementById)  // DOM3 = IE5, NS6
		{
			document.getElementById('apply').style.display = 'block';
			document.getElementById('cancel').style.display = 'none';
		} else {
			document.all.apply.style.display = 'block';
			document.all.cancel.style.display = 'none';
		}
	} else {//to cancel
		if (document.getElementById)  // DOM3 = IE5, NS6
		{
			document.getElementById('apply').style.display = 'none';
			document.getElementById('cancel').style.display = 'block';
		} else {
			document.all.apply.style.display = 'none';
			document.all.cancel.style.display = 'block';
		}
	}
	
	with(document.forms[0]) {
		totalbandwidth.value = totalBandwidth;
		if(traffictl_tbl.rows){
			while(traffictl_tbl.rows.length > 1) 
				traffictl_tbl.deleteRow(1);
		}
	
		for(var i = 0; i < traffictlRules.length; i++)
		{
			var row = traffictl_tbl.insertRow(i + 1);
		
			row.nowrap = true;
			row.vAlign = "center";
			row.align = "left";

			var cell_index=0;
			var cell = row.insertCell(cell_index++);
			
			cell.innerHTML = traffictlRules[i].id;
			/*cell = row.insertCell(1);
			cell.innerHTML = traffictlRules[i].inf;
			*/
			cell = row.insertCell(cell_index++);
			cell.innerHTML = protos[traffictlRules[i].proto];
			cell = row.insertCell(cell_index++);
			if (traffictlRules[i].sport == "0")
				cell.innerHTML = "";
			else
				cell.innerHTML = traffictlRules[i].sport;
			cell = row.insertCell(cell_index++);
			if (traffictlRules[i].dport == "0")
				cell.innerHTML = "";
			else
				cell.innerHTML = traffictlRules[i].dport;
		
			cell = row.insertCell(cell_index++);
			// For IPv4 and IPv6 
			if ( <%checkWrite("IPv6Show");%> ) {		
				// For IPv4
				if ( traffictlRules[i].IpProtocolType == "1 ") {
					if (traffictlRules[i].srcip == "0.0.0.0")
						cell.innerHTML = "";
					else
						cell.innerHTML = traffictlRules[i].srcip;
				}
				// For IPv6
				else if ( traffictlRules[i].IpProtocolType == "2" ) {
					if (traffictlRules[i].sip6 == "::")
						cell.innerHTML = "";
					else {							
						cell.innerHTML = traffictlRules[i].sip6;											
					}
				}				
			}
			// For IPv4
			else {			
				if (traffictlRules[i].srcip == "0.0.0.0")
					cell.innerHTML = "";
				else
					cell.innerHTML = traffictlRules[i].srcip;				
			}
			
			cell = row.insertCell(cell_index++);				
			// For IPv4 and IPv6 
			if ( <%checkWrite("IPv6Show");%> ) {		
				// For IPv4
				if ( traffictlRules[i].IpProtocolType == "1") {
					if (traffictlRules[i].dstip == "0.0.0.0")
						cell.innerHTML = "";
					else
						cell.innerHTML = traffictlRules[i].dstip;
				}
				// For IPv6
				else if ( traffictlRules[i].IpProtocolType == "2" ) {
					if (traffictlRules[i].dip6 == "::")
						cell.innerHTML = "";
					else {							
						cell.innerHTML = traffictlRules[i].dip6;											
					}
				}				
			}
			// For IPv4
			else {			
				if (traffictlRules[i].dstip == "0.0.0.0")
					cell.innerHTML = "";
				else
					cell.innerHTML = traffictlRules[i].dstip;				
			}
			
			cell = row.insertCell(cell_index++);
			cell.innerHTML = traffictlRules[i].rate;
			cell = row.insertCell(cell_index++);
			cell.align = "center";
			cell.innerHTML = "<input type=\"checkbox\" onClick=\"on_chkdel(" + i + ");\">";
			
			cell = row.insertCell(cell_index++);
			if ( <%checkWrite("IPv6Show");%> ) {			
				if (traffictlRules[i].IpProtocolType == "1")
					cell.innerHTML = "IPv4";
				else if (traffictlRules[i].IpProtocolType == "2")
					cell.innerHTML = "IPv6";
			}
			else {				
				cell.innerHTML = "IPv4";		
			}		
			
			//方向
			cell = row.insertCell(cell_index++);	
			if (traffictlRules[i].direction =="0")
				cell.innerHTML = "上行";
			else
				cell.innerHTML = "下行";			
		}
	}
}


//应用总带宽
function on_apply_bandwidth(){
	
	with(document.forms[0]) {
		var sbmtstr = "";
		var bandwidth = -1;
		bandwidth = parseInt(totalbandwidth.value);
		if(bandwidth<0 || bandwidth >Number.MAX_VALUE)
			return;
		sbmtstr = "applybandwidth&bandwidth="+bandwidth;
		lst.value = sbmtstr;
		submit();
	}
}

//取消总带宽限制
function on_cancel_bandwidth(){
	with(document.forms[0]) {
		var sbmtstr = "";
		sbmtstr = "cancelbandwidth";
		lst.value = sbmtstr;
		submit();
	}
}

//提交添加的流量控制规则
function on_submit(){
	var sbmtstr = "applysetting#id=";
	var firstFound = true;
	for(var i=0; i<traffictlRules.length; i++)
	{
		if(traffictlRules[i].select)
		{
			if(!firstFound)
				sbmtstr += "|";
			else
				firstFound = false;
			sbmtstr += traffictlRules[i].id;
		}
	}
	document.forms[0].lst.value = sbmtstr;
	document.forms[0].submit();
}
</SCRIPT>
</head>

<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init_page();">
<blockquote>
<DIV align="left" style="padding-left:20px; padding-top:5px;">
	<form id="form" action="/boaform/admin/formQosTraffictl" method="post">
			<p><strong>流量控制信息</strong></p>
			
			<p>总带宽:<input type="text" id="totalbandwidth" value="1005">Kb</p>
			<table class="flat" id="traffictl_tbl" border="1" cellpadding="0" cellspacing="1">
				   <tr class="hdb" align="center" nowrap>
						<td>ID</td>
						<!--<td>WAN接口</td>-->
						<td>协议</td>
						<td>源端口</td>
						<td>目的端口</td>
						<td>源IP</td>
				    <!--	<td>源子网掩码</td>-->
						<td>目的IP</td>
					<!--	<td>目的子网掩码</td>-->
						<td>速率(kb/s)</td>
						<td>删除</td>
						<td>IP协议版本</td>
						<td>方向</td>
					</tr>
			</table>
			<table><tr align="left">
			<td><input type="button" class="button" onClick="location.href='net_qos_traffictl_edit.asp';" value="添加"></td>
			<td><input type="button" class="button" onClick="on_submit();" value="保存/应用"></td>
			<td>
			<div  id="apply"  style="display:none">
			<input type="button" class="button" onClick="on_apply_bandwidth();" value="应用总带宽设置">
			</div>
			<div  id="cancel" style="display:none">
			<input type="button" class="button" onClick="on_cancel_bandwidth();" value="取消总带宽限制">
			</div></td>
			</tr></table>
			<input type="hidden" id="lst" name="lst" value="">
			<input type="hidden" name="submit-url" value="/net_qos_traffictl.asp">
	</form>
</DIV>
</blockquote>
</body>
<%addHttpNoCache();%>
</html>
