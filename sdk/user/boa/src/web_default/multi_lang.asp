<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang(LANG_MULTI_LANGUAL_SETTINGS); %></title>
</head>

<body>
<blockquote>
<h2><font color="#0000FF"><% multilang(LANG_MULTI_LANGUAL_SETTINGS); %></font></h2>

<form id="multilangform"action=/boaform/langSel method=POST name="mlSet">
<table border=0 width="1000" cellspacing=4 cellpadding=0>
  <tr><td><font size=2>
    <% multilang(LANG_PAGE_DESC_MULTI_LANGUAL); %>
  </font></td></tr>

  <tr><td width="26%"><font size=2><b><% multilang(LANG_LANGUAGE_SELECT); %>:</b></td>
      <td width="50%"><font size=2><select size="1" name="selinit"><% checkWrite("selinit"); %></select></td>
  </tr>

  <tr><td><hr size=1 noshade align=top></td></tr>
</table>
<table border=0 width="1000" cellspacing=4 cellpadding=0></table>
  <br>
  <input type="submit" value="<% multilang(LANG_UPDATE_SELECTED_LANGUAGE); %>" onclick="parent.location.reload();">
</form>
</blockquote>
</body>

</html>
