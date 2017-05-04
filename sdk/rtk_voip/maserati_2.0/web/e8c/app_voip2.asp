<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>日常应用</TITLE>
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
**          on document load
********************************************************************/
function on_init()
{

}

/********************************************************************
**          on document submit
********************************************************************/
function on_submit() 
{


	if( (form.preced0.value == form.preced1.value) || (form.preced0.value == form.preced2.value) || (form.preced0.value == form.preced3.value) ||
	    (form.preced1.value == form.preced2.value) || (form.preced1.value == form.preced3.value) || (form.preced2.value == form.preced3.value) )
	{
			alert("宽带电话高级设置错误，相同设定在不同优先顺位!");
			return false;
	}
	
	form.submit();
	//submit();
}
</SCRIPT>
</HEAD>

<!-------------------------------------------------------------------------------------->
<!--主页代码-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000">
	<blockquote>
		<DIV align="left" style="padding-left:20px; padding-top:5px">
			<form id="form" action=/boaform/admin/voip2_e8c_set method=POST>
				<b>SIP</b>
				<table border="0" cellpadding="0" cellspacing="0" width="500">
					<tr>
						<td width="200px">SIP 本地端口</td>
						<td width="200px"><input type=text name=sipPort size=10 maxlength=5 value="<%voip_e8c_get("sipPort"); %>"></td>
					</tr>
					<tr>
						<td>RTP初始端口</td>
						<td><input type=text name=rtpPort size=10 maxlength=5 value="<%voip_e8c_get("rtpPort"); %>"></td>
					</tr>
					<tr>
						<td>SIP DSCP</td>
						<td><select name=sipDscp>"<%voip_net_get("sipDscp");%>"</select></td>
					</tr>
					<tr>
						<td>RTP DSCP</td>
						<td><select name=rtpDscp>"<%voip_net_get("rtpDscp");%>"</select></td>
					</tr>
					<tr>
						<td>二次拨号模式</td>
						<td><select name=dtmfMode onchange="dtmfMode_change()">"<%voip_e8c_get("dtmfMode");%>"</select>
						</td>
					</tr>
				</table>
				<br>
				
				<b>宽带电话高级设置<br>
				<table border="0" cellpadding="0" cellspacing="0" width="500">
				<tr>
					<td width="200px">回声抑制设置</td>
					<td width="200px"><input type="checkbox" name="echo_cancellation" value="1" <% voip_e8c_get("useLec"); %>></td>
				</tr>
				
				<tr>
					<td>VAD</td>
					<td><input type=checkbox name=useVad size=20 <%voip_e8c_get("useVad");%>>Enable</td>
				</tr>
				<tr>
					<td>T.38</td>
					<td><input type=checkbox name=useT38 size=20 <%voip_e8c_get("useT38");%>>Enable</td>
				</tr>
				<tr>
					<td>来电显示制式</td>
					<td><%voip_e8c_get("caller_id");%></td>
				</tr>
				
				<tr>
					<td>最小最大拍叉簧时间设置（毫秒）[10的整数倍，最小：80，最大：2000]</td>
					<td><%voip_e8c_get("flash_hook_time");%></td>
				</tr>

				<tr>
					<td>摘机不拨号的时间(sec)</td>
					<td><input name="off_hook_alarm" size="16" maxlength="32" type="text" style="width:140px" value="<% voip_e8c_get("off_hook_alarm"); %>"></td>
				</tr>
				<tr>
					<td>位间短定时器时间(sec)</td>
					<td><input name="auto_dial" size="16" maxlength="32" type="text" style="width:140px" value="<% voip_e8c_get("auto_dial"); %>"></td>
				</tr>
				<tr>
					<td>位间长定时器时间(sec)</td>
					<td><input name="InterDigitTimerLong" size="16" maxlength="32" type="text" style="width:140px" value="<% voip_e8c_get("InterDigitTimerLong"); %>"></td>
				</tr>
					<td>播放忙音时间(sec)</td>
					<td><input name="BusyToneTimer" size="16" maxlength="32" type="text" style="width:140px" value="<% voip_e8c_get("BusyToneTimer"); %>"></td>
				</tr>
				<tr>
					<td>播放催挂音时间(sec)</td>
					<td><input name="HangingReminderToneTimer" size="16" maxlength="32" type="text" style="width:140px" value="<% voip_e8c_get("HangingReminderToneTimer"); %>"></td>
				</tr>

				<tr>
					<td>注册重试周期(sec)</td>
					<td><input name="RegisterRetryInterval" size="16" maxlength="32" type="text" style="width:140px" value="<% voip_e8c_get("RegisterRetryInterval"); %>"></td>
				</tr>

				<tr>
					<td>心跳时间(sec)[0:disable ]</td>
					<td><input name="HeartbeatCycle" size="16" maxlength="32" type="text" style="width:140px" value="<% voip_e8c_get("HeartbeatCycle"); %>"></td>
				</tr>
				
				<tr>
					<td>久叫不应(sec)[0:disable ]</td>
					<td><input name="NoAnswerTimer" size="16" maxlength="32" type="text" style="width:140px" value="<% voip_e8c_get("NoAnswerTimer"); %>"></td>
				</tr>
				
				<tr>
					<td>Codec</td><td>Precedence</td>
						<% voip_e8c_get("e8c_codec"); %>
				</tr>

				<tr>
			    	<td bgColor=#aaddff width=155>数图启用</td>
			    	<td bgColor=#ddeeff width=170>
			    	<%voip_e8c_get("digitmap_enable");%>
			    	</td>
				</tr>
				
			  	<tr>
			    	<td bgColor=#aaddff width=155>数图</td>

				</tr>
				
				<tr>
					<td bgColor=#aaddff><TEXTAREA COLS=50 ROWS=5 name="dialplan" maxlength=4096><%voip_e8c_get("dialplan");%></TEXTAREA></td>
				</tr>
				</table>

				<br>	



				<input type="button" class="button" name="btnDown" value="保存/应用" onClick="on_submit()">
				<input type="hidden" name="submit-url" value="">
			</form>
		</DIV>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>
