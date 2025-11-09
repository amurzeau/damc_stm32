function log(str)
{
	//document.getElementById("log").innerHTML += "<div class=\"logentry\">"+str+"</div>";
}

function logSection(str)
{
//	document.getElementById("log").innerHTML += "<div class=\"logtitle\">"+str+"</div>";
}

function clearLog()
{
//	byId("log").innerHTML = "";
}

function printBands()
{
	logSection("bands");
	for(var i in spec.bands)
	{
		var b = spec.bands[i];
		log(i + ": " + b.start + " ... " + b.stop + ", G = " + b.gain);
	}
}

function printFrequencies()
{
	var s = "";
	for(var i in spec.frequencies)
		s += spec.frequencies[i] + ", ";
	logSection("frequencies");
	log(s);
}