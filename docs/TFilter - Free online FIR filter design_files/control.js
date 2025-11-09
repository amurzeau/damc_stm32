/* 

	Copyright (c) 2011 Peter Isza (peter.isza@gmail.com)

*/

var spec = {
	bands: new Array(),
	samp_freq: 2000,
	taps: 0,
	desiredtaps: 0,
	done: false
};

var result = {
	taps: new Array()
};

var initReady = false;

var whichGraph = "freq";

function addBand(newBand)
{
	if(typeof(newBand) == 'undefined')
		band = {start: 0, stop: 0, gain: 1, ripple: 5};
	else
		band = newBand;
	
	spec.bands.push(band);
	addBandWidget(byId('bandlist_tbody'), band);
	//$('#bandlist_table').tableScroll({height:100});
}

function deleteBand(band)
{
	band.widget.parentNode.removeChild(band.widget);
	deleteFromArray(spec.bands, band);
	specChanged();
	//$('#bandlist_table').tableScroll({height:100});
}


function specChanged()
{
	if(!initReady)
		return;
		
	for(var i in spec.bands)
	{
		var band = spec.bands[i];
		band.stop = band.stopHz / spec.samp_freq * 2;
		band.start = band.startHz / spec.samp_freq * 2;
		if(band.gain != 0)
		{
			band.weight = Math.pow(10, -band.ripple / Math.E / 20);
		
			band.upper = 1/band.weight*band.gain;
			band.lower = (1-(1/band.weight-1))*band.gain;
			band.weight = 1/(1/band.weight-1)/band.gain;
		}
		else
		{
			band.weight = Math.pow(10, -band.ripple / 20);
			band.upper = Math.pow(10, band.ripple / 20);
		}
	}

	result.taps = new Array();
	spec.done = false;
	byId("result").value = "";
	byId("a_numtaps").innerHTML = "";
	
	for(var i in spec.bands)
	{
		var b = spec.bands[i];
		b.showRipple(null);
	}


	//updateGraphs();
    //updateMainGraph();
    changeGraph("freq");
}

function showOutput()
{
	if(!spec.done)
		return;
	
	var prec = parseInt(byId("precision").value);
	
	if(typeof(prec) != 'number' || isNaN(prec))
		prec = 0;
	
	byId("result").innerHTML = getOutput(
		spec,
		byId("output_format").value,
		byId("number_format").value,
		prec
	);
}

function showResult()
{	
	if(!spec.done)
		return;
	
	showOutput();
	
	byId("a_numtaps").innerHTML = spec.finaltaps;
	
	for(var i in spec.bands)
	{
		var b = spec.bands[i];
		b.showRipple(b.actripple);
	}
	
	updateGraphs();
}

function doDesign()
{
	clearLog();
	spec.bands.sort(function(a,b){return a.start > b.start ? 1 : -1;});
	optimizeFilter(spec);
	spec.done = true;
	showResult();
	designTag.className = "designButton";
	designTag.innerHTML = "DESIGN&nbsp;FILTER"
}

function checkConsistency()
{
	var overlap = getOverlappingBands(spec);
	if(overlap != false)
	{
		alert("Error! There are overlapping bands. Please leave a transition band between the passbands and stopbands.");
		graph.setSelection(overlap);
		return false;
	}
	var empty = true;
	for(var i in spec.bands)
		empty = false;
	if(empty)
	{
		alert("Error! There are no specified bands.");
		return false;
	}
	
	for(var i in spec.bands)
	{
		var b = spec.bands[i];
		if(b.gain == 0)
		{
			if(b.ripple >= 0)
			{
				alert("Error! For all stopbands (where gain = 0), the attenuation must be lower than 0 dB.");
				selectBand(b);
				return false;
			}
		}
		if(b.gain > 0)
		{
			if(b.ripple <= 0)
			{
				alert("Error! For all passbands (where gain > 0), the ripple must be higher than 0 dB.");
				selectBand(b);
				return false;
			}
		}
		if(b.gain < 0)
		{
            alert("Error! For all bands gain must be positive.");
            selectBand(b);
            return false;
		}
        if(b.stopHz > spec.samp_freq/2 + 0.001)
        {
            alert("Error! All frequencies must be less than or equal to one half of the sampling frequency.");
            selectBand(b);
            return false;        
        }

	}
	return true;
}

function design()
{
	if(!checkConsistency())
		return;
		
	designTag.innerHTML = "IN PROGRESS.."
	designTag.className = "designInProgress";
	setTimeout("doDesign()", 100);
}

var sampFreqChooser;
var tapChooser;


function applyLayout()
{
	myLayout = $('body').layout({
		applyDefaultStyles: true,
		closable:				false,
		resizable:				false,
		slidable:				false,
		south__size: 135,
		east__size: 215,
		east__closable: false,
//		resizeWhileDragging: true, 
		onresize: function() {updateGraphs();},
		east__onclose: function() {updateGraphs();},
		east__onopen: function() {updateGraphs();}
	});
	myLayout.panes.south.css("border", "none");
	//myLayout.panes.center.css("border", "none");
	myLayout.panes.south.css("padding", "0px");
	myLayout.panes.south.css("margin", "0px");
	myLayout.panes.center.css("padding", "0px");
	myLayout.panes.center.css("margin", "0px");

	myLayout.panes.east.css("padding", "4px");

	var menuLayout = $(byId('menu_layout')).layout({
		applyDefaultStyles: true,
		closable: false,
		resizable: false,
		slidable: false,
		north__size: 25
		//north__closable: false
	});	
	menuLayout.panes.north.css("border", "none");
	//$(byId("main_tile")).css("padding", "0px");
	
/*	jQuery('.ui-layout-pane').each( function() {
  		var el = jQuery(this);
  		el.width( el.width() + 2 );
	} );*/
//	menuLayout.panes.center.css("border", "none");
}

$(document).ready(function () {

	sampFreqChooser = addFreqChooser(byId("samp_fr_wrapper"), function(fr){
		spec.samp_freq = fr;
		specChanged();
	});
	
	sampFreqChooser.value = "2000 Hz";
	
	tapChooser = addTapChooser(byId("tap_chooser_wrapper"), function(taps){
		spec.desiredtaps = taps;
		specChanged();
	});
	
	tapChooser.value = "minimum";	
	
	applyLayout();
	
	/*$(byId('bottom_bar')).layout({
		applyDefaultStyles: true,
		closable:				false,
		resizable:				false,
		slidable:				false,
		west__size: 500
	});*/
 
	addBand({startHz: 0, stopHz: 400, gain: 1, ripple: 5});
	addBand({startHz: 500, stopHz: 1000, gain: 0, ripple: -40});
/*		addBand({startHz: 0, stopHz: 37, gain: 16, ripple: 1});
		addBand({startHz: 62, stopHz: 1000, gain: 0, ripple: -5});*/

//	$('#bandlist_table').tableScroll({height:100});
	
	initReady = true;
	specChanged();
});

var predefined = {
	"0": function(){
	
	},
	
	"lowpass": function(){
		addBand({startHz: 0, stopHz: 400, gain: 1, ripple: 5});
		addBand({startHz: 500, stopHz: 1000, gain: 0, ripple: -40});
	},
	
	"highpass": function(){
		addBand({startHz: 0, stopHz: 400, gain: 0, ripple: -40});
		addBand({startHz: 500, stopHz: 1000, gain: 1, ripple: 5});
	},
	
	"bandpass": function(){
		addBand({startHz: 0, stopHz: 200, gain: 0, ripple: -40});
		addBand({startHz: 300, stopHz: 500, gain: 1, ripple: 5});	
		addBand({startHz: 600, stopHz: 1000, gain: 0, ripple: -40});	
	},
	
	"bandstop": function(){
		addBand({startHz: 0, stopHz: 200, gain: 1, ripple: 5});
		addBand({startHz: 300, stopHz: 500, gain: 0, ripple: -40});	
		addBand({startHz: 600, stopHz: 1000, gain: 1, ripple: 5});	
	}
};

function setPredefined(p)
{
	if(p == '0')
		return;
	if(!confirm("Do you want to drop your current configuration and replace it by a predefined " + p + " one?"))
		return;
	initReady = false;
	
	for(var i in spec.bands)
		deleteBand(spec.bands[i]);
		
	sampFreqChooser.value = "2000 Hz";
	spec.samp_freq = 2000;
	
	spec.desiredtaps = "0";
	tapChooser.value = "minimum";
	
	predefined[p]();
	initReady = true;
	specChanged();
}

function generateCode()
{
	if(!spec.done) {
	    byId('generated_code').innerHTML = "// Filter not ready.";
   	 	byId('generated_header').innerHTML = "";
    	sh_highlightDocument();
       	return;
	}

	var coeff = byId("cgen_coefftype").value;
	var accum = byId("cgen_accumulatortype").value;
	var prec = byId("cgen_precision").value;
	
	if(byId("cg_number_format").value == "float") {
		coeff = accum = byId("cgen_floattype").value;
		prec = 0;
    }
    
    var settings = {
        name: byId("cgen_filtername").value,
        num_coeffs: spec.taps.length,
        coeff_type: coeff,
        accum_type: accum,
        shift: prec,
        unroll: byId("cgen_unroll").checked
    };
    
    byId('generated_code').innerHTML = getGeneratedCode(settings);
    byId('generated_header').innerHTML = getGeneratedHeader(settings);
    byId('cgen_cfilename').innerHTML = getGeneratedFileName(settings);
    byId('cgen_hfilename').innerHTML = getGeneratedFileName(settings);
    
    sh_highlightDocument();
}

function changeNumberFormat()
{
	var format = byId("cg_number_format").value;
	
	showElement(byId('cgenrow_coefftype'), format == "int");
	showElement(byId('cgenrow_precision'), format == "int");
	showElement(byId('cgenrow_accumulatortype'), format == "int");
	
	showElement(byId('cgenrow_floattype'), format != "int");
	
	generateCode();
}