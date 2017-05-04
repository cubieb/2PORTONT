<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang(LANG_CONNECTION_LIMIT); %></title>
<script type="text/javascript" src="share.js">
</script>
<script>
function skip () { this.blur(); }

function addClick()
{

  if (document.formConnLimitAdd.connLimitcap[0].checked)
  	return false;
 
 if (document.formConnLimitAdd.ip.value=="") {	
	alert('<% multilang(LANG_IP_ADDRESS_CANNOT_BE_EMPTY_IT_SHOULD_BE_FILLED_WITH_4_DIGIT_NUMBERS_AS_XXX_XXX_XXX_XXX); %>');
	document.formConnLimitAdd.ip.focus();
	return false;
  }
   	
   	num1 = parseInt(document.formConnLimitAdd.tcpconnlimit.value,10);
   	num4 = parseInt(document.formConnLimitAdd.udpconnlimit.value,10);
   	num2 = parseInt(document.formConnLimitAdd.connnum.value,10);
   	num3 = parseInt(document.formConnLimitAdd.protocol.value,10);
   	if ((num1!=0)&&( num3 == 0) && ( num2 > num1)){   		
		alert('<% multilang(LANG_MAX_LIMITATION_PORTS_SHOULD_BE_LOWER_THAN_GLOBAL_TCP_CONNECTION_LIMIT); %>');
   		document.formConnLimitAdd.connnum.focus();
   		return false;
   	}
   	else if ((num4 != 0)&&( num3 == 1)&&( num2 > num4)){   		
		alert('<% multilang(LANG_MAX_LIMITATION_PORTS_SHOULD_BE_LOWER_THAN_GLOBAL_UDP_CONNECTION_LIMIT); %>');
   		document.formConnLimitAdd.connnum.focus();
   		return false;
   	}


  if ( !checkDigitRange(document.formConnLimitAdd.ip.value,1,0,255) ) {      	
	alert('<% multilang(LANG_INVALID_IP_ADDRESS_RANGE_IN_1ST_DIGIT_IT_SHOULD_BE_0_255); %>');
	document.formConnLimitAdd.ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.formConnLimitAdd.ip.value,2,0,255) ) {      	
	alert('<% multilang(LANG_INVALID_IP_ADDRESS_RANGE_IN_2ND_DIGIT_IT_SHOULD_BE_0_255); %>');
	document.formConnLimitAdd.ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.formConnLimitAdd.ip.value,3,0,255) ) {      
	alert('<% multilang(LANG_INVALID_IP_ADDRESS_RANGE_IN_3RD_DIGIT_IT_SHOULD_BE_0_255); %>');
	document.formConnLimitAdd.ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.formConnLimitAdd.ip.value,4,1,254) ) {      	
	alert('<% multilang(LANG_INVALID_IP_ADDRESS_RANGE_IN_4TH_DIGIT_IT_SHOULD_BE_1_254); %>');
	document.formConnLimitAdd.ip.focus();
	return false;
  }   		
  return true;
}

function disableDelButton()
{

  if (verifyBrowser() != "ns") {
	disableButton(document.formConnLimitDel.deleteSelconnLimit);
	disableButton(document.formConnLimitDel.deleteAllconnLimit);
  }

}

function updateState()
{
	
//  if (document.formConnLimitAdd.enabled.checked) {
  if (document.formConnLimitAdd.connLimitcap[1].checked) {
 	enableTextField(document.formConnLimitAdd.ip);
	enableTextField(document.formConnLimitAdd.protocol);
	enableTextField(document.formConnLimitAdd.connnum);	
	//enableTextField(document.formConnLimitAdd.cnlm_enable);
  }
  else {
 	disableTextField(document.formConnLimitAdd.ip);
	disableTextField(document.formConnLimitAdd.protocol);
	disableTextField(document.formConnLimitAdd.connnum);
	//disableTextField(document.formConnLimitAdd.cnlm_enable);
  }

}



</script>
</head>

<body>
<blockquote>
<h2><font color="#0000FF"><% multilang(LANG_CONNECTION_LIMIT); %></font></h2>
<form action=/boaform/formConnlimit method=POST name="formConnLimitAdd">
<table border=0 width="500" cellspacing=0 cellpadding=0>
<tr><td><font size=2>
 <% multilang(LANG_ENTRIES_IN_THIS_TABLE_ALLOW_YOU_TO_LIMIT_THE_NUMBER_OF_TCP_UDP_PORTS_USED_BY_INTERNAL_USERS); %>
</font></td></tr>
<tr><td><hr size=1 noshade align=top></td></tr>
</table>

<table border=0 width="500" cellspacing=0 cellpadding=0>




<tr><td><font size=2><b><% multilang(LANG_CONNECTION_LIMIT); %>:</b>
	<input type="radio" value="0" name="connLimitcap" <% checkWrite("connLimit-cap0"); %> onClick="updateState()"><% multilang(LANG_DISABLE); %>&nbsp;&nbsp;
	<input type="radio" value="1" name="connLimitcap" <% checkWrite("connLimit-cap1"); %> onClick="updateState()"><% multilang(LANG_ENABLE); %>&nbsp;&nbsp;
	
</font></td></tr>
<tr><td><font size=2><b><% multilang(LANG_GLOBAL); %> TCP <% multilang(LANG_CONNECTION_LIMIT); %>:</b>
	<input type="text" name="tcpconnlimit" size="4" maxlength="4" value=<% getInfo("connLimit-tcp"); %>> &nbsp;(<% multilang(LANG_0_FOR_NO_LIMIT); %>) &nbsp; </td>
</font></td></tr>
<tr><td><font size=2><b><% multilang(LANG_GLOBAL); %> UDP <% multilang(LANG_CONNECTION_LIMIT); %>:</b>
	<input type="text" name="udpconnlimit" size="4" maxlength="4" value=<% getInfo("connLimit-udp"); %> > &nbsp;(<% multilang(LANG_0_FOR_NO_LIMIT); %>) &nbsp; </td>
</font></tr>
<tr><td><input type="submit" value="<% multilang(LANG_APPLY_CHANGES); %>" name="apply">&nbsp;&nbsp;
	<input type="hidden" value="/connlimit.asp" name="submit-url"></td></tr>


</table>
<table border=0 width="500" cellspacing=0 cellpadding=0>
	<tr><td><hr size=1 noshade align=top></td></tr>
	<tr>
		<td><font size=2>
			<b><% multilang(LANG_PROTOCOL); %>:</b>
				<select name="protocol">
					<option select value=0>TCP</option>
					<option value=1>UDP</option>
				</select>&nbsp;			
		</td>
	</tr>
	<tr>
		<td><font size=2><b><% multilang(LANG_LOCAL); %> <% multilang(LANG_IP_ADDRESS); %>:&nbsp;</b>
				<input type="text" name="ip" size="10" maxlength="15">&nbsp;&nbsp;&nbsp;
			<font size=2><b><% multilang(LANG_MAX_LIMITATION_PORTS); %>:</b>
				<input type="text" name="connnum" size="3" maxlength="5">
		</td>
	</tr>
	<tr>
		<td>
			<input type="submit" value="<% multilang(LANG_ADD); %>" name="addconnlimit" onClick="return addClick()">
			<input type="hidden" value="/fw-portfw.asp" name="submit-url">
		</td>
	</tr>
<script> updateState(); </script>
</form>
</table>


<br>
<form action=/boaform/formConnlimit method=POST name="formConnLimitDel">
<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><td><hr size=1 noshade align=top></td></tr>
  <tr><td><font size=2><b><% multilang(LANG_CURRENT_CONNECTION_LIMIT_TABLE); %>:</b></font></td></tr>
</table>
<table border=0 width=500>
<% connlmitList(); %>
</table>

 <br><input type="submit" value="<% multilang(LANG_DELETE_SELECTED); %>" name="deleteSelconnLimit" onClick="return deleteClick()">&nbsp;&nbsp;
     <input type="submit" value="<% multilang(LANG_DELETE_ALL); %>" name="deleteAllconnLimit" onClick="return deleteAllClick()">&nbsp;&nbsp;&nbsp;
     <input type="hidden" value="/connlimit.asp" name="submit-url">
 <script>
   	<% checkWrite("connLimitNum"); %>
 </script>
     
</form>

</td></tr></table>

</blockquote>
</body>
</html>
