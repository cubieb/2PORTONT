<html>
<! Copyright (c) Realtek Semiconductor Corp., 2004. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang(LANG_TIME_ZONE); %><% multilang(LANG_CONFIGURATION); %></title>
<script type="text/javascript" src="share.js"> </script>
<script>

var ntp_zone_index=4;

function ntp_entry(name, value) { 
	this.name = name ;
	this.value = value ;
} 

function setNtpServer(field, ntpServer){
    field.selectedIndex = 0 ;
    for(i=0 ;i < field.options.length ; i++){
    	if(field.options[i].value == ntpServer){
		field.options[i].selected = true;
		break;
	}
    }
}

function checkEmpty(field){
	if(field.value.length == 0){
		alert(field.name + " field can't be empty\n");
		field.value = field.defaultValue;
		field.focus();
		return false;
	}
	else
		return true;
}
function checkNumber(field){
    str =field.value ;
    for (var i=0; i<str.length; i++) {
    	if ( (str.charAt(i) >= '0' && str.charAt(i) <= '9'))
                        continue;
	field.value = field.defaultValue;
        alert("<% multilang(LANG_IT_SHOULD_BE_IN_NUMBER_0_9); %>");
        return false;
    }	
	return true;
}
function checkMonth(str) {
  d = parseInt(str, 10);
  if (d < 0 || d > 12)
      	return false;
  return true;
}
function checkDay(str, month) {
  d = parseInt(str, 10);
  m = parseInt (month, 10);
  if (m == 1 || m == 3 || m == 5 || m == 7 || m == 8 || m == 10 || m == 12) {
  	if (d < 0 || d > 31)
      		return false;
  }
  else if (m == 4 || m == 6 || m == 9 || m == 11) {
  	if (d < 0 || d > 31)
      		return false;
  }
  else if (m == 2) {
  	if (d < 0 || d > 29)
      		return false;
  }
  else
  	return false;
  return true;
}
function checkHour(str) {
  d = parseInt(str, 10);
  if (d < 0 || d >= 24)
      	return false;
  return true;
}
function checkTime(str) {
  d = parseInt(str, 10);
  if (d < 0 || d >= 60)
      	return false;
  return true;
}
function saveChanges(form){
	if((checkEmpty(form.year)& checkEmpty(form.month) & checkEmpty(form.hour)
	 & checkEmpty(form.day) &checkEmpty(form.minute) & checkEmpty(form.second))== false)
	 	return false;

	if((checkNumber(form.year)& checkNumber(form.month) & checkNumber(form.hour)
	 & checkNumber(form.day) &checkNumber(form.minute) & checkNumber(form.second))== false)
	 	return false;
	if(form.month.value == '0'){
		form.month.value = form.month.defaultValue;
        	alert("<% multilang(LANG_INVALID_MONTH_NUMBER_IT_SHOULD_BE_IN_NUMBER_1_9); %>");
		return false;
	}
	if (!checkMonth(form.month.value)) {
		alert("<% multilang(LANG_INVALID_MONTH_SETTING); %>");
		form.month.focus();
		return false;
	}
	if (!checkDay(form.day.value, form.month.value)) {
		alert("<% multilang(LANG_INVALID_DAY_SETTING); %>");
		form.day.focus();
		return false;
	}
	if (!checkHour(form.hour.value)) {
		alert("<% multilang(LANG_INVALID_HOUR_SETTING); %>");
		form.hour.focus();
		return false;
	}
	if (!checkTime(form.minute.value) || !checkTime(form.second.value)) {
		alert("<% multilang(LANG_INVALID_TIME_SETTING); %>");
		return false;
	}
	if (form.enabled.checked && form.ntpServerId[1].checked && form.ntpServerHost2.value != form.ntpServerHost2.defaultValue) {
		if (form.ntpServerHost2.value == "" || !checkString(form.ntpServerHost2.value)) {
			alert("<% multilang(LANG_INVALID_SERVER_STRING); %>");
			form.ntpServerHost2.value = form.ntpServerHost2.defaultValue;
			form.ntpServerHost2.focus();
			return false;
		}
	}	
	return true;
}
function updateState(form)
{
	if(form.enabled.checked){
		enableTextField(form.ntpServerHost1);
		if(form.ntpServerHost2 != null)
			enableTextField(form.ntpServerHost2);
	}
	else{
		disableTextField(form.ntpServerHost1);
		if(form.ntpServerHost2 != null)
			disableTextField(form.ntpServerHost2);
	}
}
</script>
</head>
<body>
<blockquote>
<h2><font color="#0000FF"><% multilang(LANG_TIME_ZONE); %><% multilang(LANG_CONFIGURATION); %></font></h2>
<table border=0 width="500" cellspacing=0 cellpadding=0>
  <tr><td><font size=2>
  <% multilang(LANG_YOU_CAN_MAINTAIN_THE_SYSTEM_TIME_BY_SYNCHRONIZING_WITH_A_PUBLIC_TIME_SERVER_OVER_THE_INTERNET); %>
  </font></td></tr>
  <tr><td><hr size=1 noshade align=top></td></tr>
  </table>
<form action=/boaform/formNtp method=POST name="time">
<table border="0" width=520>
	<tr>
	<td width ="25%">
	<font size=2> <b> <% multilang(LANG_CURRENT_TIME); %> : </b> </font>
	</td>
	<td width ="75%">
                <font size =2> <b>
                <% multilang(LANG_YEAR); %><input type="text" name="year" value="<% getInfo("year"); %>" size="4" maxlength="4">
                <% multilang(LANG_MONTH); %><input type="text" name="month" value="<% getInfo("month"); %>" size="2" maxlength="2">
                <% multilang(LANG_DAY); %><input type="text" name="day" value="<% getInfo("day"); %>" size="2" maxlength="2">
		<br>
                <% multilang(LANG_HOUR); %><input type="text" name="hour" value="<% getInfo("hour"); %>" size="2" maxlength="2">
                <% multilang(LANG_MIN); %><input type="text" name="minute" value="<% getInfo("minute"); %>" size="2" maxlength="2">
                <% multilang(LANG_SEC); %><input type="text" name="second" value="<% getInfo("second"); %>" size="2" maxlength="2">
                </b> </font>
        </td>
	</tr>
	
	<tr><td width="25%"><font size=2> <b><% multilang(LANG_TIME_ZONE_SELECT); %> : </b> </font></td>
	    <td width="75%">
            <select name="timeZone">
	    	<% timeZoneList(); %>
            </select>
	    </td>
	</tr>	
	<tr ><td height=10> </td> </tr>

	<tr><td colspan="2"><font size=2><b>
		<input type="checkbox" name="dst_enabled" 
		value="ON">&nbsp;&nbsp;<% multilang(LANG_ENABLE_DAYLIGHT_SAVING_TIME); %></b></font><br>
	    </td>
	</tr>

	<tr><td colspan="2"><font size=2><b>
		<input type="checkbox" name="enabled" 
		value="ON" 
		ONCLICK=updateState(document.time)>&nbsp;&nbsp;<% multilang(LANG_ENABLE_SNTP_CLIENT_UPDATE); %></b><br>
	    </td>
	</tr>
	<tr>
		<td width="25%"><font size=2><b><% multilang(LANG_WAN_INTERFACE); %>:</b></font></td>
		<td width="75%">
			<select name="ext_if" <% checkWrite("sntp0d"); %>>
				<option value=65535><% multilang(LANG_ANY); %></option>
				<% if_wan_list("rt"); %>
			</select>
		</td>
	</tr>
	<tr>
	<td width ="25%">
	<font size=2> <b> SNTP <% multilang(LANG_SERVER); %> : </b> </font>
	</td>
	<td width ="75%">
		<input type="radio" value="0" name="ntpServerId"></input>
		<select name="ntpServerHost1">
			<option value="192.5.41.41"><% multilang(LANG_192_5_41_41_NORTH_AMERICA); %></option>
			<option value="192.5.41.209"><% multilang(LANG_192_5_41_209_NORTH_AMERICA); %></option>
			<option value="130.149.17.8"><% multilang(LANG_130_149_17_8_EUROPE); %></option>
			<option value="203.117.180.36"><% multilang(LANG_203_117_180_36_ASIA_PACIFIC); %></option>
		</select>
	</td>
	</tr>	
	<tr>
	<td width ="25%"> <font size=2><b> </b></font>
	</td>
	<td width ="75%">
	<input type="radio" value="1" name="ntpServerId"></input>
	<!--ping_zhang:20081217 START:patch from telefonica branch to support WT-107-->
	<input type="text" name="ntpServerHost2" size="15" maxlength="30" value=<% getInfo("ntpServerHost2"); %>> <font size=2> (<% multilang(LANG_MANUAL_SETTING); %>) </font>
		</td>
	</tr>
</table>
  <input type="hidden" value="/tz.asp" name="submit-url">
  <p><input type="submit" value="<% multilang(LANG_APPLY_CHANGES); %>" name="save" onClick="return saveChanges(document.time)">
&nbsp;&nbsp;
  <input type="button" value="<% multilang(LANG_REFRESH); %>" name="refresh" onClick="javascript: window.location.reload()">
</form>
<script>
		<% initPage("ntp"); %>
		setNtpServer(document.time.ntpServerHost1, "<% getInfo("ntpServerHost1"); %>");	
		updateState(document.time);
		
		ifIdx = <% getInfo("ntp-ext-itf"); %>;
		document.time.ext_if.selectedIndex = 0;
		for( i = 1; i < document.time.ext_if.options.length; i++ )
		{
			if( ifIdx == document.time.ext_if.options[i].value )
				document.time.ext_if.selectedIndex = i;
		}
</script>
</blockquote>
</font>
</body>

</html>
