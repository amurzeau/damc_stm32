/* 

	Copyright (c) 2011 Peter Isza (peter.isza@gmail.com)

*/

function addFreqChooser(parent, onchange, limit)
{
	if(typeof(limit) == 'undefined')
		limit = function(r){return r};
	
    var input = document.createElement("input");
    input.type = "text";
    input.className = "freqChooser";
    input.onchange = function() {
        orig_val = parseFloat(this.value);

    	if(typeof(orig_val) != 'number' || orig_val < 0)
    		orig_val = 0;
    	
    	var val = 0;
    	
    	if(this.value.search("[kK][hH][zZ]") != -1)
    	{
    		val = limit(orig_val * 1000);
    		orig_val = val / 1000;
  			this.value = orig_val + " kHz";
    	}
    	else if(this.value.search("M[hH][zZ]") != -1)
    	{
    		val = limit(orig_val * 1000000);
    		orig_val = val / 1000000;
  			this.value = orig_val + " MHz";
    	}
    	else if(this.value.search("m[hH][zZ]") != -1)
    	{
    		val = limit(orig_val / 1000);
    		orig_val = val * 1000;
  			this.value = orig_val + " mHz";
    	}
     	else
    	{
    		val = limit(orig_val);
    		orig_val = val;
  			this.value = orig_val + " Hz";
    	}
   	
    	onchange(val);
    };

    parent.appendChild(input);
    return input;
}

function addTapChooser(parent, onchange)
{
    var input = document.createElement("input");
    input.type = "text";
    input.className = "tapChooser";
    input.title = "An integer, or nothing for minimum number of taps (slower)."
    input.onchange = function() {
        var val = parseInt(this.value);

    	if(this.value == "" || typeof(val) != 'number' || isNaN(val))
    		val = 0;

    	if(val == 0)
    		this.value = "minimum";
    	else
    		this.value = val;
    	
    	onchange(val);
    };
    
    parent.appendChild(input);
    return input;
}
function addDbChooser(parent, onchange, onlydb)
{
    var input = document.createElement("input");
    input.type = "text";
    input.className = "dbChooser";
    input.onchange = function(){
    	orig_val = parseFloat(this.value);
    	
    	if(typeof(orig_val) != 'number')
    		orig_val = 0;
    	
    	var val = 0;
    	
    	if(this.value.search("d[bB]") != -1 || onlydb)
    	{
    		val = Math.pow(10, orig_val/20);
			if(onlydb)
				val = orig_val;
  			this.value = orig_val + " dB";
    	}
    	else
    	{
    		val = orig_val;
  			this.value = val;
    	}
    	
    	onchange(val);
    };
    
    parent.appendChild(input);
    return input;
}

function addBandWidget(parent, band)
{
	var freqLimit = function(r) {
		if(r < 0)
			return 0;
		if(r > spec.samp_freq / 2)
			return spec.samp_freq / 2;
		return r;
	};

    var td;
    var tr = document.createElement("tr");
	tr.onclick = function(){
		selectBand(band);
		setTimeout("clearBandSelection()", 500);
	};

	//band.showWidth = function(){};
	band.showRipple = function(r){};
	
	// from
	td = document.createElement("td");
	td.className = "mycell";
	var ch = addFreqChooser(td, function(v){band.startHz = v; specChanged(); }, freqLimit);
	ch.value = band.startHz;
	ch.onchange();
	tr.appendChild(td);
	
    parent.appendChild(tr);
    
    // width
	/*var wtd = document.createElement("td");
	wtd.className = "bandWidth";
	band.showWidth = function(){
		var x = (band.stop - band.start);
		if(typeof(x) != 'number')
			return;
		wtd.innerHTML = x.toFixed(2);
	};
	
	tr.appendChild(wtd);*/
	

    // to
	td = document.createElement("td");
	td.className = "mycell";
	ch = addFreqChooser(td, function(v){band.stopHz = v; specChanged();  }, freqLimit);
	ch.value = band.stopHz;
	ch.onchange();
	tr.appendChild(td);

    // gain
	td = document.createElement("td");
	td.className = "mycell";
	ch = addDbChooser(td, function(v){band.gain = v; specChanged();}, false);
	ch.value = band.gain;
	ch.onchange();
	tr.appendChild(td);

    // ripple
	td = document.createElement("td");
	td.className = "mycell";
	ch = addDbChooser(td, function(v){band.ripple = v; specChanged();}, true);
	ch.value = band.ripple;
	ch.onchange();
	tr.appendChild(td);

    // a.ripple
	var rtd = document.createElement("td");
	rtd.className = "actualRipple mycell";
	band.showRipple = function(r){
		if(typeof(r) != 'number')
		{
			rtd.className = "actualRipple mycell";
			rtd.innerHTML = "";
			return;
		}
		
		if(r < band.ripple)
			rtd.className = "actualRipple passed mycell";
		else
			rtd.className = "actualRipple failed mycell";
		rtd.innerHTML = r.toFixed(2) + " dB";
	};
	tr.appendChild(rtd);

    // actions
	td = document.createElement("td");
	td.className = "mycell";
	var btn = document.createElement("a");
	btn.onclick = function(){deleteBand(band)};
	btn.href = "javascript:";
	btn.innerHTML = "";
	btn.title = "Delete band";
	btn.className = "deleteButton iconButton"
	td.appendChild(btn);
	tr.appendChild(td);
	
	band.widget = tr;
}

function showAboutBox()
{
	byId('about').style.display='block';
}

function showPrecisionChooser()
{
	if(byId('number_format').value == "int")
	{
		byId('precisionChooser').style.display = "block";
		byId('precisionChooser').className = "header";
	}
	else
	{
		byId('precisionChooser').style.display = "none";	
		byId('precisionChooser').className = "ui-layout-ignore";
	}
	applyLayout();
}

function fadeIn(obj, val)
{
	if(val == undefined)
		val = 0;
	if(val >= 1)
		val = 1;
	obj.style.opacity = val;
	if(val < 1)
		setTimeout(function() {fadeIn(obj, val+0.05);}, 50); 
}

function changeGraph(which)
{
	/*setTimeout(function() {
		var hm = byId('hireme');
		if(hm.style.display != 'block') {
			hm.style.opacity = 0;
			hm.style.display = 'block';
			fadeIn(hm);
		}
	}, 500);*/
    whichGraph = which;
    byId('m_freq_gr').className = (which == 'freq') ? "topMenuItem selectedTopMenuItem" : "topMenuItem";
    byId('m_time_gr').className = (which == 'time') ? "topMenuItem selectedTopMenuItem" : "topMenuItem";
    byId('m_code_gr').className = (which == 'code') ? "topMenuItem selectedTopMenuItem" : "topMenuItem";
    byId('m_alma_gr').className = (which == 'alma') ? "topMenuItem selectedTopMenuItem" : "topMenuItem";
    byId('m_entr_gr').className = (which == 'entr') ? "topMenuItem selectedTopMenuItem" : "topMenuItem";
    byId('m_feat_gr').className = (which == 'feat') ? "topMenuItem selectedTopMenuItem" : "topMenuItem";
    updateGraphs();
}

