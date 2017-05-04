<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title>RADVD <% multilang(LANG_CONFIGURATION); %></title>
<script type="text/javascript" src="share.js">
</script>

<SCRIPT>

function resetClick()
{
	document.radvd.reset;
}

function saveChanges()
{
	if (document.radvd.MaxRtrAdvIntervalAct.value.length !=0) {
		if ( checkDigit(document.radvd.MaxRtrAdvIntervalAct.value) == 0) {
			alert("<% multilang(LANG_INVALID_MAXRTRADVINTERVAL_IT_SHOULD_BE_THE_DECIMAL_NUMBER_0_9); %>");
			document.radvd.MaxRtrAdvIntervalAct.focus();
			return false;
		}
	}
	
	MaxRAI = parseInt(document.radvd.MaxRtrAdvIntervalAct.value, 10);
	if ( MaxRAI < 4 || MaxRAI > 1800 ) {
		alert("<% multilang(LANG_MAXRTRADVINTERVAL_MUST_BE_NO_LESS_THAN_4_SECONDS_AND_NO_GREATER_THAN_1800_SECONDS); %>");
		document.radvd.MaxRtrAdvIntervalAct.focus();
		return false;
	}
		
	if (document.radvd.MinRtrAdvIntervalAct.value.length !=0) {
		if ( checkDigit(document.radvd.MinRtrAdvIntervalAct.value) == 0) {
			alert("<% multilang(LANG_INVALID_MINRTRADVINTERVAL_IT_SHOULD_BE_THE_DECIMAL_NUMBER_0_9); %>");
			document.radvd.MinRtrAdvIntervalAct.focus();
			return false;
		}
	}
	
	MinRAI = parseInt(document.radvd.MinRtrAdvIntervalAct.value, 10);
	MaxRAI075 = 0.75 * MaxRAI;
	if ( MinRAI < 3 || MinRAI > MaxRAI075 ) {
		alert("<% multilang(LANG_MINRTRADVINTERVAL_MUST_BE_NO_LESS_THAN_3_SECONDS_AND_NO_GREATER_THAN_0_75_MAXRTRADVINTERVAL); %>");
		document.radvd.MinRtrAdvIntervalAct.focus();
		return false;
	}
	
	if (document.radvd.AdvCurHopLimitAct.value.length !=0) {
		if ( checkDigit(document.radvd.AdvCurHopLimitAct.value) == 0) {
			alert("<% multilang(LANG_INVALID_ADVCURHOPLIMIT_IT_SHOULD_BE_THE_DECIMAL_NUMBER_0_9); %>");
			document.radvd.AdvCurHopLimitAct.focus();
			return false;
		}
	} 
	
	if (document.radvd.AdvDefaultLifetimeAct.value.length !=0) {
		if ( checkDigit(document.radvd.AdvDefaultLifetimeAct.value) == 0) {
			alert("<% multilang(LANG_INVALID_ADVDEFAULTLIFETIME_IT_SHOULD_BE_THE_DECIMAL_NUMBER_0_9); %>");
			document.radvd.AdvDefaultLifetimeAct.focus();
			return false;
		}
	}
	
	dlt = parseInt(document.radvd.AdvDefaultLifetimeAct.value, 10);	
	if ( dlt != 0 && (dlt < MaxRAI || dlt > 9000) ) {
		alert("<% multilang(LANG_ADVDEFAULTLIFETIME_MUST_BE_EITHER_ZERO_OR_BETWEEN_MAXRTRADVINTERVAL_AND_9000_SECONDS); %>");
		document.radvd.AdvDefaultLifetimeAct.focus();
		return false;
	}
	
	if (document.radvd.AdvReachableTimeAct.value.length !=0) {
		if ( checkDigit(document.radvd.AdvReachableTimeAct.value) == 0) {
			alert("<% multilang(LANG_INVALID_ADVREACHABLETIME_IT_SHOULD_BE_THE_DECIMAL_NUMBER_0_9); %>");
			document.radvd.AdvReachableTimeAct.focus();
			return false;
		}
	}
	
	if (document.radvd.AdvRetransTimerAct.value.length !=0) {
		if ( checkDigit(document.radvd.AdvRetransTimerAct.value) == 0) {
			alert("<% multilang(LANG_INVALID_ADVRETRANSTIMER_IT_SHOULD_BE_THE_DECIMAL_NUMBER_0_9); %>");
			document.radvd.AdvRetransTimerAct.focus();
			return false;
		}
	}
	
	if (document.radvd.AdvLinkMTUAct.value.length !=0) {
		if ( checkDigit(document.radvd.AdvLinkMTUAct.value) == 0) {
			alert("<% multilang(LANG_INVALID_ADVLINKMTU_IT_SHOULD_BE_THE_DECIMAL_NUMBER_0_9); %>");
			document.radvd.AdvLinkMTUAct.focus();
			return false;
		}
	}
	
	lmtu= parseInt(document.radvd.AdvLinkMTUAct.value, 10);	
	if ( lmtu != 0 && (lmtu < 1280 || lmtu > 1500) ) {
		alert("<% multilang(LANG_ADVLINKMTU_MUST_BE_EITHER_ZERO_OR_BETWEEN_1280_AND_1500); %>");
		document.radvd.AdvLinkMTUAct.focus();
		return false;
	}	


    if(document.radvd.EnableULA[1].checked){
		if (document.radvd.ULAPrefix.value =="") {
			alert("<% multilang(LANG_ULA_PREFIX_IP_ADDRESS_CANNOT_BE_EMPTY_FORMAT_IS_IPV6_ADDRESS_FOR_EXAMPLE_FC01); %>");
			document.radvd.ULAPrefix.focus();
			return false;
		} else {
			if ( validateKeyV6IP(document.radvd.ULAPrefix.value) == 0) {
				alert("<% multilang(LANG_INVALID_ULA_PREFIX_IP); %>");
				document.radvd.ULAPrefix.focus();
				return false;
			}
		}
		
	   if (document.radvd.ULAPrefixlen.value =="") {
			alert("<% multilang(LANG_ULA_PREFIX_LENGTH_CANNOT_BE_EMPTY_FOR_EXAMPLE_64); %>");
			document.radvd.ULAPrefixlen.focus();
			return false;
		} else {
			if ( checkDigit(document.radvd.ULAPrefixlen.value) == 0) {
				alert("<% multilang(LANG_INVALID_ULA_PREFIX_LENGTH_IT_SHOULD_BE_THE_DECIMAL_NUMBER_0_9); %>");
				document.radvd.ULAPrefixlen.focus();
				return false;
			}
		}
		
		if (document.radvd.ULAPrefixValidTime.value =="") {
			alert("<% multilang(LANG_ULA_PREFIX_VALID_TIME_CANNOT_BE_EMPTY_VALID_RANGE_IS_600_4294967295); %>");
			document.radvd.ULAPrefixlen.focus();
			return false;
		} else if ( checkDigit(document.radvd.ULAPrefixValidTime.value) == 0) {
				alert("<% multilang(LANG_INVALID_ULAPREFIXVALIDTIME_IT_SHOULD_BE_THE_DECIMAL_NUMBER_0_9); %>");
				document.radvd.ULAPrefixValidTime.focus();
				return false;
		}
		
		if (document.radvd.ULAPrefixPreferedTime.value =="") {
			alert("<% multilang(LANG_ULA_PREFIX_VALID_TIME_CANNOT_BE_EMPTY_VALID_RANGE_IS_600_4294967295); %>");
			document.radvd.ULAPrefixlen.focus();
			return false;
		} else if ( checkDigit(document.radvd.ULAPrefixPreferedTime.value) == 0) {
				alert("<% multilang(LANG_INVALID_ULAPREFIXPREFEREDTIME_IT_SHOULD_BE_THE_DECIMAL_NUMBER_0_9); %>");
				document.radvd.ULAPrefixPreferedTime.focus();
				return false;
		}
    }
	
	if (document.radvd.PrefixMode.value == 1) {
		if (document.radvd.prefix_ip.value =="") {
			alert("<% multilang(LANG_PREFIX_IP_ADDRESS_CANNOT_BE_EMPTY_FORMAT_IS_IPV6_ADDRESS_FOR_EXAMPLE_3FFE_501_FFFF_100); %>");
			document.radvd.prefix_ip.value = document.radvd.prefix_ip.defaultValue;
			document.radvd.prefix_ip.focus();
			return false;
		} else {
			if ( validateKeyV6IP(document.radvd.prefix_ip.value) == 0) {
				alert("<% multilang(LANG_INVALID_PREFIX_IP); %>");
				document.radvd.prefix_ip.focus();
				return false;
			}
		}
		
		if (document.radvd.prefix_len.value =="") {
			alert("<% multilang(LANG_PREFIX_LENGTH_CANNOT_BE_EMPTY); %>");
			document.radvd.prefix_len.value = document.radvd.prefix_len.defaultValue;
			document.radvd.prefix_len.focus();
			return false;
		} else {
			if ( checkDigit(document.radvd.prefix_len.value) == 0) {
				alert("<% multilang(LANG_INVALID_PREFIX_LENGTH_IT_SHOULD_BE_THE_DECIMAL_NUMBER_0_9); %>");
				document.radvd.prefix_len.focus();
				return false;
			}
		}
		
		if (document.radvd.AdvValidLifetimeAct.value.length !=0) {
			if ( checkDigit(document.radvd.AdvValidLifetimeAct.value) == 0) {
				alert("<% multilang(LANG_INVALID_ADVVALIDLIFETIME_IT_SHOULD_BE_THE_DECIMAL_NUMBER_0_9); %>");
				document.radvd.AdvValidLifetimeAct.focus();
				return false;
			}
		}
		
		if (document.radvd.AdvPreferredLifetimeAct.value.length !=0) {
			if ( checkDigit(document.radvd.AdvPreferredLifetimeAct.value) == 0) {
				alert("<% multilang(LANG_INVALID_ADVPREFERREDLIFETIME_IT_SHOULD_BE_THE_DECIMAL_NUMBER_0_9); %>");
				document.radvd.AdvPreferredLifetimeAct.focus();
				return false;
			}
		}		
		
		vlt = parseInt(document.radvd.AdvValidLifetimeAct.value, 10);
		plt = parseInt(document.radvd.AdvPreferredLifetimeAct.value, 10);
		if ( vlt <= plt ) {			
			alert("<% multilang(LANG_ADVVALIDLIFETIME_MUST_BE_GREATER_THAN_ADVPREFERREDLIFETIME); %>");
			document.radvd.AdvValidLifetimeAct.focus();
			return false;
		}
	
	}
	

		if (document.radvd.RDNSS1.value !="") {
			if ( validateKeyV6IP(document.radvd.RDNSS1.value) == 0) {
				alert("<% multilang(LANG_INVALID_RDNSS1_IP); %>");
				document.radvd.RDNSS1.focus();
				return false;
			}
		}

		if (document.radvd.RDNSS2.value !="") {
			if ( validateKeyV6IP(document.radvd.RDNSS2.value) == 0) {
				alert("<% multilang(LANG_INVALID_RDNSS2_IP); %>");
				document.radvd.RDNSS2.focus();
				return false;
			}
		}

	return true;
}


function updateInput()
{	
/*	document.all.prefixModeDiv.style.display = 'block';

	if (document.radvd.PrefixMode.value == 1) {
		if (document.getElementById)  // DOM3 = IE5, NS6
			document.getElementById('radvdID').style.display = 'block';
			else {
			if (document.layers == false) // IE4
				document.all.radvdID.style.display = 'block';
		}
	} else {
		if (document.getElementById)  // DOM3 = IE5, NS6
			document.getElementById('radvdID').style.display = 'none';
		else {
			if (document.layers == false) // IE4
				document.all.radvdID.style.display = 'none';
		}
	}
*/
}

function ramodechange(obj)
{
	with ( document.forms[0] ) 
	{
		if(obj.value == "0")
		{
			if (document.getElementById)  // DOM3 = IE5, NS6
			document.getElementById('radvdID').style.display = 'none';
			else {
				if (document.layers == false) // IE4
					document.all.radvdID.style.display = 'none';
			}
		}
		else
		{
			if (document.getElementById)  // DOM3 = IE5, NS6
			document.getElementById('radvdID').style.display = 'block';
			else {
			if (document.layers == false) // IE4
				document.all.radvdID.style.display = 'block';
			}
		}
	}
}

function ULASelection()
{
    if(document.radvd.EnableULA[0].checked)
    {
		document.all.ULAdiv.style.display = 'none';
    }
    else
    {
		document.all.ULAdiv.style.display = 'block';
    }
}


function on_init()
{
/*
    if(document.radvd.EnableULA[0].checked)
    {
		document.all.ULAdiv.style.display = 'none';
    }
    else
    {
		document.all.ULAdiv.style.display = 'block';
    }

	document.all.radvdID.style.display = 'block';
	if(document.radvd.PrefixMode.value == 1)
		document.all.radvdID.style.display = 'block';
	else
		document.all.radvdID.style.display = 'none';
*/	
}

</SCRIPT>
</head>

<body onLoad="on_init();">
<blockquote>
<h2><font color="#0000FF">RADVD <% multilang(LANG_CONFIGURATION); %></font></h2>

<form action=/boaform/formRadvdSetup method=POST name="radvd">
<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><td><font size=2>
    <% multilang(LANG_THIS_PAGE_IS_USED_TO_SETUP_THE_RADVD_S_CONFIGURATION_OF_YOUR_DEVICE); %>
  </font></td></tr>  
  <tr><td><hr size=1 noshade align=top></td></tr>
</table>

<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr>
      <td width="30%"><font size=2><b><% multilang(LANG_MAXRTRADVINTERVAL); %>:</b></td>
      <td width="70%"><input type="text" name="MaxRtrAdvIntervalAct" size="15" maxlength="15" value=<% getInfo("V6MaxRtrAdvInterval"); %>></td>
  </tr>

  <tr>
      <td width="30%"><font size=2><b><% multilang(LANG_MINRTRADVINTERVAL); %>:</b></td>
      <td width="70%"><input type="text" name="MinRtrAdvIntervalAct" size="15" maxlength="15" value=<% getInfo("V6MinRtrAdvInterval"); %>></td>
  </tr>


  <tr><td width=200><font size=2><b><% multilang(LANG_ADVMANAGEDFLAG); %>:</b>&nbsp;&nbsp;</font></td>  
      <td width=200><font size=2>
   	<input type="radio" name="AdvManagedFlagAct" value=0 <% checkWrite("radvd_ManagedFlag0"); %>><% multilang(LANG_OFF); %>&nbsp;&nbsp;
   	<input type="radio" name="AdvManagedFlagAct" value=1 <% checkWrite("radvd_ManagedFlag1"); %>><% multilang(LANG_ON); %>&nbsp;&nbsp;
     </font></td>
  </tr>
  
  <tr><td width=200><font size=2><b><% multilang(LANG_ADVOTHERCONFIGFLAG); %>:</b>&nbsp;&nbsp;</font></td>  
      <td width=200><font size=2>
   	<input type="radio" name="AdvOtherConfigFlagAct" value=0 <% checkWrite("radvd_OtherConfigFlag0"); %>><% multilang(LANG_OFF); %>&nbsp;&nbsp;
   	<input type="radio" name="AdvOtherConfigFlagAct" value=1 <% checkWrite("radvd_OtherConfigFlag1"); %>><% multilang(LANG_ON); %>&nbsp;&nbsp;
     </font></td>
  </tr>

</table>

<div style="display:none">

<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr>
      <td width="30%"><font size=2><b><% multilang(LANG_ADVCURHOPLIMIT); %>:</b></td>
      <td width="70%"><input type="text" name="AdvCurHopLimitAct" size="15" maxlength="15" value=<% getInfo("V6AdvCurHopLimit"); %>></td>
  </tr>
  
  <tr>
      <td width="30%"><font size=2><b><% multilang(LANG_ADVDEFAULTLIFETIME); %>:</b></td>
      <td width="70%"><input type="text" name="AdvDefaultLifetimeAct" size="15" maxlength="15" value=<% getInfo("V6AdvDefaultLifetime"); %>></td>
  </tr>
  
  <tr>
      <td width="30%"><font size=2><b><% multilang(LANG_ADVREACHABLETIME); %>:</b></td>
      <td width="70%"><input type="text" name="AdvReachableTimeAct" size="15" maxlength="15" value=<% getInfo("V6AdvReachableTime"); %>></td>
  </tr>
  
  <tr>
      <td width="30%"><font size=2><b><% multilang(LANG_ADVRETRANSTIMER); %>:</b></td>
      <td width="70%"><input type="text" name="AdvRetransTimerAct" size="15" maxlength="15" value=<% getInfo("V6AdvRetransTimer"); %>></td>
  </tr>
  
  <tr>
      <td width="30%"><font size=2><b><% multilang(LANG_ADVLINKMTU); %>:</b></td>
      <td width="70%"><input type="text" name="AdvLinkMTUAct" size="15" maxlength="15" value=<% getInfo("V6AdvLinkMTU"); %>></td>
  </tr>

 
  <table border=0 width="400" cellspacing=4 cellpadding=0>
  <tr><td width=200><font size=2><b><% multilang(LANG_ADVSENDADVERT); %>:</b>&nbsp;&nbsp;</font></td>  
      <td width=200><font size=2>
   	<input type="radio" name="AdvSendAdvertAct" value=0 <% checkWrite("radvd_SendAdvert0"); %>><% multilang(LANG_OFF); %>&nbsp;&nbsp;
   	<input type="radio" name="AdvSendAdvertAct" value=1 <% checkWrite("radvd_SendAdvert1"); %>><% multilang(LANG_ON); %>&nbsp;&nbsp;
     </font></td>
  </tr>
  </table>
  
  </table>

  <table border=0 width="400" cellspacing=4 cellpadding=0>
  <tr><td width=200><font size=2><b><% multilang(LANG_ENABLE_ULA); %>:</b>&nbsp;&nbsp;</font></td>  
      <td width=200><font size=2>
   	<input type="radio" name="EnableULA" value=0 <% checkWrite("radvd_EnableULAFlag0"); %> onClick="ULASelection();"><% multilang(LANG_OFF); %>&nbsp;&nbsp;
   	<input type="radio" name="EnableULA" value=1 <% checkWrite("radvd_EnableULAFlag1"); %> onClick="ULASelection();"><% multilang(LANG_ON); %>&nbsp;&nbsp;
     </font></td>
  </tr>
  </table>

  <div ID="ULAdiv" style="display:none">
  <tr>
      <td width="30%"><font size=2><b><% multilang(LANG_ULA_PREFIX); %>:</b></td>
      <td width="70%"><input type="text" name="ULAPrefix" size="48" maxlength="48" value=<% getInfo("V6ULAPrefix"); %>></td>
  </tr><br>

  <tr>
      <td width="30%"><font size=2><b><% multilang(LANG_ULA_PREFIX_LEN); %>:</b></td>
      <td width="70%"><input type="text" name="ULAPrefixlen" size="10" maxlength="10" value=<% getInfo("V6ULAPrefixlen"); %>></td>
  </tr><br>
  <tr>
      <td width="30%"><font size=2><b><% multilang(LANG_ULA_PREFIX_VALID_TIME); %>:</b></td>
      <td width="70%"><input type="text" name="ULAPrefixValidTime" size="10" maxlength="10" value=<% getInfo("V6ULAPrefixValidLifetime"); %>></td>
  </tr><br>
  <tr>
      <td width="30%"><font size=2><b><% multilang(LANG_ULA_PREFIX_PREFERED_TIME); %>:</b></td>
      <td width="70%"><input type="text" name="ULAPrefixPreferedTime" size="10" maxlength="10" value=<% getInfo("V6ULAPrefixPreferredLifetime"); %>></td>
  </tr><br><br>
  </div>
  
  <tr></tr><tr></tr>

</table>
</div>

  <div ID="prefixModeDiv" style="display:none">
  <tr>
      <td width=100><font size=2><% multilang(LANG_PREFIX_MODE); %>:</td>
      <td><font size=2><select size="1" name="PrefixMode" id="prefixmode" onChange="ramodechange(this)">
          <OPTION VALUE="0" > <% multilang(LANG_AUTO); %></OPTION>
          <OPTION VALUE="1" > <% multilang(LANG_MANUAL); %></OPTION>
	  </select>
      </td>
  </tr>

  </div>
  
  
  <div ID="radvdID" style="display:none">
       <table border=0 width="500" cellspacing=4 cellpadding=0>
         <tr><td width=150><font size=2><b><% multilang(LANG_PREFIX); %>:</b></td>
             <td width=350><input type=text name=prefix_ip size=24 maxlength=24 value=<% getInfo("V6prefix_ip"); %>></td>
         </tr>
         
         <tr><td><font size=2><b><% multilang(LANG_PREFIX_LENGTH); %>:</b></td>
             <td><input type=text name=prefix_len size=15 maxlength=15 value=<% getInfo("V6prefix_len"); %>></td>
         </tr>         
         
         <tr><td><font size=2><b><% multilang(LANG_ADVVALIDLIFETIME); %>:</b></td>
             <td><input type=text name=AdvValidLifetimeAct size=15 maxlength=15 value=<% getInfo("V6ValidLifetime"); %>></td>
         </tr>
         
         <tr><td><font size=2><b><% multilang(LANG_ADVPREFERREDLIFETIME); %>:</b></td>
             <td><input type=text name=AdvPreferredLifetimeAct size=15 maxlength=15 value=<% getInfo("V6PreferredLifetime"); %>></td>
         </tr>
         
         <tr><td width=200><font size=2><b><% multilang(LANG_ADVONLINK); %>:</b>&nbsp;&nbsp;</font></td>  
             <td width=200><font size=2>
   	         <input type="radio" name="AdvOnLinkAct" value=0 <% checkWrite("radvd_OnLink0"); %>>off&nbsp;&nbsp;
   	         <input type="radio" name="AdvOnLinkAct" value=1 <% checkWrite("radvd_OnLink1"); %>>on&nbsp;&nbsp;
             </font></td>
         </tr>
  
         <tr><td width=200><font size=2><b><% multilang(LANG_ADVAUTONOMOUS); %>:</b>&nbsp;&nbsp;</font></td>  
             <td width=200><font size=2>
   	         <input type="radio" name="AdvAutonomousAct" value=0 <% checkWrite("radvd_Autonomous0"); %>>off&nbsp;&nbsp;
   	         <input type="radio" name="AdvAutonomousAct" value=1 <% checkWrite("radvd_Autonomous1"); %>>on&nbsp;&nbsp;
             </font></td>
         </tr>
         
  <tr>
      <td width="30%"><font size=2><b><% multilang(LANG_RDNSS_1); %>:</b></td>
      <td width="70%"><input type="text" name="RDNSS1" size="48" maxlength="48" value=<% getInfo("V6RDNSS1"); %>></td>
  </tr>


  <tr>
      <td width="30%"><font size=2><b><% multilang(LANG_RDNSS_2); %>:</b></td>
      <td width="70%"><input type="text" name="RDNSS2" size="48" maxlength="48" value=<% getInfo("V6RDNSS2"); %>></td>
  </tr>
       </table>
  </div>

  <br>
  <br>
      <input type="submit" value="<% multilang(LANG_APPLY_CHANGES); %>" name="save" onClick="return saveChanges()">&nbsp;&nbsp;
      <!--input type="reset" value="Undo" name="reset" onClick="resetClick()"-->
      <input type="hidden" value="/radvdconf.asp" name="submit-url">


<script>
	<% initPage("radvd_conf"); %>		
</script>


 </form>
</blockquote>
</body>

</html>
