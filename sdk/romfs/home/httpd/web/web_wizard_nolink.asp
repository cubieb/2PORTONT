<VWS_FUNCTION (void*)SendWebMetaStr();>
<VWS_FUNCTION (void*)SendWebCssStr();>
<title>Html Wizard</title>

<SCRIPT>





 var wanphytype = 1;

</SCRIPT>
</head>

<body>
 <div class="data_common data_common_notitle">
  <table>
   <tr class="data_prompt_info">
    <th colspan="2" style="color:red; font-size:20px;">
<script>
</script>
    </th>
   </tr>
   <tr>
    <td colspan="2" class="data_prompt_td_info">
<script>
</script>
    </th>
   </tr>
  </table>
  <table>
   <tr>
    <th width="15%">Device type</th>
    <td>
    <VWS_SCREEN (char*)xscrnRoseModelName[];>
    </td>
    <td rowspan="5" style="text-align:center;">
        <VWS_FUNCTION (void*)getAdslConnJpg();>
        <!--<img src="ADSL_connection.jpg">-->
    </td>
   </tr>
   <tr>
    <th>Firmware version</th>
    <td>
    <VWS_SCREEN (char*)xscrnHwVersion[];>
    </td>
   </tr>
   <tr>
    <th>Software version</th>
    <td>
    <VWS_SCREEN (char*)xscrnAppVersion[];>
    </td>
   </tr>
   <tr>
    <th>Mac address</th>
    <td>
    <VWS_SCREEN (char*)xscrnRoseMAC[];>
    </td>
   </tr>
   <tr>
    <th>Serial number</th>
    <td>
    <VWS_SCREEN (char*)xscrnRoseSerial[];>
    </td>
   </tr>
  </table>
 </div>
 <br>
 <form action="form2RoseWizardNoLink.cgi" method=POST name="RoseWizardNoLink">
 <div class="adsl clearfix btn_center">
  <input class="link_bg" type="submit" name="next" value="Continue">
  <input class="link_bg" type="button" name="close" value="Manual configuration" onClick="top.location.href='login.htm';" >
  <input type="hidden" value="Send" name="submit.htm?rose_wizard_nolink.htm">
 </div>
 </form>
</body>
</html>
