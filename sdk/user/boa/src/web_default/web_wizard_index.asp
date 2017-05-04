<html><! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
    <meta charset="utf-8">
<title>Html Wizard</title>
    <link rel="stylesheet" href="reset.css">
    <link rel="stylesheet" href="base.css">
    <link rel="stylesheet" href="style.css">
	<style>
		.wizard_page{
			margin: 0 auto;
			width: 960px;
			height: 100%;
		}
	</style>
</head>
<body>
	<div id="header">
		<div class="top_bg" align="right">
		</div>
	</div>
	<div class="wizard_page">
		<iframe src="web_wizard_1.asp" scrolling="no" frameborder="0" height="100%" id="mainFrame" width="100%%"></iframe>
	</div>
    <script>
        var iframe = document.getElementById('mainFrame');
        var ifrmeWindow = iframe.contentWindow;

        iframe.onload = function() {
            setInterval(syncIframe, 200);
        };
        function syncIframe() {

            var wbody = ifrmeWindow.document.body;

	     if(wbody){        
	            var height = wbody.scrollHeight;
	            var width = wbody.scrollWidth;

	            var oldMapping = {width: iframe.width, height: iframe.height};
	            var newMapping = {width: width, height: height};
	/*
	            if (oldMapping.width != newMapping.width) {
	                oldMapping.width = newMapping.width;
	                iframe.width = width;
	            }
	*/
	            if (oldMapping.height != newMapping.height) {
	                oldMapping.height = newMapping.height;
	                iframe.height = height+100;
	            }
            }
            
        }
    </script>
</body>
</html>
