<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>广域网访问设置</TITLE>
<META http-equiv=pragma content=no-cache>
<META http-equiv=cache-control content="no-cache, must-revalidate">
<META http-equiv=content-type content="text/html; charset=gbk">
<META http-equiv=content-script-type content=text/javascript>
<!--系统公共css-->
<STYLE type=text/css>
@import url(/style/default.css);
</STYLE>
<!--系统公共脚本-->
<SCRIPT language="javascript" src="common.js"></SCRIPT>
<SCRIPT language="javascript" type="text/javascript">

/********************************************************************
**          on document load
********************************************************************/
var cgi = new Object();
<%initPageDos();%>

function on_init()
{
	sji_docinit(document, cgi);
}

</SCRIPT>
</HEAD>

<!-------------------------------------------------------------------------------------->
<!--主页代码-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
	<blockquote>
		<DIV align="left" style="padding-left:20px; padding-top:5px">
			<form id="form" action=/boaform/admin/formDos method=POST name="form">
				<b>&nbsp;DoS 保护</b><br><br>
				&nbsp;如果想设定 DOS 保护，您必须设定此项目。<br>
				<hr align="left" class="sep" size="1" width="90%">
				<table width="341" border="0" cellspacing="1" cellpadding="3">
					<tr>
						<td width="93" height="27">
							<input type="radio" name="dosEnble" value="off">禁用
						</td>
						<td width="140">
							<input type="radio" name="dosEnble" value="on">启用
						</td>
					</tr>
				</table>
				<hr align="left" class="sep" size="1" width="90%">
				<input type="submit" class="button" value="确 定" name="apply">
				<input type="hidden" name="submit-url" value="">
			</form>
		</div>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>
