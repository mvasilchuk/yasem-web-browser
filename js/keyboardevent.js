(function(keyCode, which, alt, ctrl, shift, name)
{
    if(elem == undefined) return;

    var event = document.createEvent('Events');
    event.keyCode = keyCode;
    event.which = which;
    event.altKey = alt;
    event.ctrlKey = ctrl;
    event.shiftKey = shift;
    event.initEvent(name, true, true);
    return elem.dispatchEvent(event);
})(%1, %2, %3, %4, %5, '%6')
