
<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang(LANG_FINISH_MAINTENANCE); %></title>

<SCRIPT>
function confirmfinsih()
{
   if ( !confirm('do you confirm the maintenance is over?') ) {
	return false;
  }
  else
	return true;
}
</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF"><% multilang(LANG_FINISH_MAINTENANCE); %></font></h2>

<form action=/boaform/formFinishMaintenance method=POST name="cmfinish">
<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><td><font size=2>
    <% multilang(LANG_THIS_PAGE_IS_USED_TO_INFORM_ITMS_THAT_MAINTENANCE_IS_FINISHED_AND_THEN_ITMS_MAY_CHANGE_THIS_GATEWAY_S_PASSWORD); %>
  </font></td></tr>
  <tr><td><hr size=1 noshade align=top></td></tr>
</table>
  <br>
      <input type="submit" value="<% multilang(LANG_FINISH_MAINTENANCE); %>" name="finish" onclick="return confirmfinsih()">&nbsp;&nbsp;
      <input type="hidden" value="/finishmaintenance.asp" name="submit-url">
 </form>
</blockquote>
</body>

</html>
