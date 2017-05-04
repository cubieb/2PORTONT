<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title>ADSL <% multilang(LANG_PSD_MASK); %><% multilang(LANG_CONFIGURATION); %></title>

</head>

<body>
<blockquote>
<h2><font color="#0000FF">ADSL <% multilang(LANG_PSD_MASK); %><% multilang(LANG_CONFIGURATION); %></font></h2>

<table border=0 width="480" cellspacing=0 cellpadding=0>
  <tr><font size=2>
  <% multilang(LANG_THIS_PAGE_LET_USER_TO_SET_PSD_MASK); %>
  </tr>
  <tr><hr size=1 noshade align=top></tr>
</table>


<form action=/boaform/formSetAdslPSD method=POST name="formPSDTbl">

<table border=0 width=400 cellspacing=4 cellpadding=0>
<% adslPSDMaskTbl(); %>
</table>

<input type="submit" value="<% multilang(LANG_APPLY_CHANGES); %>" name="apply">&nbsp;&nbsp;
<input type="hidden" value="/adslpsd.asp" name="submit-url"> 
<input type="button" value="<% multilang(LANG_CLOSE); %>" name="close" onClick="javascript: window.close();">

</form>
</blockquote>
</body>

</html>

