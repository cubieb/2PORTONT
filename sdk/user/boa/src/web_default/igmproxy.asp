<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang(LANG_IGMP_PROXY); %><% multilang(LANG_CONFIGURATION); %></title>
<SCRIPT>
function proxySelection()
{
	if (document.igmp.proxy[0].checked) {
		document.igmp.proxy_if.disabled = true;
	}
	else {
		document.igmp.proxy_if.disabled = false;
	}
}
</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF"><% multilang(LANG_IGMP_PROXY); %><% multilang(LANG_CONFIGURATION); %></font></h2>

<form action=/boaform/formIgmproxy method=POST name="igmp">
<table border=0 width="500" cellspacing=0 cellpadding=0>
  <tr><td><font size=2>
    <% multilang(LANG_IGMP_PROXY_ENABLES_THE_SYSTEM_TO_ISSUE_IGMP_HOST_MESSAGES_ON_BEHALF_OF_HOSTS_THAT_THE_SYSTEM_DISCOVERED_THROUGH_STANDARD_IGMP_INTERFACES_THE_SYSTEM_ACTS_AS_A_PROXY_FOR_ITS_HOSTS_WHEN_YOU_ENABLE_IT_BY_DOING_THE_FOLLOWS); %>:
    <br>. <% multilang(LANG_ENABLE_IGMP_PROXY_ON_WAN_INTERFACE_UPSTREAM_WHICH_CONNECTS_TO_A_ROUTER_RUNNING_IGMP); %>
    <br>. <% multilang(LANG_ENABLE_IGMP_ON_LAN_INTERFACE_DOWNSTREAM_WHICH_CONNECTS_TO_ITS_HOSTS); %>
  </font></td></tr>
  <tr><td><hr size=1 noshade align=top></td></tr>
</table>

  <tr>
      <td width><font size=2><b><% multilang(LANG_IGMP_PROXY); %>:</b></td>
      <td width><font size=2>
      	<input type="radio" value="0" name="proxy" <% checkWrite("igmpProxy0"); %> onClick="proxySelection()"><% multilang(LANG_DISABLE); %>&nbsp;&nbsp;
     	<input type="radio" value="1" name="proxy" <% checkWrite("igmpProxy1"); %> onClick="proxySelection()"><% multilang(LANG_ENABLE); %>
      </td>
  </tr>
  <tr>
      <td><font size=2><b><% multilang(LANG_PROXY_INTERFACE); %>:</b></td>
      <td>
      	<select name="proxy_if" <% checkWrite("igmpProxy0d"); %>>
          <% if_wan_list("rt"); %>
      	</select>
      </td>
      <td><input type="submit" value="<% multilang(LANG_APPLY_CHANGES); %>" name="save">&nbsp;&nbsp;</td>
  </tr>
</table>
      <input type="hidden" value="/igmproxy.asp" name="submit-url">
<script>
	ifIdx = <% getInfo("igmp-proxy-itf"); %>;
	if (ifIdx != 255)
		document.igmp.proxy_if.value = ifIdx;
	else
		document.igmp.proxy_if.selectedIndex = 0;
	
</script>
</form>
</blockquote>
</body>

</html>
