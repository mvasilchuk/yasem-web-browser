(function()
{
    var ready = function()
    {
        console.log('Applying patch on objects!');
        var objects = document.getElementsByTagName('object');
        console.log('Objects found: ' + objects.length);

        for(var index = 0; index < objects.length; index++) {
                var obj = objects[index];
                console.log(obj + ', ' + obj.getAttribute('classid') + ", id: " + obj.getAttribute('id'));

                if(!obj.type)
                {
                    obj.setAttribute('type', 'application/x-qt-plugin');
                }

                if(obj.getAttribute('class') === 'hidden')
                {
                    obj.removeAttribute('class');
                    obj.setAttribute('width', '0');
                    obj.setAttribute('height', '0');
                }

                if(obj.style.visibility === 'hidden')
                {
                    console.warn('Object is hidden. Trying to hide it using float:left');
                    obj.style.float = 'left';
                }

                if(obj.getAttribute('width') == "")
                    obj.setAttribute('width', '0');
                if(obj.getAttribute('height') == "")
                    obj.setAttribute('height', '0');

                //console.log('info:' + obj.GetPluginInfo(0));
        }


        for(var index = 0; index < objects.length; index++) {
            var obj = objects[index];
            if(obj.getAttribute('type') != 'application/x-qt-plugin') continue;
            try
            {
                console.log('info:' + obj.id + ': ' + obj.GetPluginInfo(0));
            }
            catch(e)
            {
                console.log(obj.id + ': ' + e);
            }

            if(obj.id == 'pluginMAC')
            {
                var el = document.getElementById("pluginMAC");
                alert('FFFFUUUUUUU:  ' +  el.CreatePlugin())
                debugger;
            }

        }

        console.log('Patch applied!');
    }

    __OBJECTS_FIX__ = ready;

    document.onreadystatechange = function(e){
        if(document.readyState === 'complete')
        {
            ready();
        }
    };

    /*var old_getElementById = document.getElementById;
    var getEl = function(id)
    {
        var el = old_getElementById.call(document, id);
        try{
            if(el.tagName.toLowerCase() === 'object' && !el.getAttribute('type'))
            {
                console.log('AAA:' + el.getAttribute('id'));
                el.setAttribute('type', 'application/x-qt-plugin');
                el.removeAttribute('style');
                el.setAttribute('width', '0');
                el.setAttribute('height', '0');
                for(var iid in el)
                {
                    console.log(iid + ': ' + el[iid]);
                }

                el = old_getElementById.call(document, id);

                console.log(el.CreatePlugin);
            }
        }
        catch(e)
        {
            console.error(e);
        }

        return el;
    }

    document.getElementById = getEl;*/

})();
