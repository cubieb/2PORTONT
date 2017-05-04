<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>RADVD Configuration Setup</TITLE>
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
<script type="text/javascript" src="share.js"></script>
<STYLE type=text/css>
@import url(/style/default.css);
</STYLE>
<SCRIPT language="javascript" type="text/javascript">
function resetClick()
{
	document.radvd.reset;
}

function saveChanges()
{
	if (document.radvd.MaxRtrAdvIntervalAct.value.length !=0) {
		if ( checkDigit(document.radvd.MaxRtrAdvIntervalAct.value) == 0) {
			alert("请输入整数");	
			document.radvd.MaxRtrAdvIntervalAct.focus();
			return false;
		}
	}
	
	MaxRAI = parseInt(document.radvd.MaxRtrAdvIntervalAct.value, 10);
	if ( MaxRAI < 4 || MaxRAI > 1800 ) {
		alert("请输入整数 4~1800");
		document.radvd.MaxRtrAdvIntervalAct.focus();
		return false;
	}
		
	if (document.radvd.MinRtrAdvIntervalAct.value.length !=0) {
		if ( checkDigit(document.radvd.MinRtrAdvIntervalAct.value) == 0) {
			alert("请输入整数");	
			document.radvd.MinRtrAdvIntervalAct.focus();
			return false;
		}
	}
	
	MinRAI = parseInt(document.radvd.MinRtrAdvIntervalAct.value, 10);
	MaxRAI075 = 0.75 * MaxRAI;
	if ( MinRAI < 3 || MinRAI > MaxRAI075 ) {
		alert("请输入整数 3~0.75*最大发送间隔");
		document.radvd.MinRtrAdvIntervalAct.focus();
		return false;
	}
	
	if (document.radvd.AdvCurHopLimitAct.value.length !=0) {
		if ( checkDigit(document.radvd.AdvCurHopLimitAct.value) == 0) {
			alert("无效的AdvCurHopLimit! 必须是10进制(0-9)");	 //Invalid AdvCurHopLimit! It should be the decimal number (0-9)
			document.radvd.AdvCurHopLimitAct.focus();
			return false;
		}
	} 
	
	if (document.radvd.AdvDefaultLifetimeAct.value.length !=0) {
		if ( checkDigit(document.radvd.AdvDefaultLifetimeAct.value) == 0) {
			alert("无效的AdvDefaultLifetime! 必须是10进制(0-9)");	//Invalid AdvDefaultLifetime! It should be the decimal number (0-9)
			document.radvd.AdvDefaultLifetimeAct.focus();
			return false;
		}
	}
	
	dlt = parseInt(document.radvd.AdvDefaultLifetimeAct.value, 10);	
	if ( dlt != 0 && (dlt < MaxRAI || dlt > 9000) ) {
		alert("AdvDefaultLifetime必须是0或于9000秒及最大发送间隔之间"); //AdvDefaultLifetime must be either zero or between MaxRtrAdvInterval and 9000 seconds.
		document.radvd.AdvDefaultLifetimeAct.focus();
		return false;
	}
	
	if (document.radvd.AdvReachableTimeAct.value.length !=0) {
		if ( checkDigit(document.radvd.AdvReachableTimeAct.value) == 0) {
			alert("无效的AdvReachableTime! 必须是10进制(0-9)");	//Invalid AdvReachableTime! It should be the decimal number (0-9)
			document.radvd.AdvReachableTimeAct.focus();
			return false;
		}
	}
	
	if (document.radvd.AdvRetransTimerAct.value.length !=0) {
		if ( checkDigit(document.radvd.AdvRetransTimerAct.value) == 0) {
			alert("无效的AdvRetransTimer! 必须是10进制(0-9)");	 //Invalid AdvRetransTimer! It should be the decimal number (0-9)
			document.radvd.AdvRetransTimerAct.focus();
			return false;
		}
	}
	
	if (document.radvd.AdvLinkMTUAct.value.length !=0) {
		if ( checkDigit(document.radvd.AdvLinkMTUAct.value) == 0) {
			alert("无效的AdvLinkMTU! 必须是10进制(0-9)");	 //Invalid AdvLinkMTU! It should be the decimal number (0-9)
			document.radvd.AdvLinkMTUAct.focus();
			return false;
		}
	}
	
	lmtu= parseInt(document.radvd.AdvLinkMTUAct.value, 10);	
	if ( lmtu != 0 && (lmtu < 1280 || lmtu > 1500) ) {
		alert("AdvLinkMTU 必须是0或于1280及1500之间"); //AdvLinkMTU must be either zero or between 1280 and 1500.
		document.radvd.AdvLinkMTUAct.focus();
		return false;
	}	
	
	if (document.radvd.PrefixMode.value == 1) {
		if (document.radvd.prefix_ip.value =="") {
			alert("前缀IP地址不能为空! 格式为IPV6地址 如: 3ffe:501:ffff:100::"); //Prefix IP address cannot be empty! Format is IPV6 address. For example: 3ffe:501:ffff:100::
			document.radvd.prefix_ip.value = document.radvd.prefix_ip.defaultValue;
			document.radvd.prefix_ip.focus();
			return false;
		} else {
			if ( validateKeyV6IP(document.radvd.prefix_ip.value) == 0) {
				alert("无效的前缀IP!");	//Invalid prefix IP!
				document.radvd.prefix_ip.focus();
				return false;
			}
		}
		
		if (document.radvd.prefix_len.value =="") {
			alert("前缀长度不能为空!"); //Prefix length cannot be empty!
			document.radvd.prefix_len.value = document.radvd.prefix_len.defaultValue;
			document.radvd.prefix_len.focus();
			return false;
		} else {
			if ( checkDigit(document.radvd.prefix_len.value) == 0) {
				alert("无效的前缀长度! 必须是10进制(0-9)");	//Invalid prefix length! It should be the decimal number (0-9)
				document.radvd.prefix_len.focus();
				return false;
			}
		}
		
		if (document.radvd.AdvValidLifetimeAct.value.length !=0) {
			if ( checkDigit(document.radvd.AdvValidLifetimeAct.value) == 0) {
				alert("无效的有效寿命! 必须是10进制(0-9)");	//Invalid AdvValidLifetime! It should be the decimal number (0-9)
				document.radvd.AdvValidLifetimeAct.focus();
				return false;
			}
		}
		
		if (document.radvd.AdvPreferredLifetimeAct.value.length !=0) {
			if ( checkDigit(document.radvd.AdvPreferredLifetimeAct.value) == 0) {
				alert("无效的首选寿命! 必须是10进制(0-9)");	//Invalid AdvPreferredLifetime! It should be the decimal number (0-9)
				document.radvd.AdvPreferredLifetimeAct.focus();
				return false;
			}
		}		
		
		vlt = parseInt(document.radvd.AdvValidLifetimeAct.value, 10);
		plt = parseInt(document.radvd.AdvPreferredLifetimeAct.value, 10);
		if ( vlt <= plt ) {			
			alert("有效寿命必须大于首选寿命"); //AdvValidLifeTime must be greater than AdvPreferredLifetime.
			document.radvd.AdvValidLifetimeAct.focus();
			return false;
		}
	
	}
	
	return true;
}


function updateInput()
{	
	if (document.radvd.PrefixMode.value == 1 ) {
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
</SCRIPT>
</head>

<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000">
<blockquote>
<DIV align="left" style="padding-left:20px; padding-top:5px">

<form action=/boaform/formRadvdSetup method=POST name="radvd">
<div class="tip" style="width:90% ">
	<b>RA 配置</b><br><br>	
</div>
<table cellpadding="0px" cellspacing="2px">
  <table cellpadding="0px" cellspacing="2px">
  <tr><td width="150px">RA使能:&nbsp;&nbsp;</td>  
  <td>
  <input type="radio" name="radvd_enable" value=0 <% checkWrite("radvd_enable0"); %>>off&nbsp;&nbsp;
  <input type="radio" name="radvd_enable" value=1 <% checkWrite("radvd_enable1"); %>>on&nbsp;&nbsp;
  </td>
  </tr>
  <tr><td width="150px">管理的地址配置:&nbsp;&nbsp;</td>  
      <td>
   	<input type="radio" name="AdvManagedFlagAct" value=0 <% checkWrite("radvd_ManagedFlag0"); %>>off&nbsp;&nbsp;
   	<input type="radio" name="AdvManagedFlagAct" value=1 <% checkWrite("radvd_ManagedFlag1"); %>>on&nbsp;&nbsp;
    </td>
  </tr>
  </table>
  
  <table cellpadding="0px" cellspacing="2px">
  <tr><td width="150px">其他状态配置:&nbsp;&nbsp;</td>  
      <td>
   	<input type="radio" name="AdvOtherConfigFlagAct" value=0 <% checkWrite("radvd_OtherConfigFlag0"); %>>off&nbsp;&nbsp;
   	<input type="radio" name="AdvOtherConfigFlagAct" value=1 <% checkWrite("radvd_OtherConfigFlag1"); %>>on&nbsp;&nbsp;
    </td>
  </tr>

  <tr>
      <td width="150px">最大发送间隔:</td>
      <td><input type="text" name="MaxRtrAdvIntervalAct" size="15" maxlength="15" value=<% getInfo("V6MaxRtrAdvInterval"); %>>秒</td>
  </tr>

  <tr>
      <td width="150px">最小发送间隔:</td>
      <td><input type="text" name="MinRtrAdvIntervalAct" size="15" maxlength="15" value=<% getInfo("V6MinRtrAdvInterval"); %>>秒</td>
  </tr>
  
  </table>
  
  </table>
   <div ID="radvdID" style="display:none">
  <tr>
      <td width="150px">AdvCurHopLimit:</td>
      <td><input type="text" name="AdvCurHopLimitAct" size="15" maxlength="15" value=<% getInfo("V6AdvCurHopLimit"); %>></td>
  </tr>
  
  <tr>
      <td width="150px">AdvDefaultLifetime:</td>
      <td><input type="text" name="AdvDefaultLifetimeAct" size="15" maxlength="15" value=<% getInfo("V6AdvDefaultLifetime"); %>></td>
  </tr>
  
  <tr>
      <td width="150px">AdvReachableTime:</td>
      <td><input type="text" name="AdvReachableTimeAct" size="15" maxlength="15" value=<% getInfo("V6AdvReachableTime"); %>></td>
  </tr>
  
  <tr>
      <td width="150px">AdvRetransTimer:</td>
      <td><input type="text" name="AdvRetransTimerAct" size="15" maxlength="15" value=<% getInfo("V6AdvRetransTimer"); %>></td>
  </tr>
  
  <tr>
      <td width="150px">AdvLinkMTU:</td>
      <td><input type="text" name="AdvLinkMTUAct" size="15" maxlength="15" value=<% getInfo("V6AdvLinkMTU"); %>></td>
  </tr>

 </table>
  <table cellpadding="0px" cellspacing="2px">
  <tr><td width="150px">AdvSendAdvert:&nbsp;&nbsp;</td>  
      <td>
   	<input type="radio" name="AdvSendAdvertAct" value=0 <% checkWrite("radvd_SendAdvert0"); %>>off&nbsp;&nbsp;
   	<input type="radio" name="AdvSendAdvertAct" value=1 <% checkWrite("radvd_SendAdvert1"); %>>on&nbsp;&nbsp;
    </td>
  </tr>
  </table>
  
  
  
 
  
  <tr></tr><tr></tr>


  <table cellpadding="0px" cellspacing="2px">
  <tr>
      <td width="150px">配置模式:</td>
      <td><select size="1" name="PrefixMode" id="prefixmode" onChange="ramodechange(this)">
          <OPTION VALUE="0" > 自动配置</OPTION>
          <OPTION VALUE="1" > 手动配置</OPTION>
	  </select>
      </td>
  </tr>
  </table>
  
 
       <table cellpadding="0px" cellspacing="2px">
         <tr><td width="150px">前辍:</td>
             <td><input type=text name=prefix_ip size=24 maxlength=24 value=<% getInfo("V6prefix_ip"); %>></td>
         </tr>
         
         <tr><td width="150px">前辍长度:</td>
             <td><input type=text name=prefix_len size=15 maxlength=15 value=<% getInfo("V6prefix_len"); %>></td>
         </tr>         
         
         <tr><td width="150px">有效寿命:</td>
             <td><input type=text name=AdvValidLifetimeAct size=15 maxlength=15 value=<% getInfo("V6ValidLifetime"); %>></td>
         </tr>
         
         <tr><td width="150px">首选寿命:</td>
             <td><input type=text name=AdvPreferredLifetimeAct size=15 maxlength=15 value=<% getInfo("V6PreferredLifetime"); %>></td>
         </tr>
         
         <tr><td width="150px">AdvOnLink:&nbsp;&nbsp;</td>  
             <td>
   	         <input type="radio" name="AdvOnLinkAct" value=0 <% checkWrite("radvd_OnLink0"); %>>off&nbsp;&nbsp;
   	         <input type="radio" name="AdvOnLinkAct" value=1 <% checkWrite("radvd_OnLink1"); %>>on&nbsp;&nbsp;
             </td>
         </tr>
  
         <tr><td width="150px">AdvAutonomous:&nbsp;&nbsp;</td>  
             <td>
   	         <input type="radio" name="AdvAutonomousAct" value=0 <% checkWrite("radvd_Autonomous0"); %>>off&nbsp;&nbsp;
   	         <input type="radio" name="AdvAutonomousAct" value=1 <% checkWrite("radvd_Autonomous1"); %>>on&nbsp;&nbsp;
             </td>
         </tr>
         
       </table>
  </div>

  <br>
      <input type="submit" value="保存/应用" name="save" onClick="return saveChanges()">&nbsp;&nbsp;
      <!--input type="reset" value="Undo" name="reset" onClick="resetClick()"-->
      <input type="hidden" value="/radvdconf.asp" name="submit-url">


<script>
	<% initPage("radvd_conf"); %>		
</script>


 </form>
 </DIV>
</blockquote>
</body>

</html>
