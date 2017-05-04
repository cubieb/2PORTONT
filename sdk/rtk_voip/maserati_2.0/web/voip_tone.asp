<html>
<head>
<meta http-equiv="Content-Type" content="text/html">
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Cache-Control" CONTENT="no-cache">
<title>SIP</title>
<script language="javascript" src=voip_script.js></script>
<style> TABLE {width:375} </style>
<script language="javascript">
<!--
function ToneType_change()
{
	document.toneform.Freq2.disabled = (document.toneform.type.selectedIndex < 2);
	document.toneform.Freq3.disabled = (document.toneform.type.selectedIndex < 4);
	document.toneform.Gain2.disabled = (document.toneform.type.selectedIndex < 4);
	document.toneform.Gain3.disabled = (document.toneform.type.selectedIndex < 4);

	document.toneform.C1_Freq0.disabled = (document.toneform.type.selectedIndex < 4);
	document.toneform.C1_Freq1.disabled = (document.toneform.type.selectedIndex < 4);
	document.toneform.C1_Freq2.disabled = (document.toneform.type.selectedIndex < 4);
	document.toneform.C1_Freq3.disabled = (document.toneform.type.selectedIndex < 4);
	document.toneform.C1_Gain0.disabled = (document.toneform.type.selectedIndex < 4);
	document.toneform.C1_Gain1.disabled = (document.toneform.type.selectedIndex < 4);
	document.toneform.C1_Gain2.disabled = (document.toneform.type.selectedIndex < 4);
	document.toneform.C1_Gain3.disabled = (document.toneform.type.selectedIndex < 4);

	document.toneform.C2_Freq0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C2_Freq1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C2_Freq2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C2_Freq3.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C2_Gain0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C2_Gain1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C2_Gain2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C2_Gain3.disabled = (document.toneform.type.selectedIndex != 4);

	document.toneform.C3_Freq0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C3_Freq1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C3_Freq2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C3_Freq3.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C3_Gain0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C3_Gain1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C3_Gain2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C3_Gain3.disabled = (document.toneform.type.selectedIndex != 4);

	document.toneform.CadOn2.disabled = (document.toneform.type.selectedIndex > 4);
	document.toneform.CadOn3.disabled = (document.toneform.type.selectedIndex > 4);
	document.toneform.CadOff2.disabled = (document.toneform.type.selectedIndex > 4);
	document.toneform.CadOff3.disabled = (document.toneform.type.selectedIndex > 4);

	document.toneform.CadOn4.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOff4.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOn5.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOff5.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOn6.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOff6.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOn7.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOff7.disabled = (document.toneform.type.selectedIndex != 4);


	document.toneform.C4_Freq0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C4_Freq1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C4_Freq2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C4_Freq3.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C4_Gain0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C4_Gain1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C4_Gain2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C4_Gain3.disabled = (document.toneform.type.selectedIndex != 4);

	document.toneform.C5_Freq0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C5_Freq1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C5_Freq2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C5_Freq3.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C5_Gain0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C5_Gain1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C5_Gain2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C5_Gain3.disabled = (document.toneform.type.selectedIndex != 4);

	document.toneform.C6_Freq0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C6_Freq1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C6_Freq2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C6_Freq3.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C6_Gain0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C6_Gain1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C6_Gain2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C6_Gain3.disabled = (document.toneform.type.selectedIndex != 4);

	document.toneform.C7_Freq0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C7_Freq1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C7_Freq2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C7_Freq3.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C7_Gain0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C7_Gain1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C7_Gain2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C7_Gain3.disabled = (document.toneform.type.selectedIndex != 4);

	document.toneform.CadOn8.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOff8.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOn9.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOff9.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOn10.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOff10.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOn11.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOff11.disabled = (document.toneform.type.selectedIndex != 4);


	document.toneform.C8_Freq0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C8_Freq1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C8_Freq2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C8_Freq3.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C8_Gain0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C8_Gain1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C8_Gain2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C8_Gain3.disabled = (document.toneform.type.selectedIndex != 4);

	document.toneform.C9_Freq0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C9_Freq1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C9_Freq2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C9_Freq3.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C9_Gain0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C9_Gain1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C9_Gain2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C9_Gain3.disabled = (document.toneform.type.selectedIndex != 4);

	document.toneform.C10_Freq0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C10_Freq1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C10_Freq2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C10_Freq3.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C10_Gain0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C10_Gain1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C10_Gain2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C10_Gain3.disabled = (document.toneform.type.selectedIndex != 4);

	document.toneform.C11_Freq0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C11_Freq1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C11_Freq2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C11_Freq3.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C11_Gain0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C11_Gain1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C11_Gain2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C11_Gain3.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOn12.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOff12.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOn13.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOff13.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOn14.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOff14.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOn15.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOff15.disabled = (document.toneform.type.selectedIndex != 4);


	document.toneform.C12_Freq0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C12_Freq1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C12_Freq2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C12_Freq3.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C12_Gain0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C12_Gain1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C12_Gain2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C12_Gain3.disabled = (document.toneform.type.selectedIndex != 4);

	document.toneform.C13_Freq0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C13_Freq1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C13_Freq2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C13_Freq3.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C13_Gain0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C13_Gain1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C13_Gain2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C13_Gain3.disabled = (document.toneform.type.selectedIndex != 4);

	document.toneform.C14_Freq0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C14_Freq1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C14_Freq2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C14_Freq3.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C14_Gain0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C14_Gain1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C14_Gain2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C14_Gain3.disabled = (document.toneform.type.selectedIndex != 4);

	document.toneform.C15_Freq0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C15_Freq1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C15_Freq2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C15_Freq3.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C15_Gain0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C15_Gain1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C15_Gain2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C15_Gain3.disabled = (document.toneform.type.selectedIndex != 4);

	document.toneform.CadOn16.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOff16.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOn17.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOff17.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOn18.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOff18.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOn19.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOff19.disabled = (document.toneform.type.selectedIndex != 4);


	document.toneform.C16_Freq0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C16_Freq1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C16_Freq2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C16_Freq3.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C16_Gain0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C16_Gain1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C16_Gain2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C16_Gain3.disabled = (document.toneform.type.selectedIndex != 4);

	document.toneform.C17_Freq0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C17_Freq1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C17_Freq2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C17_Freq3.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C17_Gain0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C17_Gain1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C17_Gain2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C17_Gain3.disabled = (document.toneform.type.selectedIndex != 4);

	document.toneform.C18_Freq0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C18_Freq1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C18_Freq2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C18_Freq3.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C18_Gain0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C18_Gain1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C18_Gain2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C18_Gain3.disabled = (document.toneform.type.selectedIndex != 4);

	document.toneform.C19_Freq0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C19_Freq1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C19_Freq2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C19_Freq3.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C19_Gain0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C19_Gain1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C19_Gain2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C19_Gain3.disabled = (document.toneform.type.selectedIndex != 4);

	document.toneform.CadOn20.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOff20.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOn21.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOff21.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOn22.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOff22.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOn23.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOff23.disabled = (document.toneform.type.selectedIndex != 4);


	document.toneform.C20_Freq0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C20_Freq1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C20_Freq2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C20_Freq3.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C20_Gain0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C20_Gain1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C20_Gain2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C20_Gain3.disabled = (document.toneform.type.selectedIndex != 4);

	document.toneform.C21_Freq0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C21_Freq1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C21_Freq2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C21_Freq3.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C21_Gain0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C21_Gain1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C21_Gain2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C21_Gain3.disabled = (document.toneform.type.selectedIndex != 4);

	document.toneform.C22_Freq0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C22_Freq1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C22_Freq2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C22_Freq3.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C22_Gain0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C22_Gain1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C22_Gain2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C22_Gain3.disabled = (document.toneform.type.selectedIndex != 4);

	document.toneform.C23_Freq0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C23_Freq1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C23_Freq2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C23_Freq3.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C23_Gain0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C23_Gain1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C23_Gain2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C23_Gain3.disabled = (document.toneform.type.selectedIndex != 4);

	document.toneform.CadOn24.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOff24.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOn25.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOff25.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOn26.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOff26.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOn27.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOff27.disabled = (document.toneform.type.selectedIndex != 4);


	document.toneform.C24_Freq0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C24_Freq1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C24_Freq2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C24_Freq3.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C24_Gain0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C24_Gain1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C24_Gain2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C24_Gain3.disabled = (document.toneform.type.selectedIndex != 4);

	document.toneform.C25_Freq0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C25_Freq1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C25_Freq2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C25_Freq3.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C25_Gain0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C25_Gain1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C25_Gain2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C25_Gain3.disabled = (document.toneform.type.selectedIndex != 4);

	document.toneform.C26_Freq0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C26_Freq1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C26_Freq2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C26_Freq3.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C26_Gain0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C26_Gain1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C26_Gain2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C26_Gain3.disabled = (document.toneform.type.selectedIndex != 4);

	document.toneform.C27_Freq0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C27_Freq1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C27_Freq2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C27_Freq3.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C27_Gain0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C27_Gain1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C27_Gain2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C27_Gain3.disabled = (document.toneform.type.selectedIndex != 4);

	document.toneform.CadOn28.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOff28.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOn29.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOff29.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOn30.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOff30.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOn31.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.CadOff31.disabled = (document.toneform.type.selectedIndex != 4);


	document.toneform.C28_Freq0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C28_Freq1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C28_Freq2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C28_Freq3.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C28_Gain0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C28_Gain1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C28_Gain2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C28_Gain3.disabled = (document.toneform.type.selectedIndex != 4);

	document.toneform.C29_Freq0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C29_Freq1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C29_Freq2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C29_Freq3.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C29_Gain0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C29_Gain1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C29_Gain2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C29_Gain3.disabled = (document.toneform.type.selectedIndex != 4);

	document.toneform.C30_Freq0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C30_Freq1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C30_Freq2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C30_Freq3.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C30_Gain0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C30_Gain1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C30_Gain2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C30_Gain3.disabled = (document.toneform.type.selectedIndex != 4);

	document.toneform.C31_Freq0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C31_Freq1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C31_Freq2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C31_Freq3.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C31_Gain0.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C31_Gain1.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C31_Gain2.disabled = (document.toneform.type.selectedIndex != 4);
	document.toneform.C31_Gain3.disabled = (document.toneform.type.selectedIndex != 4);


}

function D1FreqNum_change()
{
	document.toneform.d1Freq2.disabled = (document.toneform.d1freqnum.selectedIndex == 0);

}

function D2FreqNum_change()
{
	document.toneform.d2Freq2.disabled = (document.toneform.d2freqnum.selectedIndex == 0);

}



function InitToneConfig()
{

	ToneType_change();
	D1FreqNum_change();
	D2FreqNum_change();
}

//-->
</script>
</head>
<body bgcolor="#ffffff" text="#000000" onload="InitToneConfig()">

<form method="post" action="/boaform/voip_tone_set" name=toneform>
<b><% multilang(LANG_SELECT_COUNTRY); %></b>
<table cellSpacing=1 cellPadding=2 border=0 width=400>

  	<tr>
    	<td bgColor=#aaddff><% multilang(LANG_COUNTRY); %></td>
		<td bgColor=#ddeeff>
		<select name=tone_country onchange="changeCountry();">
			 "<%voip_tone_get("tone_country");%>"
		</select>
		</td>

	</tr>	


</table>

<div id = tonetable <%voip_tone_get("display");%> >
<b>Select Tone</b>
<table cellSpacing=1 cellPadding=2 border=0 width=400>
  	<tr>
    	<td bgColor=#aaddff width=155>DIAL TONE</td>
    	<td bgColor=#ddeeff width=170>
			<select name=dial>
			 	<%voip_tone_get("dial");%>
			</select>
		</td>
	</tr>
  	<tr>
    	<td bgColor=#aaddff width=155>RING TONE</td>
    	<td bgColor=#ddeeff width=170>
			<select name=ring>
			 	<%voip_tone_get("ring");%>
			</select>
		</td>
	</tr>
  	<tr>
    	<td bgColor=#aaddff width=155>BUSY TONE</td>
    	<td bgColor=#ddeeff width=170>
			<select name=busy>
			 	<%voip_tone_get("busy");%>
			</select>
		</td>
	</tr>
  	<tr>
    	<td bgColor=#aaddff width=155>WAITING TONE</td>
    	<td bgColor=#ddeeff width=170>
			<select name=waiting>
			 	<%voip_tone_get("waiting");%>
			</select>
		</td>
	</tr>
</table>
</div>
<table cellSpacing=1 cellPadding=2 border=0 width=400>

	<tr>
    	<td colspan=3 align=center>
    		<input type="submit" name="Country" value="<% multilang(LANG_APPLY); %>" >
   	</td>
	</tr>
</table>

<b><% multilang(LANG_SELECT_CUSTOM_TONE); %></b>
<table cellSpacing=1 cellPadding=2 border=0 width=400>
  	<tr>
    	<td bgColor=#aaddff width=155><% multilang(LANG_CUSTOM_TONE); %></td>
    	<td bgColor=#ddeeff width=170>
			<select name=selfItem  onChange="toneform.submit()">
			 	<%voip_tone_get("selfItem");%>
			</select>
		</td>
	</tr>
</table>

<p>
<b><% multilang(LANG_TONE_PARAMETERS); %></b>
<table cellSpacing=1 cellPadding=2 border=0 width=400>
	<tr>
	<td bgColor=#aaddff><% multilang(LANG_TONETYPE); %></td>
	<td bgColor=#ddeeff>
		<select name=type onchange="ToneType_change()">
			 <%voip_tone_get("type");%>
		</select>
	</td>
	</tr>
	
	<tr>

		<td bgColor=#aaddff rowspan=3><% multilang(LANG_TONECYCLE); %></td>

<!--
		<td bgColor=#aaddff>ToneCycle</td>
		<td bgColor=#ddeeff>
			<select name=cycle>
			 	<%voip_tone_get("cycle");%>
			</select>

		</td>
-->
	</tr>


	<tr>
		<td bgColor=#ddeeff><% multilang(LANG_CYCLE_NO); %>: 	
		<input type=text name=cycle size=10 maxlength=5 value="<%voip_tone_get("cycle");%>">
		</td>	
	</tr>

	
	<tr>
		<td bgColor=#ddeeff><font size=1><% multilang(LANG_0_CYCLE_IS_CONTINUOUS); %></font>
		</td>	
	</tr>
	
	<tr>
		<td bgColor=#aaddff>CadNum</td>	
    	<td bgColor=#ddeeff>
		<input type=text name=cadNUM size=10 maxlength=2 value=<%voip_tone_get("cadNUM");%>> <font size=1>(<% multilang(LANG_1_4_FOR_ADDITIVE_MODULATED_SUCC_SUCC_ADD_1_32_FOR_FOUR_FREQ_2_FOR_STEP_INC_TWO_STEP); %>)</font></td>
	</tr>

   <tr>
		<td  bgColor=#aaddff>CadOn0</td>
    	<td bgColor=#ddeeff><input type=text name=CadOn0 size=20 maxlength=9 value="<%voip_tone_get("CadOn0");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
	
   <tr>
		<td  bgColor=#aaddff>CadOff0</td>
    	<td bgColor=#ddeeff><input type=text name=CadOff0 size=20 maxlength=9 value="<%voip_tone_get("CadOff0");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
	
   <tr>
		<td  bgColor=#aaddff>CadOn1</td>
    	<td bgColor=#ddeeff><input type=text name=CadOn1 size=20 maxlength=9 value="<%voip_tone_get("CadOn1");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOff1</td>
    	<td bgColor=#ddeeff><input type=text name=CadOff1 size=20 maxlength=9 value="<%voip_tone_get("CadOff1");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOn2</td>
    	<td bgColor=#ddeeff><input type=text name=CadOn2 size=20 maxlength=9 value="<%voip_tone_get("CadOn2");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOff2</td>
    	<td bgColor=#ddeeff><input type=text name=CadOff2 size=20 maxlength=9 value="<%voip_tone_get("CadOff2");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOn3</td>
    	<td bgColor=#ddeeff><input type=text name=CadOn3 size=20 maxlength=9 value="<%voip_tone_get("CadOn3");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>

   <tr>
		<td  bgColor=#aaddff>CadOff3</td>
    	<td bgColor=#ddeeff><input type=text name=CadOff3 size=20 maxlength=9 value="<%voip_tone_get("CadOff3");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
	
<!--
   	<tr>
    	<td bgColor=#aaddff>PatternOff</td>
    	<td bgColor=#ddeeff><input type=text name=PatternOff size=20 maxlength=9 value="<%voip_tone_get("PatternOff"); %>">	</td>
	</tr>
	<tr>
-->
	<tr>	
	<td bgColor=#aaddff><% multilang(LANG_TONENUM);%></td>	
    	<td bgColor=#ddeeff><input type=text name=ToneNUM size=1 maxlength=1 value=<%voip_tone_get("ToneNUM");%> > <font size=1>(1~4)</font>
		</td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff><% multilang(LANG_FREQ);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=Freq0 size=20 maxlength=4 value="<%voip_tone_get("Freq0");%>"> <font size=1>(Hz)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff><% multilang(LANG_FREQ);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=Freq1 size=20 maxlength=4 value="<%voip_tone_get("Freq1");%>"> <font size=1>(Hz)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff rowspan=2><% multilang(LANG_FREQ);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=Freq2 size=20 maxlength=4 value="<%voip_tone_get("Freq2");%>"> <font size=1>(Hz)</font>
	</td>

	</tr>
	
	<tr>
		<td bgColor=#ddeeff><font size=1><% multilang(LANG_FREQ2_IS_SUPPORTED_WHEN_SUCC_FOUR_FREQ_STEP_INC_TWO_STEP_TONE_TYPE);%></font>
		</td>	
	</tr>

   	<tr>
    	<td bgColor=#aaddff rowspan=2><% multilang(LANG_FREQ);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=Freq3 size=20 maxlength=4 value="<%voip_tone_get("Freq3");%>"> <font size=1>(Hz)</font>
	</td>
	</tr>

	<tr>
		<td bgColor=#ddeeff><font size=1><% multilang(LANG_FREQ3_IS_SUPPORTED_WHEN_FOUR_FREQ_STEP_INC_TWO_STEP_TONE_TYPE);%></font>
		</td>	
	</tr>


   	<tr>
    	<td bgColor=#aaddff><% multilang(LANG_GAIN);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=Gain0 size=20 maxlength=2 value="<%voip_tone_get("Gain0");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff><% multilang(LANG_GAIN);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=Gain1 size=20 maxlength=2 value="<%voip_tone_get("Gain1");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff rowspan=2><% multilang(LANG_GAIN);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=Gain2 size=20 maxlength=2 value="<%voip_tone_get("Gain2");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>
	<tr>
		<td bgColor=#ddeeff><font size=1><% multilang(LANG_GAIN2_IS_SUPPORTED_WHEN_FOUR_FREQ_STEP_INC_TWO_STEP_TONE_TYPE);%></font>
		</td>	
	</tr>
   	<tr>
    	<td bgColor=#aaddff rowspan=2><% multilang(LANG_GAIN);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=Gain3 size=20 maxlength=2 value="<%voip_tone_get("Gain3");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>
	<tr>
		<td bgColor=#ddeeff><font size=1><% multilang(LANG_GAIN3_IS_SUPPORTED_WHEN_FOUR_FREQ_STEP_INC_TWO_STEP_TONE_TYPE);%></font>
		</td>	
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C1_<% multilang(LANG_FREQ);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C1_Freq0 size=20 maxlength=4 value="<%voip_tone_get("C1_Freq0");%>"> <font size=1>(Hz)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C1_<% multilang(LANG_FREQ);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C1_Freq1 size=20 maxlength=4 value="<%voip_tone_get("C1_Freq1");%>"> <font size=1>(Hz)</font></td>
	</tr>
	
   	<tr>
    	<td bgColor=#aaddff>C1_<% multilang(LANG_FREQ);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C1_Freq2 size=20 maxlength=4 value="<%voip_tone_get("C1_Freq2");%>"> <font size=1>(Hz)</font></td>
	</tr>
 	<tr>
    	<td bgColor=#aaddff>C1_<% multilang(LANG_FREQ);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C1_Freq3 size=20 maxlength=4 value="<%voip_tone_get("C1_Freq3");%>"> <font size=1>(Hz)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C1_<% multilang(LANG_GAIN);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C1_Gain0 size=20 maxlength=2 value="<%voip_tone_get("C1_Gain0");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C1_<% multilang(LANG_GAIN);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C1_Gain1 size=20 maxlength=2 value="<%voip_tone_get("C1_Gain1");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C1_<% multilang(LANG_GAIN);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C1_Gain2 size=20 maxlength=2 value="<%voip_tone_get("C1_Gain2");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C1_<% multilang(LANG_GAIN);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C1_Gain3 size=20 maxlength=2 value="<%voip_tone_get("C1_Gain3");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C2_<% multilang(LANG_FREQ);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C2_Freq0 size=20 maxlength=4 value="<%voip_tone_get("C2_Freq0");%>"> <font size=1>(Hz)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C2_<% multilang(LANG_FREQ);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C2_Freq1 size=20 maxlength=4 value="<%voip_tone_get("C2_Freq1");%>"> <font size=1>(Hz)</font></td>
	</tr>
	
   	<tr>
    	<td bgColor=#aaddff>C2_<% multilang(LANG_FREQ);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C2_Freq2 size=20 maxlength=4 value="<%voip_tone_get("C2_Freq2");%>"> <font size=1>(Hz)</font></td>
	</tr>
 	<tr>
    	<td bgColor=#aaddff>C2_<% multilang(LANG_FREQ);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C2_Freq3 size=20 maxlength=4 value="<%voip_tone_get("C2_Freq3");%>"> <font size=1>(Hz)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C2_<% multilang(LANG_GAIN);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C2_Gain0 size=20 maxlength=2 value="<%voip_tone_get("C2_Gain0");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C2_<% multilang(LANG_GAIN);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C2_Gain1 size=20 maxlength=2 value="<%voip_tone_get("C2_Gain1");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C2_<% multilang(LANG_GAIN);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C2_Gain2 size=20 maxlength=2 value="<%voip_tone_get("C2_Gain2");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C2_<% multilang(LANG_GAIN);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C2_Gain3 size=20 maxlength=2 value="<%voip_tone_get("C2_Gain3");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>	

   	<tr>
    	<td bgColor=#aaddff>C3_<% multilang(LANG_FREQ);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C3_Freq0 size=20 maxlength=4 value="<%voip_tone_get("C3_Freq0");%>"> <font size=1>(Hz)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C3_<% multilang(LANG_FREQ);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C3_Freq1 size=20 maxlength=4 value="<%voip_tone_get("C3_Freq1");%>"> <font size=1>(Hz)</font></td>
	</tr>
	
   	<tr>
    	<td bgColor=#aaddff>C3_<% multilang(LANG_FREQ);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C3_Freq2 size=20 maxlength=4 value="<%voip_tone_get("C3_Freq2");%>"> <font size=1>(Hz)</font></td>
	</tr>
 	<tr>
    	<td bgColor=#aaddff>C3_<% multilang(LANG_FREQ);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C3_Freq3 size=20 maxlength=4 value="<%voip_tone_get("C3_Freq3");%>"> <font size=1>(Hz)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C3_<% multilang(LANG_GAIN);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C3_Gain0 size=20 maxlength=2 value="<%voip_tone_get("C3_Gain0");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C3_<% multilang(LANG_GAIN);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C3_Gain1 size=20 maxlength=2 value="<%voip_tone_get("C3_Gain1");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C3_<% multilang(LANG_GAIN);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C3_Gain2 size=20 maxlength=2 value="<%voip_tone_get("C3_Gain2");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C3_<% multilang(LANG_GAIN);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C3_Gain3 size=20 maxlength=2 value="<%voip_tone_get("C3_Gain3");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>

   <tr>
		<td  bgColor=#aaddff>CadOn4</td>
    	<td bgColor=#ddeeff><input type=text name=CadOn4 size=20 maxlength=9 value="<%voip_tone_get("CadOn4");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
	
   <tr>
		<td  bgColor=#aaddff>CadOff4</td>
    	<td bgColor=#ddeeff><input type=text name=CadOff4 size=20 maxlength=9 value="<%voip_tone_get("CadOff4");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
	
   <tr>
		<td  bgColor=#aaddff>CadOn5</td>
    	<td bgColor=#ddeeff><input type=text name=CadOn5 size=20 maxlength=9 value="<%voip_tone_get("CadOn5");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOff5</td>
    	<td bgColor=#ddeeff><input type=text name=CadOff5 size=20 maxlength=9 value="<%voip_tone_get("CadOff5");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOn6</td>
    	<td bgColor=#ddeeff><input type=text name=CadOn6 size=20 maxlength=9 value="<%voip_tone_get("CadOn6");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOff6</td>
    	<td bgColor=#ddeeff><input type=text name=CadOff6 size=20 maxlength=9 value="<%voip_tone_get("CadOff6");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOn7</td>
    	<td bgColor=#ddeeff><input type=text name=CadOn7 size=20 maxlength=9 value="<%voip_tone_get("CadOn7");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>

   <tr>
		<td  bgColor=#aaddff>CadOff7</td>
    	<td bgColor=#ddeeff><input type=text name=CadOff7 size=20 maxlength=9 value="<%voip_tone_get("CadOff7");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C4_<% multilang(LANG_FREQ);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C4_Freq0 size=20 maxlength=4 value="<%voip_tone_get("C4_Freq0");%>"> <font size=1>(Hz)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C4_<% multilang(LANG_FREQ);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C4_Freq1 size=20 maxlength=4 value="<%voip_tone_get("C4_Freq1");%>"> <font size=1>(Hz)</font></td>
	</tr>
	
   	<tr>
    	<td bgColor=#aaddff>C4_<% multilang(LANG_FREQ);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C4_Freq2 size=20 maxlength=4 value="<%voip_tone_get("C4_Freq2");%>"> <font size=1>(Hz)</font></td>
	</tr>
 	<tr>
    	<td bgColor=#aaddff>C4_<% multilang(LANG_FREQ);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C4_Freq3 size=20 maxlength=4 value="<%voip_tone_get("C4_Freq3");%>"> <font size=1>(Hz)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C4_<% multilang(LANG_GAIN);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C4_Gain0 size=20 maxlength=2 value="<%voip_tone_get("C4_Gain0");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C4_<% multilang(LANG_GAIN);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C4_Gain1 size=20 maxlength=2 value="<%voip_tone_get("C4_Gain1");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C4_<% multilang(LANG_GAIN);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C4_Gain2 size=20 maxlength=2 value="<%voip_tone_get("C4_Gain2");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C4_<% multilang(LANG_GAIN);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C4_Gain3 size=20 maxlength=2 value="<%voip_tone_get("C4_Gain3");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C5_<% multilang(LANG_FREQ);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C5_Freq0 size=20 maxlength=4 value="<%voip_tone_get("C5_Freq0");%>"> <font size=1>(Hz)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C5_<% multilang(LANG_FREQ);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C5_Freq1 size=20 maxlength=4 value="<%voip_tone_get("C5_Freq1");%>"> <font size=1>(Hz)</font></td>
	</tr>
	
   	<tr>
    	<td bgColor=#aaddff>C5_<% multilang(LANG_FREQ);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C5_Freq2 size=20 maxlength=4 value="<%voip_tone_get("C5_Freq2");%>"> <font size=1>(Hz)</font></td>
	</tr>
 	<tr>
    	<td bgColor=#aaddff>C5_<% multilang(LANG_FREQ);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C5_Freq3 size=20 maxlength=4 value="<%voip_tone_get("C5_Freq3");%>"> <font size=1>(Hz)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C5_<% multilang(LANG_GAIN);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C5_Gain0 size=20 maxlength=2 value="<%voip_tone_get("C5_Gain0");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C5_<% multilang(LANG_GAIN);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C5_Gain1 size=20 maxlength=2 value="<%voip_tone_get("C5_Gain1");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C5_<% multilang(LANG_GAIN);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C5_Gain2 size=20 maxlength=2 value="<%voip_tone_get("C5_Gain2");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C5_<% multilang(LANG_GAIN);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C5_Gain3 size=20 maxlength=2 value="<%voip_tone_get("C5_Gain3");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C6_<% multilang(LANG_FREQ);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C6_Freq0 size=20 maxlength=4 value="<%voip_tone_get("C6_Freq0");%>"> <font size=1>(Hz)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C6_<% multilang(LANG_FREQ);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C6_Freq1 size=20 maxlength=4 value="<%voip_tone_get("C6_Freq1");%>"> <font size=1>(Hz)</font></td>
	</tr>
	
   	<tr>
    	<td bgColor=#aaddff>C6_<% multilang(LANG_FREQ);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C6_Freq2 size=20 maxlength=4 value="<%voip_tone_get("C6_Freq2");%>"> <font size=1>(Hz)</font></td>
	</tr>
 	<tr>
    	<td bgColor=#aaddff>C6_<% multilang(LANG_FREQ);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C6_Freq3 size=20 maxlength=4 value="<%voip_tone_get("C6_Freq3");%>"> <font size=1>(Hz)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C6_<% multilang(LANG_GAIN);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C6_Gain0 size=20 maxlength=2 value="<%voip_tone_get("C6_Gain0");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C6_<% multilang(LANG_GAIN);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C6_Gain1 size=20 maxlength=2 value="<%voip_tone_get("C6_Gain1");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C6_<% multilang(LANG_GAIN);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C6_Gain2 size=20 maxlength=2 value="<%voip_tone_get("C6_Gain2");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C6_<% multilang(LANG_GAIN);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C6_Gain3 size=20 maxlength=2 value="<%voip_tone_get("C6_Gain3");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C7_<% multilang(LANG_FREQ);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C7_Freq0 size=20 maxlength=4 value="<%voip_tone_get("C7_Freq0");%>"> <font size=1>(Hz)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C7_<% multilang(LANG_FREQ);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C7_Freq1 size=20 maxlength=4 value="<%voip_tone_get("C7_Freq1");%>"> <font size=1>(Hz)</font></td>
	</tr>
	
   	<tr>
    	<td bgColor=#aaddff>C7_<% multilang(LANG_FREQ);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C7_Freq2 size=20 maxlength=4 value="<%voip_tone_get("C7_Freq2");%>"> <font size=1>(Hz)</font></td>
	</tr>
 	<tr>
    	<td bgColor=#aaddff>C7_<% multilang(LANG_FREQ);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C7_Freq3 size=20 maxlength=4 value="<%voip_tone_get("C7_Freq3");%>"> <font size=1>(Hz)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C7_<% multilang(LANG_GAIN);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C7_Gain0 size=20 maxlength=2 value="<%voip_tone_get("C7_Gain0");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C7_<% multilang(LANG_GAIN);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C7_Gain1 size=20 maxlength=2 value="<%voip_tone_get("C7_Gain1");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C7_<% multilang(LANG_GAIN);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C7_Gain2 size=20 maxlength=2 value="<%voip_tone_get("C7_Gain2");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C7_<% multilang(LANG_GAIN);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C7_Gain3 size=20 maxlength=2 value="<%voip_tone_get("C7_Gain3");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOn8</td>
    	<td bgColor=#ddeeff><input type=text name=CadOn8 size=20 maxlength=9 value="<%voip_tone_get("CadOn8");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
	
   <tr>
		<td  bgColor=#aaddff>CadOff8</td>
    	<td bgColor=#ddeeff><input type=text name=CadOff8 size=20 maxlength=9 value="<%voip_tone_get("CadOff8");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
	
   <tr>
		<td  bgColor=#aaddff>CadOn9</td>
    	<td bgColor=#ddeeff><input type=text name=CadOn9 size=20 maxlength=9 value="<%voip_tone_get("CadOn9");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOff9</td>
    	<td bgColor=#ddeeff><input type=text name=CadOff9 size=20 maxlength=9 value="<%voip_tone_get("CadOff9");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOn10</td>
    	<td bgColor=#ddeeff><input type=text name=CadOn10 size=20 maxlength=9 value="<%voip_tone_get("CadOn10");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOff10</td>
    	<td bgColor=#ddeeff><input type=text name=CadOff10 size=20 maxlength=9 value="<%voip_tone_get("CadOff10");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOn11</td>
    	<td bgColor=#ddeeff><input type=text name=CadOn11 size=20 maxlength=9 value="<%voip_tone_get("CadOn11");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>

   <tr>
		<td  bgColor=#aaddff>CadOff11</td>
    	<td bgColor=#ddeeff><input type=text name=CadOff11 size=20 maxlength=9 value="<%voip_tone_get("CadOff11");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C8_<% multilang(LANG_FREQ);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C8_Freq0 size=20 maxlength=4 value="<%voip_tone_get("C8_Freq0");%>"> <font size=1>(Hz)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C8_<% multilang(LANG_FREQ);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C8_Freq1 size=20 maxlength=4 value="<%voip_tone_get("C8_Freq1");%>"> <font size=1>(Hz)</font></td>
	</tr>
	
   	<tr>
    	<td bgColor=#aaddff>C8_<% multilang(LANG_FREQ);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C8_Freq2 size=20 maxlength=4 value="<%voip_tone_get("C8_Freq2");%>"> <font size=1>(Hz)</font></td>
	</tr>
 	<tr>
    	<td bgColor=#aaddff>C8_<% multilang(LANG_FREQ);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C8_Freq3 size=20 maxlength=4 value="<%voip_tone_get("C8_Freq3");%>"> <font size=1>(Hz)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C8_<% multilang(LANG_GAIN);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C8_Gain0 size=20 maxlength=2 value="<%voip_tone_get("C8_Gain0");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C8_<% multilang(LANG_GAIN);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C8_Gain1 size=20 maxlength=2 value="<%voip_tone_get("C8_Gain1");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C8_<% multilang(LANG_GAIN);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C8_Gain2 size=20 maxlength=2 value="<%voip_tone_get("C8_Gain2");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C8_<% multilang(LANG_GAIN);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C8_Gain3 size=20 maxlength=2 value="<%voip_tone_get("C8_Gain3");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C9_<% multilang(LANG_FREQ);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C9_Freq0 size=20 maxlength=4 value="<%voip_tone_get("C9_Freq0");%>"> <font size=1>(Hz)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C9_<% multilang(LANG_FREQ);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C9_Freq1 size=20 maxlength=4 value="<%voip_tone_get("C9_Freq1");%>"> <font size=1>(Hz)</font></td>
	</tr>
	
   	<tr>
    	<td bgColor=#aaddff>C9_<% multilang(LANG_FREQ);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C9_Freq2 size=20 maxlength=4 value="<%voip_tone_get("C9_Freq2");%>"> <font size=1>(Hz)</font></td>
	</tr>
 	<tr>
    	<td bgColor=#aaddff>C9_<% multilang(LANG_FREQ);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C9_Freq3 size=20 maxlength=4 value="<%voip_tone_get("C9_Freq3");%>"> <font size=1>(Hz)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C9_<% multilang(LANG_GAIN);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C9_Gain0 size=20 maxlength=2 value="<%voip_tone_get("C9_Gain0");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C9_<% multilang(LANG_GAIN);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C9_Gain1 size=20 maxlength=2 value="<%voip_tone_get("C9_Gain1");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C9_<% multilang(LANG_GAIN);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C9_Gain2 size=20 maxlength=2 value="<%voip_tone_get("C9_Gain2");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C9_<% multilang(LANG_GAIN);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C9_Gain3 size=20 maxlength=2 value="<%voip_tone_get("C9_Gain3");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C10_<% multilang(LANG_FREQ);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C10_Freq0 size=20 maxlength=4 value="<%voip_tone_get("C10_Freq0");%>"> <font size=1>(Hz)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C10_<% multilang(LANG_FREQ);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C10_Freq1 size=20 maxlength=4 value="<%voip_tone_get("C10_Freq1");%>"> <font size=1>(Hz)</font></td>
	</tr>
	
   	<tr>
    	<td bgColor=#aaddff>C10_<% multilang(LANG_FREQ);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C10_Freq2 size=20 maxlength=4 value="<%voip_tone_get("C10_Freq2");%>"> <font size=1>(Hz)</font></td>
	</tr>
 	<tr>
    	<td bgColor=#aaddff>C10_<% multilang(LANG_FREQ);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C10_Freq3 size=20 maxlength=4 value="<%voip_tone_get("C10_Freq3");%>"> <font size=1>(Hz)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C10_<% multilang(LANG_GAIN);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C10_Gain0 size=20 maxlength=2 value="<%voip_tone_get("C10_Gain0");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C10_<% multilang(LANG_GAIN);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C10_Gain1 size=20 maxlength=2 value="<%voip_tone_get("C10_Gain1");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C10_<% multilang(LANG_GAIN);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C10_Gain2 size=20 maxlength=2 value="<%voip_tone_get("C10_Gain2");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C10_<% multilang(LANG_GAIN);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C10_Gain3 size=20 maxlength=2 value="<%voip_tone_get("C10_Gain3");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C11_<% multilang(LANG_FREQ);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C11_Freq0 size=20 maxlength=4 value="<%voip_tone_get("C11_Freq0");%>"> <font size=1>(Hz)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C11_<% multilang(LANG_FREQ);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C11_Freq1 size=20 maxlength=4 value="<%voip_tone_get("C11_Freq1");%>"> <font size=1>(Hz)</font></td>
	</tr>
	
   	<tr>
    	<td bgColor=#aaddff>C11_<% multilang(LANG_FREQ);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C11_Freq2 size=20 maxlength=4 value="<%voip_tone_get("C11_Freq2");%>"> <font size=1>(Hz)</font></td>
	</tr>
 	<tr>
    	<td bgColor=#aaddff>C11_<% multilang(LANG_FREQ);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C11_Freq3 size=20 maxlength=4 value="<%voip_tone_get("C11_Freq3");%>"> <font size=1>(Hz)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C11_<% multilang(LANG_GAIN);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C11_Gain0 size=20 maxlength=2 value="<%voip_tone_get("C11_Gain0");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C11_<% multilang(LANG_GAIN);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C11_Gain1 size=20 maxlength=2 value="<%voip_tone_get("C11_Gain1");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C11_<% multilang(LANG_GAIN);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C11_Gain2 size=20 maxlength=2 value="<%voip_tone_get("C11_Gain2");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C11_<% multilang(LANG_GAIN);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C11_Gain3 size=20 maxlength=2 value="<%voip_tone_get("C11_Gain3");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOn12</td>
    	<td bgColor=#ddeeff><input type=text name=CadOn12 size=20 maxlength=9 value="<%voip_tone_get("CadOn12");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
	
   <tr>
		<td  bgColor=#aaddff>CadOff12</td>
    	<td bgColor=#ddeeff><input type=text name=CadOff12 size=20 maxlength=9 value="<%voip_tone_get("CadOff12");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
	
   <tr>
		<td  bgColor=#aaddff>CadOn13</td>
    	<td bgColor=#ddeeff><input type=text name=CadOn13 size=20 maxlength=9 value="<%voip_tone_get("CadOn13");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOff13</td>
    	<td bgColor=#ddeeff><input type=text name=CadOff13 size=20 maxlength=9 value="<%voip_tone_get("CadOff13");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOn14</td>
    	<td bgColor=#ddeeff><input type=text name=CadOn14 size=20 maxlength=9 value="<%voip_tone_get("CadOn14");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOff14</td>
    	<td bgColor=#ddeeff><input type=text name=CadOff14 size=20 maxlength=9 value="<%voip_tone_get("CadOff14");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOn15</td>
    	<td bgColor=#ddeeff><input type=text name=CadOn15 size=20 maxlength=9 value="<%voip_tone_get("CadOn15");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>

   <tr>
		<td  bgColor=#aaddff>CadOff15</td>
    	<td bgColor=#ddeeff><input type=text name=CadOff15 size=20 maxlength=9 value="<%voip_tone_get("CadOff15");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C12_<% multilang(LANG_FREQ);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C12_Freq0 size=20 maxlength=4 value="<%voip_tone_get("C12_Freq0");%>"> <font size=1>(Hz)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C12_<% multilang(LANG_FREQ);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C12_Freq1 size=20 maxlength=4 value="<%voip_tone_get("C12_Freq1");%>"> <font size=1>(Hz)</font></td>
	</tr>
	
   	<tr>
    	<td bgColor=#aaddff>C12_<% multilang(LANG_FREQ);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C12_Freq2 size=20 maxlength=4 value="<%voip_tone_get("C12_Freq2");%>"> <font size=1>(Hz)</font></td>
	</tr>
 	<tr>
    	<td bgColor=#aaddff>C12_<% multilang(LANG_FREQ);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C12_Freq3 size=20 maxlength=4 value="<%voip_tone_get("C12_Freq3");%>"> <font size=1>(Hz)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C12_<% multilang(LANG_GAIN);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C12_Gain0 size=20 maxlength=2 value="<%voip_tone_get("C12_Gain0");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C12_<% multilang(LANG_GAIN);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C12_Gain1 size=20 maxlength=2 value="<%voip_tone_get("C12_Gain1");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C12_<% multilang(LANG_GAIN);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C12_Gain2 size=20 maxlength=2 value="<%voip_tone_get("C12_Gain2");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C12_<% multilang(LANG_GAIN);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C12_Gain3 size=20 maxlength=2 value="<%voip_tone_get("C12_Gain3");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C13_<% multilang(LANG_FREQ);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C13_Freq0 size=20 maxlength=4 value="<%voip_tone_get("C13_Freq0");%>"> <font size=1>(Hz)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C13_<% multilang(LANG_FREQ);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C13_Freq1 size=20 maxlength=4 value="<%voip_tone_get("C13_Freq1");%>"> <font size=1>(Hz)</font></td>
	</tr>
	
   	<tr>
    	<td bgColor=#aaddff>C13_<% multilang(LANG_FREQ);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C13_Freq2 size=20 maxlength=4 value="<%voip_tone_get("C13_Freq2");%>"> <font size=1>(Hz)</font></td>
	</tr>
 	<tr>
    	<td bgColor=#aaddff>C13_<% multilang(LANG_FREQ);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C13_Freq3 size=20 maxlength=4 value="<%voip_tone_get("C13_Freq3");%>"> <font size=1>(Hz)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C13_<% multilang(LANG_GAIN);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C13_Gain0 size=20 maxlength=2 value="<%voip_tone_get("C13_Gain0");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C13_<% multilang(LANG_GAIN);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C13_Gain1 size=20 maxlength=2 value="<%voip_tone_get("C13_Gain1");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C13_<% multilang(LANG_GAIN);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C13_Gain2 size=20 maxlength=2 value="<%voip_tone_get("C13_Gain2");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C13_<% multilang(LANG_GAIN);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C13_Gain3 size=20 maxlength=2 value="<%voip_tone_get("C13_Gain3");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C14_<% multilang(LANG_FREQ);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C14_Freq0 size=20 maxlength=4 value="<%voip_tone_get("C14_Freq0");%>"> <font size=1>(Hz)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C14_<% multilang(LANG_FREQ);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C14_Freq1 size=20 maxlength=4 value="<%voip_tone_get("C14_Freq1");%>"> <font size=1>(Hz)</font></td>
	</tr>
	
   	<tr>
    	<td bgColor=#aaddff>C14_<% multilang(LANG_FREQ);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C14_Freq2 size=20 maxlength=4 value="<%voip_tone_get("C14_Freq2");%>"> <font size=1>(Hz)</font></td>
	</tr>
 	<tr>
    	<td bgColor=#aaddff>C14_<% multilang(LANG_FREQ);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C14_Freq3 size=20 maxlength=4 value="<%voip_tone_get("C14_Freq3");%>"> <font size=1>(Hz)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C14_<% multilang(LANG_GAIN);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C14_Gain0 size=20 maxlength=2 value="<%voip_tone_get("C14_Gain0");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C14_<% multilang(LANG_GAIN);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C14_Gain1 size=20 maxlength=2 value="<%voip_tone_get("C14_Gain1");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C14_<% multilang(LANG_GAIN);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C14_Gain2 size=20 maxlength=2 value="<%voip_tone_get("C14_Gain2");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C14_<% multilang(LANG_GAIN);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C14_Gain3 size=20 maxlength=2 value="<%voip_tone_get("C14_Gain3");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C15_<% multilang(LANG_FREQ);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C15_Freq0 size=20 maxlength=4 value="<%voip_tone_get("C15_Freq0");%>"> <font size=1>(Hz)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C15_<% multilang(LANG_FREQ);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C15_Freq1 size=20 maxlength=4 value="<%voip_tone_get("C15_Freq1");%>"> <font size=1>(Hz)</font></td>
	</tr>
	
   	<tr>
    	<td bgColor=#aaddff>C15_<% multilang(LANG_FREQ);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C15_Freq2 size=20 maxlength=4 value="<%voip_tone_get("C15_Freq2");%>"> <font size=1>(Hz)</font></td>
	</tr>
 	<tr>
    	<td bgColor=#aaddff>C15_<% multilang(LANG_FREQ);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C15_Freq3 size=20 maxlength=4 value="<%voip_tone_get("C15_Freq3");%>"> <font size=1>(Hz)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C15_<% multilang(LANG_GAIN);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C15_Gain0 size=20 maxlength=2 value="<%voip_tone_get("C15_Gain0");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C15_<% multilang(LANG_GAIN);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C15_Gain1 size=20 maxlength=2 value="<%voip_tone_get("C15_Gain1");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C15_<% multilang(LANG_GAIN);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C15_Gain2 size=20 maxlength=2 value="<%voip_tone_get("C15_Gain2");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C15_<% multilang(LANG_GAIN);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C15_Gain3 size=20 maxlength=2 value="<%voip_tone_get("C15_Gain3");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOn16</td>
    	<td bgColor=#ddeeff><input type=text name=CadOn16 size=20 maxlength=9 value="<%voip_tone_get("CadOn16");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
	
   <tr>
		<td  bgColor=#aaddff>CadOff16</td>
    	<td bgColor=#ddeeff><input type=text name=CadOff16 size=20 maxlength=9 value="<%voip_tone_get("CadOff16");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
	
   <tr>
		<td  bgColor=#aaddff>CadOn17</td>
    	<td bgColor=#ddeeff><input type=text name=CadOn17 size=20 maxlength=9 value="<%voip_tone_get("CadOn17");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOff17</td>
    	<td bgColor=#ddeeff><input type=text name=CadOff17 size=20 maxlength=9 value="<%voip_tone_get("CadOff17");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOn18</td>
    	<td bgColor=#ddeeff><input type=text name=CadOn18 size=20 maxlength=9 value="<%voip_tone_get("CadOn18");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOff18</td>
    	<td bgColor=#ddeeff><input type=text name=CadOff18 size=20 maxlength=9 value="<%voip_tone_get("CadOff18");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOn19</td>
    	<td bgColor=#ddeeff><input type=text name=CadOn19 size=20 maxlength=9 value="<%voip_tone_get("CadOn19");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>

   <tr>
		<td  bgColor=#aaddff>CadOff19</td>
    	<td bgColor=#ddeeff><input type=text name=CadOff19 size=20 maxlength=9 value="<%voip_tone_get("CadOff19");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C16_<% multilang(LANG_FREQ);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C16_Freq0 size=20 maxlength=4 value="<%voip_tone_get("C16_Freq0");%>"> <font size=1>(Hz)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C16_<% multilang(LANG_FREQ);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C16_Freq1 size=20 maxlength=4 value="<%voip_tone_get("C16_Freq1");%>"> <font size=1>(Hz)</font></td>
	</tr>
	
   	<tr>
    	<td bgColor=#aaddff>C16_<% multilang(LANG_FREQ);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C16_Freq2 size=20 maxlength=4 value="<%voip_tone_get("C16_Freq2");%>"> <font size=1>(Hz)</font></td>
	</tr>
 	<tr>
    	<td bgColor=#aaddff>C16_<% multilang(LANG_FREQ);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C16_Freq3 size=20 maxlength=4 value="<%voip_tone_get("C16_Freq3");%>"> <font size=1>(Hz)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C16_<% multilang(LANG_GAIN);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C16_Gain0 size=20 maxlength=2 value="<%voip_tone_get("C16_Gain0");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C16_<% multilang(LANG_GAIN);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C16_Gain1 size=20 maxlength=2 value="<%voip_tone_get("C16_Gain1");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C16_<% multilang(LANG_GAIN);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C16_Gain2 size=20 maxlength=2 value="<%voip_tone_get("C16_Gain2");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C16_<% multilang(LANG_GAIN);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C16_Gain3 size=20 maxlength=2 value="<%voip_tone_get("C16_Gain3");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C17_<% multilang(LANG_FREQ);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C17_Freq0 size=20 maxlength=4 value="<%voip_tone_get("C17_Freq0");%>"> <font size=1>(Hz)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C17_<% multilang(LANG_FREQ);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C17_Freq1 size=20 maxlength=4 value="<%voip_tone_get("C17_Freq1");%>"> <font size=1>(Hz)</font></td>
	</tr>
	
   	<tr>
    	<td bgColor=#aaddff>C17_<% multilang(LANG_FREQ);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C17_Freq2 size=20 maxlength=4 value="<%voip_tone_get("C17_Freq2");%>"> <font size=1>(Hz)</font></td>
	</tr>
 	<tr>
    	<td bgColor=#aaddff>C17_<% multilang(LANG_FREQ);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C17_Freq3 size=20 maxlength=4 value="<%voip_tone_get("C17_Freq3");%>"> <font size=1>(Hz)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C17_<% multilang(LANG_GAIN);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C17_Gain0 size=20 maxlength=2 value="<%voip_tone_get("C17_Gain0");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C17_<% multilang(LANG_GAIN);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C17_Gain1 size=20 maxlength=2 value="<%voip_tone_get("C17_Gain1");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C17_<% multilang(LANG_GAIN);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C17_Gain2 size=20 maxlength=2 value="<%voip_tone_get("C17_Gain2");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C17_<% multilang(LANG_GAIN);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C17_Gain3 size=20 maxlength=2 value="<%voip_tone_get("C17_Gain3");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C18_<% multilang(LANG_FREQ);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C18_Freq0 size=20 maxlength=4 value="<%voip_tone_get("C18_Freq0");%>"> <font size=1>(Hz)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C18_<% multilang(LANG_FREQ);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C18_Freq1 size=20 maxlength=4 value="<%voip_tone_get("C18_Freq1");%>"> <font size=1>(Hz)</font></td>
	</tr>
	
   	<tr>
    	<td bgColor=#aaddff>C18_<% multilang(LANG_FREQ);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C18_Freq2 size=20 maxlength=4 value="<%voip_tone_get("C18_Freq2");%>"> <font size=1>(Hz)</font></td>
	</tr>
 	<tr>
    	<td bgColor=#aaddff>C18_<% multilang(LANG_FREQ);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C18_Freq3 size=20 maxlength=4 value="<%voip_tone_get("C18_Freq3");%>"> <font size=1>(Hz)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C18_<% multilang(LANG_GAIN);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C18_Gain0 size=20 maxlength=2 value="<%voip_tone_get("C18_Gain0");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C18_<% multilang(LANG_GAIN);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C18_Gain1 size=20 maxlength=2 value="<%voip_tone_get("C18_Gain1");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C18_<% multilang(LANG_GAIN);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C18_Gain2 size=20 maxlength=2 value="<%voip_tone_get("C18_Gain2");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C18_<% multilang(LANG_GAIN);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C18_Gain3 size=20 maxlength=2 value="<%voip_tone_get("C18_Gain3");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C19_<% multilang(LANG_FREQ);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C19_Freq0 size=20 maxlength=4 value="<%voip_tone_get("C19_Freq0");%>"> <font size=1>(Hz)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C19_<% multilang(LANG_FREQ);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C19_Freq1 size=20 maxlength=4 value="<%voip_tone_get("C19_Freq1");%>"> <font size=1>(Hz)</font></td>
	</tr>
	
   	<tr>
    	<td bgColor=#aaddff>C19_<% multilang(LANG_FREQ);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C19_Freq2 size=20 maxlength=4 value="<%voip_tone_get("C19_Freq2");%>"> <font size=1>(Hz)</font></td>
	</tr>
 	<tr>
    	<td bgColor=#aaddff>C19_<% multilang(LANG_FREQ);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C19_Freq3 size=20 maxlength=4 value="<%voip_tone_get("C19_Freq3");%>"> <font size=1>(Hz)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C19_<% multilang(LANG_GAIN);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C19_Gain0 size=20 maxlength=2 value="<%voip_tone_get("C19_Gain0");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C19_<% multilang(LANG_GAIN);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C19_Gain1 size=20 maxlength=2 value="<%voip_tone_get("C19_Gain1");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C19_<% multilang(LANG_GAIN);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C19_Gain2 size=20 maxlength=2 value="<%voip_tone_get("C19_Gain2");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C19_<% multilang(LANG_GAIN);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C19_Gain3 size=20 maxlength=2 value="<%voip_tone_get("C19_Gain3");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOn20</td>
    	<td bgColor=#ddeeff><input type=text name=CadOn20 size=20 maxlength=9 value="<%voip_tone_get("CadOn20");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
	
   <tr>
		<td  bgColor=#aaddff>CadOff20</td>
    	<td bgColor=#ddeeff><input type=text name=CadOff20 size=20 maxlength=9 value="<%voip_tone_get("CadOff20");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
	
   <tr>
		<td  bgColor=#aaddff>CadOn21</td>
    	<td bgColor=#ddeeff><input type=text name=CadOn21 size=20 maxlength=9 value="<%voip_tone_get("CadOn21");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOff21</td>
    	<td bgColor=#ddeeff><input type=text name=CadOff21 size=20 maxlength=9 value="<%voip_tone_get("CadOff21");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOn22</td>
    	<td bgColor=#ddeeff><input type=text name=CadOn22 size=20 maxlength=9 value="<%voip_tone_get("CadOn22");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOff22</td>
    	<td bgColor=#ddeeff><input type=text name=CadOff22 size=20 maxlength=9 value="<%voip_tone_get("CadOff22");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOn23</td>
    	<td bgColor=#ddeeff><input type=text name=CadOn23 size=20 maxlength=9 value="<%voip_tone_get("CadOn23");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>

   <tr>
		<td  bgColor=#aaddff>CadOff23</td>
    	<td bgColor=#ddeeff><input type=text name=CadOff23 size=20 maxlength=9 value="<%voip_tone_get("CadOff23");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C20_<% multilang(LANG_FREQ);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C20_Freq0 size=20 maxlength=4 value="<%voip_tone_get("C20_Freq0");%>"> <font size=1>(Hz)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C20_<% multilang(LANG_FREQ);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C20_Freq1 size=20 maxlength=4 value="<%voip_tone_get("C20_Freq1");%>"> <font size=1>(Hz)</font></td>
	</tr>
	
   	<tr>
    	<td bgColor=#aaddff>C20_<% multilang(LANG_FREQ);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C20_Freq2 size=20 maxlength=4 value="<%voip_tone_get("C20_Freq2");%>"> <font size=1>(Hz)</font></td>
	</tr>
 	<tr>
    	<td bgColor=#aaddff>C20_<% multilang(LANG_FREQ);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C20_Freq3 size=20 maxlength=4 value="<%voip_tone_get("C20_Freq3");%>"> <font size=1>(Hz)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C20_<% multilang(LANG_GAIN);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C20_Gain0 size=20 maxlength=2 value="<%voip_tone_get("C20_Gain0");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C20_<% multilang(LANG_GAIN);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C20_Gain1 size=20 maxlength=2 value="<%voip_tone_get("C20_Gain1");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C20_<% multilang(LANG_GAIN);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C20_Gain2 size=20 maxlength=2 value="<%voip_tone_get("C20_Gain2");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C20_<% multilang(LANG_GAIN);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C20_Gain3 size=20 maxlength=2 value="<%voip_tone_get("C20_Gain3");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C21_<% multilang(LANG_FREQ);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C21_Freq0 size=20 maxlength=4 value="<%voip_tone_get("C21_Freq0");%>"> <font size=1>(Hz)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C21_<% multilang(LANG_FREQ);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C21_Freq1 size=20 maxlength=4 value="<%voip_tone_get("C21_Freq1");%>"> <font size=1>(Hz)</font></td>
	</tr>
	
   	<tr>
    	<td bgColor=#aaddff>C21_<% multilang(LANG_FREQ);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C21_Freq2 size=20 maxlength=4 value="<%voip_tone_get("C21_Freq2");%>"> <font size=1>(Hz)</font></td>
	</tr>
 	<tr>
    	<td bgColor=#aaddff>C21_<% multilang(LANG_FREQ);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C21_Freq3 size=20 maxlength=4 value="<%voip_tone_get("C21_Freq3");%>"> <font size=1>(Hz)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C21_<% multilang(LANG_GAIN);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C21_Gain0 size=20 maxlength=2 value="<%voip_tone_get("C21_Gain0");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C21_<% multilang(LANG_GAIN);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C21_Gain1 size=20 maxlength=2 value="<%voip_tone_get("C21_Gain1");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C21_<% multilang(LANG_GAIN);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C21_Gain2 size=20 maxlength=2 value="<%voip_tone_get("C21_Gain2");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C21_<% multilang(LANG_GAIN);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C21_Gain3 size=20 maxlength=2 value="<%voip_tone_get("C21_Gain3");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C22_<% multilang(LANG_FREQ);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C22_Freq0 size=20 maxlength=4 value="<%voip_tone_get("C22_Freq0");%>"> <font size=1>(Hz)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C22_<% multilang(LANG_FREQ);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C22_Freq1 size=20 maxlength=4 value="<%voip_tone_get("C22_Freq1");%>"> <font size=1>(Hz)</font></td>
	</tr>
	
   	<tr>
    	<td bgColor=#aaddff>C22_<% multilang(LANG_FREQ);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C22_Freq2 size=20 maxlength=4 value="<%voip_tone_get("C22_Freq2");%>"> <font size=1>(Hz)</font></td>
	</tr>
 	<tr>
    	<td bgColor=#aaddff>C22_<% multilang(LANG_FREQ);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C22_Freq3 size=20 maxlength=4 value="<%voip_tone_get("C22_Freq3");%>"> <font size=1>(Hz)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C22_<% multilang(LANG_GAIN);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C22_Gain0 size=20 maxlength=2 value="<%voip_tone_get("C22_Gain0");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C22_<% multilang(LANG_GAIN);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C22_Gain1 size=20 maxlength=2 value="<%voip_tone_get("C22_Gain1");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C22_<% multilang(LANG_GAIN);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C22_Gain2 size=20 maxlength=2 value="<%voip_tone_get("C22_Gain2");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C22_<% multilang(LANG_GAIN);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C22_Gain3 size=20 maxlength=2 value="<%voip_tone_get("C22_Gain3");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C23_<% multilang(LANG_FREQ);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C23_Freq0 size=20 maxlength=4 value="<%voip_tone_get("C23_Freq0");%>"> <font size=1>(Hz)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C23_<% multilang(LANG_FREQ);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C23_Freq1 size=20 maxlength=4 value="<%voip_tone_get("C23_Freq1");%>"> <font size=1>(Hz)</font></td>
	</tr>
	
   	<tr>
    	<td bgColor=#aaddff>C23_<% multilang(LANG_FREQ);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C23_Freq2 size=20 maxlength=4 value="<%voip_tone_get("C23_Freq2");%>"> <font size=1>(Hz)</font></td>
	</tr>
 	<tr>
    	<td bgColor=#aaddff>C23_<% multilang(LANG_FREQ);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C23_Freq3 size=20 maxlength=4 value="<%voip_tone_get("C23_Freq3");%>"> <font size=1>(Hz)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C23_<% multilang(LANG_GAIN);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C23_Gain0 size=20 maxlength=2 value="<%voip_tone_get("C23_Gain0");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C23_<% multilang(LANG_GAIN);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C23_Gain1 size=20 maxlength=2 value="<%voip_tone_get("C23_Gain1");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C23_<% multilang(LANG_GAIN);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C23_Gain2 size=20 maxlength=2 value="<%voip_tone_get("C23_Gain2");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C23_<% multilang(LANG_GAIN);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C23_Gain3 size=20 maxlength=2 value="<%voip_tone_get("C23_Gain3");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOn24</td>
    	<td bgColor=#ddeeff><input type=text name=CadOn24 size=20 maxlength=9 value="<%voip_tone_get("CadOn24");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
	
   <tr>
		<td  bgColor=#aaddff>CadOff24</td>
    	<td bgColor=#ddeeff><input type=text name=CadOff24 size=20 maxlength=9 value="<%voip_tone_get("CadOff24");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
	
   <tr>
		<td  bgColor=#aaddff>CadOn25</td>
    	<td bgColor=#ddeeff><input type=text name=CadOn25 size=20 maxlength=9 value="<%voip_tone_get("CadOn25");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOff25</td>
    	<td bgColor=#ddeeff><input type=text name=CadOff25 size=20 maxlength=9 value="<%voip_tone_get("CadOff25");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOn26</td>
    	<td bgColor=#ddeeff><input type=text name=CadOn26 size=20 maxlength=9 value="<%voip_tone_get("CadOn26");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOff26</td>
    	<td bgColor=#ddeeff><input type=text name=CadOff26 size=20 maxlength=9 value="<%voip_tone_get("CadOff26");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOn27</td>
    	<td bgColor=#ddeeff><input type=text name=CadOn27 size=20 maxlength=9 value="<%voip_tone_get("CadOn27");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>

   <tr>
		<td  bgColor=#aaddff>CadOff27</td>
    	<td bgColor=#ddeeff><input type=text name=CadOff27 size=20 maxlength=9 value="<%voip_tone_get("CadOff27");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C24_<% multilang(LANG_FREQ);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C24_Freq0 size=20 maxlength=4 value="<%voip_tone_get("C24_Freq0");%>"> <font size=1>(Hz)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C24_<% multilang(LANG_FREQ);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C24_Freq1 size=20 maxlength=4 value="<%voip_tone_get("C24_Freq1");%>"> <font size=1>(Hz)</font></td>
	</tr>
	
   	<tr>
    	<td bgColor=#aaddff>C24_<% multilang(LANG_FREQ);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C24_Freq2 size=20 maxlength=4 value="<%voip_tone_get("C24_Freq2");%>"> <font size=1>(Hz)</font></td>
	</tr>
 	<tr>
    	<td bgColor=#aaddff>C24_<% multilang(LANG_FREQ);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C24_Freq3 size=20 maxlength=4 value="<%voip_tone_get("C24_Freq3");%>"> <font size=1>(Hz)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C24_<% multilang(LANG_GAIN);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C24_Gain0 size=20 maxlength=2 value="<%voip_tone_get("C24_Gain0");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C24_<% multilang(LANG_GAIN);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C24_Gain1 size=20 maxlength=2 value="<%voip_tone_get("C24_Gain1");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C24_<% multilang(LANG_GAIN);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C24_Gain2 size=20 maxlength=2 value="<%voip_tone_get("C24_Gain2");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C24_<% multilang(LANG_GAIN);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C24_Gain3 size=20 maxlength=2 value="<%voip_tone_get("C24_Gain3");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C25_<% multilang(LANG_FREQ);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C25_Freq0 size=20 maxlength=4 value="<%voip_tone_get("C25_Freq0");%>"> <font size=1>(Hz)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C25_<% multilang(LANG_FREQ);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C25_Freq1 size=20 maxlength=4 value="<%voip_tone_get("C25_Freq1");%>"> <font size=1>(Hz)</font></td>
	</tr>
	
   	<tr>
    	<td bgColor=#aaddff>C25_<% multilang(LANG_FREQ);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C25_Freq2 size=20 maxlength=4 value="<%voip_tone_get("C25_Freq2");%>"> <font size=1>(Hz)</font></td>
	</tr>
 	<tr>
    	<td bgColor=#aaddff>C25_<% multilang(LANG_FREQ);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C25_Freq3 size=20 maxlength=4 value="<%voip_tone_get("C25_Freq3");%>"> <font size=1>(Hz)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C25_<% multilang(LANG_GAIN);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C25_Gain0 size=20 maxlength=2 value="<%voip_tone_get("C25_Gain0");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C25_<% multilang(LANG_GAIN);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C25_Gain1 size=20 maxlength=2 value="<%voip_tone_get("C25_Gain1");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C25_<% multilang(LANG_GAIN);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C25_Gain2 size=20 maxlength=2 value="<%voip_tone_get("C25_Gain2");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C25_<% multilang(LANG_GAIN);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C25_Gain3 size=20 maxlength=2 value="<%voip_tone_get("C25_Gain3");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C26_<% multilang(LANG_FREQ);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C26_Freq0 size=20 maxlength=4 value="<%voip_tone_get("C26_Freq0");%>"> <font size=1>(Hz)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C26_<% multilang(LANG_FREQ);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C26_Freq1 size=20 maxlength=4 value="<%voip_tone_get("C26_Freq1");%>"> <font size=1>(Hz)</font></td>
	</tr>
	
   	<tr>
    	<td bgColor=#aaddff>C26_<% multilang(LANG_FREQ);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C26_Freq2 size=20 maxlength=4 value="<%voip_tone_get("C26_Freq2");%>"> <font size=1>(Hz)</font></td>
	</tr>
 	<tr>
    	<td bgColor=#aaddff>C26_<% multilang(LANG_FREQ);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C26_Freq3 size=20 maxlength=4 value="<%voip_tone_get("C26_Freq3");%>"> <font size=1>(Hz)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C26_<% multilang(LANG_GAIN);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C26_Gain0 size=20 maxlength=2 value="<%voip_tone_get("C26_Gain0");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C26_<% multilang(LANG_GAIN);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C26_Gain1 size=20 maxlength=2 value="<%voip_tone_get("C26_Gain1");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C26_<% multilang(LANG_GAIN);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C26_Gain2 size=20 maxlength=2 value="<%voip_tone_get("C26_Gain2");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C26_<% multilang(LANG_GAIN);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C26_Gain3 size=20 maxlength=2 value="<%voip_tone_get("C26_Gain3");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C27_<% multilang(LANG_FREQ);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C27_Freq0 size=20 maxlength=4 value="<%voip_tone_get("C27_Freq0");%>"> <font size=1>(Hz)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C27_<% multilang(LANG_FREQ);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C27_Freq1 size=20 maxlength=4 value="<%voip_tone_get("C27_Freq1");%>"> <font size=1>(Hz)</font></td>
	</tr>
	
   	<tr>
    	<td bgColor=#aaddff>C27_<% multilang(LANG_FREQ);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C27_Freq2 size=20 maxlength=4 value="<%voip_tone_get("C27_Freq2");%>"> <font size=1>(Hz)</font></td>
	</tr>
 	<tr>
    	<td bgColor=#aaddff>C27_<% multilang(LANG_FREQ);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C27_Freq3 size=20 maxlength=4 value="<%voip_tone_get("C27_Freq3");%>"> <font size=1>(Hz)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C27_<% multilang(LANG_GAIN);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C27_Gain0 size=20 maxlength=2 value="<%voip_tone_get("C27_Gain0");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C27_<% multilang(LANG_GAIN);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C27_Gain1 size=20 maxlength=2 value="<%voip_tone_get("C27_Gain1");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C27_<% multilang(LANG_GAIN);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C27_Gain2 size=20 maxlength=2 value="<%voip_tone_get("C27_Gain2");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C27_<% multilang(LANG_GAIN);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C27_Gain3 size=20 maxlength=2 value="<%voip_tone_get("C27_Gain3");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOn28</td>
    	<td bgColor=#ddeeff><input type=text name=CadOn28 size=20 maxlength=9 value="<%voip_tone_get("CadOn28");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
	
   <tr>
		<td  bgColor=#aaddff>CadOff28</td>
    	<td bgColor=#ddeeff><input type=text name=CadOff28 size=20 maxlength=9 value="<%voip_tone_get("CadOff28");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
	
   <tr>
		<td  bgColor=#aaddff>CadOn29</td>
    	<td bgColor=#ddeeff><input type=text name=CadOn29 size=20 maxlength=9 value="<%voip_tone_get("CadOn29");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOff29</td>
    	<td bgColor=#ddeeff><input type=text name=CadOff29 size=20 maxlength=9 value="<%voip_tone_get("CadOff29");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOn30</td>
    	<td bgColor=#ddeeff><input type=text name=CadOn30 size=20 maxlength=9 value="<%voip_tone_get("CadOn30");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOff30</td>
    	<td bgColor=#ddeeff><input type=text name=CadOff30 size=20 maxlength=9 value="<%voip_tone_get("CadOff30");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOn31</td>
    	<td bgColor=#ddeeff><input type=text name=CadOn31 size=20 maxlength=9 value="<%voip_tone_get("CadOn31");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>

   <tr>
		<td  bgColor=#aaddff>CadOff31</td>
    	<td bgColor=#ddeeff><input type=text name=CadOff31 size=20 maxlength=9 value="<%voip_tone_get("CadOff31");%>"> <font size=1>(<% multilang(LANG_MSEC);%>)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C28_<% multilang(LANG_FREQ);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C28_Freq0 size=20 maxlength=4 value="<%voip_tone_get("C28_Freq0");%>"> <font size=1>(Hz)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C28_<% multilang(LANG_FREQ);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C28_Freq1 size=20 maxlength=4 value="<%voip_tone_get("C28_Freq1");%>"> <font size=1>(Hz)</font></td>
	</tr>
	
   	<tr>
    	<td bgColor=#aaddff>C28_<% multilang(LANG_FREQ);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C28_Freq2 size=20 maxlength=4 value="<%voip_tone_get("C28_Freq2");%>"> <font size=1>(Hz)</font></td>
	</tr>
 	<tr>
    	<td bgColor=#aaddff>C28_<% multilang(LANG_FREQ);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C28_Freq3 size=20 maxlength=4 value="<%voip_tone_get("C28_Freq3");%>"> <font size=1>(Hz)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C28_<% multilang(LANG_GAIN);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C28_Gain0 size=20 maxlength=2 value="<%voip_tone_get("C28_Gain0");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C28_<% multilang(LANG_GAIN);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C28_Gain1 size=20 maxlength=2 value="<%voip_tone_get("C28_Gain1");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C28_<% multilang(LANG_GAIN);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C28_Gain2 size=20 maxlength=2 value="<%voip_tone_get("C28_Gain2");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C28_<% multilang(LANG_GAIN);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C28_Gain3 size=20 maxlength=2 value="<%voip_tone_get("C28_Gain3");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C29_<% multilang(LANG_FREQ);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C29_Freq0 size=20 maxlength=4 value="<%voip_tone_get("C29_Freq0");%>"> <font size=1>(Hz)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C29_<% multilang(LANG_FREQ);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C29_Freq1 size=20 maxlength=4 value="<%voip_tone_get("C29_Freq1");%>"> <font size=1>(Hz)</font></td>
	</tr>
	
   	<tr>
    	<td bgColor=#aaddff>C29_<% multilang(LANG_FREQ);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C29_Freq2 size=20 maxlength=4 value="<%voip_tone_get("C29_Freq2");%>"> <font size=1>(Hz)</font></td>
	</tr>
 	<tr>
    	<td bgColor=#aaddff>C29_<% multilang(LANG_FREQ);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C29_Freq3 size=20 maxlength=4 value="<%voip_tone_get("C29_Freq3");%>"> <font size=1>(Hz)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C29_<% multilang(LANG_GAIN);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C29_Gain0 size=20 maxlength=2 value="<%voip_tone_get("C29_Gain0");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C29_<% multilang(LANG_GAIN);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C29_Gain1 size=20 maxlength=2 value="<%voip_tone_get("C29_Gain1");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C29_<% multilang(LANG_GAIN);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C29_Gain2 size=20 maxlength=2 value="<%voip_tone_get("C29_Gain2");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C29_<% multilang(LANG_GAIN);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C29_Gain3 size=20 maxlength=2 value="<%voip_tone_get("C29_Gain3");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C30_<% multilang(LANG_FREQ);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C30_Freq0 size=20 maxlength=4 value="<%voip_tone_get("C30_Freq0");%>"> <font size=1>(Hz)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C30_<% multilang(LANG_FREQ);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C30_Freq1 size=20 maxlength=4 value="<%voip_tone_get("C30_Freq1");%>"> <font size=1>(Hz)</font></td>
	</tr>
	
   	<tr>
    	<td bgColor=#aaddff>C30_<% multilang(LANG_FREQ);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C30_Freq2 size=20 maxlength=4 value="<%voip_tone_get("C30_Freq2");%>"> <font size=1>(Hz)</font></td>
	</tr>
 	<tr>
    	<td bgColor=#aaddff>C30_<% multilang(LANG_FREQ);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C30_Freq3 size=20 maxlength=4 value="<%voip_tone_get("C30_Freq3");%>"> <font size=1>(Hz)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C30_<% multilang(LANG_GAIN);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C30_Gain0 size=20 maxlength=2 value="<%voip_tone_get("C30_Gain0");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C30_<% multilang(LANG_GAIN);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C30_Gain1 size=20 maxlength=2 value="<%voip_tone_get("C30_Gain1");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C30_<% multilang(LANG_GAIN);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C30_Gain2 size=20 maxlength=2 value="<%voip_tone_get("C30_Gain2");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C30_<% multilang(LANG_GAIN);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C30_Gain3 size=20 maxlength=2 value="<%voip_tone_get("C30_Gain3");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C31_<% multilang(LANG_FREQ);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C31_Freq0 size=20 maxlength=4 value="<%voip_tone_get("C31_Freq0");%>"> <font size=1>(Hz)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C31_<% multilang(LANG_FREQ);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C31_Freq1 size=20 maxlength=4 value="<%voip_tone_get("C31_Freq1");%>"> <font size=1>(Hz)</font></td>
	</tr>
	
   	<tr>
    	<td bgColor=#aaddff>C31_<% multilang(LANG_FREQ);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C31_Freq2 size=20 maxlength=4 value="<%voip_tone_get("C31_Freq2");%>"> <font size=1>(Hz)</font></td>
	</tr>
 	<tr>
    	<td bgColor=#aaddff>C31_<% multilang(LANG_FREQ);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C31_Freq3 size=20 maxlength=4 value="<%voip_tone_get("C31_Freq3");%>"> <font size=1>(Hz)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C31_<% multilang(LANG_GAIN);%>0</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C31_Gain0 size=20 maxlength=2 value="<%voip_tone_get("C31_Gain0");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C31_<% multilang(LANG_GAIN);%>1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C31_Gain1 size=20 maxlength=2 value="<%voip_tone_get("C31_Gain1");%>"> <font size=1>(- dBm)(60~0)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>C31_<% multilang(LANG_GAIN);%>2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C31_Gain2 size=20 maxlength=2 value="<%voip_tone_get("C31_Gain2");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>C31_<% multilang(LANG_GAIN);%>3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=C31_Gain3 size=20 maxlength=2 value="<%voip_tone_get("C31_Gain3");%>"> <font size=1>(- dBm)(60~0)</td>
	</tr>

	<tr>
    	<td colspan=3 align=center>
    		<input type="submit" name="Tone" value="<% multilang(LANG_APPLY);%>" >
    		&nbsp;&nbsp;&nbsp;&nbsp;    	
    		<input type="reset" value="<% multilang(LANG_RESET);%>">	
    	</td>
	</tr>
</table>
</p>

<p <%voip_tone_get("display_distone");%>>
<b>Select Disconnect Tone Number</b>
<table cellSpacing=1 cellPadding=2 border=0 width=400>

  	<tr>
    	<td bgColor=#aaddff>Number</td>
		<td bgColor=#ddeeff>
		<select name=distone_num onchange="changeNum();">
			 "<%voip_tone_get("distone_num");%>"
		</select>
		</td>

	</tr>	


</table>
</p>

<div id = distone1tab <%voip_tone_get("d1display");%> >
<b>Disconnect Tone 1 Parameters</b>
<table cellSpacing=1 cellPadding=2 border=0 width=400>
  	<tr>
    	<td bgColor=#aaddff width=155>FreqNum</td>
    	<td bgColor=#ddeeff width=170>
			<select name=d1freqnum onchange="D1FreqNum_change();">
			 	<%voip_tone_get("d1freqnum");%>
			</select>
		</td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>Freq1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=d1Freq1 size=20 maxlength=4 value="<%voip_tone_get("d1Freq1");%>"> <font size=1>(Hz)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>Freq2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=d1Freq2 size=20 maxlength=4 value="<%voip_tone_get("d1Freq2");%>"> <font size=1>(Hz)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>Freq Accuracy</td>
    	<td bgColor=#ddeeff>
	<input type=text name=d1Accur size=20 maxlength=3 value="<%voip_tone_get("d1Accur");%>"> <font size=1>(Default:16)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>Level</td>
    	<td bgColor=#ddeeff>
	<input type=text name=d1Level size=20 maxlength=5 value="<%voip_tone_get("d1Level");%>"> <font size=1>(Default:800)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>Tone ON up limit</td>
    	<td bgColor=#ddeeff>
	<input type=text name=d1ONup size=20 maxlength=5 value="<%voip_tone_get("d1ONup");%>"> <font size=1>(Unit:10ms)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>Tone ON low limit</td>
    	<td bgColor=#ddeeff>
	<input type=text name=d1ONlow size=20 maxlength=5 value="<%voip_tone_get("d1ONlow");%>"> <font size=1>(Unit:10ms)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>Tone OFF up limit</td>
    	<td bgColor=#ddeeff>
	<input type=text name=d1OFFup size=20 maxlength=5 value="<%voip_tone_get("d1OFFup");%>"> <font size=1>(Unit:10ms)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>Tone OFF low limit</td>
    	<td bgColor=#ddeeff>
	<input type=text name=d1OFFlow size=20 maxlength=5 value="<%voip_tone_get("d1OFFlow");%>"> <font size=1>(Unit:10ms)</font></td>
	</tr>

</table>
</div>

<div id = distone2tab <%voip_tone_get("d2display");%> >
<b>Disconnect Tone 2 Parameters</b>
<table cellSpacing=1 cellPadding=2 border=0 width=400>
  	<tr>
    	<td bgColor=#aaddff width=155>FreqNum</td>
    	<td bgColor=#ddeeff width=170>
			<select name=d2freqnum onchange="D2FreqNum_change();">
			 	<%voip_tone_get("d2freqnum");%>
			</select>
		</td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>Freq1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=d2Freq1 size=20 maxlength=4 value="<%voip_tone_get("d2Freq1");%>"> <font size=1>(Hz)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>Freq2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=d2Freq2 size=20 maxlength=4 value="<%voip_tone_get("d2Freq2");%>"> <font size=1>(Hz)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>Freq Accuracy</td>
    	<td bgColor=#ddeeff>
	<input type=text name=d2Accur size=20 maxlength=3 value="<%voip_tone_get("d2Accur");%>"> <font size=1>(Default:16)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>Level</td>
    	<td bgColor=#ddeeff>
	<input type=text name=d2Level size=20 maxlength=5 value="<%voip_tone_get("d2Level");%>"> <font size=1>(Default:800)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>Tone ON up limit</td>
    	<td bgColor=#ddeeff>
	<input type=text name=d2ONup size=20 maxlength=5 value="<%voip_tone_get("d2ONup");%>"> <font size=1>(Unit:10ms)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>Tone ON low limit</td>
    	<td bgColor=#ddeeff>
	<input type=text name=d2ONlow size=20 maxlength=5 value="<%voip_tone_get("d2ONlow");%>"> <font size=1>(Unit:10ms)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>Tone OFF up limit</td>
    	<td bgColor=#ddeeff>
	<input type=text name=d2OFFup size=20 maxlength=5 value="<%voip_tone_get("d2OFFup");%>"> <font size=1>(Unit:10ms)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>Tone OFF low limit</td>
    	<td bgColor=#ddeeff>
	<input type=text name=d2OFFlow size=20 maxlength=5 value="<%voip_tone_get("d2OFFlow");%>"> <font size=1>(Unit:10ms)</font></td>
	</tr>

</table>
</div>

<p <%voip_tone_get("display_distone");%>>
<table cellSpacing=1 cellPadding=2 border=0 width=400>

	<tr>
    	<td colspan=3 align=center>
    		<input type="submit" name="Distone" value="<% multilang(LANG_APPLY); %>" >
   	</td>
	</tr>
</table>

</p>

</form>
</body>
</html>
