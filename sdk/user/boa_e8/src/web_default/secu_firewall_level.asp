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
<%initPageFirewall();%>

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
			<form action=/boaform/admin/formFirewall method=POST name="form">
				<b>选择<font color="red">防火墙等级</font>，进行相应的设置.</b><br><br>
				<hr align="left" class="sep" size="1" width="90%">
				<table width="538px" border="0" cellspacing="1" cellpadding="3">
					<tr>
						<td width="162" height="27">防火墙等级:</td>
						<td width="73">
							<select name="filterLevel">
								<option value="1">低</option>
								<!--<option value="2">中</option>-->
								<option value="3">高</option>
							</select>
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
