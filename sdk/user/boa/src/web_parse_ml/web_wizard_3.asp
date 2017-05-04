<VWS_FUNCTION (void*)SendWebMetaStr();>
<VWS_FUNCTION (void*)SendWebCssStr();>
<title>Html Wizard</title>
<SCRIPT>
</SCRIPT>
</head>
<body>
<form action="form2RoseWizard3.cgi" method=POST name="RoseWizard3">
 <div class="data_common data_common_notitle">
  <table>
   <tr>
    <th width="25%">Choose branch</th>
    <th>
    <select name="branch" size=1>
        <option value="0">North-west</option>
        <option value="1">Volga</option>
        <option value="2">Ural</option>
    </select>
    </th>
    <td rowspan="3" class="data_prompt_td_info">
     Please select the macro regional branch and the branch where you are.
    </td>
   </tr>
   <tr>
    <th width="25%">Choose city</th>
    <th>
    <select name="city" size=1>
        <option value="0">Nizhny Novgorod</option>
        <option value="1">Yoshkar Ola</option>
        <option value="2">Cheboksary</option>
    </select>
    </th>
   </tr>
   <tr>
    <th width="25%">Choose service</th>
    <th>
    <select name="service" size=1>
        <option value="0">Internet</option>
        <option value="1">Internet + TB</option>
        <option value="2">Internet + VOIP</option>
        <option value="3">Internet + VOIP+TB</option>
    </select>
    </th>
   </tr>
  </table>
 </div>
 <br>
 <div class="adsl clearfix btn_center">
  <input class="link_bg" type="submit" name="continue" value="Continue">
  <input type="hidden" value="Send" name="submit.htm?rose_wizard_3.htm">
 </div>
</form>
</body>
</html>
