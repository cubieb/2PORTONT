<VWS_FUNCTION (void*)SendWebMetaStr();>
<VWS_FUNCTION (void*)SendWebCssStr();>
<title>Html Wizard</title>

<SCRIPT>



<VWS_FUNCTION (void*)getCurrentLinkState();>





 var wanphytype = 1;


function onload()
{
 if(currentLinkState == 0)
 {
  adslAttempts++;
  document.getElementById("adslAttempts").value = adslAttempts;
 }
}

</SCRIPT>
</head>

<body onload="onload()">
<form action="form2RoseTroubleWizard2.cgi" method=POST name="RoseTroubleWizard2">
 <div class="data_common data_common_notitle">
  <table>
   <tr class="data_prompt_info">
    <td colspan="2">
<script>
</script>
     <input type="hidden" id="adslAttempts" name="adslAttempts">
    </td>
   </tr>
  </table>
  <table>
   <tr>
    <td colspan="2" style="text-align:center;">
     <VWS_FUNCTION (void*)getAdslConnJpg();>
    </td>
   </tr>
  </table>
 </div>
 <br>
 <div class="adsl clearfix btn_center">
  <input class="link_bg" type="submit" value="Continue">
  <input class="link_bg" type="button" value="Manual configuration" onClick="top.location.href='index.htm';">
  <input type="hidden" value="Send" name="submit.htm?rose_troublewizard_2.htm">
 </div>
</form>
</body>
</html>
