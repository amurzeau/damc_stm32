/* 

	Copyright (c) 2011 Peter Isza (peter.isza@gmail.com)

*/

function getOutput(spec, format, numberformat, precision)
{
	if(spec.done == false)
		return "";
	
    var settings = {
        name: "",
        num_coeffs: spec.taps.length,
        coeff_type: numberformat == "int" ? "int" : "double",
        accum_type: numberformat == "int" ? "int" : "double",
        shift: numberformat == "int" ? precision : 0,
        unroll: false
    };
    	
	if(format == "text")
		return getTextOutput(settings);
	if(format == "c")
		return getCppHeaderComment(settings) + getCppTaps(settings);
}

