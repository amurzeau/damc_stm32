/* 

	Copyright (c) 2011 Peter Isza (peter.isza@gmail.com)

*/

function byId(a)
{
	return document.getElementById(a);
}

function deleteFromArray(a, o)
{
	for(var i in a)
		if(a[i] == o)
			delete a[i];
}

function showElement(obj, show)
{
	if(show)
		obj.style.display = "";
	else
		obj.style.display = "none";
}

function fnSelect(obj)
{
   fnDeSelect();
   if (document.selection) 
   {
      var range = document.body.createTextRange();
      range.moveToElementText(obj);
      range.select();
   }
   else if (window.getSelection) 
   {
      var range = document.createRange();
      range.selectNode(obj);
      window.getSelection().addRange(range);
   }
}

function fnDeSelect() 
{
   if (document.selection)
             document.selection.empty();
   else if (window.getSelection)
              window.getSelection().removeAllRanges();
} 