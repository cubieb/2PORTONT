<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<HEAD>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title>MLD Proxy</title>
<title><% multilang("25" "LANG_MLD_PROXY"); %><% multilang("212" "LANG_CONFIGURATION"); %></title>
<script type="text/javascript" src="share.js">
</script>
<SCRIPT>

function proxySelection()
{
 if(document.mldproxy.daemon[0].checked)
 {
  document.mldproxy.ext_if.disabled = true;
 }
 else
 {
  document.mldproxy.ext_if.disabled = false;
 }
}
</SCRIPT>
</HEAD>

<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000">
  <blockquote>
 <h2><font color="#0000FF"><% multilang("25" "LANG_MLD_PROXY"); %><% multilang("212" "LANG_CONFIGURATION"); %></font></h2>
 <DIV align="left" style="padding-left:20px; padding-top:5px">
  <form id="form" action=/boaform/admin/formMLDProxy method=POST name="mldproxy">
   <table border=0 width="500" cellspacing=4 cellpadding=0>
   <tr><td><font size=2><% multilang("367" "LANG_THIS_PAGE_BE_USED_TO_CONFIGURE_MLD_PROXY"); %></font></td></tr>
   <tr><td><hr size=1 noshade align=top></td></tr>
   </table>

   <table border=0 width="500" cellspacing=4 cellpadding=0>
     <tr>
      <td><font size=2><b><% multilang("25" "LANG_MLD_PROXY"); %>:</b></td>
    <td><font size=2>
         <input type="radio" value="0" name="daemon" <% checkWrite("mldproxy0"); %> onClick="proxySelection()"><% multilang("221" "LANG_DISABLE"); %>&nbsp;&nbsp;
         <input type="radio" value="1" name="daemon" <% checkWrite("mldproxy1"); %> onClick="proxySelection()"><% multilang("222" "LANG_ENABLE"); %></td>
     </tr>
     <tr>
      <td><font size=2><b><% multilang("369" "LANG_WAN_INTERFACE"); %>:</b>&nbsp;</td>
        <td><font size=2><select name="ext_if" <% checkWrite("mldproxy0d"); %>> <% if_wan_list("rtv6"); %> </select> </td>
   </table>
   <br><br>
   <input type="submit" class="button" value="<% multilang("134" "LANG_APPLY_CHANGES"); %>" name="save">
   <input type="hidden" value="/app_mldProxy.asp" name="submit-url">
  </form>
 </DIV>
  </blockquote>

<script>
 initUpnpDisable = document.mldproxy.daemon[0].checked;
 ifIdx = <% getInfo("mldproxy-ext-itf"); %>;

 document.mldproxy.ext_if.selectedIndex = -1;

 for( i = 0; i < document.mldproxy.ext_if.options.length; i++ )
 {
  if( ifIdx == document.mldproxy.ext_if.options[i].value )
   document.mldproxy.ext_if.selectedIndex = i;
 }

 proxySelection();
</script>
</body>
</html>
