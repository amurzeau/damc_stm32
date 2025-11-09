/* 

	Copyright (c) 2012 Peter Isza (peter.isza@gmail.com)

*/

function generateIntTaps(precision)
{
	spec.inttaps = new Array();
	for(var i in spec.taps)
		spec.inttaps[i] = Math.round(spec.taps[i] * Math.pow(2, precision-1));
}

function getCppTaps(settings)
{
	if(settings.shift > 0)
	    generateIntTaps(settings.shift);
	    
	var s = "";
	s += "static " + settings.coeff_type;	
	
	s += " filter_taps["+settings.name.toUpperCase()+"FILTER_TAP_NUM] = {<br/>";

	for(var i in spec.taps)
	{
		if(settings.shift > 0)
			tap = spec.inttaps[i];
		else
			tap = spec.taps[i];		
		s += "&nbsp;&nbsp;" + tap + ((i == spec.taps.length-1) ? "" : ",") + "<br/>";
	}
	s += "};<br/>";
	
	return s;
}

function getCppHeaderComment(settings)
{
	s = "";
	s += '<span class="code_comment">'
	s += "/*<br/><br/>FIR filter designed with<br/> http://t-filter.appspot.com<br/><br/>";
	s += "sampling frequency: " + spec.samp_freq + " Hz<br/><br/>";
	
	if(settings.shift > 0)
		s += "fixed point precision: " + settings.shift + " bits<br/><br/>";
	
//	s += "specification: <br/>";
	for(var i in spec.bands)
	{
		var b = spec.bands[i];
		s += "* " + b.startHz + " Hz - " + b.stopHz + " Hz<br/>";
		s += "&nbsp;&nbsp;gain = " + b.gain + "<br/>";
		
		var tmp = (b.gain == 0 ? "attenuation" : "ripple");
		
		s += "&nbsp;&nbsp;desired " + tmp + " = " + b.ripple + " dB<br/>";
		
		if(settings.shift > 0)
			s += "&nbsp;&nbsp;actual " + tmp + " = n/a<br/>";
		else
			s += "&nbsp;&nbsp;actual " + tmp + " = " + b.actripple + " dB<br/>";
			
		s += "<br/>";
	}
	s += "*/</span><br/><br/>"
	s += "#define "+settings.name.toUpperCase()+"FILTER_TAP_NUM " + spec.taps.length + "<br/><br/>";
			
	return s;//.replace(/ /g,"&nbsp;");
	
}

function getTextOutput(settings)
{
    if(settings.shift > 0)
		generateIntTaps(settings.shift);
	var s = "";
	
	if(settings.shift > 0)
		for(var i in spec.taps)
			s += spec.inttaps[i] + "<br/>";
	else
		for(var i in spec.taps)
			s += spec.taps[i] + "<br/>";
		
	
	return "<p style=\"text-align:center\">" + s + "</p>";
}

function getGeneratedFileName(settings) {
	return settings.name+"Filter";
}

function getGeneratedHeader(settings) {
    var r = "";
    var ftype = settings.name+"Filter";
    var ffunc = settings.name+"Filter";
    
    r += "#ifndef "+settings.name.toUpperCase()+"FILTER_H_\n";
    r += "#define "+settings.name.toUpperCase()+"FILTER_H_\n\n";
    
    r += getCppHeaderComment(settings);

    r += "typedef struct {\n"
    
    r += "  "+settings.coeff_type+" history["+settings.name.toUpperCase()+"FILTER_TAP_NUM];\n";
    r += "  unsigned int last_index;\n"
    r += "} "+settings.name+"Filter"+";\n";

	r += "\n";
    r += "void "+ffunc+"_init("+ftype+"* f);\n";
    r += "void "+ffunc+"_put("+ftype+"* f, "+settings.coeff_type+" input);\n";
    r += settings.coeff_type+" "+ffunc+"_get("+ftype+"* f);\n";

    r += "\n#endif\n";
    
    return r;
}

function mylog2(n) {
    // lame linear time implementation
    var i;
    for(i = 0; (1 << i) <= n; i++);
    return i-1;
}

function getGeneratedCode(settings) {
    var n = settings.num_coeffs;
    var pow2 = (n & (n-1)) == 0;
    var log2n = mylog2(n);
    var ftype = settings.name+"Filter";
    var ffunc = settings.name+"Filter";

    var r = "";
    
    r += "#include \""+getGeneratedFileName(settings)+".h\"\n";
    r += "\n";    
    
    r += getCppTaps(settings);
    r += "\n";

    r += "void "+ffunc+"_init("+ftype+"* f) {\n";
    r += "  int i;\n";
    r += "  for(i = 0; i < "+settings.name.toUpperCase()+"FILTER_TAP_NUM; ++i)\n";
    r += "    f->history[i] = 0;\n";
    r += "  f->last_index = 0;\n";
    r += "}\n";

    r += "\n";

    r += "void "+ffunc+"_put("+ftype+"* f, "+settings.coeff_type+" input) {\n";
    if(pow2) {
        r += "  f->history[(f->last_index++) & "+(n-1)+"] = input;\n";
    } else {
        r += "  f->history[f->last_index++] = input;\n";    
        r += "  if(f->last_index == "+settings.name.toUpperCase()+"FILTER_TAP_NUM)\n";    
        r += "    f->last_index = 0;\n";    
    }
    r += "}\n";

    r += "\n";
    
    r += settings.coeff_type+" "+ffunc+"_get("+ftype+"* f) {\n";
    r += "  "+settings.accum_type+" acc = 0;\n";
    
    var lc = "";
    var cast = "";
    
    if(settings.coeff_type != settings.accum_type)
        cast = "(" + settings.accum_type + ")";

    if(pow2) {
        lc += "    acc += "+cast+"f->history[(index--) & "+(n-1)+"] * filter_taps[i];\n";
    } else {
        lc += "    index = index != 0 ? index-1 : "+settings.name.toUpperCase()+"FILTER_TAP_NUM-1;\n";
        lc += "    acc += "+cast+"f->history[index] * filter_taps[i];\n";
    }
    
    if(settings.unroll) {
	    r += "  int index = f->last_index;\n";
        for(i = 0; i < n; ++i) {
            r += lc.replace("[i]", "["+i+"]");
        }
    } else {
	    r += "  int index = f->last_index, i;\n";
        r += "  for(i = 0; i < "+settings.name.toUpperCase()+"FILTER_TAP_NUM; ++i) {\n";
        r += lc;
        r += "  };\n"
    }
    if(settings.shift > 0) {
        r += "  return acc >> "+settings.shift+";\n";
    } else {
        r += "  return acc;\n";
    }
    
    r += "}\n";
    
    return r;
}
