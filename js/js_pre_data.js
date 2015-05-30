var activeFrame =  window, elem = null;
for(var index = 0; index < frames.length; index++)
{
   var frame = frames[index];
   activeFrame = frame;
}
if(document.activeElement) {
    elem = document.activeElement;

    if(elem.nodeName.toLowerCase() == 'iframe')
    {
        elem = activeFrame.document.activeElement;
    }
}
else  { elem = activeFrame.document.body;}
elem.focus();
