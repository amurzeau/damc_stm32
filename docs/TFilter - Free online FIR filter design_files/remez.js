
/**************************************************************************
 * Parks-McClellan algorithm for FIR filter design (javascript version)
 *
 *-------------------------------------------------
 *  Copyright (c) 2011 Peter Isza (peter.isza@gmail.com)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *	Here it is: http://t-filter.appspot.com/fir/GPL
 *
 *  This code is loosely based on the work of 
 *  Jake Janovetz (janovetz@uiuc.edu).
 *  
 *************************************************************************/
 
/*
 *
 * This code is a huge unstructured mess. It would need a major refactoring.
 * If you are interested in developing t-filter, just drop me an email to
 * peter.isza@gmail.com, and I will allocate time to get it in a better shape
 * before someone touches it. 
 *
 */
 
 
function getOverlappingBands(spec)
{
	var last = -1;
	for(var i in spec.bands)
	{
		var b = spec.bands[i];
		if(last >= b.startHz)
			return {x1: last, x2: b.startHz};
		last = b.stopHz;
	}
	return false;
}
 
function getLinearFreq(spec, w)
{
	for(var i in spec.bands)
	{
		var b = spec.bands[i];
		if(w < b.stop - b.start)
			return b.start + w;
		else
			w -= b.stop - b.start;
	}
}

function initFrequencies(spec)
{
	spec.sum_bands = 0.0;
	
	spec.frequencies = new Array();
	
	for(var i in spec.bands)
	{
		var b = spec.bands[i];
		spec.sum_bands += b.stop - b.start;
	}
	
	var step = spec.sum_bands / (spec.r+2);
	
	for(i = 0; i < spec.r+1; i++)
		spec.frequencies[i] = getLinearFreq(spec, step*(i+1));	
}

function getDesiredGain(spec, w)
{
	for(var i in spec.bands)
	{
		var b = spec.bands[i];
		if(w >= b.start && w <= b.stop)
			return (spec.numtaps % 2 != 0) ? b.gain : b.gain / Math.cos(w * Math.PI / 2);
	}
	return 0;
}

function getBand(spec, w)
{
	for(var i in spec.bands)
	{
		var b = spec.bands[i];
		if(w >= b.start && w <= b.stop)
			return b;
	}
	return null;
}

function isInLimits(spec, w)
{
	for(var i in spec.bands)
	{
		var b = spec.bands[i];
		if(w >= b.start && w <= b.stop)
		{
			var gain = Math.abs(getActualGain(spec, w));
			//alert(gain + " <" + b.lower + ".." + b.upper + ">");

			if(b.gain != 0)
				return gain >= b.lower && gain <= b.upper;
			else
				return gain <= b.upper;
		}
	}
	
	return true;
}

function getWeight(spec, w)
{
	for(var i in spec.bands)
	{
		var b = spec.bands[i];
		if(w >= b.start && w <= b.stop)
			return (spec.numtaps % 2 != 0) ? b.weight : b.weight * Math.cos(w * Math.PI / 2) ;
	}
	return 0.0;
}

// performs a barycentric Lagrange interpolation based on the 
// precalculated variables and the delta property
function getActualGain(spec, w)
{
	var numerator = 0, denominator = 0;
	var cosw = Math.cos(w * Math.PI);
	
	for(var i in spec.frequencies)
	{
		var c = cosw - spec.x[i];
		if(Math.abs(c) < 1e-12)
		{
			numerator = spec.y[i];
			denominator = 1;
			break;
		}
		c = spec.dn[i] / c;
		denominator += c;
		numerator += c * spec.y[i];
	}

	return numerator / denominator;
}

function getPreciseError(spec, w)
{
	var numerator = 0, denominator = 0, numerator2 = 0;
	var negarray = new Array();
	var posarray = new Array();
	var cosw = Math.cos(w * Math.PI);

	for(var i in spec.frequencies)
	{
		var c = cosw - spec.x[i];
		if(Math.abs(c) < 1e-10)
			return spec.dy[i];
		
		c = 1/c/spec.di[i];
		
		denominator += c;
		var tmp = -c * spec.dy[i];
		if(tmp > 0)
			posarray.push(tmp);
		else
			negarray.push(-tmp);

		tmp = c * (spec.d[i] - getDesiredGain(spec, w));
		
		if(tmp > 0)
			posarray.push(tmp);
		else
			negarray.push(-tmp);
	}
	
	negarray.sort(function(a,b){return Math.abs(a)>Math.abs(b) ? 1 : -1;});
	posarray.sort(function(a,b){return Math.abs(a)>Math.abs(b) ? 1 : -1;});
	
	var pos = 0, neg = 0;
	for(i in posarray)
		pos += posarray[i];
	for(i in negarray)
		neg -= negarray[i];
		
	return - getWeight(spec,w) * ((neg+pos) / denominator);
}
function getDecibel(x)
{
	return 20*Math.log(x)/Math.log(10);
}

function getMagnitude(spec,w)
{
	var x = Math.abs(getActualGain(spec,w));
	if (spec.numtaps % 2 == 0)
		x *= Math.cos(w * Math.PI / 2);
	
	return getDecibel(x);
}
function getTaps(spec)
{
	var taps = new Array();
	var h = new Array();
	
	for(var i = 0; i <= Math.floor(spec.numtaps / 2); i++)
	{
		if(spec.numtaps % 2 == 0)
			c = Math.cos(Math.PI * i / spec.numtaps);
		else
			c = 1;
		taps[i] = getActualGain(spec, i * 2.0 / spec.numtaps) * c;
	}
	console.log("taps: " + JSON.stringify(taps));
	var M = (spec.numtaps - 1.0) / 2.0;
	
	if(spec.numtaps % 2 == 0)
	{
		for(var i = 0; i < spec.numtaps; i++)
		{
			var val = taps[0];
			var x = Math.PI * 2 * (i-M)/spec.numtaps;
			for(var k = 1; k <= spec.numtaps/2 - 1; k++)
 				val += 2.0 * taps[k] * Math.cos(x*k);
 			h[i] = val / spec.numtaps;
		} 
	}
	else
	{
//		var tmpstr = "";
		for(var i = 0; i < spec.numtaps; i++)
		{
			var val = taps[0];
			var x = Math.PI * 2 * (i-M)/spec.numtaps;
			for(var k = 1; k <= M; k++)
			{
//				tmpstr += (2.0 * taps[k] * Math.cos(x*k)) + " ";
 				val += 2.0 * taps[k] * Math.cos(x*k);
 			}
//			tmpstr += "\n";
 			h[i] = val / spec.numtaps;
		} 
//		console.log(tmpstr);
	}
	
	return h;
}

function getHiResTaps(spec, numpoints)
{
	var taps = new Array();
	var h = new Array();
	
	for(var i = 0; i <= Math.floor(spec.numtaps / 2); i++)
	{
		if(spec.numtaps % 2 == 0)
			c = Math.cos(Math.PI * i / spec.numtaps);
		else
			c = 1;
		taps[i] = getActualGain(spec, i * 2.0 / spec.numtaps) * c;
	}
	var M = (spec.numtaps - 1.0) / 2.0;
	
	if(spec.numtaps % 2 == 0)
	{
		for(var i = 0; i < numpoints; i++)
		{
			var val = taps[0];
            var fi = i * (spec.numtaps-1) / numpoints;
            var x = Math.PI * 2 * (fi-M)/spec.numtaps;
			for(var k = 1; k <= spec.numtaps/2 - 1; k++)
 				val += 2.0 * taps[k] * Math.cos(x*k);
 			h[fi] = val / spec.numtaps;
		} 
	}
	else
	{
		for(var i = 0; i < numpoints; i++)
		{
			var val = taps[0];
            var fi = i * (spec.numtaps-1) / numpoints;
			var x = Math.PI * 2 * (fi-M)/spec.numtaps;
			for(var k = 1; k <= M; k++)
 				val += 2.0 * taps[k] * Math.cos(x*k);
 			h[fi] = val / spec.numtaps;
		} 
	}
	
	return h;
}

function getError(spec, w)
{
	return getWeight(spec, w) * (getDesiredGain(spec, w) - getActualGain(spec, w));
}

// calculates the desired amplitude of the alternation (delta)
// based on the corner frequencies
// it also precalculetes some temporary variables
function calcDelta(spec)
{
	spec.x = new Array();
	spec.y = new Array();
	spec.di = new Array();
	spec.dn = new Array();
	var numerator = 0, denominator = 0;
	
	for(var i in spec.frequencies)
		spec.x[i] = Math.cos(spec.frequencies[i] * Math.PI);
		
	for(var i in spec.x)
	{
		spec.di[i] = 1.0;
		for(var j in spec.x)
			if(i != j)
				spec.di[i] *= (spec.x[j] - spec.x[i]);
		spec.dn[i] = 1/spec.di[i];
	}
	
	var sign = 1;
	for(var i in spec.frequencies)
	{
		numerator += getDesiredGain(spec, spec.frequencies[i]) * spec.dn[i];
		denominator += sign / getWeight(spec, spec.frequencies[i]) * spec.dn[i];
		sign = -sign;
	}

	spec.delta = numerator / denominator;
	
	/*if(Math.abs(spec.delta) < 1e-10)
		if(spec.delta >= 0)
			spec.delta = 1e-10
		else
			spec.delta = -1e-10;*/
	
	spec.dy = new Array();
	spec.w = new Array();
	spec.d = new Array();
	sign = 1;
	for(var i in spec.frequencies)
	{
		spec.w[i] =	getWeight(spec, spec.frequencies[i])
		spec.dy[i] = sign * spec.delta / spec.w[i];
		spec.d[i] =	getDesiredGain(spec, spec.frequencies[i]);
		spec.y[i] =	spec.d[i] - spec.dy[i];
			
		sign = -sign;
	}
}

function calcMaxError(extrema)
{
	var maxerror = 0;
	for(var i in extrema)
		if(Math.abs(extrema[i].error) > maxerror)
		{
			maxerror = Math.abs(extrema[i].error);
		}
	return maxerror;
}

function makeAlternating(firstOneSign, candidates)
{
	while(candidates.length > 0 && candidates[0].error * firstOneSign < 0)
		candidates.splice(0,1);
			
	var req_length = spec.r+1;
	while(candidates.length > req_length)
	{		
		for(var i = 1; i < candidates.length; i++)
		{
			if(candidates[i].error * candidates[i-1].error >= 0)
			{
				if(Math.abs(candidates[i].error) > Math.abs(candidates[i-1].error))
					candidates.splice(i-1, 1);
				else
					candidates.splice(i, 1);
				i--;
			}
		}
		if(candidates.length == req_length+1)
		{
			var last = candidates.length - 1;
			if(Math.abs(candidates[0].error) > Math.abs(candidates[last].error))
				candidates.splice(last, 1);
			else
				candidates.splice(0, 1);	
		}					
		else if(candidates.length > req_length)
		{
			var min = 0;
			for(var i in candidates)
				if(Math.abs(candidates[i].error) < Math.abs(candidates[min].error))
					min = i;
			candidates.splice(min, 1);
		}
	}
	
	if(candidates.length < req_length)
		return -1;
		
	return calcMaxError(candidates);
}

function getExtrema(spec, resolution)
{
	var step = spec.sum_bands / (resolution+1);
	
	spec.extrema = new Array();
	var candidates = new Array();
	
	var E = new Array();
	var F = new Array();

	for(var i = 0; i < resolution; i++)
	{
		F[i] = getLinearFreq(spec, spec.sum_bands*(i+1.0)/(resolution+1.0));
		E[i] = getError(spec, F[i]);
	}

   if (((E[0]>0.0) && (E[0]>E[1])) ||
       ((E[0]<0.0) && (E[0]<E[1])))		
		candidates.push({freq: F[0], error: E[0]});
		
   var j = resolution - 1;
   if (((E[j]>0.0) && (E[j]>E[j-1])) ||
       ((E[j]<0.0) && (E[j]<E[j-1])))
		candidates.push({freq: F[j], error: E[j]});
		
	for(var i = 1; i < resolution-1; i++)
		if ((E[i]>=E[i-1] && E[i]>E[i+1] && E[i]>0.0) ||
          	(E[i]<=E[i-1] && E[i]<E[i+1] && E[i]<0.0))
			candidates.push({freq: F[i], error: E[i]});
	
	for(var i in spec.bands)
	{
		var b = spec.bands[i];
		candidates.push({freq: b.start, error: getError(spec, b.start)});
		candidates.push({freq: b.stop, error: getError(spec, b.stop)});		
	}
	
	candidates.sort(function(a,b){return a.freq > b.freq ? 1 : -1;});

	spec.allextrema = new Array();
	firstUp = new Array();
	firstDown = new Array();
	for(var i in candidates)
	{
		spec.allextrema.push(candidates[i]);
		firstUp.push(candidates[i]);
		firstDown.push(candidates[i]);
	}
	firstUpError = makeAlternating(1, firstUp);
	firstDownError = makeAlternating(-1, firstDown);
				
	if(firstUpError > firstDownError)
		spec.extrema = firstUp;
	else
		spec.extrema = firstDown;
		
	spec.maxerror = calcMaxError(spec.allextrema);
	
	delete E;
	delete F;
	
	return spec.extrema.length;
}

function updateFrequencies(spec)
{
	spec.frequencies = new Array();
	var e = spec.extrema;
	e.sort(function(a,b){
	    return Math.abs(a.error)<Math.abs(b.error) ? 1 : -1;
	});
	for(var i in e)
	{
		spec.frequencies.push(e[i].freq);
//		console.log("        f=" + e[i].freq);
		//log(spec.extrema[i].error);
		if(spec.frequencies.length == spec.r+1)
			break;
	}
	spec.frequencies.sort(function(a,b){return Math.abs(a)>Math.abs(b) ? 1 : -1;});
}


function getExtremaAdaptively(spec)
{
	var resolution = spec.r*16;
	var num_extrema = 0;
	
	while(num_extrema < spec.r+1)
	{
		if(resolution > 200000)
			return false;
		
		num_extrema = getExtrema(spec, resolution);
		console.log("resolution = ", resolution);
		resolution *= 10;
	}
	
	return true;
}

function getOnlyDelta(spec)
{
	if(spec.numtaps % 2 == 0)
		spec.r = spec.numtaps / 2;
	else
		spec.r = (spec.numtaps + 1) / 2;
		
	initFrequencies(spec);
	calcDelta(spec);
	
	return spec.delta;
}

function designFilter(spec)
{	
	spec.no_convergence = false;
	spec.requirements_met = false;
	spec.delta_too_small = false;

	if(spec.numtaps % 2 == 0)
		spec.r = spec.numtaps / 2;
	else
		spec.r = (spec.numtaps + 1) / 2;
	
	initFrequencies(spec);
	
	var iter;
	var lasterror = 1;
	var maxiter = 20;
	for(iter = 0; iter < maxiter; iter++)
	{
		calcDelta(spec);
		/*if(Math.abs(spec.delta) < 1e-12)
		{
			spec.delta_too_small = true;
			return;
		}*/
		
		if(!getExtremaAdaptively(spec))
		{
			spec.no_convergence = true;
//			console.log("ERROR: not enough extrema");
			return false;
		}
		var errorchange = lasterror - spec.maxerror;

		console.log("error = " + spec.maxerror + ", errorchange = " + errorchange + "\n");
		if(errorchange >= 0 && errorchange < lasterror * 0.001 && errorchange < 1E-10)
		{
//			console.log("wtf quitting");
			break;
		}
		updateFrequencies(spec);
		lasterror = spec.maxerror;
	}	
	
	spec.requirements_met = true;
	for(var i in spec.allextrema)
	{
		if(!isInLimits(spec, spec.allextrema[i].freq))
		{
			spec.requirements_met = false;
			break;
		}
	}

	if(!spec.requirements_met && iter == maxiter)
	{
		spec.no_convergence = true;
		return;
	}

	
//	spec.requirements_met = getRipples(spec);    
	return;
}

function binarySearch(min, max, isgood)
{	
	while(1)
	{
		var c = Math.round((max+min)/2);
		if(c % 2 == 0)
			c++;
			
		if(isgood(c))
			min = c;
		else
			max = c;
		
		if(max - min < 3)
		{
			return min;
		}
	}
}

function getRipples(spec)
{
	for(var i in spec.bands)
	{
		var b = spec.bands[i];
		if(b.gain > 0)
			b.maxgain = b.mingain = getDecibel(b.gain);
		else
			b.maxgain = b.mingain = -1000;
	}
	
	for(var i in spec.allextrema)
	{
		var w = spec.allextrema[i].freq;
		var b = getBand(spec, w);
		
		if(b == null)
			continue;
			
		var g = getMagnitude(spec, w);
		if(g > b.maxgain)
			b.maxgain = g;
		if(g < b.mingain)
			b.mingain = g;
	}
	
	var ok = true;
	for(var i in spec.bands)
	{
		var b = spec.bands[i];
		if(b.gain > 0)
			b.actripple = b.maxgain - b.mingain;
		else
			b.actripple = b.maxgain;
		
		if(b.actripple > b.ripple)
			ok = false;
	}	
	
	return ok;
}

function optimizeFilter(spec)
{
	spec.numtaps = 21;
	
	var deltalimit = 1e-12;
	
	while(1)
	{
		var delta = Math.abs(getOnlyDelta(spec));
		if(delta < deltalimit && delta > 0)
			break;
		else
			spec.numtaps = Math.round(spec.numtaps * 1.5);
			
		if(spec.numtaps > 2000)
			break;
	}
	
	//alert("delta max = " + spec.numtaps);

	var opt = binarySearch(1, spec.numtaps, function(c){
		spec.numtaps = c;
		return Math.abs(getOnlyDelta(spec)) > deltalimit;
	}) + 2;
	
	if(spec.desiredtaps != 0)
	{
		if(opt > spec.desiredtaps)
		{
			opt = spec.desiredtaps;
		}
		else
			alert("The specified tap number can't be achieved due to the limitations of javascript's floating point precision. Try specifying narrower transition bands.");

	}
	
	if(spec.desiredtaps == 0)
	{
		opt = binarySearch(1, opt, function(c){
			//alert("trying " + c);
			spec.numtaps = c;
			designFilter(spec);
			return !spec.requirements_met || spec.no_convergence;
		});
		spec.numtaps = opt + 2;
	}
	else
	{
		spec.numtaps = opt;
	}
	
	designFilter(spec);
	var h = getTaps(spec);
	spec.taps = h;
    spec.hiresTaps = getHiResTaps(spec, 641);
	result.taps = h;

	
	getRipples(spec);
	
	
	spec.finaltaps = spec.numtaps;
	//alert(spec.numtaps);
}