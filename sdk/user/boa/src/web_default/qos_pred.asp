<html>
<! Copyright (c) Realtek Semiconductor Corp., 2008. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang(LANG_IP_PRECEDENCE_PRIORITY_SETTINGS); %></title>
<script type="text/javascript" src="share.js">
</script>

</head>
<BODY>
<blockquote>
<h2><font color="#0000FF"><% multilang(LANG_IP_PRECEDENCE_PRIORITY_SETTINGS); %></font></h2>

<table border=0 width="500" cellspacing=4 cellpadding=0>
<tr><td><font size=2>
<% multilang(LANG_THIS_PAGE_IS_USED_TO_CONFIG_IP_PRECEDENCE_PRIORITY); %>
</font></td></tr>
<tr><td><hr size=1 noshade align=top></td></tr>
</table>
<form action=/boaform/formIPQoS method=POST name=qos_set1p>
<table border="0" width=500>
<tr><font size=2><% multilang(LANG_IP_PRECEDENCE_RULE); %>:</font></tr>
<% settingpred(); %>
</table>	
<input type="hidden" value="/qos_pred.asp" name="submit-url">
<td><input type="submit" value="<% multilang(LANG_MODIFY); %>" name=setpred ></td>
<input type="button" value="<% multilang(LANG_CLOSE); %>" name="close" onClick="javascript: window.close();"></p>
</form>
</blockquote>
</body>
</html>

