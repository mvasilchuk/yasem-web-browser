(function(name)
{
    var event = document.createEvent('MouseEvents');
    event.initMouseEvent(name, true, true, window, 1, 0, 0, 0, 0, false, false, false, false, 0, null);

    if(elem.onclick)
    {
        //elem.onclick(event);
    }

    return elem.dispatchEvent(event);
})('%1')
