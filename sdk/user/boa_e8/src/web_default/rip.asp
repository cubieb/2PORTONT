<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>动态路由</TITLE>
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
<script type="text/javascript" src="share.js"></script>
<STYLE type=text/css>
@import url(/style/default.css);
</STYLE>
<SCRIPT language="javascript" type="text/javascript">
var ifnum;

function selected()
{
	document.rip.ripDel.disabled = false;
}

function resetClicked()
{
	document.rip.ripDel.disabled = true;
}

function disableDelButton()
{
  if (verifyBrowser() != "ns") {
	disableButton(document.rip.ripDel);
	disableButton(document.rip.ripDelAll);
  }
}
</SCRIPT>
</head>

<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000">
<blockquote>
<DIV align="left" style="padding-left:20px; padding-top:5px">

<form action=/boaform/formRip method=POST name="rip">
<div class="tip" style="width:90% ">
	<b>路由 -- RIP 配置</b><br><br>	
</div>
<table border=0 width="500" cellspacing=0 cellpadding=0>
  <tr>
    为激活设备的RIP,选择全局 RIP 模式的‘启用’单选按钮.为配置一个单独接口,请选择所需 RIP 的版本,接着点击‘添加’按钮来保存配置.
  </tr>
  <tr><hr size=1 noshade align=top></tr>
  <tr>
      <td>全局 RIP 模式:</td>
      <td>
      	<input type="radio" value="0" name="rip_on" <% checkWrite("rip-on-0"); %> >禁用&nbsp;&nbsp;
     	<input type="radio" value="1" name="rip_on" <% checkWrite("rip-on-1"); %> >启用&nbsp;&nbsp;
      </td>
      <td><input type="submit" value="保存/应用" name="ripSet">&nbsp;&nbsp;</td>
  </tr>
</table>

<table border=0 width="500" cellspacing=0 cellpadding=0> 
  <tr><hr size=1 noshade align=top></tr>
  <br>

  <tr>
      <td width="30%">接口:</td>
      <td width="35%">
      	<select name="rip_if">
      	<option value="65535">br0</option>
      	<%  if_wan_list("rt");  %>
      	</select>
      </td>
  </tr>
  
  <tr>
      <td width="30%">接收版本:</td>
      <td width="70%">
      <select size="1" name="receive_mode">
      <option value="0">None</option>
      <option value="1">RIP1</option>
      <option value="2">RIP2</option>
      <option value="3">Both</option>
      </select>
      </td>
  </tr>
  
  <tr>
      <td width="30%">传送版本:</td>
      <td width="50%">
      <select size="1" name="send_mode">
      <option value="0">None</option>
      <option value="1">RIP1</option>
      <option value="2">RIP2</option>
      <option value="4">RIP1COMPAT</option>
      </select>
      </td>
      <td width="20%"><input type="submit" value="添加" name="ripAdd"></td>
  </tr>  
</table>   

<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><hr size=1 noshade align=top></tr>
  <tr>RIP 配置表:</tr>
  <% showRipIf(); %>
</table>

<br>
   <input type="submit" value="删除" name="ripDel" onClick="return deleteClick()">&nbsp;&nbsp;      
   <input type="submit" value="删除全部" name="ripDelAll" onClick="return deleteAllClick()">&nbsp;&nbsp;&nbsp;
   
   <input type="hidden" value="/rip.asp" name="submit-url">
 <script>
 	<% checkWrite("ripNum"); %>
  </script>
</form>
</DIV>
</blockquote>
</body>

</html>
