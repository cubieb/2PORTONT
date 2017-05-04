<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang(LANG_PORT_MAPPING); %><% multilang(LANG_CONFIGURATION); %></title>
<SCRIPT>
function adminClick()
{
	with (document.eth2pvc) {
		if (pmap[0].checked) {
			rmbtn.disabled = true;
			adbtn.disabled = true;
			lstGrp.disabled = true;
			lstAvail.disabled = true;
			for (i=0; i<4; i++)
				select[i].disabled = true;
		}
		else {
			rmbtn.disabled = false;
			adbtn.disabled = false;
			lstGrp.disabled = false;
			lstAvail.disabled = false;
			for (i=0; i<4; i++)
				select[i].disabled = false;
		}
	}
}

function btnRemove() {
   with ( document.eth2pvc ) {
      var arrSelected = new Array();
      var count = 0;
      for ( i = 0; i < lstGrp.options.length; i++ ) {
         if ( lstGrp.options[i].selected == true ) {
            arrSelected[count] = lstGrp.options[i].value;
         }
         count++;
      }
      var x = 0;
      for (i = 0; i < lstGrp.options.length; i++) {
         for (x = 0; x < arrSelected.length; x++) {
            if (lstGrp.options[i].value == arrSelected[x]) {
               varOpt = new Option(lstGrp.options[i].text, lstGrp.options[i].value);
               lstAvail.options[lstAvail.length] = varOpt;
               lstGrp.options[i] = null;
            }
         }
      }
   }
}

function btnAdd() {
   with ( document.eth2pvc ) {
      var arrSelected = new Array();
      var count = 0;
      for ( i = 0; i < lstAvail.options.length; i++ ) {
         if ( lstAvail.options[i].selected == true ) {
            arrSelected[count] = lstAvail.options[i].value;
         }
         count++;
      }
      var x = 0;
      for (i = 0; i < lstAvail.options.length; i++) {
         for (x = 0; x < arrSelected.length; x++) {
            if (lstAvail.options[i].value == arrSelected[x]) {
               varOpt = new Option(lstAvail.options[i].text, lstAvail.options[i].value);
               lstGrp.options[lstGrp.length] = varOpt;
               lstAvail.options[i] = null;
            }
         }
      }
   }
}

function btnApply() {
   with ( document.eth2pvc ) {
      for (i = 0; i < lstGrp.options.length; i++)
         itfsGroup.value+=lstGrp.options[i].value + ',';
      for (i = 0; i < lstAvail.options.length; i++)
         itfsAvail.value+=lstAvail.options[i].value + ',';
   }
}

function postit(groupitf, groupval, availitf, availval) {
   var interfaces;
   with ( document.eth2pvc ) {
      interfaces = groupitf.split(',');
      itfvals = groupval.split(',');
      lstGrp.options.length = 0;
      for ( i = 0; i < interfaces.length; i++ ) {
         if (interfaces[i] != '') {
            lstGrp.options[i] = new Option(interfaces[i], itfvals[i]);
         }
      }

      interfaces = availitf.split(',');
      itfvals = availval.split(',');
      lstAvail.options.length = 0;
      for ( i = 0; i < interfaces.length; i++ ) {
         if (interfaces[i] != '') {
            lstAvail.options[i] = new Option(interfaces[i], itfvals[i]);
         }
      }
   }
}

</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF"><% multilang(LANG_PORT_MAPPING); %><% multilang(LANG_CONFIGURATION); %></font></h2>

<form action=/boaform/formItfGroup method=POST name="eth2pvc">
<table border=0 width="700" cellspacing=4 cellpadding=0>
  <tr><font size=2>
    <% multilang(LANG_TO_MANIPULATE_A_MAPPING_GROUP); %>:<br>
    <b>1.</b> <% multilang(LANG_SELECT_A_GROUP_FROM_THE_TABLE); %><br>
    <b>2.</b> <% multilang(LANG_SELECT_INTERFACES_FROM_THE_AVAILABLE_GROUPED_INTERFACE_LIST_AND_ADD_IT_TO_THE_GROUPED_AVAILABLE_INTERFACE_LIST_USING_THE_ARROW_BUTTONS_TO_MANIPULATE_THE_REQUIRED_MAPPING_OF_THE_PORTS); %><br>
    <b>3.</b> <% multilang(LANG_CLICK_APPLY_CHANGES_BUTTON_TO_SAVE_THE_CHANGES); %><br><br>
    <b><% multilang(LANG_NOTE_THAT_THE_SELECTED_INTERFACES_WILL_BE_REMOVED_FROM_THEIR_EXISTING_GROUPS_AND_ADDED_TO_THE_NEW_GROUP); %></b>
  </tr>
  <tr><td><hr size=1 noshade align=top></td></tr>
</table>
<table>
      <input type="radio" name=pmap value=0 onClick="return adminClick()"><% multilang(LANG_DISABLED); %>&nbsp;&nbsp;
      <input type="radio" name=pmap value=1 onClick="return adminClick()"><% multilang(LANG_ENABLED); %></td>
</table><br>
<table border="0" cellpadding="0" cellspacing="0">
   <tr>
      <td width="150"><font size=2><b><% multilang(LANG_GROUPED_INTERFACES); %></b></td>
      <td width="100"></td>
      <td width="150"><font size=2><b><% multilang(LANG_AVAILABLE_INTERFACES); %></b></td>
   </tr>
   <!--
   <tr><td colspan="3">&nbsp;</td></tr>
   -->
   <tr>
      <td>
          <select multiple name="lstGrp" size="8" style="width: 120"></select>
      </td>
      <td>
         <table border="0" cellpadding="0" cellspacing="5">
            <tr><td>
               <input type="button" name="rmbtn" value="->" onClick="btnRemove()" style="width: 30; height: 30">
            </td></tr>
            <tr><td>
               <input type="button" name="adbtn" value="<-" onClick="btnAdd()" style="width: 30; height: 30">
            </td></tr>
         </table>
      </td>
      <td>
          <select multiple name="lstAvail" size="8" style="width: 120">
          </select>
      </td>
   </tr>
</table>
<br>
<!--table border='1' cellpadding='4' cellspacing='0'-->
<table border='0' width=700>
	<% itfGrpList(); %>
</table>
  <br>
      <input type="hidden" name=itfsGroup>
      <input type="hidden" name=itfsAvail>
      <input type=submit value="<% multilang(LANG_APPLY_CHANGES); %>" name="save" onClick=btnApply()>&nbsp;&nbsp;
      <input type="hidden" value="/eth2pvc.asp" name="submit-url">
<script>
	<% initPage("portMap"); %>
	adminClick();
</script>
</form>
</blockquote>
</body>

</html>
