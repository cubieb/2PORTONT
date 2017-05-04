<html>
<head>
<meta http-equiv="Content-Type" content="text/html">
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Cache-Control" CONTENT="no-cache">
<title>SIP</title>
<script language="javascript" src=voip_script.js></script>
</head>
<body bgcolor="#ffffff" text="#000000">

<form method="get" action="/boaform/voip_ring_set" name=ringform>

<%voip_general_get("not_ipphone_option_start");%>
<b><% multilang("967" "LANG_RING_CADENCE_DETECTION_SETTING"); %></b>
<%voip_general_get("not_ipphone_option_end");%>
<table cellSpacing=1 cellPadding=2 border=0 width=325 <%voip_general_get("not_ipphone_table");%> >

 <tr>
     <td bgColor=#aaddff width=155><% multilang("968" "LANG_CADENCE"); %></td>
     <td bgColor=#ddeeff width=170>
   <select name=ring_cad>
     <%voip_ring_get("ring_cad");%>
   </select>
  </td>
 </tr>

 <tr>
     <td colspan=3 align=center>
      <input type="submit" name="Ring_Cad" value="<% multilang("279" "LANG_APPLY"); %>" >
    </td>
 </tr>

</table>

<!--
<b>Select Group</b>
<table cellSpacing=1 cellPadding=2 border=0 width=325>


 <tr>
     <td bgColor=#aaddff width=155>Group</td>
     <td bgColor=#ddeeff width=170>
   <select name=group onChange="ringform.submit()">
     <%voip_ring_get("group");%>
   </select>
  </td>
 </tr>


 <tr>
     <td bgColor=#aaddff>Phone Number</td>
     <td bgColor=#ddeeff>
  <input type=text name=phonenumber size=20 maxlength=39 value="<%voip_ring_get("phonenumber");%>"></td>
 </tr>
 <tr>


   <tr>
     <td bgColor=#aaddff>Cadence</td>
  <td bgColor=#ddeeff>
  <select name=cadence_use>
    "<%voip_ring_get("cadence_use");%>"
  </select>
  </td>
 </tr>



 <tr>
     <td colspan=3 align=center>
      <input type="submit" name="Ring_Group" value="Apply" >
    </td>
 </tr>


</table>
-->

<%voip_general_get("not_ipphone_option_start");%>
<b><% multilang("969" "LANG_SELECT_CADENCE"); %></b>
<%voip_general_get("not_ipphone_option_end");%>
<table cellSpacing=1 cellPadding=2 border=0 width=325 <%voip_general_get("not_ipphone_table");%> >
 <tr>
     <td bgColor=#aaddff width=155><% multilang("968" "LANG_CADENCE"); %></td>
     <td bgColor=#ddeeff width=170>
   <select name=cadence_sel onChange="ringform.submit()">
     <%voip_ring_get("cadence_sel");%>
   </select>
  </td>
 </tr>
</table>

<%voip_general_get("not_ipphone_option_start");%>
<b><% multilang("970" "LANG_CUSTOM_CADENCE"); %></b>
<%voip_general_get("not_ipphone_option_end");%>
<table cellSpacing=1 cellPadding=2 border=0 width=325 <%voip_general_get("not_ipphone_table");%> >

 <tr>
     <td bgColor=#aaddff><% multilang("971" "LANG_CADENCE_ON"); %> (<% multilang("861" "LANG_MSEC"); %>)</td>
     <td bgColor=#ddeeff>
  <input type=text name=cad_on size=20 maxlength=39 value="<%voip_ring_get("cad_on");%>"></td>
 </tr>
 <tr>

 <tr>
     <td bgColor=#aaddff><% multilang("972" "LANG_CADENCE_OFF"); %> (<% multilang("861" "LANG_MSEC"); %>)</td>
     <td bgColor=#ddeeff>
  <input type=text name=cad_off size=20 maxlength=39 value="<%voip_ring_get("cad_off");%>"></td>
 </tr>
 <tr>

 <tr>
     <td colspan=3 align=center>
      <input type="submit" name="Ring_Cadence" value="<% multilang("279" "LANG_APPLY"); %>" >
    </td>
 </tr>


</table>

</form>
</body>
</html>
