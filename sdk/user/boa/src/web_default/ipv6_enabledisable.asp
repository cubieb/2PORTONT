<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<HEAD>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title>IPv6 Enable/Disable</title>
<title><% multilang(LANG_IPV6_E); %> <% multilang(LANG_CONFIGURATION); %></title>
<script type="text/javascript" src="share.js">
</script>
</HEAD>

<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000">
  <blockquote>
	<h2><font color="#0000FF"><% multilang(LANG_IPV6); %><% multilang(LANG_CONFIGURATION); %></font></h2>
	<DIV align="left" style="padding-left:20px; padding-top:5px">
		<form id="form" action=/boaform/admin/formIPv6EnableDisable method=POST name="ipv6enabledisable">			
			<table border=0 width="500" cellspacing=4 cellpadding=0>
			<tr><td><font size=2><% multilang(LANG_THIS_PAGE_BE_USED_TO_CONFIGURE_IPV6_ENABLE_DISABLE); %></font></td></tr>
			<tr><td><hr size=1 noshade align=top></td></tr>
			</table>
			
			<table border=0 width="500" cellspacing=4 cellpadding=0>
			  <tr>
			  	<td><font size=2><b><% multilang(LANG_IPV6); %>:</b></td>
				<td><font size=2>
	      		<input type="radio" value="0" name="ipv6switch" <% checkWrite("ipv6enabledisable0"); %> ><% multilang(LANG_DISABLE); %>&nbsp;&nbsp;
	      		<input type="radio" value="1" name="ipv6switch" <% checkWrite("ipv6enabledisable1"); %> ><% multilang(LANG_ENABLE); %></td>
			  </tr>
			  <tr>
			</table>
			<br><br>
			<input type="submit" class="button" value="<% multilang(LANG_APPLY_CHANGES); %>" name="save">
			<input type="hidden" value="/ipv6_enabledisable.asp" name="submit-url">
		</form>
	</DIV>
  </blockquote>

</body>
</html>
