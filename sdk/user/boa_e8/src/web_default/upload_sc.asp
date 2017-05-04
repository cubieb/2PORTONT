<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>Firmware Update</TITLE>
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
function submitonce(theform)
{
	if (document.all||document.getElementById)
	{
		for (i=0;i<theform.length;i++)
		{
			var tempobj=theform.elements[i]
			if(tempobj.type.toLowerCase()=="submit"||tempobj.type.toLowerCase()=="reset")
				tempobj.disabled=true;
		}
	}
	document.getElementById("displayFile").innerHTML = document.password.binary.value;
		
	if (document.getElementById) {  // DOM3 = IE5, NS6
    	document.getElementById("fileinput").style.display = "none";
    	document.getElementById("filetext").style.display = "block";
  	} else {
     	if (document.layers == false) {// IE4
			document.all.fileinput.style.display = "none";
			document.all.filetext.style.display = "block";
		}
  	}
}

function sendClicked()
{
	if (!confirm('如果升级文件较大,可能会占用较长时间,你真的要升级版本吗?'))
		return false;
	else{
		return true;
	}
}
</SCRIPT>

</head>
<BODY>
<blockquote>
<h2><font color="#0000FF">升级版本</font></h2>
<form action=/boaform/admin/formUpload method=POST enctype="multipart/form-data" name="password" onSubmit="javascript:submitonce(this)">
<table border="0" cellspacing="4" width="500">
 <tr><td align=left><font size=2>步骤 1:  取得系统升级文件.</td></tr>
 <tr><td align=left><font size=2>步骤 2:  指定系统升级文件所保存的位置. 按 "浏览" 指定系统升级文件
.</td></tr>
 <tr><td><font size=2>步骤 3:  按 "软件更新" 上载系统升级文件以更新系统软件.</td></tr>
 <tr><td><font size=2>注意:  系统软件更新需要约两分钟时间, 请勿在更新过程中关机. 系统软件更新后系统将自
动重新启动.</td></tr>
 <tr><td><hr size=1 noshade align=top></td></tr>
 <tr><td><font size=2>目前软件版本:<% ZTESoftwareVersion(); %></td></tr>
 <tr><td><hr size=1 noshade align=top></td></tr>
</table>
<div  id="fileinput" style="display:none">
<table border="0" cellspacing="4" width="500">
  <tr>
      <td width="20%"><font size=2><b>选择文件:</b>&nbsp;&nbsp;&nbsp;&nbsp;</td>
      <td width="80%"><font size=2><input type="file" name="binary" size=20></td>
  </tr>
</table>
</div>
<div  id="filetext" style="display:none">
<table border="0" cellspacing="4" width="500">
  <tr>
      <td width="15%"><font size=2><b>文件:</b>&nbsp;&nbsp;&nbsp;&nbsp;</td>
      <td width="85%"><font size=2 ID="displayFile"></td>
  </tr>
 <tr>
  <td width="15%"></td>
  <td width="85%">正在上传,请耐心等待!</td></tr>
</table>
</div>
  <p>
  <input type="submit" value="软件更新" name="send" onclick="return sendClicked()">&nbsp;&nbsp;
  <input type="reset" value="清空" name="reset">
  <input type="hidden" value="/admin/upload_sc.asp" name="submit-url">
  </p>

  <script>
    if (document.password.send.disabled == true) {
	  	if (document.getElementById) {  // DOM3 = IE5, NS6
	    	document.getElementById("fileinput").style.display = "none";
	    	document.getElementById("filetext").style.display = "block";
	  	} else {
	     	if (document.layers == false) {// IE4
				document.all.fileinput.style.display = "none";
				document.all.filetext.style.display = "block";
			}
	  	}
  	} else {
  		if (document.getElementById) {  // DOM3 = IE5, NS6
	    	document.getElementById("fileinput").style.display = "block";
	    	document.getElementById("filetext").style.display = "none";
	  	} else {
	     	if (document.layers == false) {// IE4
				document.all.fileinput.style.display = "block";
				document.all.filetext.style.display = "none";
			}
	  	}
  	}
  </script>
 </form>
 </blockquote>
</body>
<%addHttpNoCache();%>
</html>
