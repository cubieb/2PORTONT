<html>
<head>
<meta http-equiv="Content-Type" content="text/html">
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Cache-Control" CONTENT="no-cache">
<title>SIP</title>
<script language="javascript" src=voip_script.js></script>
<script language="javascript">
<!--
function enableAutoBypass()
{
 document.other_form.auto_bypass_warning.disabled =
  !document.other_form.auto_bypass_relay.checked;
}
function InitOther()
{
 enableAutoBypass();
}
-->
</script>
</head>
<body bgcolor="#ffffff" text="#000000" onload="InitOther()">
<form method="get" action="/boaform/voip_other_set" name=other_form>

<%voip_general_get("not_ipphone_option_start");%>
<b><% multilang("973" "LANG_FUNCTION_KEY"); %></b>
<%voip_general_get("not_ipphone_option_end");%>
<table cellSpacing=1 cellPadding=2 border=0 width=450 <%voip_general_get("not_ipphone_table");%> >
<tr bgColor=#888888>
 <td colspan=2>
 <font color=#ffffff>
 <% multilang("974" "LANG_MUST_BE"); %> * + 0~9
 </font>
 </td>
</tr>
<tr <%voip_other_get("display_funckey_pstn");%>>
    <td bgColor=#aaddff width=150><% multilang("975" "LANG_SWITCH_TO_PSTN"); %></td>
 <td bgColor=#ddeeff>
  <input type=text name=funckey_pstn size=5 maxlength=2 value="<%voip_other_get("funckey_pstn");%>">
  ( default: *0 )
 </td>
</tr>
<tr>
    <td bgColor=#aaddff><% multilang("976" "LANG_CALL_TRANSFER"); %></td>
 <td bgColor=#ddeeff>
  <input type=text name=funckey_transfer size=5 maxlength=2 value="<%voip_other_get("funckey_transfer");%>">
  ( <% multilang("977" "LANG_DEFAULT"); %>: *1 )
 </td>
</tr>
</table>

<%voip_other_get("auto_dial_display_title");%>
<table cellSpacing=1 cellPadding=2 border=0 width=450 <%voip_other_get("auto_dial_display");%>>
<tr>
    <td bgColor=#aaddff width=150><% multilang("979" "LANG_AUTO_DIAL_TIME"); %></td>
 <td bgColor=#ddeeff>
  <input type=text name=auto_dial size=3 maxlength=1 value="<%voip_other_get("auto_dial");%>">
  ( 3~9 <% multilang("511" "LANG_SEC"); %>, <% multilang("981" "LANG_0_IS_DISABLE"); %> )
 </td>
</tr>
<tr>
    <td bgColor=#aaddff width=150><% multilang("980" "LANG_DIAL_OUT_BY_HASH_KEY"); %></td>
 <td bgColor=#ddeeff>
  <input type=checkbox name=auto_dial_always <%voip_other_get("auto_dial_always");%>><% multilang("161" "LANG_DISABLED"); %>
 </td>
</tr>
</table>

<p <%voip_other_get("display_funckey_pstn");%>>
<b><% multilang("982" "LANG_PSTN_RELAY"); %><b>
<table cellSpacing=1 cellPadding=2 border=0 width=450>
<tr>
    <td bgColor=#aaddff width=150><% multilang("983" "LANG_AUTO_BYPASS_RELAY"); %></td>
 <td bgColor=#ddeeff>
  <input type=checkbox name=auto_bypass_relay onclick=enableAutoBypass() <%voip_other_get("auto_bypass_relay");%>><% multilang("222" "LANG_ENABLE");%>
 </td>
</tr>
<tr>
    <td bgColor=#aaddff width=150><% multilang("984" "LANG_WARNING_TONE");%></td>
 <td bgColor=#ddeeff>
  <input type=checkbox name=auto_bypass_warning <%voip_other_get("auto_bypass_warning");%>><% multilang("222" "LANG_ENABLE");%>
 </td>
</tr>
</table>
</p>

<%voip_other_get("off_hook_alarm_display_title");%>
<table cellSpacing=1 cellPadding=2 border=0 width=450 <%voip_other_get("off_hook_alarm_display");%>>
<tr>
    <td bgColor=#aaddff width=150><% multilang("986" "LANG_OFF_HOOK_ALARM_TIME");%></td>
 <td bgColor=#ddeeff>
  <input type=text name=off_hook_alarm size=3 maxlength=2 value="<%voip_other_get("off_hook_alarm");%>">
  ( 10~60 <% multilang("511" "LANG_SEC");%>, <% multilang("981" "LANG_0_IS_DISABLE");%> )
 </td>
</tr>
</table>

<p <%voip_other_get("display_cid_det");%>>
<b><% multilang("987" "LANG_VOIP_TO_PSTN");%></b>
<table cellSpacing=1 cellPadding=2 border=0 width=450>
<tr bgColor=#888888>
 <td bgColor=#aaddff width=150><% multilang("988" "LANG_ONE_STAGE_DIALING");%></td>
 <td bgColor=#ddeeff>
  <input type=checkbox name=one_stage_dial <%voip_other_get("one_stage_dial");%>>Enable
 </td>
</tr>
<tr bgColor=#888888>
    <td bgColor=#aaddff width=150><% multilang("989" "LANG_TWO_STAGE_DIALING");%></td>
 <td bgColor=#ddeeff>
  <input type=checkbox name=two_stage_dial <%voip_other_get("two_stage_dial");%>>Enable
 </td>
</tr>
</table>
</p>

<p <%voip_other_get("display_cid_det");%>>
<b><% multilang("990" "LANG_PSTN_TO_VOIP");%></b>
<table cellSpacing=1 cellPadding=2 border=0 width=450>
<tr bgColor=#888888>
 <td bgColor=#aaddff width=150><% multilang("991" "LANG_CALLER_ID_AUTO_DETECTION");%></td>
 <td bgColor=#ddeeff><%voip_other_get("caller_id_auto_det");%></td>
 <input type=hidden name=caller_id_test value=hello>
</tr>
<tr bgColor=#888888>
 <td bgColor=#aaddff width=150><% multilang("992" "LANG_CALLER_ID_DETECTION_MODE");%></td>
 <td bgColor=#ddeeff><%voip_other_get("caller_id_det");%></td>
</tr>
</table>
</p>

<%voip_general_get("not_ipphone_option_start");%>
<p>
<b><% multilang("993" "LANG_FXS_PULSE_DIAL_DETECTION");%></b>
<%voip_general_get("not_ipphone_option_end");%>
<table cellSpacing=1 cellPadding=2 border=0 width=450 <%voip_general_get("not_ipphone_table");%> >
 <tr>
 <td bgColor=#aaddff>
  <%voip_other_get("pulse_dial_detection");%>
 </td>
 </tr>

 <tr>
    <td bgColor=#aaddff width=150><% multilang("994" "LANG_INTERDIGIT_PAUSE_DURATION");%></td>
 <td bgColor=#ddeeff>
  <input type=text name=pulse_det_Pause size=5 maxlength=4 value="<%voip_other_get("pulse_det_Pause");%>">
  (<% multilang("861" "LANG_MSEC");%>)
 </td>
 </tr>

</table>
</p>


<p <%voip_other_get("display_pulse_dial_gen");%>>
<b><% multilang("995" "LANG_FXO_PULSE_DIAL_GENERATION");%></b>
<table cellSpacing=1 cellPadding=2 border=0 width=450>

 <tr>
 <td bgColor=#aaddff>
  <%voip_other_get("pulse_dial_generation");%>
 </td>
 </tr>

 <tr>
 <td bgColor=#aaddff width=150><% multilang("996" "LANG_PULSE_PER_SECOND");%></td>
 <td bgColor=#ddeeff>
  <select name=pulse_gen_PPS>
  <%voip_other_get("pulse_gen_PPS");%>
  </select>
 </td>
 </tr>

    <tr>
     <td bgColor=#aaddff rowspan=2><% multilang("1006" "LANG_MAKE_DURATION");%></td>
     <td bgColor=#ddeeff>
  <input type=text name=pulse_gen_Make size=5 maxlength=2 value="<%voip_other_get("pulse_gen_Make");%>">
  (<% multilang("861" "LANG_MSEC");%>)
 </td>

 </tr>

 <tr>
  <td bgColor=#ddeeff><font size=2><% multilang("997" "LANG_1_99_MSEC_FOR_10PPS_1_49_MSEC_FOR_20PPS");%></font>
  </td>
 </tr>


 <tr>
    <td bgColor=#aaddff width=150><% multilang("994" "LANG_INTERDIGIT_PAUSE_DURATION");%></td>
 <td bgColor=#ddeeff>
  <input type=text name=pulse_gen_Pause size=5 maxlength=4 value="<%voip_other_get("pulse_gen_Pause");%>">
  (<% multilang("861" "LANG_MSEC");%>)
 </td>
 </tr>

</table>
</p>
<p>
<b><% multilang("998" "LANG_SIP_SETTING");%></b>

<table cellSpacing=1 cellPadding=2 border=0 width=450 >
<tr>
    <td bgColor=#aaddff width=150><% multilang("999" "LANG_SIP_PRACK");%></td>
 <td bgColor=#ddeeff>
  <input type=checkbox name=sip_disable_prack <%voip_other_get("sip_disable_prack");%>><% multilang("161" "LANG_DISABLED");%>
 </td>
</tr>

<tr>
    <td bgColor=#aaddff width=150><% multilang("1000" "LANG_SIP_SERVER_RENDUNDACY");%></td>
 <td bgColor=#ddeeff>
  <input type=checkbox name=sip_enable_redundancy <%voip_other_get("sip_enable_redundancy");%>><% multilang("160" "LANG_ENABLED");%>
 </td>
</tr>

<tr>
    <td bgColor=#aaddff width=150><% multilang("1001" "LANG_SIP_CLIR_ANONYMOUSE_FROM_HEADER");%></td>
 <td bgColor=#ddeeff>
  <input type=checkbox name=sip_anonymouse_from <%voip_other_get("sip_anonymouse_from");%>><% multilang("160" "LANG_ENABLED");%>
 </td>
</tr>
<tr>
    <td bgColor=#aaddff width=150><% multilang("1002" "LANG_NON_SIP_INBOX_CALL");%></td>
 <td bgColor=#ddeeff>
  <input type=checkbox name=sip_internalcall <%voip_other_get("sip_internalcall");%>><% multilang("160" "LANG_ENABLED");%>
 </td>
</tr>
<tr>
    <td bgColor=#aaddff width=150><% multilang("1003" "LANG_HOOK_FLASH_RELAY_SETTING");%>:</td>
   <td bgColor=#ddeeff>
   <select name=hookflash_type>
     <%voip_other_get("hookflash_type");%>
   </select>

  </td>
</tr>
</table>
<p>


<b><% multilang("1004" "LANG_SIP_OPTIONS");%></b>

<table cellSpacing=1 cellPadding=2 border=0 width=450>
 <tr>
 <td bgColor=#aaddff>
  <%voip_other_get("sip_option_flag");%>
 </td>
 </tr>

 <tr>
    <td bgColor=#aaddff width=150><% multilang("1005" "LANG_OPTIONS_INTERVAL_TIME");%></td>
 <td bgColor=#ddeeff>
  <input type=text name=sip_option_HeartbeatCycle size=5 maxlength=4 value="<%voip_other_get("sip_option_HeartbeatCycle");%>">
  (<% multilang("511" "LANG_SEC");%>)
 </td>
 </tr>

</table>
</p>

<%voip_general_get("not_ipphone_option_start");%>
<table cellSpacing=1 cellPadding=2 border=0 width=375>
<tr>
    <td colspan=3 align=center>
     <input type="submit" value="<% multilang("279" "LANG_APPLY");%>" onclick="return check_other()">
     &nbsp;&nbsp;&nbsp;&nbsp;
     <input type="reset" value="<% multilang("196" "LANG_RESET");%>">
    </td>
</tr>
</table>
<%voip_general_get("not_ipphone_option_end");%>



</form>
</body>
</html>
