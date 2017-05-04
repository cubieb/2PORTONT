<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>中国电信</TITLE>
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
**          menu class
********************************************************************/
function menu(name)
{
	this.name = name;
	this.names = new Array();
	this.objs = new Array();
	
	this.destroy = function(){delete map;map = null;}
	this.add = function(obj, name){var i = this.names.length; if(name){this.names[i] = name;}else{this.names[i] = obj.name;} this.objs[i] = obj;}
	
	return this;
}

var mnroot = new menu("root");
<% createMenuDiag(); %>


/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	var fst = null;
	
	if(!topmenu) topmenu = document.getElementById("topmenu");
	if(!submenu) submenu = document.getElementById("submenu");
	
	if(topmenu.cells){while(topmenu.cells.length > 0) topmenu.deleteCell(0);}
	
	for(var i = 0; i < mnroot.names.length; i++)
	{
		var cell = topmenu.insertCell(i);
        var txt = "<p align=\"center\"><b><font size=\"2\">"; 
		txt += "<a href=\"#\" onClick=\"on_catolog(" + i + ");\">";
		txt += "<span style=\"font-size:9pt\">" + mnroot.names[i] + "</span></a></font></b>";
		cell.bgColor = "#EF8218";
		cell.width = "15%";
		cell.innerHTML = txt;
		cell.mnobj = mnroot.objs[i];
		if(fst == null)fst = i;
	}
	topmenu.sel = 0;
	topmenu.cells[0].bgColor = "#427594";
	menuname.innerHTML = mnroot.names[0];
	on_catolog(fst);
}

/********************************************************************
**          on catolog changed
********************************************************************/
function on_catolog(index)
{
	var fst = null;
	
	if(!topmenu.cells || index >= topmenu.cells.length)return;
	
	if(topmenu.sel != index)
	{
		topmenu.cells[topmenu.sel].bgColor = "#EF8218";
		topmenu.cells[index].bgColor = "#427594";
		topmenu.sel = index;
		menuname.innerHTML = mnroot.names[index];
	}
	
	var mnobj = topmenu.cells[index].mnobj;
	
	if(submenu.cells){while(submenu.cells.length > 1) submenu.deleteCell(1);}

	for(var i = 0; i < mnobj.names.length; i++)
	{
		var cell = submenu.insertCell(i * 2 + 1);
		cell.width = "8px";
		cell.innerHTML = "|";
		var index = i * 2 + 2;
		cell = submenu.insertCell(index);
		var txt = "<p align=\"center\">&nbsp;&nbsp;";
        txt += "<a href=\"#\" onClick=\"on_menu(" + index + ");\">";
        txt += "<span style=\"font-size:9pt; color:#C0C0C0\">" + mnobj.names[i] + "</span></a>&nbsp;&nbsp;";
		//cell.width = "75px";
		cell.innerHTML = txt;
		cell.nowrap = true;
		cell.name = mnobj.names[i];
		cell.mnobj = mnobj.objs[i];
		if(fst == null)fst = index;
	}
	on_menu(fst);
}

/********************************************************************
**          on menu fire
********************************************************************/
function on_menu(index)
{
	if(!submenu.cells || index >= submenu.cells.length)return;
	
	tbobj = submenu.cells[index];
	var mnobj = tbobj.mnobj;
	var lstmenu = top.leftFrame.lstmenu;
	if(!lstmenu) lstmenu = top.leftFrame.document.getElementById("lstmenu");
	if(!lstmenu)return;
	if(lstmenu.rows){while(lstmenu.rows.length > 0) lstmenu.deleteRow(0);}
	
	for(var i = 0; i < mnobj.names.length; i++)
	{
		var row = lstmenu.insertRow(i);
		
		row.nowrap = true;
		row.vAlign = "top";
		
		var cell = row.insertCell(0);
		
		cell.width = "100%";
		cell.innerHTML = "<br><p>&nbsp;&nbsp;<a href=\"" + mnobj.objs[i] + "\", target=\"mainFrame\">" + mnobj.names[i] + "</a></p>";
		cell.nowrap = true;
		cell.name = mnobj.names[i];
		cell.mnobj = mnobj.objs[i];
	}
	top.mainFrame.location.href=mnobj.objs[0];
}

</SCRIPT>
</HEAD>

<!-------------------------------------------------------------------------------------->
<!--主页代码-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
<form action=/boaform/admin/formLogout method=POST name="top" target="_top">
<table border="0" width="100%" cellspacing="0" cellpadding="0" height="43%">
  <tr>
    <td  width="100%"> <IMG height="100%" src="image/background_top.gif" width=100% border=0> </td>
  </tr>
	<tr>
		<td height="11" background="image/bar_top.gif"></td>
	</tr>
</table>
<table border="0" cellpadding="0" cellspacing="0" width="100%" height="52%">
  <tr nowrap> 
    <td width="156" rowspan="3" bgcolor="#EF8218" style="font-size:24pt" nowrap>&nbsp;<b><font color="#FFFFFF"><label id="menuname">状态</label></font></b>&nbsp;</td>
    <td height="18" bgcolor="#427594" width="100%" style="font-size:9pt" align="right"><font color="white">网关名称: 家庭网关&nbsp;&nbsp;&nbsp;&nbsp;</font><font color="black"><input type="submit" value="退出"></font>&nbsp;&nbsp;</td>
  </tr>
  <tr> 
    <td height="43" valign="bottom"> 
		<table border="0" cellpadding="0" cellspacing="0" width="100%" height="43" bgcolor="#EF8218">
			<tr> 
			  <td width="15%" height="10" bgcolor="#EF8218" background="image/back_button.gif"></td>
			  <td width="15%" height="10" bgcolor="#EF8218" background="image/back_button.gif"></td>
			  <td width="15%" height="10" bgcolor="#EF8218" background="image/back_button.gif"></td>
			  <td width="15%" height="10" bgcolor="#EF8218" background="image/back_button.gif"></td>
			  <td width="15%" height="10" bgcolor="#EF8218" background="image/back_button.gif"></td>
			  <td width="15%" height="10" bgcolor="#EF8218" background="image/back_button.gif"></td>
			</tr>
			<tr id="topmenu" nowrap>
			  <td width="15">　</td>
			</tr>
		</table>
	</td>
  </tr>
  <tr> 
    <td height="22" bgcolor="#427594"> 
	<table border="0" cellpadding="0" cellspacing="0" height="15">
        <tr id="submenu" style="font-size:9pt; color:#C0C0C0" nowrap> 
		  <td width="15">　</td>
        </tr>
      </table>
    </td>
  </tr>
</table>
<table border="0" cellpadding="0" cellspacing="0" width="100%" height="8px">
   <tr>
    <td width="100%" height="8px" background="image/UI_05.gif"></td>
  </tr>
</table>
</form>
</body>
<%addHttpNoCache();%>
</html>
