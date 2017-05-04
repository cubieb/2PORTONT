<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>中国电信-DSL设置</TITLE>
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

var glite_sw = true;
var gdmt_sw = true;
var t1413_sw = true;
var adsl2_sw = true;
var adsl2p_sw = true;
var anxl_sw = true;
var anxm_sw = false;

//<% init_dsl_page(); %>

function on_init()
{	
/*
  set_adsl.glite.checked = glite_sw;
  set_adsl.gdmt.checked = gdmt_sw;
  set_adsl.t1413.checked = t1413_sw;
  set_adsl.adsl2.checked = adsl2_sw;
  set_adsl.adsl2p.checked = adsl2p_sw;
  set_adsl.anxl.checked = anxl_sw;    
  set_adsl.anxm.checked = anxm_sw;
 */
}

function checkForm()
{
	with (document.set_adsl)
	{
		if(!glite && !gdmt && !t1413 && !adsl2 && !adsl2p)
		{
			alert("请您至少选择一种ADSL调制模式！");
			return false;
		}
		if((!adsl2 && !adsl2p ) && anxl)
		{
			alert("只有在ADSL2和ADSL2+模式下才能启用AnnexL，请您选中 ADSL2 或/和 ADSL2+ 调制模式！");
			return false;
		}
		if((!adsl2 && !adsl2p ) && anxm)
		{
			alert("只有在ADSL2和ADSL2+模式下才能启用AnnexM，请您选中 ADSL2 或/和 ADSL2+ 调制模式！");
			return false;
		}
	}
	return true;
}
function on_submit()
{
	document.set_adsl.submit();		
}
</script>
   </head>
   <body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
      <blockquote>
	    <DIV align="left" style="padding-left:20px; padding-top:5px">
         <form action=/boaform/formSetAdsl method="post" name="set_adsl" onSubmit="return checkForm(this);">
            <b>DSL设置</b>
			<br>
            <br>
				请选择下列的调制模式。
			<br>
            <table border="0" cellpadding="4" cellspacing="0">
               <tr>
                  <td width='20'>　</td>
                  <td><input type="checkbox" name="gdmt" <% checkWrite("adslmode-gdmt"); %>> G.Dmt 启用</td>
               </tr>
               <tr>
                  <td>　</td>
                  <td><input type="checkbox" name="glite" <% checkWrite("adslmode-glite"); %>> G.lite 启用</td>
               </tr>
               <tr>
                  <td>　</td>
                  <td><input type="checkbox" name="t1413" <% checkWrite("adslmode-t1413"); %>> T1.413 启用</td>
               </tr>
               <tr>
                  <td>　</td>
                  <td><input type="checkbox" name="adsl2" <% checkWrite("adslmode-adsl2"); %>> ADSL2 启用</td>
               </tr>
               <tr>
                  <td>　</td>
                  <td><input type="checkbox" name="adsl2p" <% checkWrite("adslmode-adsl2p"); %>> ADSL2+ 启用</td>
               </tr>
               <tr>
                  <td>　</td>
                  <td><input type="checkbox" name="anxl" <% checkWrite("adslmode-anxl"); %>> AnnexL 启用</td>
               </tr>
               <tr>
                  <td>　</td>
                  <td><input type="checkbox" name="anxm" <% checkWrite("adslmode-anxm"); %>> AnnexM 启用</td>
               </tr>
            </table>  
    		<br><br>
				<input type='submit' name="save" value='保存/应用' onClick = "on_submit();">
				  <input type="hidden" value="/net_dsl.asp" name="submit-url">
	     </form>
		</DIV>
      </blockquote>
   </body>
<%addHttpNoCache();%>
</html>
