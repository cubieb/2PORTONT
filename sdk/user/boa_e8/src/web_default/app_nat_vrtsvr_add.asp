<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>虚拟服务器添加</TITLE>
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

var protos = new Array("TCP/UDP", "TCP", "UDP");
var root = new Array( 
	new it_mlo("Active Worlds",
		new Array(3000, 3000, 1),
		new Array(5670, 5670, 1),
		new Array(7777, 7777, 1),
		new Array(7000, 7000, 1)),

	new it_mlo("Age of Empires",
		new Array(47624, 47624, 1),
		new Array(6073,   6073, 1),
		new Array(2300,   2400, 1),
		new Array(2300,   2400, 2)),

	new it_mlo("Age of Empires Expansion: The Rise of Rome",
		new Array(47624, 47624, 1),
		new Array(6073, 6073, 1),
		new Array(2300, 2400, 1),
		new Array(2300, 2400, 2)),

	new it_mlo("Age of Empires II Expansion: The Conquerors",
		new Array(47624, 47624, 1),
		new Array(6073, 6073, 1),
		new Array(2300, 2400, 1),
		new Array(2300, 2400, 2)),

	new it_mlo("Age of Empires II: The Age of Kings",
		new Array(47624, 47624, 1),
		new Array(6073, 6073, 1),
		new Array(2300, 2400, 1),
		new Array(2300, 2400, 2)),

	new it_mlo("Age of Kings",
		new Array(47624, 47624, 1),
		new Array(6073, 6073, 1),
		new Array(2300, 2400, 1),
		new Array(2300, 2400, 2)),

	new it_mlo("Age of Wonders",
		new Array(47624, 47624, 1),
		new Array(6073, 6073, 1),
		new Array(2300, 2400, 1),
		new Array(2300, 2400, 2)),

	new it_mlo("Aliens vs. Predator",
		new Array(80, 80, 2),
		new Array(2300, 2400, 2),
		new Array(8000, 8999, 2),
		new Array(2300, 2400, 2)),

	new it_mlo("AOL Instant Messenger",
		new Array(443, 443, 1)),

	new it_mlo("Audiogalaxy Satellite",
		new Array(41000, 50000, 1),
		new Array(1117, 5190, 1)),
 
	new it_mlo("AUTH",
		new Array(113, 113, 1)),

	new it_mlo("Baldur's Gate",
		new Array(47624, 47624, 1),
		new Array(47624, 47624, 2)),

	new it_mlo("BattleCom",
		new Array(47624, 47624, 1),
		new Array(47624, 47624, 2),
		new Array(2300, 2400, 1),
		new Array(2300, 2400, 2)),

	new it_mlo("Battlefield Communicator",
		new Array(47624, 47624, 1),
		new Array(47624, 47624, 2)),

	new it_mlo("Black and White",
		new Array(2611, 2612, 1),
		new Array(6667, 6667, 1),
		new Array(6500, 6500, 2),
		new Array(27900, 27900, 2)),

	new it_mlo("Blizzard Battle.net",
		new Array(4000, 4000, 1),
		new Array(6112, 6112, 1),
		new Array(6112, 6112, 2)),

	new it_mlo("Buddy Phone",
		new Array(700, 701, 2)),

	new it_mlo("Bungie.net",
		new Array(3453, 3453, 1)),

	new it_mlo("Camerades",
		new Array(2047, 2048, 1),
		new Array(2047, 2048, 2)),

	new it_mlo("CART Precision Racing",
		new Array(47624, 47624, 1),
		new Array(6073, 6073, 1),
		new Array(2300, 2400, 1),
		new Array(2300, 2400, 2)),

	new it_mlo("Close Combat for Windows",
		new Array(47624, 47624, 1),
		new Array(6073, 6073, 1),
		new Array(2300, 2400, 1),
		new Array(2300, 2400, 2)),

	new it_mlo("Close Combat III: The Russian Front",
		new Array(47624, 47624, 1),
		new Array(6073, 6073, 1),
		new Array(2300, 2400, 1),
		new Array(2300, 2400, 2)),

	new it_mlo("Close Combat: A Bridge Too Far",
		new Array(47624, 47624, 1),
		new Array(6073, 6073, 1),
		new Array(2300, 2400, 1),
		new Array(2300, 2400, 2)),

	new it_mlo("Combat Flight Simulator 2: WWII Pacific Theater",
		new Array(47624, 47624, 1),
		new Array(6073, 6073, 1),
		new Array(2300, 2400, 1),
		new Array(2300, 2400, 2)),

	new it_mlo("Combat Flight Simulator: WWII Europe Series",
		new Array(47624, 47624, 1),
		new Array(6073, 6073, 1),
		new Array(2300, 2400, 1),
		new Array(2300, 2400, 2)),

	new it_mlo("Crimson Skies",
		new Array(28805, 28805, 1),
		new Array(28801, 28801, 1),
		new Array(3040, 3040, 1),
		new Array(1121, 1121, 1)),

	new it_mlo("CuSeeMe",
		new Array(24032, 24032, 2), 
		new Array(1414, 1414, 2),
		new Array(1424, 1424, 2),
		new Array(1503, 1503, 1),
		new Array(1720, 1720, 1),
		new Array(1812, 1813, 2), 
		new Array(7640, 7640, 2), 
		new Array(7642, 7642, 1),
		new Array(7648, 7648, 1),
		new Array(7648, 7648, 2),
		new Array(7649, 7649, 1),
		new Array(56800, 56800, 2)),

	new it_mlo("Dark Reign 2",
		new Array(26214, 26214, 1),
		new Array(26214, 26214, 2)),

	new it_mlo("Delta Force 2",
		new Array(3568, 3569, 2)),

	new it_mlo("Delta Three PC to Phone",
		new Array(12053, 12053, 1),
		new Array(12083, 12083, 1),
		new Array(12080, 12080, 2),
		new Array(12120, 12120, 2), 
		new Array(12122, 12122, 2),
		new Array(24150, 24179, 2)),

	new it_mlo("Descent 3",
		new Array(7170, 7170, 1),
		new Array(2092, 2092, 2),
		new Array(3445, 3445, 2)),

	new it_mlo("Descent Freespace",
		new Array(3999, 3999, 1),
		new Array(4000, 4000, 2),
		new Array(7000, 7000, 2),
		new Array(3493, 3493, 2), 
		new Array(3440, 3440, 2)),

	new it_mlo("Diablo I",
		new Array(6112, 6112, 1),
		new Array(6112, 6112, 2)),

	new it_mlo("Diablo II",
		new Array(4000, 4000, 1),
		new Array(6112, 6119, 2)),

	new it_mlo("DialPad.Com",
		new Array(51200, 51201, 2),
		new Array(7175, 7175, 1),
		new Array(51210, 51210, 1),
		new Array(8680, 8686, 1), 
		new Array(1584, 1585, 1)),

	new it_mlo("DirectX 7 Games",
		new Array(47624, 47624, 1),
		new Array(2300, 2400, 1),
		new Array(2300, 2400, 2)),

	new it_mlo("DirectX 8 Games",
		new Array(6073, 6073, 1),
		new Array(2300, 2400, 2)),

	new it_mlo("Domain Name Server (DNS)",
		new Array(53, 53, 1),
		new Array(53, 53, 2)),

	new it_mlo("Doom",
		new Array(666, 666, 1),
		new Array(666, 666, 2)),

	new it_mlo("Dune 2000",
		new Array(4000, 4000, 1),
		new Array(1140, 1234, 2)),

	new it_mlo("Dwyco Video Conferencing",
		new Array(12000, 16090, 2),
		new Array(1024, 5000, 1),
		new Array(6700, 6702, 1),
		new Array(6880, 6880, 1)),

	new it_mlo("Elite Force",
		new Array(26000, 26000, 2),
		new Array(27500, 27500, 2),
		new Array(27910, 27910, 2),
		new Array(27960, 27960, 2)),

	new it_mlo("Everquest",
		new Array(1024, 6000, 1),
		new Array(1024, 6000, 2),
		new Array(6001, 7000, 1)),

	new it_mlo("F-16",
		new Array(3862, 3863, 2)),

	new it_mlo("F-22 Lightning 3",
		new Array(3875, 3875, 2),
		new Array(4533, 4534, 2),
		new Array(4660, 4670, 2)),

	new it_mlo("F-22 Raptor",
		new Array(3874, 3875, 2)),

	new it_mlo("F22 Raptor (Novalogic)",
		new Array(3874, 3874, 2)),

	new it_mlo("Falcon 4.0",
		new Array(2934, 2935, 2)),

	new it_mlo("Fighter Ace II",
		new Array(50000, 50100, 1),
		new Array(50000, 50100, 2),
		new Array(47624, 47624, 1),
		new Array(6073, 6073, 1),
		new Array(2300, 2400, 1),
		new Array(2300, 2400, 2)),

	new it_mlo("Flight Simulator 2000",
		new Array(47624, 47624, 1),
		new Array(6073, 6073, 1),
		new Array(2300, 2400, 1),
		new Array(2300, 2400, 2)),

	new it_mlo("Freetel",
		new Array(21300, 21303, 2)),

	new it_mlo("FTP Server",
		new Array(21, 21, 1)),

	new it_mlo("GNUtella",
		new Array(6346, 6347, 1),
		new Array(6346, 6347, 2)),

	new it_mlo("Golf 2001 Edition",
		new Array(47624, 47624, 1),
		new Array(6073, 6073, 1),
		new Array(2300, 2400, 1),
		new Array(2300, 2400, 2)),

	new it_mlo("Go2Call",
		new Array(2090, 2091, 2),
		new Array(2090, 2090, 1)),

	new it_mlo("Half Life",
		new Array(6003, 6003, 1),
		new Array(6003, 6003, 2),
		new Array(7001, 7001, 1),
		new Array(7001, 7001, 2),
		new Array(27005, 27005, 2),
		new Array(27010, 27015, 2)),

	new it_mlo("Half Life Server",
		new Array(27015, 27015, 2)),

	new it_mlo("Heretic II Server",
		new Array(28910, 28910, 1),
		new Array(28910, 28910, 2)),

	new it_mlo("I76",
		new Array(21154, 21156, 2)),
        
	new it_mlo("ICUII Client",
		new Array(2019, 2019, 1), 
		new Array(2000, 2038, 1),
		new Array(2050, 2051, 1),
		new Array(2069, 2069, 1),
		new Array(2085, 2085, 1),
		new Array(3010, 3030, 1)),  
        
	new it_mlo("IPSEC", 
		new Array(500, 500, 2)), 
       
	new it_mlo("Ivisit",
		new Array(9943, 9943, 2),
		new Array(56768, 56768, 2)),

	new it_mlo("IRC",
		new Array(1024, 5000, 1),
		new Array(6660, 6669, 1),
		new Array(113, 113, 2)),

	new it_mlo("IStreamVideo2HP",
		new Array(8076, 8077, 1),
		new Array(8076, 8077, 2)),

	new it_mlo("KaZaA",
		new Array(1024, 1024, 1)),

	new it_mlo("Kohan Immortal Sovereigns",
		new Array(3855, 3855, 1),
		new Array(3855, 3855, 2),
		new Array(17437, 17437, 1),
		new Array(17437, 17437, 2)),

	new it_mlo("LapLink Gold",
		new Array(1547, 1547, 1)),

	new it_mlo("Links 2001",
		new Array(47624, 47624, 1),
		new Array(6073, 6073, 1),
		new Array(2300, 2400, 1),
		new Array(2300, 2400, 2)),

	new it_mlo("Lotus Notes Server",
		new Array(1352, 1352, 1)),

	new it_mlo("Mail (POP3)",
		new Array(110, 110, 1)),

	new it_mlo("Mail (SMTP)",
		new Array(25, 25, 1)),

	new it_mlo("MechCommander 2.0",
		new Array(47624, 47624, 1),
		new Array(6073, 6073, 1),
		new Array(2300, 2400, 1),
		new Array(2300, 2400, 2)),

	new it_mlo("MechWarrior 4",
		new Array(47624, 47624, 1),
		new Array(6073, 6073, 1),
		new Array(2300, 2400, 1),
		new Array(2300, 2400, 2)),

	new it_mlo("Media Player 7",
		new Array(1755, 1755, 1),
		new Array(70, 7000, 2)),

	new it_mlo("Midtown Madness 2",
		new Array(47624, 47624, 1),
		new Array(6073, 6073, 1),
		new Array(2300, 2400, 1),
		new Array(2300, 2400, 2)),

	new it_mlo("Mig 29",
		new Array(3862, 3863, 2)),

	new it_mlo("Monster Truck Madness 2",
		new Array(47624, 47624, 1),
		new Array(6073, 6073, 1),
		new Array(2300, 2400, 1),
		new Array(2300, 2400, 2)),

	new it_mlo("Motocross Madness 2",   
		new Array(47624, 47624, 1),
		new Array(6073, 6073, 1),
		new Array(2300, 2400, 1),
		new Array(2300, 2400, 2)),

	new it_mlo("Motorhead Server",   
		new Array(16000, 16000, 1),
		new Array(16000, 16000, 2)),

	new it_mlo("MSN Gaming Zone",
		new Array(6667, 6667, 1),
		new Array(28800, 29000, 1),
		new Array(47624, 47624, 1),
		new Array(6073, 6073, 1),
		new Array(2300, 2400, 1),
		new Array(2300, 2400, 2)),

	new it_mlo("MSN Messenger",
		new Array(6891, 6901, 1),
		new Array(1863, 1963, 1),
		new Array(1863, 1863, 2),
		new Array(5190, 5190, 2),
		new Array(6901, 6901, 2)),

	new it_mlo("Myth",
		new Array(3453, 3453, 1)),

	new it_mlo("Myth II Server",
		new Array(3453, 3453, 1)),

	new it_mlo("Myth: The Fallen Lords",
		new Array(3453, 3453, 1)),

	new it_mlo("Need for Speed",
		new Array(9442, 9442, 1), 
		new Array(6112, 6112, 2)),

	new it_mlo("NetMech",
		new Array(21154, 21156, 2)),

	new it_mlo("Netmeeting 2.0, 3.0, Intel Video Phone",
		new Array(1024, 65534, 2), 
		new Array(1024, 1502, 1),
		new Array(1504, 1730, 1),
		new Array(1732, 65534, 1),
		new Array(1503, 1503, 1),
		new Array(1731, 1731, 1)),

	new it_mlo("Network Time Protocol (NTP)",
		new Array(123, 123, 2)),

	new it_mlo("News Server (NNTP)",
		new Array(119, 119, 1)),

	new it_mlo("OKWeb",
		new Array(80, 80, 1), 
		new Array(443, 443, 1),
		new Array(5210, 5220, 1)),

	new it_mlo("OKWin",
		new Array(1729, 1729, 1), 
		new Array(1909, 1909, 1),
		new Array(80, 80, 1)),

	new it_mlo("Outlaws",
		new Array(5310, 5310, 1), 
		new Array(5310, 5310, 2)),

	new it_mlo("Pal Talk",
		new Array(2090, 2090, 2), 
		new Array(2091, 2091, 2),
		new Array(2090, 2090, 1),
		new Array(2091, 2091, 1),
		new Array(2095, 2095, 1)),

	new it_mlo("pcAnywhere v7.5",
		new Array(5631, 5631, 1), 
		new Array(5631, 5631, 2)),

	new it_mlo("PhoneFree",
		new Array(1034, 1035, 1), 
		new Array(1034, 1035, 2), 
		new Array(9900, 9901, 2), 
		new Array(2644, 2644, 1),
		new Array(8000, 8000, 1)),

	new it_mlo("Polycom ViaVideo H.323",
		new Array(3230, 3235, 1)),

	new it_mlo("Polycom ViaVideo H.324",
		new Array(3230, 3235, 2)),
      
	new it_mlo("PPTP",
		new Array(1723, 1723, 1)),
            
	new it_mlo("Quake",
		new Array(26000, 26000, 1),
		new Array(26000, 26000, 2)),

	new it_mlo("Quake II (Client/Server)",
		new Array(27910, 27910, 2)),

	new it_mlo("Quake III",
		new Array(27660, 27680, 2)),

	new it_mlo("Red Alert",
		new Array(5009, 5009, 2)),

	new it_mlo("RealAudio",
		new Array(6790, 32000, 2)),

	new it_mlo("Real Player 8 Plus",
		new Array(7070, 7070, 2)),

	new it_mlo("Rise of Rome",
		new Array(47624, 47624, 1),
		new Array(47624, 47624, 2)),

	new it_mlo("Roger Wilco",
		new Array(3782, 3782, 1),
		new Array(3782, 3783, 2)),

	new it_mlo("Rogue Spear",
		new Array(2346, 2346, 1)),

	new it_mlo("Secure Shell Server (SSH)",
		new Array(22, 22, 1)),

	new it_mlo("Secure Web Server (HTTPS)",
		new Array(443, 443, 1)),

	new it_mlo("ShoutCast",
		new Array(8000, 8005, 1)),

	new it_mlo("SNMP",
		new Array(161, 161, 2)),

	new it_mlo("SNMP Trap",
		new Array(162, 162, 2)),

	new it_mlo("Speak Freely",
		new Array(2074, 2076, 2)),

	new it_mlo("StarCraft",
		new Array(6112, 6112, 2)),

	new it_mlo("Starfleet Command",   
		new Array(47624, 47624, 1),
		new Array(6073, 6073, 1),
		new Array(2300, 2400, 1),
		new Array(2300, 2400, 2)),

	new it_mlo("StarLancer",   
		new Array(47624, 47624, 1),
		new Array(6073, 6073, 1),
		new Array(2300, 2400, 1),
		new Array(2300, 2400, 2)),

	new it_mlo("SWAT3",   
		new Array(16639, 16639, 1),
		new Array(16638, 16638, 2)),

	new it_mlo("Telnet Server",   
		new Array(23, 23, 1)),
        
	new it_mlo("TFTP",   
		new Array(69, 69, 2)),
        
	new it_mlo("The 4th Coming",   
		new Array(11677, 11677, 2),
		new Array(11679, 11679, 2)),

	new it_mlo("Tiberian Sun: C&C III",   
		new Array(1234, 1234, 2)),

	new it_mlo("Total Annihilation",   
		new Array(47624, 47624, 2)),

	new it_mlo("Ultima",
		new Array(5001, 5010, 1), 
		new Array(7775, 7777, 1), 
		new Array(8800, 8900, 1), 
		new Array(9999, 9999, 1),
		new Array(7875, 7875, 1)),

	new it_mlo("Unreal Tournament",
		new Array(7777, 7790, 2), 
		new Array(27900, 27900, 2), 
		new Array(8080, 8080, 1)),

	new it_mlo("Urban Assault",
		new Array(47624, 47624, 1), 
		new Array(6073, 6073, 1), 
		new Array(2300, 2400, 1), 
		new Array(2300, 2400, 2)),

	new it_mlo("VoxPhone 3.0",
		new Array(12380, 12380, 1), 
		new Array(12380, 12380, 2)),

	new it_mlo("Warbirds 2",
		new Array(912, 912, 1)),

	new it_mlo("Web Server (HTTP)",
		new Array(80, 80, 1)),

	new it_mlo("WebPhone 3.0",
		new Array(21845, 21845, 1)),

	new it_mlo("Westwood Online",
		new Array(4000, 4000, 2), 
		new Array(1140, 1234, 1),
		new Array(1140, 1234, 2)),

	new it_mlo("Windows 2000 Terminal Server",
		new Array(3389, 3389, 1), 
		new Array(3389, 3389, 2)),

	new it_mlo("X Windows",
		new Array(6000, 6000, 1), 
		new Array(6000, 6000, 2)),

	new it_mlo("Yahoo Pager",
		new Array(5050, 5050, 1)),

	new it_mlo("Yahoo Messenger Chat",
		new Array(5000, 5001, 1), 
		new Array(5055, 5055, 2))
		);

/********************************************************************
**          on document load
********************************************************************/
var numToConfig=0;
function on_init()
{
	for(var i = 0; i < root.length; i++)
	{
		form.srvName.options.add(new Option(root[i].name, root[i].name));
	}
	var lan = "<% getInfo("lanip"); %>";
	var mask = "<% getInfo("subnetMask"); %>";
	form.sIp.value = sji_ipprefix(lan, mask);
	<%virtualSvrLeft();%>
	on_mode();
}

/********************************************************************
**          on document submit
********************************************************************/
function on_submit() 
{
	var arrsubmit = new Array();
	var strname = "";
    var numReady =0;
	with ( document.forms[0] ) 
	{
		if(radiosrv[0].checked == true && srvName.selectedIndex == 0)
		{
			srvName.focus();
			alert("请选择一个服务！");
			return;
		}
		/*ql:20080717 START: remove sIP check, because SIP column is just used to fill severIP column, do not need to submit*/
		//if(sji_checkvip(sIp.value) == false)
		//{
		//	sIp.focus();
		//	alert("服务器IP地址：" + sIp.value + "为无效IP，请重新输入！");
		//	return;
		//}
		/*ql:20080717 END*/
		if(radiosrv[1].checked == true && sji_checkstrnor(cusSrvName.value, 1, 60) == false)
		{
			cusSrvName.focus();
			alert("自定义服务器：" + cusSrvName.value + "为无效名称，请重新输入！");
			return;
		}
		strname = ((radiosrv[0].checked == true) ? srvName.value : cusSrvName.value);
		for(var i = 0; i < wanStartPort.length; i++) 
		{
			if(wanStartPort[i].value == "")continue;
            		numReady++;
            		if(numReady > numToConfig)
            		{
		                alert("超过最大可设规则数,请先删除多余规则!");
		                return;
		       }
			if(sji_checkdigitrange(wanStartPort[i].value, 1, 65535) == false)
			{
				wanStartPort[i].focus();
				alert("外部初始端口：" + wanStartPort[i].value + "为无效端口，请重新输入！");
				return;
			}
			if(sji_checkdigitrange(wanEndPort[i].value, parseInt(wanStartPort[i].value, 10), 65535) == false)
			{
				wanEndPort[i].focus();
				alert("外部终止端口：" + wanEndPort[i].value + "为无效端口，请重新输入！");
				return;
			}
			if(sji_checkvip(serverIp[i].value) == false)
			{
				serverIp[i].focus();
				alert("服务器IP地址：" + serverIp[i].value + "为无效IP，请重新输入！");
				return;
			}
			if(sji_checkdigitrange(lanPort[i].value, 1, 65535) == false)
			{
				lanPort[i].focus();
				alert("服务器端口：" + lanPort[i].value + "为无效端口，请重新输入！");
				return;
			}
			
			var itrc = new it_nr("", 
			new it("svrName", strname),
			new it(wanStartPort[i].name, parseInt(wanStartPort[i].value, 10)),
			new it(wanEndPort[i].name, parseInt(wanEndPort[i].value, 10)),
			new it(protoType[i].name, parseInt(protoType[i].value, 10)),
			new it(lanPort[i].name, parseInt(lanPort[i].value, 10)),
			new it(serverIp[i].name, serverIp[i].value)
			);
			itrc.select = true;
			arrsubmit.push(itrc);
		}
/*star:20080715 START add for no set*/
		if(numReady==0){
			wanStartPort[0].focus();
			alert("请输入配置!");
			return;
		}
/*star:20080715 END*/
			
		form.bcdata.value = sji_encode(arrsubmit, "select");
		submit();
	}
}

function on_change(index)
{
	var tmpsip = null;
	
	if(sji_checkvip(form.sIp.value) == false)
	{
		form.srvName.selectedIndex = 0;
		form.sIp.focus();
		alert("服务器IP地址：" + form.sIp.value + "为无效IP，请重新输入！");
		return;
	}
	tmpsip = form.sIp.value;

	if(index < 0 || index > root.length)return false;
	if(index == 0)
	{
		reset_all();
		return;
	}
	index--;
	
	reset_all(root[index].childs.length);
	
	with (form) 
	{
		for(var i = 0; i < root[index].childs.length; i++) 
		{
			wanStartPort[i].value = root[index].childs[i][0];
			wanEndPort[i].value = root[index].childs[i][1];
			protoType[i].value = root[index].childs[i][2];;
			if(tmpsip != null)serverIp[i].value = tmpsip;
			lanPort[i].value = root[index].childs[i][0];
		}
	}
}

function reset_all(start)
{
	if(typeof start == "undefined")start = 0;
	with (form) 
	{
		for(var i = start; i < wanStartPort.length; i++) 
		{
			wanStartPort[i].value = wanEndPort[i].value = serverIp[i].value = lanPort[i].value = "";
			protoType[i].value = 1;
		}
	}
}

function on_mode()
{
	if (form.radiosrv[0].checked == true)
	{
		form.srvName.disabled = false;
		form.cusSrvName.disabled = true;
	}
	else
	{
		form.srvName.disabled = true;
		form.cusSrvName.disabled = false;
	}
	//reset_all();
	form.srvName.selectedIndex = 0;
}

</SCRIPT>
</HEAD>

<!-------------------------------------------------------------------------------------->
<!--主页代码-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
	<blockquote>
		<DIV align="left" style="padding-left:20px; padding-top:5px">
			<form id="form" action="/boaform/admin/formVrtsrv" method="post">
				<b>NAT -- 虚拟服务器</b>
				<div class="tip" style="width:90% ">
					<br><br>选择服务名并输入服务器的IP地址，然后点击"保存/应用"，为这项服务将IP包转发到指定的服务器。<br><br>
				</div>
				<hr align="left" class="sep" size="1" width="90%">
				<br>
				<table border="0" cellpadding="0" cellspacing="0">
					<tr>
						<td><b>可配置的剩余数:</b></td><td><b><label id="numleft">4</label></b></td>
					</tr>
				</table>
				</br>
				<table border="0" cellpadding="0" cellspacing="0">
				   <tr>
					  <td colspan="2">服务器名:</td>
				   </tr>
				   <tr>
					  <td align="right"><input type="radio" name="radiosrv" onclick="on_mode()" value="auto" checked>&nbsp;&nbsp;选择一项服务:&nbsp;&nbsp;</td>
					  <td><select name="srvName" size="1" onChange="on_change(this.selectedIndex);" style="width:380px ">
							<option value="none">Select One</option>
						</select>
					  </td>
				   </tr>
				   <tr>
					  <td align="right"><input type="radio" name="radiosrv" onclick="on_mode()" value="">&nbsp;&nbsp;自定义服务器:&nbsp;&nbsp;</td>
					  <td><input type="text" size="45" name="cusSrvName" style="width:380px "></td>
				   </tr>
				   <tr><td colspan="2">&nbsp;</td></tr>
				   <tr>
					  <td align="right">服务器IP地址:&nbsp;&nbsp;<td><input type="text" size="15" name="sIp"></td>
				   </tr>
				</table>
				</br>
				<table id="lstrule" class="flat" border="1" cellpadding="0" cellspacing="0">
				   <tr class="hdb" align="center">
					  <td class="hd">外部初始端口</td>
					  <td class="hd">外部终止端口</td>
					  <td class="hd">协议</td>
					  <td class="hd">服务器IP地址</td>
					<td class="hd">服务器端口</td><!--menker add for  soruce port-->
				   </tr>
				   <tr>
					  <td><input type="text" size="12" name="wanStartPort"></td>
					  <td><input type="text" size="12" name="wanEndPort"></td>
					  <td><select name="protoType">
							<option value="0">
							TCP/UDP
							<option value="1" selected>
							TCP
							<option value="2">
							UDP
						 </select></td>
					   <td><input type="text" size="15" name="serverIp"></td>
					  <td><input type="text" size="12" name="lanPort"></td>
				   </tr>
				   
				   <tr>
					  <td><input type="text" size="12" name="wanStartPort"></td>
					  <td><input type="text" size="12" name="wanEndPort"></td>
					  <td><select name="protoType">
							<option value="0">
							TCP/UDP
							<option value="1" selected>
							TCP
							<option value="2">
							UDP
						 </select></td>
					  <td><input type="text" size="15" name="serverIp"></td>
					  <td><input type="text" size="12" name="lanPort"></td>
				   </tr>
				   
				   <tr>
					  <td><input type="text" size="12" name="wanStartPort"></td>
					  <td><input type="text" size="12" name="wanEndPort"></td>
					  <td><select name="protoType">
							<option value="0">
							TCP/UDP
							<option value="1" selected>
							TCP
							<option value="2">
							UDP
						 </select></td>
					  <td><input type="text" size="15" name="serverIp"></td>
					  <td><input type="text" size="12" name="lanPort"></td>
				   </tr>
				   
				   <tr>
					  <td><input type="text" size="12" name="wanStartPort"></td>
					  <td><input type="text" size="12" name="wanEndPort"></td>
					  <td><select name="protoType">
							<option value="0">
							TCP/UDP
							<option value="1" selected>
							TCP
							<option value="2">
							UDP
						 </select></td>
					  <td><input type="text" size="15" name="serverIp"></td>
					  <td><input type="text" size="12" name="lanPort"></td>
				   </tr>
				   
				   <tr>
					  <td><input type="text" size="12" name="wanStartPort"></td>
					  <td><input type="text" size="12" name="wanEndPort"></td>
					  <td><select name="protoType">
							<option value="0">
							TCP/UDP
							<option value="1" selected>
							TCP
							<option value="2">
							UDP
						 </select></td>
					  <td><input type="text" size="15" name="serverIp"></td>
					  <td><input type="text" size="12" name="lanPort"></td>
				   </tr>
				   
				   <tr>
					  <td><input type="text" size="12" name="wanStartPort"></td>
					  <td><input type="text" size="12" name="wanEndPort"></td>
					  <td><select name="protoType">
							<option value="0">
							TCP/UDP
							<option value="1" selected>
							TCP
							<option value="2">
							UDP
						 </select></td>
					  <td><input type="text" size="15" name="serverIp"></td>
					  <td><input type="text" size="12" name="lanPort"></td>
				   </tr>

				   <tr>
					  <td><input type="text" size="12" name="wanStartPort"></td>
					  <td><input type="text" size="12" name="wanEndPort"></td>
					  <td><select name="protoType">
							<option value="0">
							TCP/UDP
							<option value="1" selected>
							TCP
							<option value="2">
							UDP
						 </select></td>
					  <td><input type="text" size="15" name="serverIp"></td>
					  <td><input type="text" size="12" name="lanPort"></td>
				   </tr>
				   
				   <tr>
					  <td><input type="text" size="12" name="wanStartPort"></td>
					  <td><input type="text" size="12" name="wanEndPort"></td>
					  <td><select name="protoType">
							<option value="0">
							TCP/UDP
							<option value="1" selected>
							TCP
							<option value="2">
							UDP
						 </select></td>
					  <td><input type="text" size="15" name="serverIp"></td>
					  <td><input type="text" size="12" name="lanPort"></td>
				   </tr>
				   
				   <tr>
					  <td><input type="text" size="12" name="wanStartPort"></td>
					  <td><input type="text" size="12" name="wanEndPort"></td>
					  <td><select name="protoType">
							<option value="0">
							TCP/UDP
							<option value="1" selected>
							TCP
							<option value="2">
							UDP
						 </select></td>
					  <td><input type="text" size="15" name="serverIp"></td>
					  <td><input type="text" size="12" name="lanPort"></td>
				   </tr>
				   
				   <tr>
					  <td><input type="text" size="12" name="wanStartPort"></td>
					  <td><input type="text" size="12" name="wanEndPort"></td>
					  <td><select name="protoType">
							<option value="0">
							TCP/UDP
							<option value="1" selected>
							TCP
							<option value="2">
							UDP
						 </select></td>
					  <td><input type="text" size="15" name="serverIp"></td>
					  <td><input type="text" size="12" name="lanPort"></td>
				   </tr>
				   
				   <tr>
					  <td><input type="text" size="12" name="wanStartPort"></td>
					  <td><input type="text" size="12" name="wanEndPort"></td>
					  <td><select name="protoType">
							<option value="0">
							TCP/UDP
							<option value="1" selected>
							TCP
							<option value="2">
							UDP
						 </select></td>
					  <td><input type="text" size="15" name="serverIp"></td>
					  <td><input type="text" size="12" name="lanPort"></td>
				   </tr>
				   
				   <tr>
					  <td><input type="text" size="12" name="wanStartPort"></td>
					  <td><input type="text" size="12" name="wanEndPort"></td>
					  <td><select name="protoType">
							<option value="0">
							TCP/UDP
							<option value="1" selected>
							TCP
							<option value="2">
							UDP
						 </select></td>
					  <td><input type="text" size="15" name="serverIp"></td>
					  <td><input type="text" size="12" name="lanPort"></td>
				   </tr>
				</table>
			   <br>
				<br>
				<hr align="left" class="sep" size="1" width="90%">
				<input type="button" class="button" onClick="on_submit()" value="保存/应用">
				<input type="hidden" id="action" name="action" value="add">
				<input type="hidden" name="bcdata" value="le">
				<input type="hidden" name="submit-url" value="/app_nat_vrtsvr_cfg.asp">
			</form>
		</DIV>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>
