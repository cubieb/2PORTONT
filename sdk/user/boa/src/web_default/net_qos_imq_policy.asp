<html>
<head>
<title>IP QoS <% multilang(LANG_CONFIGURATION); %></title>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">

<!--ç³»ç??¬å…±css-->

<!--ç³»ç??¬å…±?šæœ¬-->
<script language="javascript" src="common.js"></script>
<script language="javascript" type="text/javascript">
var policy = 1;
var rules = new Array();
var queues = new Array();
var totalBandwidth = 1000;
<% initQueuePolicy(); %>

function queue_display() {
	var hrow=lstrc.rows[0];
	var hcell=hrow.cells[1];
	
	if (document.forms[0].queuepolicy[0].checked)
		hcell.innerHTML = '<% multilang(LANG_PRIORITY); %>';
	else
		hcell.innerHTML = "Weight";
	if(lstrc.rows){while(lstrc.rows.length > 1) lstrc.deleteRow(1);}
	for(var i = 0; i < queues.length; i++) {
		var row = lstrc.insertRow(i + 1);
		row.nowrap = true;
		row.vAlign = "center";
		row.align = "center";

		var cell = row.insertCell(0);
		cell.innerHTML = queues[i].qname;
		cell = row.insertCell(1);
		if (document.forms[0].queuepolicy[0].checked)
			cell.innerHTML = queues[i].prio;
		else
			cell.innerHTML = "<input type=\"text\" name=w" + i + " value=" + queues[i].weight + " size=3>";
		cell = row.insertCell(2);
		qcheck= queues[i].enable? " checked":"";
		cell.innerHTML = "<input type=\"checkbox\" name=qen" + i + qcheck + ">";
	}
	
		document.getElementById('displayTotalBandwidth').innerHTML=
			'<p><% multilang(LANG_TOTAL_BANDWIDTH_LIMIT); %>:<input type="text" name="totalbandwidth" id="totalbandwidth" value="1005">Kb</p>';
		document.forms[0].totalbandwidth.value = totalBandwidth;
	}

function on_init(){
	with(document.forms[0]){
		if(policy != 0 && policy !=1)
			policy = 0;
		queuepolicy[policy].checked = true;
		qosen[qosEnable].checked = true;
		qosPly.style.display = qosEnable==0 ? "none":"block";		
	}
	queue_display();	
}

function on_save() {
	with(document.forms[0]) {
		var sbmtstr = "";
		if(queuepolicy[0].checked==true)
			sbmtstr = "policy=0";
		else
			sbmtstr = "policy=1";

		d = parseInt(document.forms[0].totalbandwidth.value, 10);
		if(d<=0){
			alert("Invalid totalbandwidth number!");
			document.forms[0].totalbandwidth.focus();
			return false;
		}
		lst.value = sbmtstr;
		submit();
	}	
}

function qosen_click() {
	document.all.qosPly.style.display = document.all.qosen[0].checked ? "none":"block";
}

function qpolicy_click() {
	queue_display();
}

</script>
</head>
<body onLoad="on_init();">
<blockquote>
	<DIV align="left" style="padding-left:20px; padding-top:5px;">
		<h2><font color="#0000FF">IP QoS <% multilang(LANG_CONFIGURATION); %></font></h2>
		<form id="form" action="/boaform/admin/formQosPolicy" method="post">		  

		<table border=0 width="500" cellspacing=4 cellpadding=0>
			<tr><td><hr size=1 noshade align=top></td></tr>
		</table>
		<table border=0 width="500" cellspacing=4 cellpadding=0>
		  	<tr>
		  		<td><font size=2><b><% multilang(LANG_IP_QOS); %></b></td>
					<td><font size=2><input type="radio"  name=qosen value=0 onClick=qosen_click();><% multilang(LANG_DISABLE); %></td>	
						<td><font size=2><input type="radio"  name=qosen value=1 onClick=qosen_click();><% multilang(LANG_ENABLE); %></td>	
			</tr>
		  </table>
		 
		  <div  id="qosPly"  style="display:none">
		  <p><strong>QoS <% multilang(LANG_QUEUE_CONFIG); %></strong></p>
		  <p><% multilang(LANG_PAGE_DESC_CONFIGURE_QOS_POLICY); %></p>
		  <table>
		  	<tr>
				<td><font size=2><b><% multilang(LANG_POLICY); %>:</b></td>
				<td><font size=2><input type="radio"  name="queuepolicy" value="prio" onClick=qpolicy_click();><% multilang(LANG_PRIO); %></td>	
				<td><font size=2><input type="radio"  name="queuepolicy" value="wrr" onClick=qpolicy_click();><% multilang(LANG_WRR); %></td>	
			</tr>
		  </table>
		  <table class="flat" id="lstrc" border="1" cellpadding="0" cellspacing="1" width=30%>
			<tr class="hdb" align="center" nowrap bgcolor="#CCCCCC">
				<td><font size=2><% multilang(LANG_QUEUE); %></td>
				<td><font size=2><% multilang(LANG_LANG_PRIORITY); %></td>
				<td><font size=2><% multilang(LANG_ENABLE); %></td>
			</tr>			
		  </table>
		  <br>
		  <table border="0" width="500" cellpadding="0" cellspacing="0">
			<!--<tr><td><hr size=2 noshade align=top></td></tr>-->
			<tr><td ID="displayTotalBandwidth"></td></tr>
		  </table>
		  </div>		  

		  <br><br>
		  <input type="button" class="button" value="<% multilang(LANG_APPLY_CHANGES); %>" onClick="on_save();">
		  <input type="hidden" id="lst" name="lst" value="">
		  <input type="hidden" name="submit-url" value="/net_qos_imq_policy.asp">
		</form>
	</DIV>
</blockquote>
</body>
</html>
