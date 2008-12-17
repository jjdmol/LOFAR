# Check http://www.python.org/peps/pep-0249.html

import psycopg2, sys, cStringIO, time
import system_stata

def format_table(rows, cols, first_line_indent = '', line_indent = '', indent = '    ', reverse = False):

    save_stdout = sys.stdout
    sys.stdout = cStringIO.StringIO()
    
    print first_line_indent + '<table class="dbtable">'

    print line_indent + indent + '<tr>'
    for col in cols:
        print line_indent + 2 * indent + '<th>' + col + '</th>'
    print line_indent + indent + '</tr>'

    for row in rows:

        #link1 = subsystem_link(row)
        #link2 = diag_link(row)
        print line_indent + indent + '<tr>'
        print line_indent + 2 * indent + '<td>' + '<a href="http://www.google.com">' + str(row)+ '</a>''</td>' + \
              2 * indent + '<td>' + '  ' + system_stata.subsystem_status(row) + '' + '</td>' + \
              line_indent + indent + '</tr>'

    print line_indent + '</table>', # omit newline

    result = sys.stdout.getvalue()
    sys.stdout.close()
    sys.stdout = save_stdout

    return result

    
def query_table(shmdb, query, first_line_indent = '', line_indent = '', indent = '    ', reverse = False):

    save_stdout = sys.stdout
    sys.stdout = cStringIO.StringIO()

    results = shmdb.perform_query(query)

    fieldnames = [x1 for (x1, x2, x3, x4, x5, x6, x7) in results.description]

    if reverse:
        results.reverse()

    print first_line_indent + '<table class="dbtable">'

    print line_indent + indent + '<tr>'
    for fieldname in fieldnames:
        print line_indent + 2 * indent + '<th>' + fieldname + '</th>'
    print line_indent + indent + '</tr>'

    for result in results:

        print line_indent + indent + '<tr>'
        for fieldname in fieldnames:
            entry = getattr(result, fieldname)
            if entry is None:
                entry = "<i>n/a</i>"
            print line_indent + 2 * indent + '<td>' + str(entry) + '</td>'
        print line_indent + indent + '</tr>'

    print line_indent + '</table>', # omit newline

    result = sys.stdout.getvalue()
    sys.stdout.close()
    sys.stdout = save_stdout

    return result

def footer(req, first_line_indent = '', line_indent = '', indent = '    '):

    save_stdout = sys.stdout
    sys.stdout = cStringIO.StringIO()

    print first_line_indent + '<hr />'
    print
    print line_indent + 0 * indent + '<p class="copyright-notice">Copyright &copy; 2004-2008 <a href="http://www.astron.nl">Netherlands Foundation For Research In Astronomy (ASTRON)</a> and <a href="http://www.science-and-technology.nl">Science and Technology BV, The Netherlands</a>.</p>'
    print
    print line_indent + 0 * indent + '<p class="access-info">'
    print line_indent + 1 * indent + 'You are accessing this page from IP address <b>' + req.subprocess_env["REMOTE_ADDR"] + '</b>.<br />'
    print line_indent + 1 * indent + 'The local server time is <b>' + time.strftime("%Y-%m-%d %H:%M:%S") + '</b>.'
    print line_indent + 0 * indent + '</p>'
    print
    print line_indent + 0 * indent + '<table>'
    print line_indent + 1 * indent + '<tr>'
    print line_indent + 2 * indent + '<td><a href="http://validator.w3.org/check?uri=referer"><img src="images/valid-xhtml11.png" alt="Valid XHTML 1.1!" /></a></td>'
    print line_indent + 2 * indent + '<td><a href="http://jigsaw.w3.org/css-validator/check/referer"><img src="images/vcss.png" alt="Valid CSS!"/></a></td>'
    print line_indent + 1 * indent + '</tr>'
    print line_indent + 0 * indent + '</table>', # omit newline

    result = sys.stdout.getvalue()
    sys.stdout.close()
    sys.stdout = save_stdout

    return result
