<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>写入等级设置</TITLE>
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

var cgi = new Object();
<%init_dhcp_device_page();%>

/********************************************************************
**          on document load
********************************************************************/

function on_init()
{
	sji_docinit(document, cgi);
}

/********************************************************************
**          on document submit
********************************************************************/

function on_submit()
{
	with ( document.forms[0] )
	{
		if ( sji_checkvip(pcRangeStart.value) == false )
		{
			alert("地址 \"" + pcRangeStart.value + "\" 是无效IP地址.");
			return;
		}
		if ( sji_checkvip(pcRangeEnd.value) == false )
		{
			alert("地址 \"" + pcRangeEnd.value + "\" 是无效IP地址.");
			return;
		}
		if ( sji_checkvip(cmrRangeStart.value) == false )
		{
			alert("地址 \"" + cmrRangeStart.value + "\" 是无效IP地址.");
			return;
		}
		if ( sji_checkvip(cmrRangeEnd.value) == false )
		{
			alert("地址 \"" + cmrRangeEnd.value + "\" 是无效IP地址.");
			return;
		}
		if ( sji_checkvip(stbRangeStart.value) == false )
		{
			alert("地址 \"" + stbRangeStart.value + "\" 是无效IP地址.");
			return;
		}
		if ( sji_checkvip(stbRangeEnd.value) == false )
		{
			alert("地址 \"" + stbRangeEnd.value + "\" 是无效IP地址.");
			return;
		}
		if ( sji_checkvip(phoneRangeStart.value) == false )
		{
			alert("地址 \"" + phoneRangeStart.value + "\" 是无效IP地址.");
			return;
		}
		if ( sji_checkvip(phoneRangeEnd.value) == false )
		{
			alert("地址 \"" + phoneRangeEnd.value + "\" 是无效IP地址.");
			return;
		}

		submit();
	}
}

function on_back()
{
	var loc = "net_dhcpd.asp";
	var code = "location=\"" + loc + "\"";
	eval(code);
}

</SCRIPT>
</HEAD>
	<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init()">
		<blockquote>
			<DIV align="left" style="padding-left:20px; padding-top:10px">
				<form action=/boaform/formIpRange method= "post">
					<b>DHCP地址区间设置及与设备类型的映射</b><br>
					<div id="rstip" style="display:none;"><font color="red">提示：本页面的设置，需要重启路由器才能生效！</font><br></div>
					<table class="flat" border="1" cellpadding="0" cellspacing="0">
						<tr>
						<td class="hd" width="150">PC起始地址:</td>
						<td><input type="text" name="pcRangeStart" ></td>
						</tr>
						<tr>
						<td class="hd">PC终止地址:</td>
						<td><input type="text" name="pcRangeEnd"></td>
						</tr>
						<tr>
						<td colspan="2">&nbsp;</td>
						</tr>
						<tr>
						<td class="hd">摄像头起始地址:</td>
						<td><input type="text" name="cmrRangeStart" ></td>
						</tr>
						<tr>
						<td class="hd">摄像头终止地址:</td>
						<td><input type="text" name="cmrRangeEnd"></td>
						</tr>
						<tr>
						<td colspan="2">&nbsp;</td>
						</tr>
						<tr>
						<td class="hd">机顶盒起始地址:</td>
						<td><input type="text" name="stbRangeStart" ></td>
						</tr>
						<tr>
						<td class="hd">机顶盒终止地址:</td>
						<td><input type="text" name="stbRangeEnd"></td>
						</tr>
						<tr>
						<td colspan="2">&nbsp;</td>
						</tr>
						<tr>
						<td class="hd">Phone起始地址:</td>
						<td><input type="text" name="phoneRangeStart" ></td>
						</tr>
						<tr>
						<td class="hd">Phone终止地址:</td>
						<td><input type="text" name="phoneRangeEnd"></td>
						</tr>
					</table>
					<br>
					&nbsp;
					<input type="button" class="button" onClick="on_back()" value="后退">
					&nbsp; &nbsp; &nbsp;
					<input type="button" class="button" onClick="on_submit()" value="应用">
					<input type="hidden" name="submit-url" value="">
				</form>
			</DIV>
		</blockquote>
	</body>
<%addHttpNoCache();%>
</html>
