<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<HTML>
<HEAD>
<TITLE>维护</TITLE>
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
var pingResult;			//pass or fail
var pingHead;			// = "ping -c 4 -s" + size + " " + host +" > /tmp/pinglog"
var pingPassInfo;		//存储成功时的统计信息
var pingFailInfo;		//存储失败时的信息				Network unreachable or host timeout. 
//var cgi = new Object();
<% initPagePingResult(); %>
/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	//sji_docinit(document, cgi);
	if(pingResult==0)
	{
		pingFail.style.display = "block";
		pingPass.style.display = "none";
	}
	else
	{
		pingFail.style.display = "none";
		pingPass.style.display = "block";			
	}
	
}
</SCRIPT>
</HEAD>

<!-------------------------------------------------------------------------------------->
<!--主页代码-->
<BODY onload="on_init();">
	<BLOCKQUOTE>
		<DIV align="left" style="padding-left:20px; padding-top:5px">
		  	<FORM><B><FONT color=black>Diagnostics -- Ping </FONT></B>
		  		<div class="tip" id="pingPass">
			  		<BR>
				  	<BR>测试结果:<FONT color=green>PASS</FONT>
				  	<hr align="left" class="sep" size="1" width="90%">
				  	<BR>
				  	<BR><B><FONT color=black>Network is Reachable:</FONT></B>
				  	<BR>
				  	<BR><B><FONT color=black><script language="javascript">document.writeln(pingHead);</script></FONT></B>
					<BR><FONT color=black><script language="javascript">document.writeln(pingPassInfo);</script></FONT>
				  	<BR>
				  	<BR>
				  	<BR>
			  	</div>
			  	<div class="tip" id="pingFail">
			  		<BR>
			  		<BR>测试结果:<FONT color=red>FAIL</FONT>
			  		<hr align="left" class="sep" size="1" width="90%">
			  		<BR>
			  		<BR><B><FONT color=black><script language="javascript">document.writeln(pingHead);</script></FONT></B>
			  		<BR><B><FONT color=red>
			  		<BR>
			  		<BR><script language="javascript">document.writeln(pingFailInfo);</script></FONT></B>
			  		<BR>
			  		<BR>
			  		<BR>
			  		<BR>
			  	</div>
		  	</FORM>
	  	</DIV>
  	</BLOCKQUOTE>
</BODY></HTML>
