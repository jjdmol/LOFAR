function Ajax()
{
    this.request = null;

    if(window.XMLHttpRequest)
    {
        this.request = new XMLHttpRequest();
        if(this.request.overrideMimeType)
        {
            this.request.overrideMimeType("application/xml");
        }
    }
    else if(window.ActiveXObject)
    {
        try
        {
            this.request = new ActiveXObject("Msxml2.XMLHTTP");
        }
        catch(_ex)
        {
            try
            {
                this.request = new ActiveXObject("Microsoft.XMLHTTP");
            }
            catch(_ex)
            {
                alert("could not create XMLHTTP instance");
            }
        }
    }

    this.stub = Ajax_stub;
    this.get = Ajax_get;
    this.post = Ajax_post;
}


function Ajax_stub(callback)
{
    if(this.request.readyState == 4)
    {
        /*
            Safari generates a call with request.status == undefined
            when navigating away from a page when a request is active
        */
        if(!this.request.status)
        {
            return;
        }

        if(this.request.status == 200)
        {
            callback(this.request);
        }
        /*
        else if(this.request.status == 400)
        {
            alert("AJAX request returned an error:\n" + this.request.responseText);
        }
        */
        else
        {
            alert("AJAX request returned an error (code: " + this.request.status + ").");
        }
    }
}


function Ajax_get(url, callback)
{
    if(this.request && (this.request.readyState == 0 || this.request.readyState == 4))
    {
        /*
            seems required, else IE won't fetch the same url twice.
            (may be caching-related?)
        */
        this.request.abort();

        var instance = this;
        this.request.onreadystatechange = function ()
        {
            instance.stub(callback);
        }

        this.request.open("GET", url, true);
        this.request.send(null);
        return true;
    }

    /*
            fall-through
    */
    return false;
}


function Ajax_post(url, data, callback)
{
    if(this.request && (this.request.readyState == 0 || this.request.readyState == 4))
    {
        /*
            seems required, else IE won't fetch the same url twice.
            (may be caching-related?)
        */
        this.request.abort();

        var instance = this;
        this.request.onreadystatechange = function ()
        {
            instance.stub(callback);
        }

        this.request.open("POST", url, true);
        this.request.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
        this.request.send(data);

        return true;
    }

    /*
        fall-through
    */
    return false;
}
