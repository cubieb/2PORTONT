<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<HEAD>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang("26" "LANG_MLD_SNOOPING"); %><% multilang("212" "LANG_CONFIGURATION"); %></title>
<script type="text/javascript" src="share.js">
</script>
</HEAD>

<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000">
 <blockquote>
  <h2><font color="#0000FF"><% multilang("26" "LANG_MLD_SNOOPING"); %><% multilang("212" "LANG_CONFIGURATION"); %></font></h2>
  <DIV align="left" style="padding-left:20px; padding-top:5px">
   <form id="form" action=/boaform/formMLDSnooping method=POST name="mldsnoop">
    <table border=0 width="500" cellspacing=4 cellpadding=0>
    <tr><td><font size=2><% multilang("370" "LANG_THIS_PAGE_BE_USED_TO_CONFIGURE_MLD_SNOOPING"); %></font></td></tr>
    <tr><td><hr size=1 noshade align=top></td></tr>
    </table>

    <table border=0 width="500" cellspacing=4 cellpadding=0>
     <tr><td width=150><font size=2><b><% multilang("26" "LANG_MLD_SNOOPING"); %>:</b></td>
      <td width=350><font size=2>
      <input type="radio" name=snoop value=0><% multilang("221" "LANG_DISABLE"); %>&nbsp;&nbsp;
      <input type="radio" name=snoop value=1><% multilang("222" "LANG_ENABLE"); %></td>
    </tr></table>
    <br>
    <input type="submit" class="button" name="apply" value="<% multilang("134" "LANG_APPLY_CHANGES"); %>">
    <input type="hidden" name="submit-url" value="/app_mld_snooping.asp">
   <script>
    <% initPage("mldsnooping"); %>
   </script>
   </form>
  </DIV>
 </blockquote>
</body>
</html>
