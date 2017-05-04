<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>中国电信-中间件设置</TITLE>
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

/*
var  cgi_enblChinaTelcomMDW = 1; //中间件开关  
var  cgi_mwsURLIP = "10.10.10.25";//中间件服务器地址
var  cgi_mwsURLPort="80";//中间件服务器端口
*/

/********************************************************************
**          on document load
********************************************************************/
/*
function on_init() 
{
	with (document.forms[0]) {
		enblChinaTelcomMDW = cgi_enblChinaTelcomMDW; //中间件开关  
		if ( enblChinaTelcomMDW == '1' )
			ctMDW[1].checked = true;
		else
			ctMDW[0].checked = true;
		
		mwsURLIP.value = cgi_mwsURLIP;
		
		mwsURLPort.value = cgi_mwsURLPort;
	}
}
*/		

function isValidPort(port) 
{
	var fromport = 0;
	var toport = 100;
	
	portrange = port.split(':');
	if (portrange.length < 1 || portrange.length > 2) {
		return false;
	}
	if (isNaN(portrange[0]))
		return false;
	fromport = parseInt(portrange[0]);
	
	if (portrange.length > 1) {
		if (isNaN(portrange[1]))
			return false;
			
		toport = parseInt(portrange[1]);
		
		if (toport <= fromport)
			return false;      
	}
	
	if (fromport < 1 || fromport > 65535 || toport < 1 || toport > 65535) {
		return false;
	}
	
	return true;
}

/********************************************************************
**          on document submit
********************************************************************/
function on_submit(act) 
{
	with ( document.forms[0] ) 
	{
		action.value = act;
		if(act=="sv")
		{
			if(ctMDW[1].checked == true){
				if (mwsURLIP.value.length > 256 ) 
				{
					alert('中间件服务器地址长度过长 (' + mwsURLIP.value.length + ') [0-256]'); //The length of Middleware Server URL is too long [0-256].
					return;
				}
				if(isIncludeInvalidChar(mwsURLIP.value))
				{
					alert("中间件服务器地址含有非法字符，请重新输入!");
					return;
				}
				if(isValidPort(mwsURLPort.value) == false)
				{
					alert('无效的中间件服务器地址端口(' + mwsURLPort.value + ')'); //Middleware Server URL Port is invalid.
					return;
				}
			}
			applyMidwareConfig.value = "applyMidwareConfig";
			submit(); 
		}
	}
}

</script>
   </head>
   <body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000">
     <div align="left" style="padding-left:20px; padding-top:10px">
      <blockquote>
         <form action=/boaform/admin/formTR069Config method="post">
           <b>中国电信中间件配置<br>
            </b>
            <br>
           本页可以设置中国电信中间件的工作模式、中间件服务器的地址和端口。
           <br>
           <br>
          	  <table border="0" cellpadding="0" cellspacing="0">
				<tr><td><font color='red'>(“关闭中间件”和“启用中间件（不含TR069功能）”之间切换不需要重启，其他切换需要重启！)</font></td></tr>
			  </table>
			  <br>
                <table border="0" cellpadding="0" cellspacing="0">
                  <tr>
                  <script language="javascript">
						document.write("<td width=\"200\">中间件工作模式</td>");		
				</script>
                     <td><input name='ctMDW' value='0' type='radio'  <% checkWrite("midware-enable1"); %> >
          启用中间件（含TR069功能）</td>
          		</tr>
          		<tr>
          			 <td width="200">&nbsp;&nbsp;</td>
          			 <td><input name='ctMDW' value='1' type='radio' <% checkWrite("midware-disable"); %> >
          关闭中间件&nbsp;&nbsp;</td>
          		</tr>
          		<tr>
          			 <td width="200">&nbsp;&nbsp;</td>
                     <td><input name='ctMDW' value='2' type='radio'  <% checkWrite("midware-enable2"); %> >
          启用中间件（不含TR069功能）</td>
                </tr>
               </table>
          	   <br>
               <table border="0" cellpadding="0" cellspacing="0">
  	              <tr>
                     <td width="200">中间件管理平台URL</td>
                     <td>地址:&nbsp;<input type='text' name='mwsURLIP' size="20" maxlength="256" value="<% getInfo("midwareServerAddr"); %>"></td>
                     <td>&nbsp;&nbsp;端口号:&nbsp;<input type='text' name='mwsURLPort' size="20" maxlength="256" value="<% getInfo("midwareServerPort"); %>"></td>
                  </tr>
               </table>
            
    <p > 
      <input type='button'  onClick="on_submit('sv')" value='保存/应用'>
	  <input type="hidden" name="applyMidwareConfig" value="">
	  <input type="hidden" id="action" name="action" value="none">
	  <input type="hidden" name="submit-url" value="/net_midware.asp">
    </P>
        </form>
		</div>
      </blockquote>
   </body>
<%addHttpNoCache();%>
</html>
