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

var lkmodes = new Array("Bridge", "Route");
//var svkinds = new Array("UBR Without PCR", "UBR With PCR", "CBR", "Non Realtime VBR", "Realtime VBR");
//var cpmodes = new Array("LLC", "VC");
//var apmodes = new Array("TR069_INTERNET", "INTERNET", "TR069", "Other", "VOICE", "TR069_VOICE", "VOICE_INTERNET", "TR069_VOICE_INTERNET");
var dlmodes = new Array("自动连接", "有流量时自动连接");
var md802ps = new Array("(无)", "0", "1", "2", "3", "4", "5", "6", "7");
<% initPageEth2(); %>
<% initVlanRange(); %>
var opts = new Array(new Array("lkmode", lkmodes), 
	new Array("applicationtype", apmodes),
	new Array("pppCtype", dlmodes), new Array("vprio", md802ps));

var curlink = null;
var g_dnsMode;
var cgi = new Object();
var links = new Array();
with(links){<% initPageEth(); %>}

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
			lkmode.value = vprio.value = pppCtype.value = 0;
			if (4 == applicationtype.options.length)
			{
				applicationtype.value = 0;
			}
			else
			{
				applicationtype.value = 1;
			}
			
			//radio, default 
			ipmode[2].checked = true;
			
			//checkbox
			//PPPoEProxyEnable.checked = brmode.checked = napt.checked = vlan.checked = qos.checked = dgw.checked = false;
			PPPoEProxyEnable.checked = brmode.checked = napt.checked = vlan.checked = dgw.checked = false;
			
			//checkbox array
			chkpt[0].checked = chkpt[1].checked = chkpt[2].checked = chkpt[3].checked = chkpt[4].checked = chkpt[5].checked = chkpt[6].checked = chkpt[7].checked = chkpt[8].checked = false;
			
			//input number
			PPPoEProxyMaxUser.value = "0";
			
			//input ip
			ipAddr.value = remoteIpAddr.value = "0.0.0.0";
			netMask.value = "255.255.255.255";
			
			//input text
			pppUsername.value = pppPassword.value = pppServiceName.value = "";
			
			// Mason Yu:20110524 ipv6 setting.			
			if(<%checkWrite("IPv6Show");%>) {
				IpProtocolType.value = 3;
				Ipv6Addr.value = "";
				Ipv6PrefixLen.value = "";
				Ipv6Gateway.value = "";
				Ipv6Dns1.value = "";
				Ipv6Dns2.value = "";
				//DSLiteLocalIP.value = "";
				//DSLiteRemoteIP.value = "";
				//DSLiteGateway.value = "";
				document.getElementById('secIPv6Div').style.display="none";
				document.getElementById('DSLiteDiv').style.display="none";
				if (1 == lkmode.value) //Route and enable IPv6, default enable prefix delegation
					iapd.checked = true;	
			}
			
		}
		else
		{
			sji_onchanged(document, itlk);
			
			//select
			lkmode.value = itlk.cmode >= 1 ? 1 : 0;
			g_dnsMode = itlk.dnsMode;
			
			//checkbox array
			var ptmap = itlk.itfGroup;
			for(var i = 0; i < 9; i ++) chkpt[i].checked = (ptmap & (0x1 << i));			
			
			//radio
			//
//			1845 #define CHANNEL_MODE_BRIDGE     0
// 1846 #define CHANNEL_MODE_IPOE        1
//1847 #define CHANNEL_MODE_PPPOE      2
//1848 #define CHANNEL_MODE_PPPOA      3
//1849 #define CHANNEL_MODE_RT1483     4
//1850 #define CHANNEL_MODE_RT1577     5
//1851 #define CHANNEL_MODE_DSLITE     6
			if (curlink.cmode ==2 ) //PPPOE
				ipmode[2].checked = true;
			else if(curlink.IpProtocolType==2 && curlink.cmode == 1 && curlink.AddrMode ==1)  //IPv6 only, Addr mode == SLAAC
				ipmode[0].checked = true;
			else if(curlink.IpProtocolType==2 && curlink.cmode == 1 && curlink.AddrMode ==16)  //IPv6 only, Addr mode == DHCP
					ipmode[0].checked = true;
			else if(curlink.IpProtocolType==2 && curlink.cmode == 1)  //IPv6 only, ipv6 mer
				ipmode[3].checked = true;
			else if(curlink.cmode != 0 && curlink.ipDhcp == 0)
			{
			    if(curlink.cmode ==1 )
					ipmode[1].checked = true; //static
						 
				/*
				// Mason Yu:20110524 ipv6 setting.				
				if (<%checkWrite("IPv6Show");%>) 
				{									
					if ( curlink.IpProtocolType != 2 )
						ipmode[curlink.cmode].checked = true;
					else {   // If protocol is IPv6 only.
						// If it is a MER(cmode=1) mode

						if (curlink.cmodeV6 == 4) {
							if (<%checkWrite("DSLiteShow");%>) 
								ipmode[4].checked = true;
						}
						else 
						if (curlink.cmode == 1)             //DHCP
							ipmode[0].checked = true;
						// If it is a PPPoE(cmode=2) mode
						else
							ipmode[2].checked = true;
					}
					
				}
				else 									
					ipmode[curlink.cmode].checked = true;				
					*/
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
		protocolChange2();
		on_ctrlupdate(lkmode);
		//on_ctrlupdate(svtype);
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

function show_ipv4_only_or_ipv4v6_ipmode()
{
	document.getElementById('tbipmode_v4').style.display="block";
	document.getElementById('tbipmode_v4v6').style.display="block";
	document.getElementById('tbipmode_v4v6_2').style.display="block";
	document.getElementById('tbipmode_v6').style.display="none";	
}


function show_ipv6_only_ipmode()
{
	document.getElementById('tbipmode_v4').style.display="none";
	document.getElementById('tbipmode_v4v6').style.display="block";
	document.getElementById('tbipmode_v4v6_2').style.display="block";
	document.getElementById('tbipmode_v6').style.display="block";	
}

function addrModeChangeAccordingToipmode()
{
	options = document.getElementsByName
	
	document.adsl.AddrMode[0].disabled = false;
	document.adsl.AddrMode[1].disabled = false;
	document.adsl.AddrMode[2].disabled = false;
	
	if(document.adsl.ipmode[0].checked == true )  { //dhcp
		if(document.adsl.AddrMode.value == 2)
			document.adsl.AddrMode.value = 16;
		document.adsl.AddrMode[1].disabled = true;
    }
	else if(document.adsl.ipmode[1].checked == true )  {//static 

			document.adsl.AddrMode[0].disabled = true;
			document.adsl.AddrMode[2].disabled = true;
    }
	else if(document.adsl.ipmode[2].checked == true ) { //pppoe 
	   if(document.adsl.AddrMode.value == 2)
			document.adsl.AddrMode.value = 16;
	   document.adsl.AddrMode[1].disabled = true;	
    }
	else if(document.adsl.ipmode[3].checked == true ) { //ipv6 mer 
			document.adsl.AddrMode[0].disabled = true;
			document.adsl.AddrMode[2].disabled = true;
    }
}

//If connection type is TR069 return false, else return true
function is_configurable()
{
	var lk = document.forms[0].lkname.value;

	if(cwmp_configurable == 0 && lk != "new" && links[lk].name.search("TR069") != -1)
		return false;

	return true;
}

function check_vlan_reserved(vlanID)
{
	var num = reservedVlanA.length;
	//var vlanID = document.forms[0].vid.value;
	for(var i = 0; i<num; i++){
		if(vlanID == reservedVlanA[i])
			return true;
	}
			
	return false;
}

function on_ctrlupdate(obj)
{
	with ( document.forms[0] ) 
	{		
		if(obj.name == "lkname")
		{
			var configurable = is_configurable();
			if(configurable)
			{
				for(var i=0; i<document.forms[0].length;i++)
					document.forms[0].elements[i].disabled = false;
			}

			// Mason Yu:20110524 ipv6 setting.				
			if ( !(<%checkWrite("IPv6Show");%>) ||  (lkmode.value == 0))
			{
				document.getElementById('tbprotocol').style.display="none";
				ipv6SettingsDisable();
			}
			else
			{
				document.getElementById('tbprotocol').style.display="block";
				ipv6SettingsEnable();
			}
			
			if(obj.value == "new")
			{
				curlink = null;
				on_linkchange(curlink);
				//pcr.value = 6000;
			}
			else
			{
				curlink = links[obj.value];
				on_linkchange(curlink);
				// Mason Yu:20110524 ipv6 setting.				
			}

			if(!configurable)
			{
				for(var i=0; i<document.forms[0].length;i++)
				{
					if(document.forms[0].elements[i].name != "lkname")
						document.forms[0].elements[i].disabled = true;
				}
			}
		}
		else if(obj.name == "lkmode")
		{
			tbipmode_v4.style.display = obj.value == 0 ? "none" : "block";
			tbipmode_v4v6.style.display = obj.value == 0 ? "none" : "block";
			tbipmode_v4v6_2.style.display = obj.value == 0 ? "none" : "block";
			tbipmode_v6.style.display = obj.value == 0 ? "none" : "block";
			tbmtu.style.display = obj.value == 0 ? "none" : "block";			
			tbnat.style.display = obj.value == 0 ? "none" : "block";
			tbdgw.style.display = obj.value == 0 ? "none" : "block";
			if(obj.value == 1 && applicationtype.value != 2) 
			{					
				napt.checked = true;
			}
			else{				
				napt.checked = false;
				tbnat.style.display = "none";
				tbdgw.style.display = "none";
			}
			
			// Mason Yu:20110524 ipv6 setting.			
			if (<%checkWrite("IPv6Show");%>)
			{
				// If cmode is Router
				if(obj.value == 1)
				{
					document.getElementById('tbprotocol').style.display="block";
					if(document.getElementById('IpProtocolType').value == 2 || document.getElementById('IpProtocolType').value == 3)
					ipv6SettingsEnable();
					if(document.getElementById('IpProtocolType').value == 1)	
						ipv6SettingsDisable();
						
					if((curlink==null) &&(document.getElementById('IpProtocolType').value != 1)) //new link, Router mode, and protocol is not IPv4,default enable iapd
						iapd.checked = true;
				}
				// If cmode is Bridge
				else
				{
					document.getElementById('tbprotocol').style.display="block";
					ipv6SettingsDisable();
				}			
			}			
			
			on_ctrlupdate(ipmode[0]);
			
			var orgapptype;

			if (0 == obj.value)
			{
				if (2 != applicationtype.options.length)
				{
					orgapptype = applicationtype.value;

					// INTERNET and Other option
					applicationtype.options.length = 0;
					applicationtype.options.add(new Option(apmodes[1], 1));
					applicationtype.options.add(new Option(apmodes[3], 3));

					if ((1 != orgapptype) && (3 != orgapptype))
					{
						applicationtype.value = 1;
					}
					else
						applicationtype.value = orgapptype;
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
			tbdnsset.style.display = "none";
			if (cgi.poe_proxy) {
				tbpppprxy.style.display = "none";
				tbpppbr.style.display = "none";
				tbpppnum.style.display = "none";
			}
			//if(<%checkWrite("IPv6Show");%>)
			//	DSLiteDiv.style.display = "none";
			
			if(lkmode.value == 1)
			{
				//alert("g_dnsMode="+g_dnsMode);				
				if ( g_dnsMode == 0)
					// disable
					document.adsl.dnsMode[1].checked = true;
				else
					// enable
					document.adsl.dnsMode[0].checked = true;
					
				if (ipmode[0].checked == true)                             //DHCP, IPv4 only, or IPv4/IPv6
				{					
					show_ipv4_only_or_ipv4v6_ipmode();
					tbdnsset.style.display = "block";
					document.adsl.dnsMode[0].disabled = false;
					document.adsl.dnsMode[1].disabled = false;
					mtu.value = 1500;
					dnsModeClicked();
				}
				else if (ipmode[1].checked == true)                        //static, IPv4 only, or IPv4/IPv6
				{
					show_ipv4_only_or_ipv4v6_ipmode();
					tbip.style.display = "block";
					tbdnsset.style.display = "block";
					document.adsl.dnsMode[1].checked = true;
					document.adsl.dnsMode[0].disabled = true;
					document.adsl.dnsMode[1].disabled = true;
					mtu.value = 1500;
					dnsModeClicked();
				}				
				else if (ipmode[2].checked == true)            				//PPPoE, IPv4 only, or IPv4/IPv6, or IPv6 only
				{
					show_ipv4_only_or_ipv4v6_ipmode();
					tbdial.style.display = "block";
					if (cgi.poe_proxy) {
						if (applicationtype.value != 2)
						{
							tbpppprxy.style.display = "block";
							tbpppbr.style.display = "block";
							on_ctrlupdate(PPPoEProxyEnable);
						}else
							tbpppbr.style.display = "block";
					}
					
					if(mtu.value > 1492)
						mtu.value = 1492;
						
					if(<%checkWrite("IPv6Show");%> && (document.getElementById('IpProtocolType').value != 1)) 
						ipv6SettingsEnable();
					if(<%checkWrite("IPv6Show");%> && (document.getElementById('IpProtocolType').value == 2)) 
						show_ipv6_only_ipmode();
				}
				else if(<%checkWrite("IPv6Show");%> && (document.getElementById('IpProtocolType').value == 2)) {           
					if (ipmode[3].checked == true){ 			//IPv6 MER
						show_ipv6_only_ipmode();
						ipv6SettingsEnable();
						mtu.value = 1500;
					}
					
				}
				
					if(<%checkWrite("IPv6Show");%> && (document.getElementById('IpProtocolType').value != 1)) {
						ipv6SettingsEnable();
						
					}	
					if(<%checkWrite("IPv6Show");%> && (document.getElementById('IpProtocolType').value == 2)) 
						show_ipv6_only_ipmode();				
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
		else if(obj.name == "mtu")
		{
			mtu.disabled = !obj.checked;
		}		
		else if(obj.name == "applicationtype")
		{
			if(obj.value == 2 || obj.value == 4 || obj.value == 5){
				napt.checked = false;
				tbnat.style.display = "none";
				tbbind.style.display = "none";
				tbindip.style.display = "none";
				dgw.checked = false;
				tbdgw.style.display = "none";
				
				if (cgi.poe_proxy) {
					PPPoEProxyEnable.checked = false;
                                	
					tbpppprxy.style.display = "none";
					//tbpppbr.style.display = "none";
									
					//brmode.checked = false;				
					on_ctrlupdate(PPPoEProxyEnable);
				}
				
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

					
					if (cgi.poe_proxy && ipmode[2].checked == true)
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
		if (!cgi.poe_proxy) {
			tbpppprxy.style.display = "none";
			tbpppbr.style.display = "none";
			tbpppnum.style.display = "none";
		}
		for(var k in links)
		{
			var lk = links[k];
			lkname.options.add(new Option(lk.name, k));
		}
		if(links.length > 0)lkname.value = 0;
		lpppnumleft.innerHTML = cgi.pppnumleft;
		on_ctrlupdate(lkname);
		if (cgi.vlan_map != null) {
			for (var i=0; i<9; i++) {
				if (cgi.vlan_map & (0x1 << i))
				chkpt[i].disabled = true;
			}
		}	
		mtu.value = 1500;

		protocolChange2();
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
			if(lkmode.value == 0 && (applicationtype.value != 1 && applicationtype.value != 3))
			{
				lkmode.focus();
				alert("设置错误，桥模式下服务模式不支持tr069和语音服务!");
				return;
			}
			
			if(lkmode.value != 0) {
				if (<%checkWrite("IPv6Show");%>) {
					if (<%checkWrite("DSLiteShow");%>) {
						if ( ipmode[0].checked == false  && 
							ipmode[1].checked == false  &&
							ipmode[2].checked == false  &&
							ipmode[3].checked == false /* &&
							ipmode[4].checked == false   */ ) {
							alert("请设置ISP服务模式！(DHCP, static, PPPoE, IPv6 MER");
							return;				 
						}
					}
					else {
						if ( ipmode[0].checked == false  && 
							ipmode[1].checked == false  &&
							ipmode[2].checked == false  &&
							ipmode[3].checked == false  ) {
							alert("请设置ISP服务模式！(DHCP, static, PPPoE, IPv6 MER");
							return;				 
						}
					}
				}
				else {
					if ( ipmode[0].checked == false  && 
						 ipmode[1].checked == false  &&
						 ipmode[2].checked == false  ) {						
						alert("请设置ISP服务模式！");
						return;				 
					}
				}
			}
			
			//input number
			if(cgi.poe_proxy==1 && lkmode.value != 0 && ipmode[2].checked == true && sji_checkdigitrange(PPPoEProxyMaxUser.value, 0, cgi.pppnumleft) == false)
			{
				PPPoEProxyMaxUser.focus();
				alert("代理用户数\"" + PPPoEProxyMaxUser.value + "\"为无效用户数，请重新输入！");
				return;
			}
			if(vlan.checked == true && ((sji_checkdigitrange(vid.value, otherVlanStart, otherVlanEnd) == true) || (check_vlan_reserved(vid.value) == true)))
			{
				vid.focus();
				//alert("VLAN ID\"" + vid.value + "\"为无效数值，请重新输入！");
				alert("VLAN ID \"" + alertVlanStr + "\" 为系统保留数值，请重新输入！");
				 
				return;
			}
			if(sji_checkdigitrange(mtu.value, 576, 1500) == false)
			{
				mtu.focus();
				alert("MTU\"" + mtu.value + "\"为无效数值或范围不在576-1500之间，请重新输入！");
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
			
			if (v4dns1.value != "")
			{
				if(lkmode.value == 1 && (ipmode[0].checked == true || ipmode[1].checked == true ) && dnsMode[1].checked == true && sji_checkvip(v4dns1.value) == false)
				{
					v4dns1.focus();
					alert("首选DNS地址\"" + v4dns1.value + "\"为无效地址，请重新输入！");
					return;
				}
			}
			
			if (v4dns2.value != "")
			{
			if(lkmode.value == 1 && (ipmode[0].checked == true || ipmode[1].checked == true ) && dnsMode[1].checked == true && sji_checkvip(v4dns2.value) == false)
				{
					v4dns2.focus();
					alert("备选DNS地址\"" + v4dns2.value + "\"为无效地址，请重新输入！");
					return;
				}
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
			
		//	cmodeV6.value = 0;
			if(lkmode.value == 0)cmode.value = 0;
			else if(ipmode[2].checked)
			{
				cmode.value = 2;
			}
			else if ((ipmode[0].checked) || (ipmode[1].checked))
			{					
				cmode.value = 1;
				ipDhcp.value = ipmode[0].checked ? 1 : 0;
			}
			else if(<%checkWrite("IPv6Show");%>) {
				//alert("lkmode.value="+lkmode.value);
				if(ipmode[3].checked) 
					cmode.value = 1;
				else if (<%checkWrite("DSLiteShow");%>) {
				/*
					if(ipmode[4].checked) {
						cmode.value = 1;
			//			cmodeV6.value = 4;
					}
					*/
				}
			}			
			
			//checkbox array
			var ptmap = 0;
			for(var i = 0; i < 9; i ++) if(chkpt[i].checked == true) ptmap |= (0x1 << i);
			itfGroup.value = ptmap;
			
			<!-- Mason Yu:20110524 ipv6 setting -->
			if (<%checkWrite("IPv6Show");%>) {					
				if(document.getElementById('IpProtocolType').value == 3 || document.getElementById('IpProtocolType').value == 2)
				{

					if(document.adsl.lkmode.value != 0) {					
/*
						if(document.adsl.staticIpv6.checked == false &&
						document.adsl.itfenable.checked == false &&
						document.adsl.slacc.checked == false &&
						document.adsl.DSLiteRemoteIP.value == "" ) {
							alert("请设置IPv6 WAN信息获取方式/DS-Lite WAN 配置!");									
							return false;
						}						
*/						
						if(document.getElementById('AddrMode').value==2 && (document.adsl.ipmode[0].checked || document.adsl.ipmode[1].checked || document.adsl.ipmode[2].checked || document.adsl.ipmode[3].checked)) {						
								if(document.adsl.Ipv6Addr.value == "" ){
									alert("请设置ipv6地址或 open DHCPv6 client!"); //Please input ipv6 address or open DHCPv6 client!
									document.adsl.Ipv6Addr.focus();
									return false;
								}
							if(document.adsl.Ipv6Addr.value != ""){
								if (! isGlobalIpv6Address( document.adsl.Ipv6Addr.value) ){
									alert("无效的ipv6地址!"); //Invalid ipv6 address!
									document.adsl.Ipv6Addr.focus();
									return false;
								}
								var prefixlen= getDigit(document.adsl.Ipv6PrefixLen.value, 1);
								if (prefixlen > 128 || prefixlen <= 0) {
									alert("无效的ipv6前缀长度!"); //Invalid ipv6 prefix length!
									document.adsl.Ipv6PrefixLen.focus();
									return false;
								}
							}
							if(document.adsl.Ipv6Gateway.value != "" ){
								if (! isUnicastIpv6Address( document.adsl.Ipv6Gateway.value) ){
									alert("无效的ipv6网关地址!"); //Invalid ipv6 gateway address!
									document.adsl.Ipv6Gateway.focus();
									return false;
								}
							}
							if(document.adsl.Ipv6Dns1.value != "" ){
								if (! isUnicastIpv6Address( document.adsl.Ipv6Dns1.value) ){
									alert("首选 IPv6 DNS 地址\"" + document.adsl.Ipv6Dns1.value + "\"为无效地址，请重新输入！");
									document.adsl.Ipv6Dns1.focus();
									return false;
								}
							}
							if(document.adsl.Ipv6Dns2.value != "" ){
								if (! isUnicastIpv6Address( document.adsl.Ipv6Dns2.value) ){
									alert("备选 IPv6 DNS 地址\"" + document.adsl.Ipv6Dns1.value + "\"为无效地址，请重新输入！");
									document.adsl.Ipv6Dns2.focus();
									return false;
								}
							}
							//document.adsl.DSLiteLocalIP.value == "";
							///document.adsl.DSLiteRemoteIP.value == "";
							//document.adsl.DSLiteGateway.value == "";
						}						
						// Check DS-Lite parameter
						if (<%checkWrite("DSLiteShow");%>) {
							if(dslite_enable.checked == true){
								var dslitemode =dslite_aftr_mode.value;
								if(dslitemode=="1") { //static	
									if(dslite_aftr_hostname.value == ""){ //Should not be empty
										alert(dslite_aftr_hostname.value+"為空?FTR 地址或域名,请重新输入!");
										dslite_aftr_hostname.focus();
									return false;
								}
								
									var index_of_dot = dslite_aftr_hostname.value.indexOf('.');
									if(!isUnicastIpv6Address(dslite_aftr_hostname.value) &&
									   (index_of_dot==-1) || (dslite_aftr_hostname.value.charAt(0)=='.') || ( (dslite_aftr_hostname.value.charAt(dslite_aftr_hostname.value.length-1)=='.')))  {
										//Not unicast IPv6 address and not is a hostname (has character '.')
										alert(dslite_aftr_hostname.value+"为不正确AFTR 地址或域名,请重新输入!");
										dslite_aftr_hostname.focus();
										return false;
								
								}
									}
								}
						}
						
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

function add_dsl()
{
   var loc = "net_dsl.asp";
   var code = "window.location.href=\"" + loc + "\"";
   eval(code);
}

// Mason Yu:20110524 ipv6 setting. START

function ipv6SettingsEnable()
{
    addrModeChangeAccordingToipmode();
	if(document.getElementById('IpProtocolType').value != 1){  // not IPv4 only
	/*
	   	if (<%checkWrite("DSLiteShow");%> && document.adsl.ipmode[4].checked == true)
			document.getElementById('DSLiteDiv').style.display="block";	
		else { //not dslite
		*/
			document.getElementById('tbipv6wan').style.display="block";
		//}	
		
		if(document.getElementById('IpProtocolType').value == 2){  //IPv6 only	
			document.getElementById('tbip').style.display="none";
			document.getElementById('DSLiteDiv').style.display="block";
			dsliteSettingChange();
		}	
  	}
	wanAddrModeChange();
	
}

function wanAddrModeChange()
{
	document.getElementById('secIPv6Div').style.display="none";
	document.getElementById('dhcp6c_ctrlblock').style.display="none";	
	document.getElementById('DSLiteDiv').style.display="none";	

	if(document.getElementById('AddrMode').value==""){

		if(document.adsl.ipmode[0].checked == true )  //dhcp
			document.getElementById('AddrMode').value = 1; 
		else if(document.adsl.ipmode[1].checked == true ) //static 
			document.getElementById('AddrMode').value = 2; //set to static
		else if(document.adsl.ipmode[2].checked == true ) //pppoe
			document.getElementById('AddrMode').value = 1; // set to slaac
		else if(document.adsl.ipmode[3].checked == true ) //ipv6 mer
			document.getElementById('AddrMode').value = 2; //set to slaac
		
	}
	
	if( document.getElementById('IpProtocolType').value == 3 && document.adsl.ipmode[1].checked == true ){ //ipv4/v6  and static
		document.getElementById('AddrMode').value = 2;	
	}  
	else if(document.adsl.ipmode[3].checked == true){ //ipv6 mer
		document.getElementById('AddrMode').value = 2;	
	} 
    //else if (document.getElementById('IpProtocolType').value!=1 && ( ) //ipmode not static , means dhcp or pppoe. then addrmode should not be static
	//	document.getElementById('AddrMode').value = 1; 
	
	switch(document.getElementById('AddrMode').value)
	{
		case '1':  //SLAAC
			break;
		case '2':	//Static
			document.getElementById('secIPv6Div').style.display="block";
			break;
		case '16': //DHCP
			//document.getElementById('dhcp6c_ctrlblock').style.display="block";
			document.adsl.iana.checked = true;
			break;
	}

	if(document.getElementById('IpProtocolType').value == 2){  //IPv6 only	
		document.getElementById('DSLiteDiv').style.display="block";
	}
}

function ipv6SettingsDisable()
{
	document.getElementById('tbipmode_v6').style.display="none";
	document.getElementById('tbipv6wan').style.display="none";
	document.getElementById('secIPv6Div').style.display="none";
	document.getElementById('dhcp6c_ctrlblock').style.display="none";	
	document.getElementById('DSLiteDiv').style.display="none";	
}

function showUIBridgeMode()
{
		document.getElementById('tbipmode_v4').style.display="none";
		document.getElementById('tbipmode_v4v6').style.display="none";
		document.getElementById('tbipmode_v4v6_2').style.display="none";
		document.getElementById('tbipmode_v6').style.display="none";
		document.getElementById('tbip').style.display = "none";
}

function protocolChange2()
{
	ipver = document.getElementById('IpProtocolType').value;

	// If protocol is IPv4 only.
	if(document.getElementById('IpProtocolType').value == 1){	//IPv4 only		
		if(document.adsl.ipmode[3].checked == true)
			document.adsl.ipmode[1].checked = true; 
		//document.adsl.ipmode[0].disabled = false;
		document.adsl.ipmode[1].disabled = false;
		//document.adsl.ipmode[2].disabled = false;
		document.adsl.ipmode[3].disabled = true;
		document.adsl.dslite_enable.checked = false;
		document.adsl.ipmode[3].checked = false;		
		document.getElementById('tbipmode_v4').style.display="block";
		document.getElementById('tbipmode_v4v6').style.display="block";
		document.getElementById('tbipmode_v4v6_2').style.display="block";
		document.getElementById('tbipmode_v6').style.display="none";			
		
		if(document.adsl.ipmode[1].checked == true) //show static UI
			document.getElementById('tbip').style.display = "block";
			
		ipv6SettingsDisable();			
	}else if(document.getElementById('IpProtocolType').value == 2){  //IPv6 only
		if(document.adsl.ipmode[1].checked == true)
			document.adsl.ipmode[3].checked = true;	
		//document.adsl.ipmode[0].disabled = false;
		document.adsl.ipmode[1].disabled = true;
		//document.adsl.ipmode[2].disabled = false;
		document.adsl.ipmode[3].disabled = false;
		//document.adsl.ipmode[0].checked = false;
		document.adsl.ipmode[1].checked = false;
		document.getElementById('tbipmode_v4').style.display="none";
		document.getElementById('tbipmode_v4v6').style.display="block";
		document.getElementById('tbipmode_v4v6_2').style.display="block";
		document.getElementById('tbipmode_v6').style.display="block";						
		if ( document.adsl.lkmode.value == 1 )
			ipv6SettingsEnable();
		}
	else {  //IPv4/IPv6
		if(document.adsl.ipmode[3].checked == true)
			document.adsl.ipmode[1].checked = true; 	
		//document.adsl.ipmode[0].disabled = false;
		document.adsl.ipmode[1].disabled = false;
		//document.adsl.ipmode[2].disabled = false;
		document.adsl.ipmode[3].disabled = true;	
		document.adsl.ipmode[3].checked = false;	
		document.adsl.dslite_enable.checked = false;
		document.getElementById('tbipmode_v4').style.display="block";
		document.getElementById('tbipmode_v4v6').style.display="block";
		document.getElementById('tbipmode_v4v6_2').style.display="block";
		document.getElementById('tbipmode_v6').style.display="none";	
		if ( document.adsl.lkmode.value == 1 )
			ipv6SettingsEnable();
			
		if(document.adsl.ipmode[1].checked == true) //show static UI
			document.getElementById('tbip').style.display = "block";			
	}	

	with ( document.forms[0] ) 
	{
		if(lkmode.value==0) { //bridge mode
			showUIBridgeMode();
		}	
	}	
}
// Mason Yu:20110524 ipv6 setting. END

function dnsModeClicked()
{
	if ( document.adsl.dnsMode[0].checked )
	{		
		document.adsl.v4dns1.disabled = true;		
		document.adsl.v4dns2.disabled = true;
	}
	
	if ( document.adsl.dnsMode[1].checked )
	{		
		document.adsl.v4dns1.disabled = false;		
		document.adsl.v4dns2.disabled = false;
	}
}

function dsliteSettingChange()
{
	with ( document.forms[0] ) 
	{
		if(dslite_enable.checked == true){
			dslite_mode_div.style.display = 'block';
			dsliteAftrModeChange();
		}	
		else{
			dslite_mode_div.style.display = 'none';
			dslite_aftr_hostname_div.style.display = 'none';
		}	
	}	
}

function dsliteAftrModeChange()
{

	with ( document.forms[0] ) 
	{
		var dslitemode =dslite_aftr_mode.value;
		
		dslite_aftr_hostname_div.style.display = 'none';	
		switch(dslitemode){
			case '0': //AUTO
					break;
			case '1': //Static
					dslite_aftr_hostname_div.style.display = 'block';
					if(dslite_aftr_hostname.value == "::") //clear the value
						dslite_aftr_hostname.value ="";

					break;
					
		}
	}

}

</SCRIPT>
</HEAD>

<!-------------------------------------------------------------------------------------->
<!--主页代码-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
	<blockquote>
		<DIV align="left" style="padding-left:20px; padding-top:5px">
			<form id="form" action="/boaform/admin/formEthernet" method="post" name="adsl">
				<div class="tip" style="width:90% ">
					<b>WAN 配置</b><br><br>
					<div id="rstip" style="display:none;"><font color="red">提示：本页面的设置，需要重启路由器才能生效！</font><br></div>
				</div>
				<hr align="left" class="sep" size="1" width="90%">
				<br>
				<table id="tbupmode" cellpadding="0px" cellspacing="2px">
					<!--<tr nowrap><td width="150px">上行方式：</td><td><select id="upmode" name="upmode" disabled=1 onChange="on_ctrlupdate(this)" size="1"></select></td></tr>-->
					<tr nowrap><td width="150px">连接名称：</td><td><select id="lkname" name="lkname" onChange="on_ctrlupdate(this)" size="1"><option value="new" selected>新增WAN连接</option></select></td></tr>
				</table>
				<table id="tblkmode" cellpadding="0px" cellspacing="2px">
					<tr nowrap><td width="150px">模式 ：</td><td><select id="lkmode" name="lkmode" onChange="on_ctrlupdate(this)" size="1"></select></td></tr>
				</table>
				
				<!-- Mason Yu:20110524 ipv6 setting -->	
				<div id=tbprotocol style="display:none">
				<table cellpadding="0px" cellspacing="2px">				
					<tr nowrap id=TrIpProtocolType>
						<td width="150px">IP协议版本:</td>
						<td><select id="IpProtocolType" onChange="protocolChange2()" name="IpProtocolType">
							<option value="3" > Ipv4/Ipv6</option>
							<option value="1" > Ipv4</option>
							<option value="2" > Ipv6</option>
							</select>
						</td>
					</tr>
				</table>
				</div>
				
				<div  id="tbipmode_v4v6_2" >
				
				<table cellpadding="0px" cellspacing="2px">
				<tr class="sep"><td colspan="2"><hr align="left" class="sep" size="1" width="100%"></td></tr>
				<tr nowrap><td width="150px"><input type="radio" id="ipmode"  name="ipmode" value="0" onClick="on_ctrlupdate(this)">DHCP</td><td >从ISP处自动得到一个地址</td></tr>
				</table>					
				</div> 
				
				<div  id="tbipmode_v4" >
				<table  cellpadding="0px" cellspacing="2px">
					
					<tr class="sep"><td colspan="2"><hr align="left" class="sep" size="1" width="100%"></td></tr>					
					<tr nowrap><td  width="150px"><input type="radio" id="ipmode" name="ipmode" value="1" onClick="on_ctrlupdate(this)">Static</td><td>经ISP处配置一个静态地址</td></tr>					
				</table>	
                </div>
				<div  id="tbipmode_v4v6" >
				<table  cellpadding="0px" cellspacing="2px">
				
				<tr class="sep"><td colspan="2"><hr align="left" class="sep" size="1" width="100%"></td></tr>
				<tr nowrap><td width="150px"><input type="radio" id="ipmode"  name="ipmode" value="2" onClick="on_ctrlupdate(this)">PPPoE </td><td>若使用PPPOE请选择此项</td></tr>
				</table>	
				</div>
				
                <div  id="tbipmode_v6">
				<table cellpadding="0px" cellspacing="2px">
					<tr class="sep"><td colspan="2"><hr align="left" class="sep" size="1" width="100%"></td></tr>
					<% checkWrite("IPv6vcCheck9"); %>
				</table>				
				</div>
				
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
				<!--
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
					<tr nowrap><td width="150px">峰值信元速率：</td><td><input type="text" name="pcr" value="" maxlength="5" size="5" style="width:40px ">cells/s</td></tr>
				</table>
				<table id="tbothercell" cellpadding="0px" cellspacing="2px">
					<tr nowrap><td width="150px">持续信元速率：</td><td><input type="text" name="scr" value="" maxlength="4" size="4" style="width:40px ">cells/s</td></tr>
					<tr nowrap><td>最大突发信元大小：</td><td><input type="text" name="mbs"  value="" maxlength="5" size="5" style="width:40px ">cells</td></tr>
				</table>
				-->
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
				<table id="tbmtu" cellpadding="0px" cellspacing="2px">
					<tr class="sep"><td colspan="2"><hr align="left" class="sep" size="1" width="100%"></td></tr>
					<tr nowrap><td width="150px">MTU：</td><td><input type="text" name="mtu" maxlength="4"  size="4" style="width:40px "></td></tr>
				</table>
				<table id="tbdial" cellpadding="0px" cellspacing="2px">
					<tr class="sep"><td colspan="2"><hr align="left" class="sep" size="1" width="100%"></td></tr>
					<tr nowrap><td width="150px">用户名：</td><td><input type="text" name="pppUsername" maxlength="20" size="16" style="width:150px "></td></tr>
					<tr nowrap><td>密码：</td><td><input type="password"name="pppPassword" maxlength="IP协议版本20" size="16" style="width:150px "></td></tr>
					<tr nowrap><td>服务名称：</td><td><input type="text" name="pppServiceName" maxlength="20" size="16" style="width:150px "></td></tr>
					<tr nowrap><td>拨号方式：</td><td><select id="pppCtype" name="pppCtype" size="1"></select></td></tr>
				</table> 
				<div id="tbip" >
				<table cellpadding="0px" cellspacing="2px">
					<tr class="sep"><td colspan="2"><hr align="left" class="sep" size="1" width="100%"></td></tr>
					<tr nowrap><td width="150px">IP地址：</td><td><input type="text" name="ipAddr" maxlength="15" size="15" style="width:150px "></td></tr>
					<tr nowrap><td>子网掩码：</td><td><input type="text" name="netMask" maxlength="15" size="15" style="width:150px "></td></tr>
					<tr nowrap><td>缺省网关：</td><td><input type="text" name="remoteIpAddr" maxlength="15" size="15" style="width:150px "></td></tr>
					<!--<tr nowrap><td>首选DNS：</td><td><input type="text" name="fstdns" maxlength="15" size="15" style="width:150px "></td></tr>
					<tr nowrap><td>备选DNS：</td><td><input type="text" name="secdns" maxlength="15" size="15" style="width:150px "></td></tr>-->
				</table>				
				</div>
				<table id="tbdnsset" cellpadding="0px" cellspacing="2px">
					<tr class="sep"><td colspan="2"><hr align="left" class="sep" size="1" width="100%"></td></tr>
					<tr nowrap>
						<td width="150px">Request DNS:</td>
						<td><input type="radio" value="1" name="dnsMode" checked onClick='dnsModeClicked()'>Enable</td>
					</tr>
					<tr nowrap>
						<td width="150px"></td>
						<td><input type="radio" value="0" name="dnsMode" onClick='dnsModeClicked()'>Disable</td>							
					</tr>
					<tr nowrap>		
						<td width="150px">首选DNS:</td>
						<td><input type="text" name="v4dns1" size="18" maxlength="15" value= ></td>
					</tr>
					<tr nowrap>
						<td width="150px">备选DNS:</td>
						<td><input type=text name="v4dns2" size="18" maxlength="15" value=></td>
					</tr>
				</table>
				<table id="tbdgw" cellpadding="0px" cellspacing="2px">
					<tr class="sep"><td colspan="3"><hr align="left" class="sep" size="1" width="100%"></td></tr>
					<tr nowrap><td width="150px">默认路由: </td><td><input type="checkbox" name="dgw"></td></tr>
				</table>	
				<table id="tbqos" cellpadding="0px" cellspacing="2px">
					<tr class="sep"><td colspan="2"><hr align="left" class="sep" size="1" width="100%"></td></tr>
					<!--<tr nowrap><td width="150px">启用QoS:</td><td><input type="checkbox" name="qos"></td></tr>-->
					<!--<tr nowrap><td>封装方式:</td><td><select id="encap" name="encap" size="1"></select></td></tr>-->
					<tr nowrap><td width="150px">服务模式:</td><td><select id="applicationtype" onChange="on_ctrlupdate(this)" name="applicationtype" size="1"></select></td></tr>
				</table>
				<!--<table id="tbbind" cellpadding="0px" cellspacing="2px">
					<tr class="sep"><td colspan="2"><hr align="left" class="sep" size="1" width="100%"></td></tr>
					<tr nowrap><td width="150px">绑定端口：</td><td>&nbsp;</td></tr>
					<tr nowrap><td><input type="checkbox" name="chkpt">端口_1</td><td ><input type="checkbox" name="chkpt">端口_2</td></tr>
					<tr nowrap><td><input type="checkbox" name="chkpt">端口_3</td><td ><input type="checkbox" name="chkpt">端口_4</td></tr>
					<tr nowrap><td><input type="checkbox" name="chkpt">无线(ROOT/SSID1)</td></tr>
					<tr nowrap><td><input type="checkbox" name="chkpt">无线(SSID2)</td><td ><input type="checkbox" name="chkpt">无线(SSID3)</td></tr>
					<tr nowrap><td><input type="checkbox" name="chkpt">无线(SSID4)</td><td ><input type="checkbox" name="chkpt">无线(SSID5)</td></tr>
					
				</table>-->
				<% ShowPortMapping(); %>
				
				<!-- Mason Yu:20110524 ipv6 setting -->
				<div id=tbipv6wan style="display:block">
				<table border=0 width="600" cellspacing=4 cellpadding=0>
					<tr><td width="150px"><font size=1>IPv6 WAN信息获取方式:</td></tr>
					<tr nowrap id=TrIpv6AddrType>
						<td width="150px"><font size=1>全局地址获取方式:</td>
						<td><select id="AddrMode" style="WIDTH: 150px" onChange="wanAddrModeChange()" name="AddrMode">
						    <option value="1" >无状态自动配置</option>                                                              
						    <option value="2" >手动</option>                                                                   
						    <option value="16" > DHCP</option>                                                                   
						    </select>                                                                                           
						</td>                             
					<tr>
						<td width="150px"><font size=1>DHCP启用前缀代理</td>
						<td>
							<input type="checkbox" value="ON" name="iapd" id="send2">
						</td>
					</tr>                        
					</tr>
				</table>
				</div>
				
				<div id=DSLiteDiv style="display:none">
                <table border=0 width="600" cellspacing=4 cellpadding=0>
					<tr><td width="150px">DS-Lite 启用:</td>
						<td> <input type="checkbox" value="ON" name="dslite_enable" id="dslite_enable" onChange="dsliteSettingChange()"> </td>
					</tr>
                </table>
					<div id="dslite_mode_div" style="display:none;"><br>
					<table border="0" cellpadding="0" cellspacing="0">
						<tr>
						<td width="150">AFTR 地址获取方式</td>
						<td><select name="dslite_aftr_mode"  onChange="dsliteAftrModeChange()">
  							<option value="0">DHCPv6</option>
  							<option value="1">手动</option>
							</select></td> 
					</tr>
                </table>
                </div>
				
					<div id='dslite_aftr_hostname_div' style="display:none;"> 
					<table border="0" cellpadding="0" cellspacing="0">
					<tr>
						<td width="150px">AFTR 地址或域名:</td>
						<td><input type="text" name="dslite_aftr_hostname" size="64" maxlength="64"  style="width:150px"></td>
					</tr>
                </table>
                </div>
                </div>
				
				<div id=secIPv6Div style="display:none">
				<table cellpadding="0px" cellspacing="2px">
					<tr id=TrIpv6Addr>
						<td width="150px">IPv6 地址:</td>
						<td nowrap><input  id="Ipv6Addr" maxLength=39 size=36 name="Ipv6Addr" style="width:150px ">
						/
						<input id="Ipv6PrefixLen" maxLength=3 size=3 name="Ipv6PrefixLen">
						</td>
					</tr>
					<tr id=TrIpv6Gateway>
						<td width="150px">IPv6 网关:</td>
						<td><input  id="Ipv6Gateway"  maxLength=39 size=36 name="Ipv6Gateway" style="width:150px "></td>
					</tr>
					<tr>
						<td width="150px">首选 IPv6 DNS:</td>
						<td><input type="text" name="Ipv6Dns1" size="36" maxlength="39" value="" style="width:150px"></td>
					</tr>
					<tr>
						<td width="150px">备选 IPv6 DNS:</td>
						<td><input type=text name="Ipv6Dns2" size="36" maxlength="39" value="" style="width:150px"></td>
					</tr>
				</table>
				</div>
				
				<br>
				<div  id="dhcp6c_ctrlblock"  style="display:none">
					
					<table  border=0 cellspacing=4 cellpadding=0>
					<tr nowrap>
						<td width="150px"><font size=1>Request Options:</td>
						<td ></td>
					</tr>
					<tr nowrap>
						<td width="150px"><font size=1><b></b></td>
						<td>
							<input type="checkbox" value="ON" name="iana" id="send1"><font size=1>Request Address
						</td>
					</tr>
					</table>					
				</div>				
				
			   
				<br>
				<div class="tip" id="tbindip" style="width:90% ">
					<tr nowrap><td colspan="2">注: WAN连接之间不能共享绑定端口，最后绑定端口的WAN连接绑定操作将冲掉之前其它WAN连接对该端口的绑定操作！</td></tr>
				</div>
				<br>
				<div class="tip" id="attention" style="width:90% ">
					<tr nowrap><td colspan="2"><font color="red">服务模式为Other的Bridge连接绑定时，端口对应的PC不会通过网关动态得到IP地址。当服务模式为Other时，请注意不要把所有局域网端口绑定为此种情况！</font></td></tr>
				</div>
				<br>				
				<hr align="left" class="sep" size="1" width="90%">
				<br>
				<input type="button" class="button" onClick="on_submit('sv');;" value="应用">&nbsp; &nbsp; &nbsp; &nbsp; 
				<!--<input type="button" class="button" onClick="on_submit('rs');" value="重启">&nbsp; &nbsp; &nbsp; &nbsp;-->
				<input type="button" class="button" onClick="on_submit('rm');" value="删除">&nbsp; &nbsp; &nbsp; &nbsp; 
				<input type="hidden" name="cmode" value="0">
				<!--<input type="hidden" name="dslite_enable" value="0">  -->
				<input type="hidden" name="ipDhcp" value="0">
				<input type="hidden" name="itfGroup" value="0">
				<input type="hidden" id="lst" name="lst" value="">
				<input type="hidden" id="action" name="action" value="none">
				<input type="hidden" name="submit-url" value="">
				<input type="hidden" id="acnameflag" name="acnameflag" value="none">
			</form>
		</DIV>
	</blockquote>
</body>
<%addHttpNoCache();%>
</HTML>
