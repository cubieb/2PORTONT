var keyStr = "ABCDEFGHIJKLMNOPQRSTUVWXYZ" +
             "abcdefghijklmnopqrstuvwxyz" +
             "0123456789+/=";

function encode64(input) {
	var output = "";
	var i = 0, len = input.length;

	for (i = 0; i <= len - 3; i += 3)
    {
        output += keyStr.charAt(input.charCodeAt(i) >>> 2);
        output += keyStr.charAt(((input.charCodeAt(i) & 3) << 4) | (input.charCodeAt(i+1) >>> 4));
        output += keyStr.charAt(((input.charCodeAt(i+1) & 15) << 2) | (input.charCodeAt(i+2) >>> 6));
        output += keyStr.charAt(input.charCodeAt(i+2) & 63);
    }

    if (len % 3 == 2)
    {
        output += keyStr.charAt(input.charCodeAt(i) >>> 2);
        output += keyStr.charAt(((input.charCodeAt(i) & 3) << 4) | (input.charCodeAt(i+1) >>> 4));
        output += keyStr.charAt(((input.charCodeAt(i+1) & 15) << 2));
        output += keyStr.charAt(64);
    }
    else if (len % 3 == 1)
    {
        output += keyStr.charAt(input.charCodeAt(i) >>> 2);
        output += keyStr.charAt(((input.charCodeAt(i) & 3) << 4));
        output += keyStr.charAt(64);
        output += keyStr.charAt(64);
    }

	return output;
}

function decode64(input) {
	var output = "";
	var i, a, b, c, d, z;
	for (i = 0; i < input.length - 3; i += 4) {
        a = keyStr.indexOf(input.charAt(i+0));
        b = keyStr.indexOf(input.charAt(i+1));
        c = keyStr.indexOf(input.charAt(i+2));
        d = keyStr.indexOf(input.charAt(i+3));

        output += String.fromCharCode((a << 2) | (b >>> 4));
        if (input.charAt(i+2) != keyStr.charAt(64))
            output += String.fromCharCode(((b << 4) & 0xF0) | ((c >>> 2) & 0x0F));
        if (input.charAt(i+3) != keyStr.charAt(64))
            output += String.fromCharCode(((c << 6) & 0xC0) | d);
    }
	return output;
}

