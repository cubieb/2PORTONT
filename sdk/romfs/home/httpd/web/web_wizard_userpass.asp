<VWS_FUNCTION (void*)SendWebMetaStr();>
<VWS_FUNCTION (void*)SendWebCssStr();>
<title>Html Wizard</title>
<SCRIPT>






 var wanphytype = 1;





function checkPPPSetting()
{
 if(document.RoseWizardUserPass.pppusername.value == "")
 {
  alert("User name cannot be empty!");
  document.RoseWizardUserPass.pppusername.focus();
  return false;
 }
 return true;
}
function onload()
{
}
</SCRIPT>
</head>
<body onload="onload();">
<form action="form2RoseWizardUserPass.cgi" method=POST name="RoseWizardUserPass">
        <div id="pppoesetting">
 <div class="data_common data_common_notitle">
  <table>
   <tr>
    <th width="25%">Enter username</th>
    <th>
    <input type="text" name="pppusername">
    </th>
    <td rowspan="2" class="data_prompt_td_info">
     Please input username and password received from your ISP.
    </td>
   </tr>
   <tr>
    <th width="25%">Enter password</th>
    <th>
    <input type="text" name="ppppassword">
    </th>
   </tr>
  </table>
 </div>
 <br>
 <div class="adsl clearfix btn_center">
  <input class="link_bg" type="submit" name="continue" value="Next" onClick="return checkPPPSetting();">
 </div>
 </div>
 <input type="hidden" value="Send" name="submit.htm?rose_wizard_userpass.htm">
</form>
<SCRIPT>
</SCRIPT>
</body>
</html>
