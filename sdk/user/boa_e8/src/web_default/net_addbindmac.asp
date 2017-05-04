<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>中国电信-DHCP</TITLE>
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

/********************************************************************
**          on document submit
********************************************************************/
function on_submit(act)
{
	with ( document.form )
	{
		action.value = act;
		if(act == "sv")
		{
			if ( macAddr_Dhcp.value.length == 0 )
			{
				alert("MAC地址不能为空.");
				return;
			}
			else if ( sji_checkmac(macAddr_Dhcp.value) == false )
			{
				msg = "MAC地址 \"" + macAddr_Dhcp.value + "\" 非法.";
				alert(msg);
				return;
			}
			if ( ipAddr_Dhcp.value.length == 0 )
			{
				alert("IP地址不能为空.");
				return;
			}
			else if ( sji_checkvip(ipAddr_Dhcp.value) == false )
			{
				msg = "IP地址 \"" + ipAddr_Dhcp.value + "\" 非法.";
				alert(msg);
				return;
			}
			submit();
		}
	}
}

// done hiding -->
</script>
   </head>

<!-------------------------------------------------------------------------------------->
<!--主页代码-->

   <body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000">
        <DIV align="left" style="padding-left:20px; padding-top:10px">
         <form action=/boaform/formMacAddrBase method="post" name="form">
            <b>请添加一个预留IP地址与MAC地址的映射关系</b>
            <br>
            <br>
            <table border="0" cellpadding="0" cellspacing="0">
               <tr>
                  <td width="200">MAC地址:<br>(e.g.,00-90-96-01-2A-3B)</td>
                  <td valign="top">
                  <input type="text" name="macAddr_Dhcp" size="20"></td>
               </tr>
               <tr>
                  <td width="200">预留IP地址:<br>(e.g.,192.168.1.2)</td>
                  <td valign="top">
                  <input type="text" name="ipAddr_Dhcp" size="20"></td>
               </tr>
            </table>
            <br>

			<table border="0" width="80%">
				<tr>
				<td align="left" width="30%"><input type="button" class="button" onClick="history.back();" value="返回"></td>
				<td align="left"><input type="button" class="button" onClick="on_submit('sv');" value="确定"></td>
				<input type="hidden" id="action" name="action" value="sv">
				<input type="hidden" name="submit-url" value="/net_mopreipaddr.asp">
				</tr>
			</table>
         </form>
	  </DIV>
   </body>
<%addHttpNoCache();%>
</html>
