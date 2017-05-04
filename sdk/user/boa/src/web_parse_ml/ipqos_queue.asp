<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title>IP <% multilang("39" "LANG_QOS_QUEUE"); %></title>
<script type="text/javascript" src="share.js">
</script>
<script>
var qDesclist;
function addClick() {
 if (document.getElementById){ // DOM3 = IE5, NS6
  document.getElementById('queueadd').style.display = 'block';
 } else {
  if (document.layers == false) {// IE4
   document.all.queueadd.style.display = 'block';
  }
 }
}

function removeClick(rml) {
   var lst = '';
   if (rml.length > 0)
      for (i = 0; i < rml.length; i++) {
         if ( rml[i].checked == true )
            lst += rml[i].value + ', ';
      }
   else if ( rml.checked == true )
      lst = rml.value;

   document.qos.removeQueueList.value = lst;
   //document.write(document.qos.removeQueueList.value);
   //var loc = 'qosqueue.cmd?action=remove&rmLst=' + lst;

   //var code = 'location="' + loc + '"';
   //eval(code);
}

function savRebootClick(ebl) {
   var eblLst = '';
   if (ebl.length > 0)
      for (i = 0; i < ebl.length; i++) {
         if ( ebl[i].checked == true )
            eblLst += ebl[i].value + ', ';
      }
   else if ( ebl.checked == true )
      eblLst = ebl.value;

   document.qos.eblQueueList.value = eblLst;
}

function updateDesc() {
 var currDesc;
 var i;
 var desc = qDesclist.split(";");
 var vpi_vci;
 var prior;

 for (i=0; i<desc.length; i++){
   vpi_vci = desc[i].split(",");
   with ( document.forms[0]) {
         if(queueintf.value != vpi_vci[0])
             continue;
         prior = queuepriority.value-1;
         if(prior <0)
     currDesc = vpi_vci[1] + "_p";
    else
     currDesc = vpi_vci[1] + "_p" + prior;
    queuedesc.value = currDesc;
   }
 }
}

function btnApply() {

   with ( document.forms[0] ) {
      if ( queuedesc.value == "") {
         msg = 'Please input description for this queue.'
         alert(msg);
         return false;
      }

      if ( queueenbl.selectedIndex == 0 ) {
         msg = 'Please select status for this queue.'
         alert(msg);
         return false;
      }

      if ( queueintf.selectedIndex == 0 ) {
         msg = 'Specify an egress interface for this queue.'
         alert(msg);
         return false;
      }

      if ( queuepriority.selectedIndex == 0 ) {
         msg = 'Please select precedence for this queue.'
         alert(msg);
         return false;
      }

   }

   return true;
}

function btnCancel(){
 var loc = 'ipqos_queue.asp'
 var code = 'location="' + loc + '"';
 eval(code);
}

</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">IP <% multilang("39" "LANG_QOS_QUEUE"); %></font></h2>

<table id=box_header border=0 width=600 cellSpacing=0>
<tr><td colspan=4><font size=2>
IP <% multilang("39" "LANG_QOS_QUEUE"); %><% multilang("212" "LANG_CONFIGURATION"); %>
</font></td></tr>
<tr><td colspan=4><hr size=1 noshade align=top></td></tr>

<form action=/boaform/admin/formQueueAdd method=POST name=qos>

<table id=body_header border=0 width=600>
 <tr><td width=100% colspan=7><font size=2><b><% multilang("599" "LANG_QUEUE_CONFIG_LIST"); %></b></font></td></tr>
 <% ipQosQueueList("queueList"); %>
</table>

<input type="hidden" name=removeQueueList>
<input type="hidden" name=eblQueueList>
<input type="hidden" name=check>
<input type='button' onClick='addClick()' value='<% multilang("195" "LANG_ADD"); %>'>
<!--
<input type='submit' name="RemoveQueue" onClick=removeClick(this.form.removeQ) value='Remove'>
<input type='submit' name="SaveAndReboot" onClick='savRebootClick(this.form.enableQ)' value='Save'></p>
-->
<% ipQosQueueList("QueueButton"); %>

<div id="queueadd" style="display:none">
<tr><td colspan=4><hr size=1 noshade align=top></td></tr>
<table id=body_header border=0 width=600 cellSpacing=0>
  <tr>
   <td>
                     <table border="0" cellpadding="0" cellspacing="0" width=100%>
                            <tr>
                                      <td width=30%><% multilang("600" "LANG_QUEUE_DESCRIPTION"); %>&nbsp;:</td>
                                      <td width=70%><input type="text" name="queuedesc" size="16" maxlength="30" readonly>
                                      </td>
                                   </tr>

                                   <tr>
                                      <td width=30%><% multilang("601" "LANG_QUEUE_STATUS"); %>&nbsp;:</td>
                                      <td width=70%>
                                         <select name='queueenbl' size="1">
                                            <option value="0">(<% multilang("602" "LANG_CLICK_TO_SELECT"); %>)
                                            <option value="1"> <% multilang("221" "LANG_DISABLE"); %>
                                            <option value="2"> <% multilang("222" "LANG_ENABLE"); %>
                                         </select>
                                      </td>
                                   </tr>

                                   <tr>
                                      <td width=30%><% multilang("603" "LANG_QUEUE_INTERFACE"); %>&nbsp;:</td>
                                      <td width=70%><select name='queueintf' size="1" onChange="updateDesc()">
       <% if_wan_list("queueITF"); %>
                                        </select>
                                      </td>
                                   </tr>

                                   <tr>
                                      <td width=30%><% multilang("604" "LANG_QUEUE_PRIORITY"); %>&nbsp;:</td>
                                      <td width=70%>
                                         <select name='queuepriority' size="1" onChange="updateDesc()">
                                            <option value="0">(<% multilang("602" "LANG_CLICK_TO_SELECT"); %>)
                                            <option value="1"> 0
                                            <option value="2"> 1
                                            <option value="3"> 2
         <option value="4"> 3
                                         </select>
                                      </td>
                                   </tr>
                            </table>
                        </td>
                 </tr>
         </table>
         <br>
<input type='submit' onClick='return btnApply()' value='<% multilang("279" "LANG_APPLY"); %>' name="save"><input type='button' onClick='return btnCancel()' value='<% multilang("605" "LANG_CANCEL"); %>'>
</div>

<input type="hidden" value="/ipqos_queue.asp" name="submit-url">
<script>
 <% initPage("qosQueue"); %>
</script>

</form>

<script type='text/javascript'>
if ( document.qos.check.value == "0")
 setTimeout("alert('Please create an Internet Setting with QoS enabled.');", 400);

</script>

</blockquote>
</body>

</html>
