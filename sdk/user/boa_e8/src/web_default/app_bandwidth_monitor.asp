<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>上下行带宽监测配置</TITLE>
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

</SCRIPT>
</HEAD>

<!-------------------------------------------------------------------------------------->
<!--主页代码-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000">
	<blockquote>
		<DIV align="left" style="padding-left:20px; padding-top:5px">
			<form id="form" action=/boaform/formBandwidthMonitor method=POST name="bandwidthmonitor">
				<b>实时带宽监测功能</b><br><br>
				这个页面允许你启用或者禁用下挂设备上下行带宽实时监测功能<br><br>
			
				<hr align="left" class="sep" size="1" width="90%">				
				<table border=0 width="500" cellspacing=4 cellpadding=0>
					<tr><td width=150>上下行带宽监测:</td>
						<td width=350>
						<input type="radio" name=monitor value=0>禁用&nbsp;&nbsp;
						<input type="radio" name=monitor value=1>启用</td>
				</tr></table>
				<br>
				<input type="submit" class="button" name="apply" value="保存/应用"> 
				<input type="hidden" name="submit-url" value="/app_bandwidth_monitor.asp">
			<script>
				<% initPage("bandwidth_monitor"); %>	
			</script>			
			</form>
		</DIV>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>
