<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>中国电信-ITMS服务器</TITLE>
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

var certname = "acscert";
var certtype = "cs";
var certsubject = "C/CN/ST=nanjing/O=127.24.22.1/OU=172.24.22.1";
var certContent="-----BEGIN CERTIFICATE-----\r\n"+
"MIICVTCCAb4CAQAwDQYJKoZIhvcNAQEEBQAwczELMAkGA1UEBhMCQ04xEDAOBgNV\r\n"+
"BAgTB2ppYW5nc3UxEDAOBgNVBAcTB25hbmppbmcxFDASBgNVBAoTCzE3Mi4yNC4y\r\n"+
"Mi4xMRQwEgYDVQQLEwsxNzIuMjQuMjIuMTEUMBIGA1UEAxMLMTcyLjI0LjIyLjEw\r\n"+
"HhcNMDgwMTI1MDE0MjQ0WhcNMTAxMDIxMDE0MjQ0WjBzMQswCQYDVQQGEwJDTjEQ\r\n"+
"MA4GA1UECBMHamlhbmdzdTEQMA4GA1UEBxMHbmFuamluZzEUMBIGA1UEChMLMTcy\r\n"+
"LjI0LjIyLjExFDASBgNVBAsTCzE3Mi4yNC4yMi4xMRQwEgYDVQQDEwsxNzIuMjQu\r\n"+
"MjIuMTCBnzANBgkqhkiG9w0BAQEFAAOBjQAwgYkCgYEA0rRMmOzOj6IiLqar5aG9\r\n"+
"JCRDJn54iYv9Fdw4jf9vph0ZGLmryqHKudyGDRT35Xby6CeApzdFNMZ1hylzDKQZ\r\n"+
"f4kjfxujTAVAopXwsbCuYesAiQBS1+HffMkb6HbNgKZ1qS53O8cBO7FPoy3TrHV1\r\n"+
"Fm8xCiRw/EQgtNWKmHmjV9kCAwEAATANBgkqhkiG9w0BAQQFAAOBgQBzFY1wjsH2\r\n"+
"bay93g3ZZSo+E4a5v0J9uf39z6hgH21L9iP8o6nsk5283yF0yhp+tISlCu59LaBD\r\n"+
"YVJe3Vw0LG7gOtfeJjMeF2YZOnpwakc7HYfvSFrMMOzEWOoaK5jh+zn+BtEbfP0j\r\n"+
"qK8h1jGLP42vGx/PmVl9RgAqj5EdNpirEw==\r\n"+
"-----END CERTIFICATE-----\r\n";
//<%showCert();%>

function on_back()
{
	var loc = "net_certca.asp";
	var code = "location.assign(\"' + loc + '\")";
	eval(code); 
}

</script>
</head>
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000">
<blockquote>
<DIV align="left" style="padding-left:20px; padding-top:10px">
<form name="showformcert">
<b>证书明细</b><br><br>
<center>
<table border='1' cellpadding='4' cellspacing='0'>
<tr>
<td>Name</td>
<td>
<script language="javascript">
var certName=certname;
document.writeln(certName);
</script>
</td>
</tr>
<tr>
<td>Type</td> 
<td>
<script language="javascript">
var certTpye=certtype;
document.writeln(certTpye);
</script>
</td>
</tr>
<tr>
<td>Subject</td> 
<td>
<script language="javascript">
var certSubject=certsubject;
document.writeln(certSubject);
</script>
</td>
</tr>
<tr>
<td>Certificate</td>
<td><textarea name="texare" cols = '80' rows='20'></textarea></td>
<script language="javascript">
showformcert.texare.value=certContent;
</script>
</tr>
</table><br>
<input type='button' value='&nbsp 后退 &nbsp' onClick="on_back();">
</center>
</form>
</DIV>
</blockquote>
</body>
<%addHttpNoCache();%>
</html>
