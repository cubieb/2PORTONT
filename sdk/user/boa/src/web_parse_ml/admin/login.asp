<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<script src="md5.js" type="text/javascript"></script>
<SCRIPT>
function setpass()
{
 <% passwd2xmit(); %>
}
function mlhandle()
{
 document.formML.submit();
 parent.location.reload();
}
</SCRIPT>
</head>

<body>
<blockquote>

<form action=/boaform/admin/formLogin method=POST name="cmlogin">
<input type="hidden" name="challenge">

<TABLE cellSpacing=0 cellPadding=0 width="100%" border=0>
  <TBODY>
  <TR vAlign=top>
    <TD width="50%"><A><IMG height=60 src="LoginFiles/realtek.jpg" width=170 useMap=#n1E6.$Body.0.1E8 border=0></A></TD>
  </TR>
  </TBODY>
</TABLE>

<CENTER>
  <TABLE cellSpacing=0 cellPadding=0 border=0>
    <TBODY>
      <TR vAlign=top>
        <TD width=350><BR>
          <TABLE cellSpacing=0 cellPadding=0 width="100%" border=0>
            <TBODY>
              <TR vAlign=top>
                <TD vAlign=center width="29%"><DIV align=right><IMG height=32 src="LoginFiles/locker.gif" width=32><BR><BR></DIV></TD>
                <TD vAlign=center width="5%"></TD>
                <TD vAlign=center width="71%"><FONT color=#0000FF size=2><% multilang("729" "LANG_INPUT_USERNAME_AND_PASSWORD"); %></FONT><BR><BR></TD>
       </TR>
              <TR vAlign=top>
                <TD vAlign=center width="29%"><DIV align=right><FONT color=#0000FF size=2><% multilang("751" "LANG_USER"); %><% multilang("619" "LANG_NAME"); %>:</FONT></DIV></TD>
                <TD vAlign=center width="5%"></TD>
                <TD vAlign=center width="71%"><FONT><INPUT maxLength=30 size=20 name=username></FONT></TD>
              </TR>
              <TR vAlign=top>
                <TD vAlign=center width="29%"><DIV align=right><FONT color=#0000FF size=2><% multilang("63" "LANG_PASSWORD"); %>:</FONT></DIV></TD>
                <TD vAlign=center width="5%"></TD>
                <TD vAlign=center width="71%"><FONT><INPUT type=password maxLength=30 size=20 name=password></FONT></TD>
       </TR>

              <TR vAlign=top>
                <TD vAlign=center width="29%"></TD>
                <TD vAlign=center width="5%"></TD>
                <TD vAlign=center width="71%"><FONT size=2></FONT><BR><INPUT type=submit value="<% multilang("730" "LANG_LOGIN"); %>" name=save onClick=setpass()></TD>
       </TR>
            </TBODY>
   </TABLE>
        </TD>
      </TR>
    </TBODY>
  </TABLE>
</CENTER>
<DIV align=center></DIV>
<input type="hidden" value="/admin/login.asp" name="submit-url">
</form>
</blockquote>

<blockquote>
<form action=/boaform/admin/formLoginMultilang method=POST name="formML">
<CENTER><TABLE cellSpacing=0 cellPadding=0 border=0>
<tr><td>
 <% checkWrite("loginSelinit"); %>
</td></tr>
</TABLE></CENTER>
</form>
</blockquote>

</BODY>
</HTML>
