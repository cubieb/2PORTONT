<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>Internet连接</TITLE>
<META http-equiv=pragma content=no-cache>
<META http-equiv=cache-control content="no-cache, must-revalidate">
<META http-equiv=content-type content="text/html; charset=gbk">
<META http-equiv=content-script-type content=text/javascript>
<!--系统公共css-->
<STYLE type=text/css>
@import url(/style/default.css);
</STYLE>
<style>
TABLE{width:320px;}
TR{height:16px;}
SELECT {width:150px;}
</style>
<!--系统公共脚本-->
<SCRIPT language="javascript" src="common.js"></SCRIPT>
<SCRIPT language="javascript" type="text/javascript">

var upmodes = new Array("ADSL", "LAN");
var lkmodes = new Array("Bridge", "Route");
var svkinds = new Array("UBR Without PCR", "UBR With PCR", "CBR", "Non Realtime VBR", "Realtime VBR");
var cpmodes = new Array("LLC", "VC");
var apmodes = new Array("TR069_INTERNET", "INTERNET", "TR069", "Other");
var dlmodes = new Array("自动连接", "有流量时自动连接");
var md802ps = new Array("(无)", "0", "1", "2", "3", "4", "5", "6", "7");

var opts = new Array(new Array("upmode", upmodes), new Array("lkmode", lkmodes), 
	new Array("svtype", svkinds), new Array("encap", cpmodes), new Array("applicationtype", apmodes),
	new Array("pppCtype", dlmodes), new Array("vprio", md802ps));

var curlink = null;

var cgi = new Object();
var links = new Array();
with(links){<% initPageAdsl(); %>}

function searchpvc(ipmode, pppcheck, vpi, vci)
{
	vpi = parseInt(vpi);
	vci = parseInt(vci);
	for(var i = 0; i < links.length; i++)
	{
		if(links[i] == curlink)continue;
		if(vpi == links[i]["vpi"] && vci == links[i]["vci"])
		{
			if(pppcheck == true && links[i]["cmode"] == 2)
				continue;
			if((ipmode ==0 && links[i]["cmode"] != 0) || (ipmode ==1 && links[i]["cmode"] == 0))
				continue;
			return true;
		}
	}
	return false;
}

function on_linkchange(itlk)
{
	with ( document.forms[0] ) 
	{
		if(itlk == null)
		{
			//select
			lkmode.value = vprio.value = pppCtype.value = encap.value = svtype.value = 0;
			if (4 == applicationtype.options.length)
			{
				applicationtype.value = 0;
			}
			else
			{
				applicationtype.value = 1;
			}
			
			//radio
			ipmode[0].checked = true;
			
			//checkbox
			PPPoEProxyEnable.checked = brmode.checked = napt.checked = vlan.checked = qos.checked = dgw.checked = false;
			
			//checkbox array
			chkpt[0].checked = chkpt[1].checked = chkpt[2].checked = chkpt[3].checked = chkpt[4].checked = chkpt[5].checked = chkpt[6].checked = chkpt[7].checked = chkpt[8].checked = false;
			
			//input number
			PPPoEProxyMaxUser.value = vpi.value = vci.value = pcr.value = scr.value = mbs.value = vid.value = "0";
			
			//input ip
			ipAddr.value = remoteIpAddr.value = "0.0.0.0";
			netMask.value = "255.255.255.255";
			
			//input text
			pppUsername.value = pppPassword.value = pppServiceName.value = pppACName.value = "";
		}
		else
		{
			sji_onchanged(document, itlk);
			
			//select
			lkmode.value = itlk.cmode >= 1 ? 1 : 0;

			//checkbox array
			var ptmap = itlk.itfGroup;
			for(var i = 0; i < 9; i ++) chkpt[i].checked = (ptmap & (0x1 << i));
			
			//radio
			if(curlink.cmode != 0 && curlink.ipDhcp == 0)
			{
				ipmode[curlink.cmode].checked = true;
			}
			else ipmode[0].checked = true;

			if ((1 == lkmode.value) && (2 == applicationtype.options.length))
			{
				//mode changed from bridge to route
				// add BOTH and tr069 option
				applicationtype.options.length = 0;
				for(var i in apmodes)
				{
					applicationtype.options.add(new Option(apmodes[i], i));
				}

				applicationtype.value = itlk.applicationtype;
			}
		}
		on_ctrlupdate(lkmode);
		on_ctrlupdate(svtype);
		on_ctrlupdate(vlan);
		//on_ctrlupdate(applicationtype);
		if(itlk != null){
			if(itlk.napt==1)
				napt.checked = true;
			else
				napt.checked = false;
		}
	}
}

function on_ctrlupdate(obj)
{
	with ( document.forms[0] ) 
	{
		if(obj.name == "lkname")
		{
			if(obj.value == "new")
			{
				curlink = null;
				on_linkchange(curlink);
			}
			else
			{
				curlink = links[obj.value];
				on_linkchange(curlink);
			}
		}
		else if(obj.name == "lkmode")
		{
			tbipmode.style.display = obj.value == 0 ? "none" : "block";
			tbnat.style.display = obj.value == 0 ? "none" : "block";
			tbdgw.style.display = obj.value == 0 ? "none" : "block";
			if(obj.value == 1 && applicationtype.value != 2)
				napt.checked = true;
			else{
				napt.checked = false;
				tbnat.style.display = "none";
				tbdgw.style.display = "none";
			}
			on_ctrlupdate(ipmode[0]);
			
			var orgapptype;

			if (0 == obj.value)
			{
				if (4 == applicationtype.options.length)
				{
					orgapptype = applicationtype.value;

					// remove BOTH and tr069 option
					applicationtype.options.remove(2);
					applicationtype.options.remove(0);

					if ((0 == orgapptype) || (2 == orgapptype))
					{
						applicationtype.value = 1;
					}
				}
			}
			else
			{
				if (2 == applicationtype.options.length)
				{
					orgapptype = applicationtype.value;

					// add BOTH and tr069 option
					applicationtype.options.length = 0;
					for(var i in apmodes)
					{
						applicationtype.options.add(new Option(apmodes[i], i));
					}

					applicationtype.value = orgapptype;
				}
			}
			
			on_ctrlupdate(applicationtype);
		}
		else if(obj.name == "ipmode")
		{
			tbip.style.display = "none";
			tbdial.style.display = "none";
			tbpppprxy.style.display = "none";
			tbpppbr.style.display = "none";
			tbpppnum.style.display = "none";		
			
			if(lkmode.value == 1)
			{
				if (ipmode[1].checked == true)
				{
					tbip.style.display = "block";
				}
				else if (ipmode[2].checked == true)
				{
					tbdial.style.display = "block";
					if (applicationtype.value != 2)
					{
						tbpppprxy.style.display = "block";
						tbpppbr.style.display = "block";
						on_ctrlupdate(PPPoEProxyEnable);
					}else
						tbpppbr.style.display = "block";
				}
			}
		}
		else if(obj.name == "PPPoEProxyEnable")
		{
			tbpppnum.style.display = obj.checked == false ? "none" : "block";
		}
		else if(obj.name == "svtype")
		{
			tbpeakcell.style.display = obj.value >= 1 ? "block" : "none";
			tbothercell.style.display = obj.value >= 3 ? "block" : "none";
		}
		else if(obj.name == "vlan")
		{
			vid.disabled = !obj.checked;
			vprio.disabled = !obj.checked;
		}
		else if(obj.name == "applicationtype")
		{
			if(obj.value == 2){
				napt.checked = false;
				tbnat.style.display = "none";
				tbbind.style.display = "none";
				tbindip.style.display = "none";
				dgw.checked = false;
				tbdgw.style.display = "none";
				
				PPPoEProxyEnable.checked = false;

				tbpppprxy.style.display = "none";
				//tbpppbr.style.display = "none";
								
				//brmode.checked = false;				
				on_ctrlupdate(PPPoEProxyEnable);
				
			}else{
				if(lkmode.value == 1){
					tbnat.style.display = "block";
					napt.checked = true;
					if (obj.value == 3)
					{
						dgw.checked = false;
						tbdgw.style.display = "none";
					}
					else
					{
						tbdgw.style.display = "block";
					}

					
					if (ipmode[2].checked == true)
					{
						tbpppprxy.style.display = "block";
						tbpppbr.style.display = "block";
						on_ctrlupdate(PPPoEProxyEnable);
					}
					
				}
				else{
					napt.checked = false;
					tbdgw.style.display = "none";
				}
				tbbind.style.display = "block";
				tbindip.style.display = "block";
			}

			if ((3 == obj.value) && (0 == lkmode.value))
			{
				attention.style.display = "block";
			}
			else
			{
				attention.style.display = "none";
			}
		}
		<% initdgwoption(); %>
	}
}

/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	sji_docinit(document, cgi);	

	for(var i in opts)
	{
		var slit = document.getElementById(opts[i][0]);
		if(typeof slit == "undefined")continue;
		for(var j in opts[i][1])
		{
			slit.options.add(new Option(opts[i][1][j], j));
		}
		slit.selectedIndex = 0;
	}
	with ( document.forms[0] ) 
	{
		for(var k in links)
		{
			var lk = links[k];
			lkname.options.add(new Option(lk.name, k));
		}
		if(links.length > 0)lkname.value = 0;
		lpppnumleft.innerHTML = cgi.pppnumleft;
		on_ctrlupdate(lkname);
	}
}

/********************************************************************
**          on document submit
********************************************************************/
function on_submit(act) 
{
	var tmplst = "";
	with ( document.forms[0] ) 
	{
		action.value = act;
		if(act == "rm")
		{
			if(lkname.value == "new")
			{
				lkname.focus();
				alert("请选择要删除的链接！");
				return;
			}
			tmplst = curlink.name;
		}
		else if(act == "sv")
		{
			if(lkname.value != "new")tmplst = curlink.name;
			if(lkmode.value == 0 && (applicationtype.value == 0 || applicationtype.value == 2))
			{
				lkmode.focus();
				alert("设置错误，桥模式下服务模式不支持tr069服务!");
				return;
			}

			//input number
			if(lkmode.value != 0 && ipmode[2].checked == true && sji_checkdigitrange(PPPoEProxyMaxUser.value, 0, cgi.pppnumleft) == false)
			{
				PPPoEProxyMaxUser.focus();
				alert("代理用户数\"" + PPPoEProxyMaxUser.value + "\"为无效用户数，请重新输入！");
				return;
			}
			if(sji_checkdigitrange(vpi.value, 0, 255) == false)
			{
				vpi.focus();
				alert("VPI\"" + vpi.value + "\"为无效数值或范围不在0-255之间！请重新输入！");
				return;
			}
			if(sji_checkdigitrange(vci.value, 32, 65535) == false)
			{
				vci.focus();
				alert("VCI\"" + vci.value + "\"为无效数值或范围不在32-65535之间，请重新输入！");
				return;
			}
			if( searchpvc(lkmode.value , ipmode[2].checked, vpi.value, vci.value) == true)
			{
				vpi.focus();
				alert("PVC\"" + vpi.value + "/" + vci.value + "\"已经存在，请重新输入！");
				return;
			}
			if(vpi.value == "0" && vci.value == "16")
			{
				vpi.focus();
				alert("PVC\"" + vpi.value + "/" + vci.value + "\"已经被系统占用，请重新输入！");
				return;
			}
			if(vpi.value == "0" && vci.value == "0")
			{
				vpi.focus();
				alert("PVC\"" + vpi.value + "/" + vci.value + "\"为无效通道，请重新输入！");
				return;
			}

			if(svtype.value >= 1 && sji_checkdigitrange(pcr.value, 1, 255000) == false)
			{
				pcr.focus();
				alert("峰值信元速率\"" + pcr.value + "\"为无效数值或范围不在1-255000之间，请重新输入！");
				return;
			}
			if(svtype.value >= 3 && sji_checkdigitrange(scr.value, 1, parseInt(pcr.value) - 1) == false)
			{
				scr.focus();
				alert("持续信元速率\"" + scr.value + "\"为无效数值或范围不在1-" + pcr.value + "之间，请重新输入！");
				return;
			}
			if(svtype.value >= 3 && sji_checkdigitrange(mbs.value, 1, 1000000) == false)
			{
				mbs.focus();
				alert("最大突发信元大小\"" + mbs.value + "\"为无效数值或范围不在1-1000000之间，请重新输入！");
				return;
			}
			
			if(vlan.checked == true && sji_checkdigitrange(vid.value, 0, 4095) == false)
			{
				vid.focus();
				alert("VLAN ID\"" + vid.value + "\"为无效数值或范围不在0-4095之间，请重新输入！");
				return;
			}
			
			if(lkmode.value == 1 && ipmode[1].checked == true && sji_checkvip(ipAddr.value) == false)
			{
				ipAddr.focus();
				alert("IP地址\"" + ipAddr.value + "\"为无效地址，请重新输入！");
				return;
			}

			if(lkmode.value == 1 && ipmode[1].checked == true && sji_checkmask(netMask.value) == false)
			{
				netMask.focus();
				alert("子网掩码\"" + netMask.value + "\"为无效掩码，请重新输入！");
				return;
			}
			if(lkmode.value == 1 && ipmode[1].checked == true && sji_checkvip(remoteIpAddr.value) == false)
			{
				remoteIpAddr.focus();
				alert("缺省网关\"" + remoteIpAddr.value + "\"为无效地址，请重新输入！");
				return;
			}

//star:20090302 START ppp username and password can be empty

			//input text
			if(pppUsername.value!="" && lkmode.value == 1 && ipmode[2].checked == true && sji_checkpppacc(pppUsername.value, 1, 20) == false)
			{
				pppUsername.focus();
				alert("用户名\"" + pppUsername.value + "\"为无效用户名或长度不在1-20字节之间，请重新输入！");
				return;
			}
			if(pppPassword.value!="" && lkmode.value == 1 && ipmode[2].checked == true && sji_checkpppacc(pppPassword.value, 1, 20) == false)
			{
				pppPassword.focus();
				alert("密码存在无效字符或长度不在1-20字节之间，请重新输入！");
				return;
			}

			
			if(pppServiceName.value!=""){
				if(lkmode.value == 1 && ipmode[2].checked == true && sji_checkusername(pppServiceName.value, 1, 20) == false)
				{
					pppServiceName.focus();
					alert("服务名称\"" + pppServiceName.value + "\"为无效名称或长度不在1-20字节之间，请重新输入！");
					return;
				}
			}
			if(pppACName.value!=""){
				if(lkmode.value == 1 && ipmode[2].checked == true && sji_checkusername(pppACName.value, 1, 20) == false)
				{
					pppAC.focus();
					alert("AC名称\"" + pppACName.value + "\"为无效名称或长度不在1-20字节之间，请重新输入！");
					return;
				}
			}
			
			if(lkmode.value == 0)cmode.value = 0;
			else if(ipmode[2].checked)
			{
				cmode.value = 2;
			}
			else
			{
				cmode.value = 1;
				ipDhcp.value = ipmode[0].checked ? 1 : 0;
			}
			
			//checkbox array
			var ptmap = 0;
			for(var i = 0; i < 9; i ++) if(chkpt[i].checked == true) ptmap |= (0x1 << i);
			itfGroup.value = ptmap;

			//check for br and route on the same pvc
			for(var i = 0; i < links.length; i++)
			{
				if(vpi.value != links[i]["vpi"] || vci.value != links[i]["vci"])
					continue;
				if(lkmode.value == 0){
					if(links[i]["cmode"] == 2 && links[i]["brmode"] == 1 && links[i]["itfGroup"] != itfGroup.value){
						alert("PVC\"" + vpi.value + "/" + vci.value + "\"在bridge模式下不能绑定到不同的lan端口，请重新设置!");
						return;
					}
				}
				if(lkmode.value == 1 && ipmode[2].checked==true && brmode.checked==true){
					if(links[i]["cmode"] == 0 && links[i]["itfGroup"] != itfGroup.value){
						alert("PVC\"" + vpi.value + "/" + vci.value + "\"在bridge模式下不能绑定到不同的lan端口，请重新设置!");
						return;
					}
				}
			}
		}
		form.lst.value = tmplst;
		submit();
	}
}

function add_dns()
{
   var loc = "net_dns.asp";
   var code = "window.location.href=\"" + loc + "\"";
   eval(code);
}

</SCRIPT>
</HEAD>

<!-------------------------------------------------------------------------------------->
<!--主页代码-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
	<blockquote>
		<DIV align="left" style="padding-left:20px; padding-top:5px">
			<form id="form" action="/boaform/admin/formAdsl" method="post">
				<div class="tip" style="width:90% ">
					<b>WAN 配置</b><br><br>
					<div id="rstip" style="display:none;"><font color="red">提示：本页面的设置，需要重启路由器才能生效！</font><br></div>
				</div>
				<hr align="left" class="sep" size="1" width="90%">
				<br>
				<table id="tbupmode" cellpadding="0px" cellspacing="2px">
					<tr nowrap><td width="150px">上行方式：</td><td><select id="upmode" name="upmode" disabled=1 onChange="on_ctrlupdate(this)" size="1"></select></td></tr>
					<tr nowrap><td>连接名称：</td><td><select id="lkname" name="lkname" onChange="on_ctrlupdate(this)" size="1"><option value="new" selected>新增WAN连接</option></select></td></tr>
				</table>
				<table id="tblkmode" cellpadding="0px" cellspacing="2px">
					<tr nowrap><td width="150px">模式 ：</td><td><select id="lkmode" name="lkmode" onChange="on_ctrlupdate(this)" size="1"></select></td></tr>
				</table>
				<table id="tbipmode" cellpadding="0px" cellspacing="2px">
					<tr class="sep"><td colspan="2"><hr align="left" class="sep" size="1" width="100%"></td></tr>
					<tr nowrap><td width="150px"><input type="radio" name="ipmode" value="0" onClick="on_ctrlupdate(this)">DHCP</td><td >从ISP处自动得到一个IP地址</td></tr>
					<tr nowrap><td><input type="radio" name="ipmode" value="1" onClick="on_ctrlupdate(this)">Static</td><td>经ISP处配置一个IP静态地址</td></tr>
					<tr nowrap><td><input type="radio" name="ipmode" value="2" onClick="on_ctrlupdate(this)">PPPoE </td><td>若ISP使用PPPOE请选择此项</td></tr>
				</table>
				<table id="tbpppprxy" cellpadding="0px" cellspacing="2px">
					<tr class="sep"><td colspan="3"><hr align="left" class="sep" size="1" width="100%"></td></tr>
					<tr nowrap><td width="150px"><input type="checkbox" name="PPPoEProxyEnable" onClick="on_ctrlupdate(this);"></td><td>启用PPPOE代理</td></tr>
				</table>	
				<table id="tbpppnum" cellpadding="0px" cellspacing="2px">
					<tr nowrap><td width="150px">代理用户数（可用代理<b><label id="lpppnumleft">5</label></b>）</td><td><input type="text" name="PPPoEProxyMaxUser" maxlength="2" size="2" style="width:40px "></td></tr>
				</table>	
				<table id="tbpppbr" cellpadding="0px" cellspacing="2px">
					<tr nowrap><td width="150px"><input type="checkbox" name="brmode"></td><td>PPPoE路由桥混合模式</td></tr>
				</table>	
				<table id="tbpvc" cellpadding="0px" cellspacing="2px">
					<tr class="sep"><td colspan="2"><hr align="left" class="sep" size="1" width="100%"></td></tr>
					<tr nowrap><td width="150px">VPI(0-255)：</td><td><input type="text" name="vpi" value="0" maxlength="3" size="4" style="width:40px "></td></tr>
					<tr nowrap><td>VCI(32-65535)：</td><td><input type="text" name="vci" value="32" maxlength="5" size="4" style="width:40px "></td></tr>
				</table>	
				<table id="tbsvkind" cellpadding="0px" cellspacing="2px">
					<tr class="sep"><td colspan="2"><hr align="left" class="sep" size="1" width="100%"></td></tr>
					<tr nowrap><td width="150px">服务类别：</td><td><select id="svtype" name="svtype" size="1" onChange="on_ctrlupdate(this)"></select></td></tr>
				</table>
				<table id ="tbpeakcell" cellpadding="0px" cellspacing="2px">
					<tr nowrap><td width="150px">峰值信元速率：</td><td><input type="text" name="pcr" value="" maxlength="4" size="4" style="width:40px ">cells/s</td></tr>
				</table>
				<table id="tbothercell" cellpadding="0px" cellspacing="2px">
					<tr nowrap><td width="150px">持续信元速率：</td><td><input type="text" name="scr" value="" maxlength="4" size="4" style="width:40px ">cells/s</td></tr>
					<tr nowrap><td>最大突发信元大小：</td><td><input type="text" name="mbs"  value="" maxlength="4" size="4" style="width:40px ">cells</td></tr>
				</table>
				<table id="tbnat" cellpadding="0px" cellspacing="2px">
					<tr class="sep"><td colspan="2"><hr align="left" class="sep" size="1" width="100%"></td></tr>
					<tr nowrap><td width="150px">启用NAT：</td><td><input type="checkbox" name="napt"></td></tr>
				</table>
				<table id="tbvlan" cellpadding="0px" cellspacing="2px">
					<tr class="sep"><td colspan="2"><hr align="left" class="sep" size="1" width="100%"></td></tr>
					<tr nowrap><td width="150px">启用Vlan：</td><td><input type="checkbox" name="vlan" onClick="on_ctrlupdate(this)"></td></tr>
					<tr nowrap><td>Vlan ID：</td><td><input type="text" name="vid" maxlength="4"  size="4" style="width:40px "></td></tr>
					<tr nowrap><td>802.1p：</td><td><select id="vprio" name="vprio" size="1"></select></td></tr>
				</table>
				<table id="tbdial" cellpadding="0px" cellspacing="2px">
					<tr class="sep"><td colspan="2"><hr align="left" class="sep" size="1" width="100%"></td></tr>
					<tr nowrap><td width="150px">用户名：</td><td><input type="text" name="pppUsername" maxlength="20" size="16" style="width:150px "></td></tr>
					<tr nowrap><td>密码：</td><td><input type="password"name="pppPassword" maxlength="20" size="16" style="width:150px "></td></tr>
					<tr nowrap><td>服务名称：</td><td><input type="text" name="pppServiceName" maxlength="20" size="16" style="width:150px "></td></tr>
					<tr nowrap><td>AC名称：</td><td><input type="text" name="pppACName" maxlength="20" size="16" style="width:150px "></td></tr>
					<tr nowrap><td>拨号方式：</td><td><select id="pppCtype" name="pppCtype" size="1"></select></td></tr>
				</table> 
				<table id="tbip" cellpadding="0px" cellspacing="2px">
					<tr class="sep"><td colspan="2"><hr align="left" class="sep" size="1" width="100%"></td></tr>
					<tr nowrap><td width="150px">IP地址：</td><td><input type="text" name="ipAddr" maxlength="15" size="15" style="width:150px "></td></tr>
					<tr nowrap><td>子网掩码：</td><td><input type="text" name="netMask" maxlength="15" size="15" style="width:150px "></td></tr>
					<tr nowrap><td>缺省网关：</td><td><input type="text" name="remoteIpAddr" maxlength="15" size="15" style="width:150px "></td></tr>
					<!--<tr nowrap><td>首选DNS：</td><td><input type="text" name="fstdns" maxlength="15" size="15" style="width:150px "></td></tr>
					<tr nowrap><td>备选DNS：</td><td><input type="text" name="secdns" maxlength="15" size="15" style="width:150px "></td></tr>-->
				</table>
				<table id="tbdgw" cellpadding="0px" cellspacing="2px">
					<tr class="sep"><td colspan="3"><hr align="left" class="sep" size="1" width="100%"></td></tr>
					<tr nowrap><td width="150px">默认路由: </td><td><input type="checkbox" name="dgw"></td></tr>
				</table>	
				<table id="tbqos" cellpadding="0px" cellspacing="2px">
					<tr class="sep"><td colspan="2"><hr align="left" class="sep" size="1" width="100%"></td></tr>
					<tr nowrap><td width="150px">启用QoS:</td><td><input type="checkbox" name="qos"></td></tr>
					<tr nowrap><td>封装方式:</td><td><select id="encap" name="encap" size="1"></select></td></tr>
					<tr nowrap><td>服务模式:</td><td><select id="applicationtype" onChange="on_ctrlupdate(this)" name="applicationtype" size="1"></select></td></tr>
				</table>
				<table id="tbbind" cellpadding="0px" cellspacing="2px">
					<tr class="sep"><td colspan="2"><hr align="left" class="sep" size="1" width="100%"></td></tr>
					<tr nowrap><td width="150px">绑定端口：</td><td>&nbsp;</td></tr>
					<tr nowrap><td><input type="checkbox" name="chkpt">端口_1</td><td ><input type="checkbox" name="chkpt">端口_2</td></tr>
					<tr nowrap><td><input type="checkbox" name="chkpt">端口_3</td><td ><input type="checkbox" name="chkpt">端口_4</td></tr>
					<tr nowrap><td><input type="checkbox" name="chkpt">无线(ROOT/SSID1)</td></tr>
					<tr nowrap><td><input type="checkbox" name="chkpt">无线(SSID2)</td><td ><input type="checkbox" name="chkpt">无线(SSID3)</td></tr>
					<tr nowrap><td><input type="checkbox" name="chkpt">无线(SSID4)</td><td ><input type="checkbox" name="chkpt">无线(SSID5)</td></tr>
					
				</table>
				<br>
				<div class="tip" id="tbindip" style="width:90% ">
					<tr nowrap><td colspan="2">注: WAN连接之间不能共享绑定端口，最后绑定端口的WAN连接绑定操作将冲掉之前其它WAN连接对该端口的绑定操作！</td></tr>
				</div>
				<br>
				<div class="tip" id="attention" style="width:90% ">
					<tr nowrap><td colspan="2"><font color="red">服务模式为Other的Bridge连接绑定时，端口对应的PC不会通过网关动态得到IP地址。当服务模式为Other时，请注意不要把所有局域网端口绑定为此种情况！</font></td></tr>
				</div>
				<br>
				<table id="tbdns" cellpadding="0px" cellspacing="2px">
					<input type="button" class="button" onClick="add_dns()" value="DNS配置">
				</table>
				<hr align="left" class="sep" size="1" width="90%">
				<br>
				<input type="button" class="button" onClick="on_submit('sv');;" value="应用">&nbsp; &nbsp; &nbsp; &nbsp; 
				<!--<input type="button" class="button" onClick="on_submit('rs');" value="重启">&nbsp; &nbsp; &nbsp; &nbsp;-->
				<input type="button" class="button" onClick="on_submit('rm');" value="删除">&nbsp; &nbsp; &nbsp; &nbsp; 
				<input type="hidden" name="cmode" value="0">
				<input type="hidden" name="ipDhcp" value="0">
				<input type="hidden" name="itfGroup" value="0">
				<input type="hidden" id="lst" name="lst" value="">
				<input type="hidden" id="action" name="action" value="none">
				<input type="hidden" name="submit-url" value="">
				<input type="hidden" id="acnameflag" name="acnameflag" value="have">
			</form>
		</DIV>
	</blockquote>
</body>
<%addHttpNoCache();%>
</HTML>
