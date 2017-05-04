<html>
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang(LANG_PACKET_DUMP); %></title>
</head>

<body>
<blockquote>
<h2><font color="#0000FF"><% multilang(LANG_PACKET_DUMP); %></font></h2>

<form action=/boaform/formCapture method=POST name="ping">
<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><td><font size=2>
	<% multilang(LANG_THIS_PAGE_IS_USED_TO_START_OR_STOP_A_WIRESHARK_PACKET_CAPTURE); %><br>
    <% multilang(LANG_YOU_NEED_TO_RETURN_TO_THIS_PAGE_TO_STOP_IT); %><br>
	<a href ="http://www.tcpdump.org/tcpdump_man.html" target=_blank"><% multilang(LANG_CLICK_HERE_FOR_THE_DOCUMENTATION_OF_THE_ADDITIONAL_ARGUMENTS); %></a>
  </font></td></tr>
  <tr><td><hr size=1 noshade align=top></td></tr>
</table>

  <tr>
      <td width="30%"><font size=2><b><% multilang(LANG_ADDITIONAL_ARGUMENTS); %>:</b></td>
      <td width="70%"><input type="text" name="tcpdumpArgs" value="-s 1500" size="50" maxlength="50"></td>
      <input type="hidden" value="yes" name="dostart">
  </tr>

</table>

  <br>
      <input type="submit" value="<% multilang(LANG_START); %>" name="start">
      <input type="hidden" value="/pdump.asp" name="submit-url">
 </form>
<p>

<form action=/boaform/formCapture method=POST name="ping">
      <input type="submit" value="<% multilang(LANG_STOP); %>" name="stop">
      <input type="hidden" value="/pdump.asp" name="submit-url">
      <input type="hidden" value="no" name="dostart">
 </form>


</blockquote>
</body>

</html>
