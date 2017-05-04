<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title>DNS <% multilang(LANG_CONFIGURATION); %></title>
<script type="text/javascript" src="share.js">
</script>
<SCRIPT>

var showIPv6 = 0;

function sji_checkip(ip) 
{
	if(typeof ip == "undefined")return false;
	var pattern = /^([0-9]|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])(\.([0-9]|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])){3}$/; 
	return pattern.test(ip);
}

function autoDNSclicked()
{
	document.dns.dns1.disabled = true;
	document.dns.dns2.disabled = true;
	document.dns.dns3.disabled = true;
	if (document.getElementById)  // DOM3 = IE5, NS6
      {
      	document.getElementById('dnsInfo').style.display = 'none';
      	document.getElementById('dnsenable').style.display = 'none';
      }
   else {
      if (document.layers == false) // IE4
         {
         	document.all.dnsInfo.style.display = 'none';
         	document.all.dnsenable.style.display = 'none';
        }
   }
	// Mason Yu for IPv6
	if ( showIPv6 == 1 )
		autoDNSv6clicked();
}

function manualDNSclicked()
{
	document.dns.dns1.disabled = false;
	document.dns.dns2.disabled = false;
	document.dns.dns3.disabled = false;
		if (document.getElementById)  // DOM3 = IE5, NS6
      {
      	document.getElementById('dnsInfo').style.display = 'block';
      	document.getElementById('dnsenable').style.display = 'block';
      }
   else {
      if (document.layers == false) // IE4
         {
         	document.all.dnsInfo.style.display = 'block';
         	document.all.dnsenable.style.display = 'block';
        }
   }
   DnsBindPvcClicked();
   
	// Mason Yu for IPv6
	if ( showIPv6 == 1 )
	{
		manualDNSv6clicked();
		Dnsv6BindPvcClicked();
	}
}

// Mason Yu for IPv6
function autoDNSv6clicked()
{
	document.dns.dnsv61.disabled = true;
	document.dns.dnsv62.disabled = true;
	document.dns.dnsv63.disabled = true;
	if (document.getElementById)  // DOM3 = IE5, NS6
      {
      	document.getElementById('dnsv6Info').style.display = 'none';
      	document.getElementById('dnsv6enable').style.display = 'none';
      }
   else {
      if (document.layers == false) // IE4
         {
         	document.all.dnsv6Info.style.display = 'none';
         	document.all.dnsv6enable.style.display = 'none';
        }
   }
}

// Mason Yu for IPv6
function manualDNSv6clicked()
{
	document.dns.dnsv61.disabled = false;
	document.dns.dnsv62.disabled = false;
	document.dns.dnsv63.disabled = false;
		if (document.getElementById)  // DOM3 = IE5, NS6
      {
      	document.getElementById('dnsv6Info').style.display = 'block';
      	document.getElementById('dnsv6enable').style.display = 'block';
      }
   else {
      if (document.layers == false) // IE4
         {
         	document.all.dnsv6Info.style.display = 'block';
         	document.all.dnsv6enable.style.display = 'block';
        }
   }
}


function saveClick()
{	
	if (document.dns.dnsMode[0].checked) {
		return true;
	}

	if(sji_checkip(document.dns.dns1.value) == false){
		document.dns.dns1.focus();		
		alert('<% multilang(LANG_IP_ADDRESS_IS_INVALID); %>');
		return;
	}
	
	
//	if (document.dns.dns2.value=="") {
	if (document.dns.dns2.value=="") {
		if (document.dns.dns3.value=="") {	// Jenny,  buglist B059, dns2 couldn't be empty if dns3 is not empty
			// Mason Yu for IPv6
			if ( showIPv6 == 1 )						
				return (saveClickIpv6());
			else
				return true;
		}
		else {			
			alert('<% multilang(LANG_THE_SECOND_DNSV4_NAME_SERVER_S_IP_IS_EMPTY); %>');
			document.dns.dns2.focus();
			return false;
		}
	}
	if(sji_checkip(document.dns.dns2.value) == false){
		document.dns.dns2.focus();		
		alert('<% multilang(LANG_IP_ADDRESS_IS_INVALID); %>');
		return;
	}
	
	if (document.dns.dns3.value=="") {
		// Mason Yu for IPv6
		if ( showIPv6 == 1 )						
			return (saveClickIpv6());
		else
			return true;				
	}
	if(sji_checkip(document.dns.dns3.value) == false){
		document.dns.dns3.focus();		
		alert('<% multilang(LANG_IP_ADDRESS_IS_INVALID); %>');
		return;
	}
	
	// Mason Yu for IPv6
	if ( showIPv6 == 1 )						
		return (saveClickIpv6());
	else
		return true;		
	
}

// Mason Yu for IPv6
function saveClickIpv6()
{
	if (document.dns.dnsv61.value=="") {
		document.dns.dnsv61.focus();		
		alert('<% multilang(LANG_THE_FIRST_DNSV6_NAME_SERVER_S_IP_IS_EMPTY); %>');
		return false;
	}
	
	if(document.dns.dnsv61.value != "" ){
		if (! isUnicastIpv6Address( document.dns.dnsv61.value) ){			
			alert('<% multilang(LANG_INVALID_THE_FIRST_DNSV6_NAME_SERVER_S_IP); %>');
			document.dns.dnsv61.focus();
			return false;
		}
	}
	
	if (document.dns.dnsv62.value=="") {
		if (document.dns.dnsv63.value=="")	// Jenny,  buglist B059, dns2 couldn't be empty if dns3 is not empty
			return true;
		else {			
			alert('<% multilang(LANG_THE_SECOND_DNSV6_NAME_SERVER_S_IP_IS_EMPTY); %>');
			document.dns.dnsv62.focus();
			return false;
		}
	}
	
	if(document.dns.dnsv62.value != "" ){
		if (! isUnicastIpv6Address( document.dns.dnsv62.value) ){			
			alert('<% multilang(LANG_INVALID_THE_SECOND_DNSV6_NAME_SERVER_S_IP); %>');
			document.dns.dnsv62.focus();
			return false;
		}
	}
	
	if(document.dns.dnsv63.value != "" ){
		if (! isUnicastIpv6Address( document.dns.dnsv63.value) ){			
			alert('<% multilang(LANG_THE_THIRD_DNSV6_NAME_SERVER_S_IP_IS_EMPTY); %>');
			document.dns.dnsv63.focus();
			return false;
		}
	}
	
	return true;
}

function DnsBindPvcClicked()
{
	if(document.dns.enableDnsBind.checked)
		{
			if(<% checkWrite("WanPvcRouter"); %>) {				
				alert('<% multilang(LANG_NO_ROUTER_WAN_INTERFACE); %>');
				document.dns.enableDnsBind.checked=false;
				return false;
			}
			if (document.getElementById)  // DOM3 = IE5, NS6
		     {
		     document.getElementById('wanlist1').style.display = 'block';
			 document.getElementById('wanlist2').style.display = 'block';
			 document.getElementById('wanlist3').style.display = 'block';
						}
		   else {
		      if (document.layers == false) // IE4
		         {
		         document.all.wanlist1.style.display = 'block';
				  document.all.wanlist2.style.display = 'block';
				   document.all.wanlist3.style.display = 'block';
		      	}
		}		
		}
		else{
				if (document.getElementById)  // DOM3 = IE5, NS6
		      				{
		      				document.getElementById('wanlist1').style.display = 'none';
						document.getElementById('wanlist2').style.display = 'none';
						document.getElementById('wanlist3').style.display = 'none';
					}
		   else {
		      if (document.layers == false) // IE4
		         		{
		         		document.all.wanlist1.style.display = 'none';
						document.all.wanlist2.style.display = 'none';
						document.all.wanlist3.style.display = 'none';
		      	}
		}		
}
return true;
}

// Mason Yu for IPv6
function Dnsv6BindPvcClicked()
{
	if(document.dns.enableDnsv6Bind.checked)
		{
			if(<% checkWrite("WanPvcRouter"); %>) {				
				alert('<% multilang(LANG_NO_ROUTER_WAN_INTERFACE); %>');
				document.dns.enableDnsv6Bind.checked=false;
				return false;
			}
			if (document.getElementById)  // DOM3 = IE5, NS6
		     {
		     document.getElementById('v6wanlist1').style.display = 'block';
			 document.getElementById('v6wanlist2').style.display = 'block';
			 document.getElementById('v6wanlist3').style.display = 'block';
						}
		   else {
		      if (document.layers == false) // IE4
		         {
		         document.all.v6wanlist1.style.display = 'block';
				  document.all.v6wanlist2.style.display = 'block';
				   document.all.v6wanlist3.style.display = 'block';
		      	}
		}		
		}
		else{
				if (document.getElementById)  // DOM3 = IE5, NS6
		      				{
		      				document.getElementById('v6wanlist1').style.display = 'none';
						document.getElementById('v6wanlist2').style.display = 'none';
						document.getElementById('v6wanlist3').style.display = 'none';
					}
		   else {
		      if (document.layers == false) // IE4
		         		{
		         		document.all.v6wanlist1.style.display = 'none';
						document.all.v6wanlist2.style.display = 'none';
						document.all.v6wanlist3.style.display = 'none';
		      	}
		}		
}
return true;
}

function DnsBindSelectdInit(selectname,ifIndex)
{
	var i,num,op;
 num=document.getElementById(selectname).length
 for(i=0;i<num;i++){
 op=document.getElementById(selectname).options[i]
 if(op.value==ifIndex) op.selected=true;
 }

}

function on_init()
{
/*
	with ( document.forms[0] ) 
	{
		for(var i = 0; i < links.length; i++)
		{
			dnspvc1.options.add(new Option(links[i].displayname(1), links[i].ifIndex));
			dnspvc2.options.add(new Option(links[i].displayname(1), links[i].ifIndex));
			dnspvc3.options.add(new Option(links[i].displayname(1), links[i].ifIndex));
		}
	}*/
	
	// Mason Yu for IPv6
	if (<% checkWrite("IPv6Show"); %>) {
		showIPv6 = 1;
		if (document.getElementById)  // DOM3 = IE5, NS6
		{
			document.getElementById('dnsv6Tbl').style.display = 'block';						
		}
		else {
			if (document.layers == false) // IE4
			{
				document.all.dnsv6Tbl.style.display = 'block';						
			}
		}
	}
	
	return true;
}

</SCRIPT>
</head>

<!-------------------------------------------------------------------------------------->
<!--主页代码-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
<blockquote>
		<h2><font color="#0000FF">DNS <% multilang(LANG_CONFIGURATION); %></font></h2>
		<DIV align="left" style="padding-left:20px; padding-top:5px">
			<form id="form" action=/boaform/admin/formDNS method=POST name="dns">
			<tr><font size=2>
				<% multilang(LANG_PAGE_DESC_CONFIGURE_DNS_SERVER_IP_ADDRESSES); %>
			</tr>			
			
			<table border=0 width="600" cellspacing=4 cellpadding=0>
			<tr><td><hr size=1 noshade align=top></td></tr>
			<tr>
		    <tr>
		      <td width="40%" colspan="3"><font size=2>
		        &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<input type="radio" value="dnsAuto" name="dnsMode" <% checkWrite("dns0"); %> onClick="autoDNSclicked()"><% multilang(LANG_ATTAIN_DNS_AUTOMATICALLY); %>
		      </td>
		      <td></td>
		    </tr>
		    <tr>
		      <td width="40%" colspan="3"><font size=2>
		        &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<input type="radio" value="dnsManual" name="dnsMode" <% checkWrite("dns1"); %> onClick="manualDNSclicked()"><% multilang(LANG_SET_DNS_MANUALLY); %>
		      </td>
		      <td><div id='dnsenable' style="display:none"><% checkWrite("DnsBindPvc"); %></div></td>
		    </tr>
		    </table>
		    
		     <div id='dnsInfo' style="display:none">
		     <table border=0 width="600" cellspacing=4 cellpadding=0>
		    <tr>
		       <td width="30%"><font size=2>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;DNSv4 1:</td>
		       <td width="30%"><font size=2><input type="text" name="dns1" size="18" maxlength="15" value=<% getInfo("wan-dns1"); %>></td>
		       <td width="40%" align=left><font size=2>       
		      	<select name="dnspvc1" id='wanlist1' style="display:none">
		      	<option value=65535>None</option>
				<!-- // Mason Yu for IPv6 -->				
		      	<% if_wan_list("rt"); %>
		      	</select>
		      </td>
		    </tr>
		    <tr>
		       <td width="30%"><font size=2>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;DNSv4 2:</td>
		       <td width="30%"><font size=2><input type="text" name="dns2" size="18" maxlength="15" value=<% getInfo("wan-dns2"); %>></td>
		      <td width="40%" align=left><font size=2>         
		      	<select name="dnspvc2" id='wanlist2' style="display:none">
		      	<option value=65535>None</option>
				<!-- // Mason Yu for IPv6 -->
		      	<% if_wan_list("rt"); %>
		      	</select>
		      </td>
		    </tr>
		    <tr>
		       <td width="30%"><font size=2>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;DNSv4 3:</td>
		       <td width="30%"><font size=2><input type="text" name="dns3" size="18" maxlength="15" value=<% getInfo("wan-dns3"); %>></td>
		      <td width="40%" align=left><font size=2>       
		      	<select name="dnspvc3" id='wanlist3' style="display:none">
		      	<option value=65535>None</option>
				<!-- // Mason Yu for IPv6 -->
		      	<% if_wan_list("rt"); %>
		      	</select>
		      </td>
		    </tr>
		    </table>
		    </div>
			
			<!-- // Mason Yu for IPv6. Start-->
			<div id='dnsv6Tbl' style="display:none">
			<table border=0 width="600" cellspacing=4 cellpadding=0>		    
		    <tr>
		      <td width="40%" colspan="3"><font size=2>		        
		      </td>
		      <td><div id='dnsv6enable' style="display:none"><% checkWrite("Dnsv6BindPvc"); %></div></td>
		    </tr>
		    </table>		
			
			 <div id='dnsv6Info' style="display:none">
		     <table border=0 width="600" cellspacing=4 cellpadding=0>
		    <tr>
		       <td width="30%"><font size=2>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;DNSv6 1:</td>
		       <td width="30%"><font size=2><input type="text" name="dnsv61" size="18" maxlength="39" value=<% getInfo("wan-dnsv61"); %>></td>
		       <td width="40%" align=left><font size=2>       
		      	<select name="dnsv6pvc1" id='v6wanlist1' style="display:none">
		      	<option value=65535>None</option>
		      	<% if_wan_list("rtv6"); %>
		      	</select>
		      </td>
		    </tr>
		    <tr>
		       <td width="30%"><font size=2>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;DNSv6 2:</td>
		       <td width="30%"><font size=2><input type="text" name="dnsv62" size="18" maxlength="39" value=<% getInfo("wan-dnsv62"); %>></td>
		      <td width="40%" align=left><font size=2>         
		      	<select name="dnsv6pvc2" id='v6wanlist2' style="display:none">
		      	<option value=65535>None</option>
		      	<% if_wan_list("rtv6"); %>
		      	</select>
		      </td>
		    </tr>
		    <tr>
		       <td width="30%"><font size=2>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;DNSv6 3:</td>
		       <td width="30%"><font size=2><input type="text" name="dnsv63" size="18" maxlength="39" value=<% getInfo("wan-dnsv63"); %>></td>
		      <td width="40%" align=left><font size=2>       
		      	<select name="dnsv6pvc3" id='v6wanlist3' style="display:none">
		      	<option value=65535>None</option>
		      	<% if_wan_list("rtv6"); %>
		      	</select>
		      </td>
		    </tr>
		    </table>
			</div>
			</div>
			<!-- // Mason Yu for IPv6. End-->
			
		     <input type="hidden" value="/dns.asp" name="submit-url">
	 		  <input type="submit" value="Apply Changes" name="save" onClick="return saveClick()">&nbsp;&nbsp;&nbsp;	 		 
		    </form>
		</DIV>
<script>
	initAutoDns = document.dns.dnsMode[0].checked;
	if (document.dns.dnsMode[0].checked)
	{
		autoDNSclicked();
		
		// Mason Yu for IPv6
		if ( <% checkWrite("IPv6Show"); %>)
			autoDNSv6clicked();
	}
	else
	{
		manualDNSclicked();
		<% checkWrite("dnsBindPvcInit"); %>
		
		// Mason Yu for IPv6
		if ( <% checkWrite("IPv6Show"); %> )
		{
			manualDNSv6clicked();
			<% checkWrite("dnsv6BindPvcInit"); %>
		}
	}
	if(document.dns.dns1.value=='')
		document.dns.dns1.value='0.0.0.0';
	if(document.dns.dns2.value=='0.0.0.0')
		document.dns.dns2.value='';
	if(document.dns.dns3.value=='0.0.0.0')
		document.dns.dns3.value='';
	
	if(document.dns.dnsv61.value=='::')
		document.dns.dnsv61.value='';
	if(document.dns.dnsv62.value=='::')
		document.dns.dnsv62.value='';
	if(document.dns.dnsv63.value=='::')
		document.dns.dnsv63.value='';
	
</script>
</blockquote>
</body>
</html>
