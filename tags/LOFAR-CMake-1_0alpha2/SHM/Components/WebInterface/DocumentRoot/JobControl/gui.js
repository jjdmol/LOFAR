var ajax = new Ajax();
var sort_params = null, filter_params = null;
var selection = new Array();
var cycle_count = 0, indicator = new Array();
var busy = false, busy_interval = 0;

function init_gui()
{
    var i;

    for(i=0; i<8; i++)
    {
        indicator[i] = document.getElementById("indicator" + i);
    }

    refresh_queue();
    window.setInterval(refresh_queue, 5000);
}


function update_indicator()
{
    indicator[cycle_count].style.backgroundColor = "#707070";
    cycle_count = (cycle_count + 1) & 7;
    indicator[cycle_count].style.backgroundColor = "#ffffff";
}


function refresh_queue()
{
    var refresh_params = "";

    if(sort_params && filter_params)
    {
        refresh_params = sort_params + "&" + filter_params;
    }
    else if(sort_params)
    {
        refresh_params = sort_params;
    }
    else if(filter_params)
    {
        refresh_params = filter_params;
    }

    ajax.post("job_queue.psp", refresh_params, xml_callback);
}


function xml_callback(request)
{
    var xmldoc, xhtml_fragment, job_descriptions;
    var container, table, new_selection;
    var i, j;

    /*
        reset busy flag, update indicator.
    */

    if(busy)
    {
        busy = false;
        window.clearInterval(busy_interval);
    }
    else
    {
        update_indicator();
    }

    xmldoc = request.responseXML;
    xhtml_fragment = xmldoc.getElementsByTagName("xhtml")[0].firstChild.nodeValue;
    job_descriptions = xmldoc.getElementsByTagName("job");

    container = document.getElementById("queue_container");
    container.innerHTML = xhtml_fragment;

    /*
        insert XHTML into encapsulating div. first, try using DOM methods; innerHTML
        is used as a fallback (Opera's support of innerHTML is not good enough).
        (NOTE: the innerHTML attribute of <table> and <tr> elements is read-only
         in IE... *sigh*)
    */

    /*
    try
    {
        var parser, parsed_fragment, updated_queue;

        parser = new DOMParser();
        parsed_fragment = parser.parseFromString(xhtml_fragment, "application/xhtml+xml");
        updated_queue = document.importNode(parsed_fragment.documentElement, true);
        container.replaceChild(updated_queue, document.getElementById("queue"));
    }
    catch(_ex)
    {
        container.innerHTML = xhtml_fragment;
    }
    */

    /*
        get a reference to the updated queue
    */
    table = document.getElementById("queue");

    /*
        - insert an associated checkbox in each row that describes a job.
        - remove unused checkboxes from selection.
    */
    new_selection = new Array();
    j = 0;

    for(i = 0; i < table.rows.length; i++)
    {
        var id, name, key, row_class, cell, cb;

        /*
            IE uses className instead of class...
        */
        if(table.rows[i].getAttribute("className"))
        {
            row_class = table.rows[i].getAttribute("className");
        }
        else
        {
            row_class = table.rows[i].getAttribute("class");
        }


        if(row_class == "header")
        {
            continue;
        }

        if(j >= job_descriptions.length)
        {
            break;
        }

        id = job_descriptions[j].getElementsByTagName("id")[0].firstChild.nodeValue;
        name = job_descriptions[j].getElementsByTagName("state")[0].firstChild.nodeValue;
        key = "id" + id;

        if(!selection[key] || selection[key].name != name)
        {
            try
            {
                /*
                    IE cannot set name attribute at runtime on input elements
                    created with createElement(); wtf??!!!?!
                    http://msdn.microsoft.com/library/default.asp?url=/workshop/author/dhtml/reference/methods/createelement.asp
                */
                cb = document.createElement("<input name=" + name + ">");
            }
            catch(_ex)
            {
                cb = document.createElement("input");
            }

            cb.setAttribute("name", name);

            /*
                Konqueror seems to ignore assignment to .type;
                setAttribute() does the trick.
            */

            cb.setAttribute("type", "checkbox");
            cb.setAttribute("value", id);

            if(selection[key])
            {
                if(selection[key].checked)
                {
                    cb.setAttribute("checked", "checked");
                }
                selection[key] = null;
            }

            new_selection[key] = cb;
        }
        else
        {
            new_selection[key] = selection[key];
        }

        /*
            IE fix; appendChild() resets checked attribute to defaultChecked?
        */

        new_selection[key].setAttribute("defaultChecked", new_selection[key].checked);

        cell = table.rows[i].insertCell(0);
        cell.appendChild(new_selection[key]);
        j++;
    }
    selection = new_selection;
}


function stub_callback(request)
{
    window.setTimeout("refresh_queue()", 10);
}


function encode_selection()
{
    var sel_string = "", first = true, cb = null;

    for(var id in selection)
    {
        cb = selection[id];

        if(cb.checked)
        {
            if(first)
            {
                first = false;
                sel_string += cb.value;
            }
            else
            {
                sel_string += "," + cb.value;
            }
        }
    }
    return sel_string;
}


function sort(column, direction)
{
    if(column)
    {
        sort_params = "sort_by=" + column;

        if(direction)
        {
            sort_params = sort_params + "&sort_dir=" + direction;
        }
    }

    refresh_queue();
}


function filter()
{
    if(document.forms["filter"].filter_txt.value == "")
    {
        filter_params = null;
        document.styleSheets[0].cssRules[4].style.backgroundColor = "#b2cbd7";
    }
    else
    {
        filter_params = "filter=" + encodeURIComponent(document.forms["filter"].filter_txt.value);
        document.styleSheets[0].cssRules[4].style.backgroundColor = "#b0aac3";
    }

    refresh_queue();
}


function start()
{
    var sel_string, result;

    if(busy)
    {
        return;
    }

    sel_string = encode_selection();
    if(sel_string.length > 0)
    {
        result = ajax.post("backend.psp", "action=start&selection=" + sel_string, stub_callback);
        if(result)
        {
            busy = true;
            busy_interval = window.setInterval(update_indicator, 100);
        }
    }
}

function stop()
{
    var sel_string, result;

    if(busy)
    {
        return;
    }

    sel_string = encode_selection();
    if(sel_string.length > 0)
    {
        result = ajax.post("backend.psp", "action=stop&selection=" + sel_string, stub_callback);
        if(result)
        {
            busy = true;
            busy_interval = window.setInterval(update_indicator, 100);
        }
    }
}


function check_group(group)
{
    var elements, i;

    elements = document.getElementsByName(group);

    for(i = 0; i < elements.length; i++)
    {
        elements[i].checked = true;
    }
}


function uncheck_group(group)
{
    var elements, i;

    elements = document.getElementsByName(group);

    for(i = 0; i < elements.length; i++)
    {
        elements[i].checked = false;
    }
}


function check_all()
{
    check_group("pending");
    check_group("running");
    check_group("stopped");
}


function uncheck_all()
{
    uncheck_group("pending");
    uncheck_group("running");
    uncheck_group("stopped");
}


function check_pending()
{
    uncheck_group("running");
    uncheck_group("stopped");
    check_group("pending");
}


function check_running()
{
    uncheck_group("pending");
    uncheck_group("stopped");
    check_group("running");
}


function check_stopped()
{
    uncheck_group("pending");
    uncheck_group("running");
    check_group("stopped");
}
