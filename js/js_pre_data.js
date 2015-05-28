var activeFrame =  window, elem = null;
for(var index = 0; index < frames.length; index++)
{
   var frame = frames[index];
   activeFrame = frame;
}
if(document.activeElement) {
    elem = document.activeElement;
    console.log(1, elem.nodeName);

    if(elem.nodeName.toLowerCase() == 'iframe')
    {
        elem = activeFrame.document.activeElement; console.log(2, elem);
    }
}
else  { elem = activeFrame.document.body; console.log(3, elem); }
elem.focus();
