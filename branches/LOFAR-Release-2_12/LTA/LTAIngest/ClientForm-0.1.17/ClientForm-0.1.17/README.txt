   [1]SourceForge.net Logo

                                  ClientForm

   ClientForm is a Python module for handling HTML forms on the client
   side, useful for parsing HTML forms, filling them in and returning the
   completed forms to the server. It developed from a port of Gisle Aas'
   Perl module HTML::Form, from the [2]libwww-perl library, but the
   interface is not the same.

   Simple example:
 from urllib2 import urlopen
 from ClientForm import ParseResponse

 forms = ParseResponse(urlopen("http://www.example.com/form.html"))
 form = forms[0]
 print form
 form["author"] = "Gisle Aas"

 # form.click() returns a urllib2.Request object
 # (see HTMLForm.click.__doc__ if you don't have urllib2)
 response = urlopen(form.click("Thanks"))

   A more complicated example:
 import ClientForm
 import urllib2
 request = urllib2.Request("http://www.example.com/form.html")
 response = urllib2.urlopen(request)
 forms = ClientForm.ParseResponse(response)
 response.close()
 form = forms[0]
 print form  # very useful!

 # Indexing allows setting and retrieval of control values
 original_text = form["comments"]  # a string, NOT a Control instance
 form["comments"] = "Blah."

 # Controls that represent lists (checkbox, select and radio lists) are
 # ListControls.  Their values are sequences of list item names.
 # They come in two flavours: single- and multiple-selection:
 print form.possible_items("cheeses")
 form["favorite_cheese"] = ["brie"]  # single
 form["cheeses"] = ["parmesan", "leicester", "cheddar"]  # multi
 #  is the "parmesan" item of the "cheeses" control selected?
 print "parmesan" in form["cheeses"]
 #  does cheeses control have a "caerphilly" item?
 print "caerphilly" in form.possible_items("cheeses")

 # Sometimes one wants to set or clear individual items in a list:
 #  select the item named "gorgonzola" in the first control named "cheeses"
 form.set(True, "gorgonzola", "cheeses")
 # You can be more specific: supply at least one of name, type, kind, id
 # and nr (most other methods on HTMLForm take the same form of arguments):
 #  deselect "edam" in third CHECKBOX control
 form.set(False, "edam", type="checkbox", nr=2)

 # You can explicitly say that you're referring to a ListControl:
 #  set whole value (rather than just one item of) "cheeses" ListControl
 form.set_value(["gouda"], name="cheeses", kind="list")
 #  last example is almost equivalent to following (but insists that the
 #  control be a ListControl -- so it will skip any non-list controls that
 #  come before the control we want)
 form["cheeses"] = ["gouda"]
 # The kind argument can also take values "multilist", "singlelist", "text",
 # "clickable" and "file":
 #  find first control that will accept text, and scribble in it
 form.set_value("rhubarb rhubarb", kind="text")
 form.set_value([""], kind="singlelist")

 # Often, a single checkbox (a CHECKBOX control with a single item) is
 # present.  In that case, the name of the single item isn't of much
 # interest, so it's useful to be able to check and uncheck the box
 # without using the item name:
 form.set_single(True, "smelly")  # check
 form.set_single(False, "smelly")  # uncheck

 # Add files to FILE controls with .add_file().  Only call this multiple
 # times if the server is expecting multiple files.
 #  add a file, default value for MIME type, no filename sent to server
 form.add_file(open("data.dat"))
 #  add a second file, explicitly giving MIME type, and telling the server
 #   what the filename is
 form.add_file(open("data.txt"), "text/plain", "data.txt")

 # Many methods have a by_label argument, allowing specification of list
 # items by label instead of by name.  At the moment, only SelectControl
 # supports this argument (this will be fixed).  Sometimes labels are
 # easier to maintain than names, sometimes the other way around.
 form.set_value(["Mozzarella", "Caerphilly"], "cheeses", by_label=True)

 # It's also possible to get at the individual controls inside the form.
 # This is useful for calling several methods in a row on a single control,
 # and for the less common operations.  The methods are quite similar to
 # those on HTMLForm:
 control = form.find_control("cheeses", type="select")
 print control.value, control.name, control.type
 print control.possible_items()
 control.value = ["mascarpone", "curd"]
 control.set(True, "limburger")

 # All Controls may be disabled (equivalent of greyed-out in browser)
 control = form.find_control("comments")
 print control.disabled
 # ...or readonly
 print control.readonly
 # readonly and disabled attributes can be assigned to
 control.disabled = False
 # convenience method, used here to make all controls writable (unless
 # they're disabled):
 form.set_all_readonly(False)
 # ListControl items may also be disabled (setting a disabled item is not
 # allowed, but clearing one is allowed):
 print control.get_item_disabled("emmenthal")
 control.set_item_disabled(True, "emmenthal")
 #  enable all items in control
 control.set_all_items_disabled(False)

 # HTMLForm.controls is a list of all controls in the form
 for control in form.controls:
     if control.value == "inquisition": sys.exit()

 request2 = form.click()  # urllib2.Request object
 response2 = urllib2.urlopen(request2)

 print response2.geturl()
 print response2.info()  # headers
 print response2.read()  # body
 response2.close()

   All of the standard control types are supported: TEXT, PASSWORD,
   HIDDEN, TEXTAREA, ISINDEX, RESET, BUTTON (INPUT TYPE=BUTTON and the
   various BUTTON types), SUBMIT, IMAGE, RADIO, CHECKBOX, SELECT/OPTION
   and FILE (for file upload). Both standard form encodings
   (application/x-www-form-urlencoded and multipart/form-data) are
   supported.

   The module is designed for testing and automation of web interfaces,
   not for implementing interactive user agents.

   Security note: Remember that any passwords you store in HTMLForm
   instances will be saved to disk in the clear if you pickle them
   (directly or indirectly). The simplest solution to this is to avoid
   pickling HTMLForm objects. You could also pickle before filling in any
   password, or just set the password to "" before pickling.

   Python 1.5.2 or above is required. To run the tests, you need the
   unittest module (from [3]PyUnit). unittest is a standard library
   module with Python 2.1 and above.

   For full documentation, see the docstrings in ClientForm.py.

   Note: this page describes the 0.1.x interface. See [4]here for the old
   0.0.x interface.

Download

   For installation instructions, see the INSTALL file included in the
   distribution.

   Stable release.. There have been many interface changes since 0.0.x,
   so I don't recommend upgrading old code from 0.0.x unless you want the
   new features.

   0.1.x includes FILE control support for file upload, handling of
   disabled list items, and a redesigned interface.
     * [5]ClientForm-0.1.17.tar.gz
     * [6]ClientForm-0_1_17.zip
     * [7]Change Log (included in distribution)
     * [8]Older versions.

   Old release.
     * [9]ClientForm-0.0.16.tar.gz
     * [10]ClientForm-0_0_16.zip
     * [11]Change Log (included in distribution)
     * [12]Older versions.

FAQs

     * Doesn't the standard Python library module, cgi, do this?
       No: the cgi module does the server end of the job. It doesn't know
       how to parse or fill in a form or how to send it back to the
       server.
     * Which version of Python do I need?
       1.5.2 or above.
     * Is urllib2 required?
       No.
     * How do I use it without urllib2?
       Use .click_request_data() instead of .click().
     * Which urllib2 do I need?
       You don't. It's convenient, though. If you have Python 2.0, you
       need to upgrade to the version from Python 2.1 (available from
       [13]www.python.org). Alternatively, use the 1.5.2-compatible
       version. If you have Python 1.5.2, use this [14]urllib2 and
       [15]urllib. Otherwise, you're OK.
     * Which license?
       The [16]BSD license (included in distribution).
     * Is XHTML supported?
       Yes, since 0.1.12.
     * How do I figure out what control names and values to use?
       print form is usually all you need. HTMLForm.possible_items can be
       useful. Note that it's possible to use item labels instead of item
       names, which can be useful -- use the by_label arguments to the
       various methods, and the .get_value_by_label() /
       .set_value_by_label() methods on ListControl. Only SelectControl
       currently supports item labels (which default to OPTION element
       contents). I might not bother to fix this, since it seems it's
       probably only useful for SELECT anyway.
     * What do those '*' characters mean in the string representations of
       list controls?
       A * next to an item means that item is selected.
     * What do those parentheses (round brackets) mean in the string
       representations of list controls?
       Parentheses (foo) around an item mean that item is disabled.
     * Why doesn't <some control> turn up in the data returned by
       .click*() when that control has non-None value?
       Either the control is disabled, or it is not successful for some
       other reason. 'Successful' (see HTML 4 specification) means that
       the control will cause data to get sent to the server.
     * Why does ClientForm not follow the HTML 4.0 / RFC 1866 standards
       for RADIO and multiple-selection SELECT controls?
       Because by default, it follows browser behaviour when setting the
       initially-selected items in list controls that have no items
       explicitly selected in the HTML. Use the select_default argument
       to ParseResponse if you want to follow the RFC 1866 rules instead.
       Note that browser behaviour violates the HTML 4.01 specification
       in the case of RADIO controls.
     * Why does .click()ing on a button not work for me?
          + Clicking on a RESET button doesn't do anything, by design -
            this is a library for web automation, not an interactive
            browser. Even in an interactive browser, clicking on RESET
            sends nothing to the server, so there is little point in
            having .click() do anything special here.
          + Clicking on a BUTTON TYPE=BUTTON doesn't do anything either,
            also by design. This time, the reason is that that BUTTON is
            only in the HTML standard so that one can attach callbacks to
            its events. The callbacks are functions in SCRIPT elements
            (such as Javascript) embedded in the HTML, and their
            execution may result in information getting sent back to the
            server. ClientForm, however, knows nothing about these
            callbacks, so it can't do anything useful with a click on a
            BUTTON whose type is BUTTON.
          + Generally, embedded script may be messing things up in all
            kinds of ways. See the answer to the next question.
     * Embedded script is messing up my form filling. What do I do?
       See the [17]General FAQs page for what to do about this.
     * I'm having trouble debugging my code.
       The [18]ClientCookie package makes it easy to get .seek()able
       response objects, which is convenient for debugging. See also
       [19]here for few relevant tips. Also see [20]General FAQs.
     * I have a control containing a list of integers. How do I select
       the one whose value is nearest to the one I want?
 import bisect
 def closest_int_value(form, ctrl_name, value):
     values = map(int, form.possible_items(ctrl_name))
     return str(values[bisect.bisect(values, value) - 1])

 form["distance"] = [closest_int_value(form, "distance", 23)]
     * Where can I find out more about the HTML and HTTP standards?
          + W3C [21]HTML 4.01 Specification.
          + [22]RFC 1866 - the HTML 2.0 standard.
          + [23]RFC 1867 - Form-based file upload.
          + [24]RFC 2616 - HTTP 1.1 Specification.

   [25]John J. Lee, January 2005.

   [26]Home
   [27]ClientCookie
   ClientForm
   [28]DOMForm
   [29]python-spidermonkey
   [30]ClientTable
   [31]mechanize
   [32]pullparser
   [33]General FAQs
   [34]1.5.2 urllib2.py
   [35]1.5.2 urllib.py
   [36]Other stuff
   [37]Download
   [38]FAQs

References

   1. http://sourceforge.net/
   2. http://www.linpro.no/lwp/
   3. http://pyunit.sourceforge.net/
   4. http://wwwsearch.sourceforge.net/ClientForm/src/README_0_0_15.html
   5. http://wwwsearch.sourceforge.net/ClientForm/src/ClientForm-0.1.17.tar.gz
   6. http://wwwsearch.sourceforge.net/ClientForm/src/ClientForm-0_1_17.zip
   7. http://wwwsearch.sourceforge.net/ClientForm/src/ChangeLog.txt
   8. http://wwwsearch.sourceforge.net/ClientForm/src/
   9. http://wwwsearch.sourceforge.net/ClientForm/src/ClientForm-0.0.16.tar.gz
  10. http://wwwsearch.sourceforge.net/ClientForm/src/ClientForm-0_0_16.zip
  11. http://wwwsearch.sourceforge.net/ClientForm/src/ChangeLog.txt
  12. http://wwwsearch.sourceforge.net/ClientForm/src/
  13. http://www.python.org/
  14. http://wwwsearch.sourceforge.net/bits/urllib2.py
  15. http://wwwsearch.sourceforge.net/bits/urllib.py
  16. http://www.opensource.org/licenses/bsd-license.php
  17. http://wwwsearch.sourceforge.net/bits/GeneralFAQ.html
  18. http://wwwsearch.sourceforge.net/ClientCookie/
  19. http://wwwsearch.sourceforge.net/ClientCookie/doc.html#debugging
  20. http://wwwsearch.sourceforge.net/bits/GeneralFAQ.html
  21. http://www.w3.org/TR/html401/
  22. http://www.ietf.org/rfc/rfc1866.txt
  23. http://www.ietf.org/rfc/rfc1867.txt
  24. http://www.ietf.org/rfc/rfc2616.txt
  25. mailto:jjl@pobox.com
  26. http://wwwsearch.sourceforge.net/
  27. http://wwwsearch.sourceforge.net/ClientCookie/
  28. http://wwwsearch.sourceforge.net/DOMForm/
  29. http://wwwsearch.sourceforge.net/python-spidermonkey/
  30. http://wwwsearch.sourceforge.net/ClientTable/
  31. http://wwwsearch.sourceforge.net/mechanize/
  32. http://wwwsearch.sourceforge.net/pullparser/
  33. http://wwwsearch.sourceforge.net/bits/GeneralFAQ.html
  34. http://wwwsearch.sourceforge.net/bits/urllib2_152.py
  35. http://wwwsearch.sourceforge.net/bits/urllib_152.py
  36. http://wwwsearch.sourceforge.net/#other
  37. http://wwwsearch.sourceforge.net/ClientForm/#download
  38. http://wwwsearch.sourceforge.net/ClientForm/#faq
