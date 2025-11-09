/* 

	Copyright (c) 2011 Peter Isza (peter.isza@gmail.com)

*/

var graph;
var response_graph;

function selectBand(band)
{
	graph.setSelection({x1: band.startHz, x2: band.stopHz});
}

function clearBandSelection()
{
	graph.clearSelection();
}

function updateMainGraph()
{	
	var d1 = [];
	var d2 = [];
	var d3 = [];
	
	if(spec.done)
	    for (var i = 0; i < 1; i += 1/640)
    	    d1.push([i*spec.samp_freq/2, getMagnitude(spec, i)]);

    d2.push([0, undefined]);
	var last = 0;
	for(var i in spec.bands)
	{
		var b = spec.bands[i];
		
		d2.push([last, undefined]);
		d3.push([last, undefined]);
		var db, dblimit;
		
		if(b.gain == 0)
		{
			db = undefined;
			dblimit = b.ripple;
			d3.push([b.startHz, dblimit]);
			d3.push([b.stopHz, dblimit]);
		}
		else
		{
			db = getDecibel(b.gain);
			var dbupper = getDecibel(b.upper);
			var dblower = getDecibel(b.lower);
			d3.push([b.startHz, dbupper]);
			d3.push([b.stopHz, dbupper]);
			d3.push([b.stopHz, undefined]);
			d3.push([b.startHz, dblower]);
			d3.push([b.stopHz, dblower]);
		}
		
		d2.push([b.startHz, db]);
		d2.push([b.stopHz, db]);

		last = b.stopHz;
	}
    
    d2.push([spec.samp_freq/2, undefined]);
    
    byId("magnitude_graph").style.display = "none";
    
    byId("magnitude_graph").style.height = (byId("main_tile").clientHeight - 40);
    byId("magnitude_graph").style.width = (byId("main_tile").clientWidth*0.92);

    byId("magnitude_graph").style.display = "block";

    graph = $.plot($("#magnitude_graph"), 
    	[
    		{data: d3, color: "darkgrey", label: "ripple bounds"},
    		{data: d2, color: "orange", label: "desired gain"},
    		{data: d1, color: "red", label: "actual gain"}
		],

    	{
    		selection: { mode: "x" }/*,
            xaxis: {
                axisLabel: 'frequency (Hz)',
                axisLabelUseCanvas: false,
                axisLabelFontSizePixels: 15,
            },
            yaxis: {
                axisLabel: 'gain (dB)',
                axisLabelUseCanvas: true
            }*/
    	}
    );

}

function updateResponseGraph()
{
	var d1 = [];
    var d2 = [];
    var dh = [];
	
	if(spec.done)
    {
        for(var i in spec.hiresTaps)
        {
            var r = spec.hiresTaps[i];
            
    	    /*d1.push([i/spec.samp_freq, 0]);
    	    d1.push([i/spec.samp_freq, r]);
    	    d1.push([i/spec.samp_freq, undefined]);*/

    	    dh.push([i/spec.samp_freq, r]);
        }
        for(var i in spec.taps)
        {
            var r = spec.taps[i];
            
    	    d1.push([i/spec.samp_freq, 0]);
    	    d1.push([i/spec.samp_freq, r]);
    	    d1.push([i/spec.samp_freq, undefined]);
    	
            d2.push([i/spec.samp_freq, r]);
        }
    }
    //d1.push([0, undefined]);
    
    byId("magnitude_graph").style.display = "none";
    
    byId("magnitude_graph").style.height = (byId("main_tile").clientHeight - 40);
    byId("magnitude_graph").style.width = (byId("main_tile").clientWidth*0.92);

    byId("magnitude_graph").style.display = "block";

    response_graph = $.plot($("#magnitude_graph"), 
    	[
    		{data: dh, color: "pink" },
    		{data: d1, color: "red", label: "impulse response"},
    		{data: d2, color: "red", points: {show:true}}
        ],

    	{
    		selection: { mode: "x" }/*,
            xaxis: {
                axisLabel: 'frequency (Hz)',
                axisLabelUseCanvas: false,
                axisLabelFontSizePixels: 15,
            },
            yaxis: {
                axisLabel: 'gain (dB)',
                axisLabelUseCanvas: true
            }*/
    	}
    );
    
}

function showGraphWindow(which)
{
    byId("code_generator").style.display = (which == "code") ? "block" : "none";
    byId("graph_box").style.display = (which == "time" || which == "freq") ? "block" : "none";
    byId("description_box").style.display = (which == "desc") ? "block" : "none";
    byId("enterprise_box").style.display = (which == "entr") ? "block" : "none";
    byId("feature_box").style.display = (which == "feat") ? "block" : "none";
    byId("almafa_box").style.display = (which == "alma") ? "block" : "none";
    if(which == "code")
    {
    	var layout = $(byId('code_generator')).layout({
            applyDefaultStyles: true,
            closable:				false,
            resizable:				false,
            slidable:				false,
            west__size: 300
        });
        layout.panes.west.css("border", "none");
        layout.panes.center.css("border", "none");
    }
}

function updateGraphs()
{
    showGraphWindow(whichGraph);

    if(whichGraph == "freq")
        updateMainGraph();
    if(whichGraph == "time")
        updateResponseGraph();
    if(whichGraph == "code")
    {
     	changeNumberFormat();
        generateCode();
    }
}