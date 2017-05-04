<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!-- add by ql_xu 2008-05-23 -->
<html>
<head>
<title>中国电信</title>
<meta http-equiv=pragma content=no-cache>
<meta http-equiv=cache-control content="no-cache, must-revalidate">
<meta http-equiv=content-type content="text/html; charset=gbk">
<meta http-equiv="refresh" content="5">
<meta http-equiv=content-script-type content=text/javascript>
<meta http-equiv="X-UA-Compatible" content="IE=Edge" />
<!--系统公共css-->
<style type=text/css>
@import url(/style/default.css);
@import url(/style/nprogress.css);
</style>
<style>
body { 
	font-family: "华文宋体";
	background-image: url('/image/loid_register.gif');
	background-repeat: no-repeat;
	background-attachment: fixed;
	background-position: center top;
	text-align:center;
}

div.msg {
	font-size: 20px;
	position: relative;
	top: 100px;
	margin: auto;
	width: 470px;
	text-align: center;
}

#ok {
	position: relative;
	margin: auto;
	top: 95px;
	width: 80px;
	font-weight: bold;
	height: 30px;
	font-size: 18px;
}

#progress-boader {
	border: 2px #ccc solid;
	border-radius: 10px;
	height: 25;
	width: 80%;
	margin: auto;
	top: 40px;
	position: relative;	
}

SELECT {width:150px;}
</style>
<!--系统公共脚本-->
<script language="javascript" src="/common.js"></script>
<script language="javascript" src="/nprogress.js"></script>
<script language="javascript" type="text/javascript">

NProgress.configure({ trickle: false, showSpinner: false, parent: '#progress-boader' });
/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	var ok = document.getElementById("ok");

	if (ok) {
		if (window.top != window.self) {
			// in a frame
			ok.style.display = "none";
		} else {
			// the topmost frame
			ok.style.display = "block";
		}
	}
}
</script>

</head>

<!--主页代码-->
<body onLoad="on_init()">
	<div class=msg>
		<% UserAccountRegResult(); %>
	</div>
</body>
<%addHttpNoCache();%>
</html>
