<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<html>
<head>
<title>用户管理</title>
<meta http-equiv=pragma content=no-cache>
<meta http-equiv=cache-control content="no-cache, must-revalidate">
<meta http-equiv=content-type content="text/html; charset=gbk">
<meta http-equiv=content-script-type content=text/javascript>
<!--系统公共css-->
<style type=text/css>
@import url(/style/default.css);
</style>
<!--系统公共脚本-->
<script language="javascript" src="common.js"></script>
<script language="javascript" type="text/javascript">

var cgi = new Object();
<% initPageMgmUser(); %>

/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	sji_docinit(document, cgi);
	if (cgi.issu)
		lstin.deleteRow(1);
	document.getElementById("sutip").style.display = cgi.issu ? "block" : "none";
}

/********************************************************************
**          on document submit
********************************************************************/
function on_submit()
{
	with (document.forms[0]) {
		if (!cgi.issu) {
			if (oldPasswd.value.length <= 0) {
				oldPasswd.focus();
				alert("密码不能为空，请输入密码!");
				return;
			}
			if (sji_checkusername(oldPasswd.value, 1, 16) == false) {
				oldPasswd.focus();
				alert("密码错误，请重新输入密码!");
				return;
			}
		}

		if (newPasswd.value.length <= 0) {
			newPasswd.focus();
			alert("新密码不能为空，请输入新密码!");
			return;
		}
		if (sji_checkusername(newPasswd.value, 1, 16) == false) {
			newPasswd.focus();
			alert("新密码错误，请重新输入新密码!");
			return;
		}
		if (affirmPasswd.value.length <= 0) {
			affirmPasswd.focus();
			alert("确认密码不能为空，请输入确认密码!");
			return;
		}
		if (sji_checkusername(affirmPasswd.value, 1, 16) == false) {
			affirmPasswd.focus();
			alert("确认密码错误，请重新输入确认密码!");
			return;
		}
		if (newPasswd.value != affirmPasswd.value) {
			affirmPasswd.focus();
			alert("新密码和确认密码不匹配，请重新输入确认密码!");
			return;
		}
		submit();
	}
}
</script>
</head>

<!-------------------------------------------------------------------------------------->
<!--主页代码-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
	<blockquote>
		<div align="left" style="padding-left:20px; padding-top:5px">
			<form id="form" action="/boaform/admin/formPasswordSetup" method="post">
				<div align="left">
					<b>访问控制 -- 密码</b><br><br>
					<div id="sutip" align="left" style="display: none">
						访问路由器是通过两个用户名来控制: telecomadmin和useradmin。<br>
						<br>
						用户名"telecomadmin"可以不受限制的浏览和修改您的家庭网关。<br>
						<br>
						用户名"useradmin"可以访问家庭网关，浏览配置和统计表。<br>
						<br>
						使用下面的区域输入最大16个字符，然后点击"保存/应用"来改变或创建密码。 注意:密码不能含空格。<br>
					</div>
					<hr align="left" class="sep" size="1" width="90%">
					<br>
					<table id="lstin" border="0" cellpadding="0" cellspacing="0">
						<tr>
							<td width="120" height="23">原用户名:</td>
							<td><input name="oldUserName" type="text" size="20" maxlength="16" style="width:200px" disabled="true" value="<% getInfo("normal-user"); %>"></td>
						</tr>
						<tr>
							<td>旧密码:</td>
							<td><input name="oldPasswd" type="password" size="20" maxlength="16" style="width:200px"></td>
						</tr>
						<tr>
							<td>新密码:</td>
							<td><input name="newPasswd" type="password" size="20" maxlength="16" style="width:200px"></td>
						</tr>
						<tr>
							<td>确认密码:</td>
							<td><input name="affirmPasswd" type="password" size="20" maxlength="16" style="width:200px"></td>
						</tr>
					</table>
				</div>
				<hr align="left" class="sep" size="1" width="90%">
				<input type="button" class="button" value="保存/应用" onclick="on_submit();">
				<input type="hidden" name="submit-url" value="">
			</form>
		</div>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>
