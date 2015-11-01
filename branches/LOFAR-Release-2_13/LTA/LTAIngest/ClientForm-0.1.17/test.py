#!/usr/bin/env python

import unittest, string
from unittest import TestCase
from cStringIO import StringIO

import ClientForm
from ClientForm import ControlNotFoundError,  ItemNotFoundError, \
     ItemCountError, ParseError

# XXX
# Base control tests on ParseFile, so can use same tests for DOMForm and
#  ClientForm.  That wouldn't be unit testing exactly, but saner than the
#  current situation with massive duplication of tests between the two
#  modules.
# HTMLForm.enctype
# XHTML

try: True
except NameError:
    True = 1
    False = 0

try: bool
except NameError:
    def bool(expr):
        if expr: return True
        else: return False

class LWPFormTests(TestCase):
    """The original tests from libwww-perl 5.64."""
    def testEmptyParse(self):
        forms = ClientForm.ParseFile(StringIO(""), "http://localhost")
        self.assert_(len(forms) == 0)

    def _forms(self):
        file = StringIO("""<form action="abc">

        <input name="firstname" value="Gisle">

        </form>

        """)
        return ClientForm.ParseFile(file, "http://localhost/")

    def testParse(self):
        forms = self._forms()
        self.assert_(len(forms) == 1)
        self.assert_(forms[0]["firstname"] == "Gisle")

    def testFillForm(self):
        forms = self._forms()
        form = forms[0]
        form["firstname"] = "Gisle Aas"
        req = form.click()
        def request_method(req):
            if req.has_data():
                return "POST"
            else:
                return "GET"
        self.assert_(request_method(req) == "GET")
        self.assert_(req.get_full_url() == "http://localhost/abc?firstname=Gisle+Aas")


class ParseTests(TestCase):
    def test_parse_error(self):
        f = StringIO(
"""<form action="abc">
<option>
</form>
""")
        base_uri = "http://localhost/"
        try:
            ClientForm.ParseFile(f, base_uri)
        except ClientForm.ParseError, e:
            self.assert_(e.base_uri == base_uri)
        else:
            self.assert_(0)

    def test_base_uri(self):
        # BASE element takes priority over document URI
        file = StringIO(
"""<base HREF="http://example.com">
<form action="abc">
<input type="submit"></input>
</form>
""")
        forms = ClientForm.ParseFile(file, "http://localhost/")
        form = forms[0]
        self.assert_(form.action == "http://example.com/abc")

        file = StringIO(
"""<form action="abc">
<input type="submit"></input>
</form>
""")
        forms = ClientForm.ParseFile(file, "http://localhost/")
        form = forms[0]
        self.assert_(form.action == "http://localhost/abc")

    def testTextarea(self):
        file = StringIO(
"""<form action="abc">

<input name="firstname" value="Gisle">
<textarea>blah, blah,
Rhubarb.

</textarea>

<textarea></textarea>

<textarea name="&quot;ta&quot;" id="foo&amp;bar">Hello testers &amp; users!</textarea>

</form>

""")
        forms = ClientForm.ParseFile(file, "http://localhost/")
        self.assert_(len(forms) == 1)
        form = forms[0]
        self.assert_(form.name is None)
        control = form.find_control(type="textarea")
        self.assert_(control.name is None)
        self.assert_(control.value == "blah, blah,\nRhubarb.\n\n")

        empty_control = form.find_control(type="textarea", nr=1)
        self.assert_(str(empty_control) == "<TextareaControl(<None>=)>")
        self.assert_(empty_control.value == "")

        entity_ctl = form.find_control(type="textarea", nr=2)
        self.assert_(entity_ctl.name == '"ta"')
        self.assertEqual(entity_ctl.attrs["id"], "foo&bar")

        self.assert_(entity_ctl.value == "Hello testers & users!")

    def testSelect(self):
        file = StringIO(
"""<form action="abc">

<select name="foo">
 <option>Hello testers &amp; users!</option>
 <option></option><option></option>
</select>

</form>

""")
        forms = ClientForm.ParseFile(file, "http://localhost/")
        self.assert_(len(forms) == 1)
        form = forms[0]

        entity_ctl = form.find_control(type="select")
        self.assert_(entity_ctl.name == "foo")
        self.assert_(entity_ctl.value[0] == "Hello testers & users!")
        opt = entity_ctl.get_item_attrs("Hello testers & users!")
        self.assert_(opt["value"] == opt["label"] == opt["contents"] ==
                     "Hello testers & users!")

    def testButton(self):
        file = StringIO(
"""<form action="abc" name="myform">

<input type="text" value="cow" name="moo">

<button name="b">blah, blah,
Rhubarb.</button>

<button type="reset" name="b2"></button>
<button type="button" name="b3"></button>

</form>

""")
        forms = ClientForm.ParseFile(file, "http://localhost/")
        form = forms[0]
        self.assert_(form.name == "myform")
        control = form.find_control(name="b")
        self.assert_(control.type == "submitbutton")
        self.assert_(control.value == "")
        self.assert_(form.find_control("b2").type == "resetbutton")
        self.assert_(form.find_control("b3").type == "buttonbutton")
        pairs = form.click_pairs()
        self.assert_(pairs == [("moo", "cow"), ("b", "")])

    def testIsindex(self):
        file = StringIO(
"""<form action="abc">

<isindex prompt=">>>">

</form>

""")
        forms = ClientForm.ParseFile(file, "http://localhost/")
        form = forms[0]
        control = form.find_control(type="isindex")
        self.assert_(control.type == "isindex")
        self.assert_(control.name is None)
        self.assert_(control.value == "")
        control.value = "some stuff"
        self.assert_(form.click_pairs() == [])
        self.assert_(form.click_request_data() ==
                     ("http://localhost/abc?some+stuff", None, []))
        self.assert_(form.click().get_full_url() ==
                     "http://localhost/abc?some+stuff")

    def testEmptySelect(self):
        file = StringIO(
"""<form action="abc">
<select name="foo"></select>

<select name="bar" multiple></select>

</form>
""")
        forms = ClientForm.ParseFile(file, "http://localhost/")
        form = forms[0]
        control0 = form.find_control(type="select", nr=0)
        control1 = form.find_control(type="select", nr=1)
        self.assert_(str(control0) == "<SelectControl(foo=[])>")
        self.assert_(str(control1) == "<SelectControl(bar=[])>")
        form.set_value([], "foo")
        self.assertRaises(ItemNotFoundError, form.set_value, ["oops"], "foo")
        self.assert_(form.click_pairs() == [])

# XXX figure out what to do in these sorts of cases
##     def badSelect(self):
##         # what objects should these generate, if any?
##         # what should happen on submission of these?
##         # what about similar checkboxes and radios?
## """<form action="abc" name="myform">

## <select multiple>
##  <option>1</option>
##  <option>2</option>
##  <option>3</option>
## </select>

## <select multiple>
##  <option>1</option>
##  <option>2</option>
##  <option>3</option>
## </select>

## </form>
## """

## """<form action="abc" name="myform">

## <select multiple>
##  <option>1</option>
##  <option>2</option>
##  <option>3</option>
##  <option>1</option>
##  <option>2</option>
##  <option>3</option>
## </select>

## </form>
## """
## <select name="foo">
##  <option>1</option>
##  <option>2</option>
##  <option>3</option>
## </select>

## <select name="foo" multiple>
##  <option>4</option>
##  <option>5</option>
##  <option>6</option>
## </select>
## """

## """<form action="abc" name="myform">

## <select>
##  <option>1</option>
##  <option>2</option>
##  <option>3</option>
## </select>

## <select>
##  <option>1</option>
##  <option>2</option>
##  <option>3</option>
## </select>

## </form>
## """

##     def testBadCheckbox(self):
##         # see comments above
##         # split checkbox -- is it one control, or two?

## """
## <html>

## <input type=checkbox name=foo value=bar>
## <input type=checkbox name=foo value=bar>

## <select>
##  <option>1</option>
##  <option>2</option>
## </select>

## <input type=checkbox name=foo value=baz>
## <input type=checkbox name=foo value=bar>

## </html>
## """

    def testUnnamedControl(self):
        file = StringIO("""
<form action="./weird.html">

<input type="checkbox" value="foo"></input>

</form>
""")
        forms = ClientForm.ParseFile(file, "http://localhost/")
        form = forms[0]
        self.assert_(form.controls[0].name is None)

    def testNamelessListControls(self):
        # XXX SELECT
        # these controls have no item names
        file = StringIO("""<form action="./weird.html">

<input type="checkbox" name="foo"></input>

<input type="radio" name="bar"></input>

<!--
<select name="baz">
  <option></option>
</select>

<select name="baz" multiple>
  <option></option>
</select>
-->

<input type="submit" name="submit">
</form>
""")
        forms = ClientForm.ParseFile(file, "http://localhost/")
        form = forms[0]
        self.assert_(form.possible_items("foo") == ["on"])
        self.assert_(form.possible_items("bar") == ["on"])
        #self.assert_(form.possible_items("baz") == [])
        self.assert_(form["foo"] == [])
        self.assert_(form["bar"] == [])
        #self.assert_(form["baz"] == [])
        form["foo"] = ["on"]
        form["bar"] = ["on"]
        pairs = form.click_pairs()
        self.assert_(pairs == [("foo", "on"), ("bar", "on"), ("submit", "")])

    def testBadSingleSelect(self):
        # HTML 4.01 section 17.6.1: single selection SELECT controls shouldn't
        # have > 1 item selected, but if they do, not more than one should end
        # up selected.
        file = StringIO("""<form action="./bad.html">

<select name="spam">
  <option selected>1</option>
  <option selected>2</option>
</select>

<input type="submit" name="submit">
</form>
""")
        forms = ClientForm.ParseFile(file, "http://localhost/")
        form = forms[0]
        self.assert_(form.possible_items("spam") == ["1", "2"])
        nr_selected = len(form.find_control("spam").pairs())
        self.assert_(nr_selected == 1)

    def testSelectDefault(self):
        file = StringIO(
"""<form action="abc" name="myform">

<select name="a" multiple>
 <option>1</option>
 <option>2</option>
 <option>3</option>
</select>

<select name="b">
 <option>1</option>
 <option>2</option>
 <option>3</option>
</select>

</form>

""")
        forms = ClientForm.ParseFile(file, "http://localhost/")
        form = forms[0]
        control = form.find_control("a")
        self.assert_(control.value == [])
        single_control = form.find_control("b")
        self.assert_(single_control.value == ["1"])

        file.seek(0)
        forms = ClientForm.ParseFile(file, "http://localhost/",
                                     select_default=1)
        form = forms[0]
        # select_default only affects *multiple* selection select controls
        control = form.find_control(type="select")
        self.assert_(control.value == ["1"])
        single_control = form.find_control(type="select", nr=1)
        self.assert_(single_control.value == ["1"])


class DisabledTests(TestCase):
    def testOptgroup(self):
        file = StringIO(
"""<form action="abc" name="myform">

<select name="foo" multiple>
 <option>1</option>
 <optgroup>
 <option>2</option>
 </optgroup>
 <option>3</option>
 <optgroup>
 <option>4</option>
 <option>5</option>
 <option>6</option>
 </optgroup>
 <optgroup disabled>
 <option selected>7</option>
 <option>8</option>
 </optgroup>
 <option>9</option>
 <optgroup disabled>
 <option>10</option>
 </optgroup>
</select>

<select name="bar">
 <option>1</option>
 <optgroup>
 <option>2</option>
 </optgroup>
 <option>3</option>
 <optgroup>
 <option>4</option>
 <option>5</option>
 <option>6</option>
 </optgroup>
 <optgroup disabled>
 <option selected>7</option>
 <option>8</option>
 </optgroup>
 <option>9</option>
 <optgroup disabled>
 <option>10</option>
 </optgroup>
</select>

</form>""")

        def get_control(name, file=file):
            file.seek(0)
            forms = ClientForm.ParseFile(file, "http://localhost/")
            form = forms[0]
            return form.find_control(name)

        # can't call item_disabled with no args
        control = get_control("foo")
        self.assertRaises(TypeError, control.get_item_disabled)

        control.set_item_disabled(True, "2")
        self.assert_(str(control) == "<SelectControl(foo="
                     "[1, (2), 3, 4, 5, 6, (*7), (8), 9, (10)])>")

        # list controls only allow assignment to .value if no attempt is
        # made to set any disabled item...

        # ...multi selection
        control = get_control("foo")
        self.assert_(control.value == ["7"])
        control.value = ["1"]
        control = get_control("foo")
        def assign_8(control=control): control.value = ["8"]
        self.assertRaises(AttributeError, assign_8)
        self.assert_(control.value == ["7"])
        # even though 7 is set already, attempt to set it again fails
        def assign_7(control=control): control.value = ["7"]
        self.assertRaises(AttributeError, assign_7)
        control.value = ["1", "3"]
        control = get_control("foo")
        def assign_multi(control=control): control.value = ["1", "7"]
        self.assertRaises(AttributeError, assign_multi)
        # enable all items
        for item in control.possible_items():
            control.set_item_disabled(False, item)
        assign_multi()

        control = get_control("foo")
        for value in 7, 8, 10:
            self.assert_(control.get_item_disabled(str(value)))
            self.assertRaises(AttributeError, control.set, True, str(value))
            control.set(False, str(value))
            self.assert_(str(value) not in control.value)
            control.set(False, str(value))
            self.assert_((str(value) not in control.value))
            self.assertRaises(AttributeError, control.toggle, str(value))
            self.assert_(str(value) not in control.value)
            self.assertRaises(AttributeError, control.set, True, str(value))
            self.assert_(str(value) not in control.value)

        control = get_control("foo")
        for value in 1, 2, 3, 4, 5, 6, 9:
            self.assert_(not control.get_item_disabled(str(value)))
            control.set(False, str(value))
            self.assert_(str(value) not in control.value)
            control.toggle(str(value))
            self.assert_(str(value) in control.value)
            control.set(True, str(value))
            self.assert_(str(value) in control.value)
            control.toggle(str(value))
            self.assert_(str(value) not in control.value)

        control = get_control("foo")
        self.assert_(control.get_item_disabled("7"))
        control.toggle("7")  # clearing, not setting, so no problem
        self.assertRaises(AttributeError, control.set, True, "7")
        control.set_item_disabled(True, "7")
        self.assert_(control.get_item_disabled("7"))
        self.assertRaises(AttributeError, control.set, True, "7")
        control.set_item_disabled(False, "7")
        self.assert_(not control.get_item_disabled("7"))
        control.set(True, "7")
        control.set(False, "7")
        control.toggle("7")
        control.toggle("7")

        # ...single-selection
        control = get_control("bar")
        self.assert_(control.value == ["7"])
        control.value = ["1"]
        control = get_control("bar")
        def assign_8(control=control): control.value = ["8"]
        self.assertRaises(AttributeError, assign_8)
        self.assert_(control.value == ["7"])
        # even though 7 is set already, attempt to set it again fails
        def assign_7(control=control): control.value = ["7"]
        self.assertRaises(AttributeError, assign_7)
        # enable all items
        for item in control.possible_items():
            control.set_item_disabled(False, item)
        assign_7()

        control = get_control("bar")
        for value in 7, 8, 10:
            self.assert_(control.get_item_disabled(str(value)))
            self.assertRaises(AttributeError, control.set, True, str(value))
            control.set(False, str(value))
            self.assert_(str(value) != control.value)
            control.set(False, str(value))
            self.assert_(str(value) != control.value)
            self.assertRaises(AttributeError, control.toggle, str(value))
            self.assert_(str(value) != control.value)
            self.assertRaises(AttributeError, control.set, True, str(value))
            self.assert_(str(value) != control.value)

        control = get_control("bar")
        for value in 1, 2, 3, 4, 5, 6, 9:
            self.assert_(not control.get_item_disabled(str(value)))
            control.set(False, str(value))
            self.assert_(str(value) not in control.value)
            control.toggle(str(value))
            self.assert_(str(value) == control.value[0])
            control.set(True, str(value))
            self.assert_(str(value) == control.value[0])
            control.toggle(str(value))
            self.assert_(str(value) not in control.value)

        control = get_control("bar")
        self.assert_(control.get_item_disabled("7"))
        control.toggle("7")  # clearing, not setting, so no problem
        self.assertRaises(AttributeError, control.set, True, "7")
        control.set_item_disabled(True, "7")
        self.assert_(control.get_item_disabled("7"))
        self.assertRaises(AttributeError, control.set, True, "7")
        control.set_item_disabled(False, "7")
        self.assert_(not control.get_item_disabled("7"))
        control.set(True, "7")
        control.set(False, "7")
        control.toggle("7")
        control.toggle("7")

        # set_all_items_disabled
        for name in "foo", "bar":
            control = get_control(name)
            control.set_all_items_disabled(False)
            control.set(True, "7")
            control.set(True, "1")
            control.set_all_items_disabled(True)
            self.assertRaises(AttributeError, control.set, True, "7")
            self.assertRaises(AttributeError, control.set, True, "1")

# XXX single select
    def testDisabledSelect(self):
        file = StringIO(
"""<form action="abc" name="myform">

<select name="foo" multiple>
 <option label="a">1</option>
 <option>2</option>
 <option>3</option>
</select>

<select name="bar" multiple>
 <option>1</option>
 <option disabled>2</option>
 <option>3</option>
</select>

<select name="baz" disabled multiple>
 <option>1</option>
 <option>2</option>
 <option>3</option>
</select>

<select name="spam" disabled multiple>
 <option>1</option>
 <option disabled>2</option>
 <option>3</option>
</select>

</form>
""")    
        forms = ClientForm.ParseFile(file, "http://localhost/")
        form = forms[0]
        for name, control_disabled, item_disabled in [
            ("foo", False, False),
            ("bar", False, True),
            ("baz", True, False),
            ("spam", True, True)]:
            control = form.find_control(name)
            self.assert_(bool(control.disabled) == control_disabled)
            item = control.get_item_attrs("2")
            self.assert_(bool(item.has_key("disabled")) == item_disabled)

            def bad_assign(value, control=control): control.value = value
            if control_disabled:
                for value in "1", "2", "3":
                    self.assertRaises(AttributeError, control.set, True, value)
                    self.assertRaises(AttributeError, bad_assign, [value])
            elif item_disabled:
                self.assertRaises(AttributeError, control.set, True, "2")
                self.assertRaises(AttributeError, bad_assign, ["2"])
                for value in "1", "3":
                    control.set(True, value)
            else:
                control.value = ["1", "2", "3"]

        control = form.find_control("foo")
        # missing disabled arg
        self.assertRaises(TypeError, control.set_item_disabled, "1")
        # by_label
        self.assert_(not control.get_item_disabled("a", by_label=True))
        control.set_item_disabled(True, "a", by_label=True)
        self.assert_(control.get_item_disabled("a", by_label=True))

    def testDisabledCheckbox(self):
        file = StringIO(
"""<form action="abc" name="myform">

<input type="checkbox" name="foo" value="1" label="a"></input>
<input type="checkbox" name="foo" value="2"></input>
<input type="checkbox" name="foo" value="3"></input>

<input type="checkbox" name="bar" value="1"></input>
<input type="checkbox" name="bar" value="2" disabled></input>
<input type="checkbox" name="bar" value="3"></input>

<input type="checkbox" name="baz" value="1" disabled></input>
<input type="checkbox" name="baz" value="2" disabled></input>
<input type="checkbox" name="baz" value="3" disabled></input>

</form>""")
        forms = ClientForm.ParseFile(file, "http://localhost/")
        form = forms[0]
        for name, control_disabled, item_disabled in [
            ("foo", False, False),
            ("bar", False, True),
            ("baz", False, True)]:
            control = form.find_control(name)
            self.assert_(bool(control.disabled) == control_disabled)
            item = control.get_item_attrs("2")
            self.assert_(bool(item.has_key("disabled")) == item_disabled)
            self.assert_(control.get_item_disabled("2") == item_disabled)

            def bad_assign(value, control=control): control.value = value
            if item_disabled:
                self.assertRaises(AttributeError, control.set, True, "2")
                self.assertRaises(AttributeError, bad_assign, ["2"])
                if not control.get_item_disabled("1"):
                    control.set(True, "1")
            else:
                control.value = ["1", "2", "3"]

        control = form.find_control("foo")
        control.set_item_disabled(False, "1")
        # missing disabled arg
        self.assertRaises(TypeError, control.set_item_disabled, "1")
        # by_label
        self.assertRaises(NotImplementedError,
                          control.get_item_disabled, "a", by_label=True)
        self.assert_(not control.get_item_disabled("1"))
        self.assertRaises(NotImplementedError,
                          control.set_item_disabled, True, "a",
                          by_label=True)
        self.assert_(not control.get_item_disabled("1"))


class ControlTests(TestCase):
    def testTextControl(self):
        attrs = {"type": "this is ignored",
                 "name": "ath_Uname",
                 "value": "",
                 "maxlength": "20",
                 "id": "foo"}
        c = ClientForm.TextControl("texT", "ath_Uname", attrs)
        c.fixup()
        self.assert_(c.type == "text")
        self.assert_(c.name == "ath_Uname")
        self.assert_(c.id == "foo")
        self.assert_(c.value == "")
        self.assert_(str(c) == "<TextControl(ath_Uname=)>")
        self.assert_(c.pairs() == [("ath_Uname", "")])
        def bad_assign(c=c): c.type = "sometype"
        self.assertRaises(AttributeError, bad_assign)
        self.assert_(c.type == "text")
        def bad_assign(c=c): c.name = "somename"
        self.assertRaises(AttributeError, bad_assign)
        self.assert_(c.name == "ath_Uname")
        c.value = "2"
        self.assert_(c.value == "2")
        self.assert_(str(c) == "<TextControl(ath_Uname=2)>")
        def bad_assign(c=c): c.value = ["foo"]
        self.assertRaises(TypeError, bad_assign)
        self.assert_(c.value == "2")
        self.assert_(not c.readonly)
        c.readonly = True
        def bad_assign(c=c): c.value = "foo"
        self.assertRaises(AttributeError, bad_assign)
        self.assert_(c.value == "2")
        c.disabled = True
        self.assert_(str(c) ==
                     "<TextControl(ath_Uname=2) (disabled, readonly)>")
        c.readonly = False
        self.assert_(str(c) == "<TextControl(ath_Uname=2) (disabled)>")
        self.assertRaises(AttributeError, bad_assign)
        self.assert_(c.value == "2")
        self.assert_(c.pairs() == [])
        c.disabled = False
        self.assert_(str(c) == "<TextControl(ath_Uname=2)>")

        self.assert_(c.attrs.has_key("maxlength"))
        for key in "name", "type", "value":
            self.assert_(c.attrs.has_key(key))

        # initialisation of readonly and disabled attributes
        attrs["readonly"] = True
        c = ClientForm.TextControl("text", "ath_Uname", attrs)
        def bad_assign(c=c): c.value = "foo"
        self.assertRaises(AttributeError, bad_assign)
        del attrs["readonly"]
        attrs["disabled"] = True
        c = ClientForm.TextControl("text", "ath_Uname", attrs)
        def bad_assign(c=c): c.value = "foo"
        self.assertRaises(AttributeError, bad_assign)
        del attrs["disabled"]
        c = ClientForm.TextControl("hidden", "ath_Uname", attrs)
        self.assert_(c.readonly)
        def bad_assign(c=c): c.value = "foo"
        self.assertRaises(AttributeError, bad_assign)

    def testIsindexControl(self):
        attrs = {"type": "this is ignored",
                 "prompt": ">>>"}
        c = ClientForm.IsindexControl("isIndex", None, attrs)
        c.fixup()
        self.assert_(c.type == "isindex")
        self.assert_(c.name is None)
        self.assert_(c.value == "")
        self.assert_(str(c) == "<IsindexControl()>")
        self.assert_(c.pairs() == [])
        def set_type(c=c): c.type = "sometype"
        self.assertRaises(AttributeError, set_type)
        self.assert_(c.type == "isindex")
        def set_name(c=c): c.name = "somename"
        self.assertRaises(AttributeError, set_name)
        def set_value(value, c=c): c.value = value
        self.assertRaises(TypeError, set_value, [None])
        self.assert_(c.name is None)
        c.value = "2"
        self.assert_(c.value == "2")
        self.assert_(str(c) == "<IsindexControl(2)>")
        c.disabled = True
        self.assert_(str(c) == "<IsindexControl(2) (disabled)>")
        self.assertRaises(AttributeError, set_value, "foo")
        self.assert_(c.value == "2")
        self.assert_(c.pairs() == [])
        c.readonly = True
        self.assert_(str(c) == "<IsindexControl(2) (disabled, readonly)>")
        self.assertRaises(AttributeError, set_value, "foo")
        c.disabled = False
        self.assert_(str(c) == "<IsindexControl(2) (readonly)>")
        self.assertRaises(AttributeError, set_value, "foo")
        c.readonly = False
        self.assert_(str(c) == "<IsindexControl(2)>")

        self.assert_(c.attrs.has_key("type"))
        self.assert_(c.attrs.has_key("prompt"))
        self.assert_(c.attrs["prompt"] == ">>>")
        for key in "name", "value":
            self.assert_(not c.attrs.has_key(key))

        c.value = "foo 1 bar 2"
        class FakeForm: action = "http://localhost/"
        form = FakeForm()
        self.assert_(c._click(form, (1,1), "request_data") == 
                     ("http://localhost/?foo+1+bar+2", None, []))

    def testIgnoreControl(self):
        attrs = {"type": "this is ignored"}
        c = ClientForm.IgnoreControl("reset", None, attrs)
        self.assert_(c.type == "reset")
        self.assert_(c.value is None)
        self.assert_(str(c) == "<IgnoreControl(<None>=<None>)>")

        def set_value(value, c=c): c.value = value
        self.assertRaises(AttributeError, set_value, "foo")
        self.assert_(c.value is None)

    def testSubmitControl(self):
        attrs = {"type": "this is ignored",
                 "name": "name_value",
                 "value": "value_value",
                 "img": "foo.gif"}
        c = ClientForm.SubmitControl("submit", "name_value", attrs)
        self.assert_(c.type == "submit")
        self.assert_(c.name == "name_value")
        self.assert_(c.value == "value_value")
        self.assert_(str(c) == "<SubmitControl(name_value=value_value) (readonly)>")
        def set_value(value, c=c): c.value = value
        self.assertRaises(TypeError, set_value, ["foo"])
        c.disabled = True
        self.assertRaises(AttributeError, set_value, "value_value")
        self.assert_(str(c) == "<SubmitControl(name_value=value_value) "
                     "(disabled, readonly)>")
        c.disabled = False
        c.readonly = False
        set_value("value_value")
        self.assert_(str(c) == "<SubmitControl(name_value=value_value)>")
        c.readonly = True

        # click on button
        form = ClientForm.HTMLForm("http://foo.bar.com/")
        c.add_to_form(form)
        self.assert_(c.pairs() == [])
        pairs = c._click(form, (1,1), "pairs")
        request = c._click(form, (1,1), "request")
        data = c._click(form, (1,1), "request_data")
        self.assert_(c.pairs() == [])
        self.assert_(pairs == [("name_value", "value_value")])
        self.assert_(request.get_full_url() ==
                     "http://foo.bar.com/?name_value=value_value")
        self.assert_(data ==
                     ("http://foo.bar.com/?name_value=value_value", None, []))
        c.disabled = True
        pairs = c._click(form, (1,1), "pairs")
        request = c._click(form, (1,1), "request")
        data = c._click(form, (1,1), "request_data")
        self.assert_(pairs == [])
        # XXX not sure if should have '?' on end of this URL, or if it really matters...
        self.assert_(request.get_full_url() == "http://foo.bar.com/")
        self.assert_(data == ("http://foo.bar.com/", None, []))

    def testImageControl(self):
        attrs = {"type": "this is ignored",
                 "name": "name_value",
                 "img": "foo.gif"}
        c = ClientForm.ImageControl("image", "name_value", attrs)
        self.assert_(c.type == "image")
        self.assert_(c.name == "name_value")
        self.assert_(c.value == "")
        self.assert_(str(c) == "<ImageControl(name_value=)>")

        # click, at coordinate (0, 55), on image
        form = ClientForm.HTMLForm("http://foo.bar.com/")
        c.add_to_form(form)
        self.assert_(c.pairs() == [])
        request = c._click(form, (0, 55), "request")
        self.assert_(c.pairs() == [])
        self.assert_(request.get_full_url() ==
                     "http://foo.bar.com/?name_value.x=0&name_value.y=55")
        self.assert_(c._click(form, (0,55), return_type="request_data") ==
                     ("http://foo.bar.com/?name_value.x=0&name_value.y=55",
                      None, []))
        c.value = "blah"
        request = c._click(form, (0, 55), "request")
        self.assert_(request.get_full_url() ==
                     "http://foo.bar.com/?name_value.x=0&name_value.y=55&name_value=blah")

        c.disabled = True
        self.assertEqual(c.value, "blah")
        self.assert_(str(c) == "<ImageControl(name_value=blah) (disabled)>")
        def set_value(value, c=c): c.value = value
        self.assertRaises(AttributeError, set_value, "blah")
        self.assert_(c._click(form, (1,1), return_type="pairs") == [])
        c.readonly = True
        self.assert_(str(c) == "<ImageControl(name_value=blah) "
                     "(disabled, readonly)>")
        self.assertRaises(AttributeError, set_value, "blah")
        self.assert_(c._click(form, (1,1), return_type="pairs") == [])
        c.disabled = c.readonly = False
        self.assert_(c._click(form, (1,1), return_type="pairs") ==
                     [("name_value.x", "1"), ("name_value.y", "1"), ('name_value', 'blah')])

    def testCheckboxControl(self):
        attrs = {"type": "this is ignored",
                 "name": "name_value",
                 "value": "value_value",
                 "alt": "some string"}
        c = ClientForm.CheckboxControl("checkbox", "name_value", attrs)
        c.fixup()
        self.assert_(c.type == "checkbox")
        self.assert_(c.name == "name_value")
        self.assert_(c.value == [])
        self.assert_(c.possible_items() == ["value_value"])
        def set_type(c=c): c.type = "sometype"
        self.assertRaises(AttributeError, set_type)
        self.assert_(c.type == "checkbox")
        def set_name(c=c): c.name = "somename"
        self.assertRaises(AttributeError, set_name)
        self.assert_(c.name == "name_value")

        # construct larger list from length-1 lists
        c = ClientForm.CheckboxControl("checkbox", "name_value", attrs)
        attrs2 = attrs.copy()
        attrs2["value"] = "value_value2"
        c2 = ClientForm.CheckboxControl("checkbox", "name_value", attrs2)
        c.merge_control(c2)
        c.fixup()
        self.assert_(str(c) == "<CheckboxControl("
                     "name_value=[value_value, value_value2])>")
        self.assert_(c.possible_items() == ["value_value", "value_value2"])

        attrs = c.get_item_attrs("value_value")
        for key in "alt", "name", "value", "type":
            self.assert_(attrs.has_key(key))
        self.assertRaises(ItemNotFoundError, c.get_item_attrs, "oops")

        def set_value(value, c=c): c.value = value

        c.value = ["value_value", "value_value2"]
        self.assert_(c.value == ["value_value", "value_value2"])
        c.value = ["value_value"]
        self.assert_(c.value == ["value_value"])
        self.assertRaises(ItemNotFoundError, set_value, ["oops"])
        self.assertRaises(TypeError, set_value, "value_value")
        c.value = ["value_value2"]
        self.assert_(c.value == ["value_value2"])
        c.toggle("value_value")
        self.assert_(c.value == ["value_value", "value_value2"])
        c.toggle("value_value2")
        self.assert_(c.value == ["value_value"])
        self.assertRaises(ItemNotFoundError, c.toggle, "oops")
        # set
        self.assert_(c.value == ["value_value"])
        c.set(True, "value_value")
        self.assert_(c.value == ["value_value"])
        c.set(True, "value_value2")
        self.assert_(c.value == ["value_value", "value_value2"])
        c.set(True, "value_value2")
        self.assert_(c.value == ["value_value", "value_value2"])
        c.set(False, "value_value2")
        self.assert_(c.value == ["value_value"])
        c.set(False, "value_value2")
        self.assert_(c.value == ["value_value"])
        self.assertRaises(ItemNotFoundError, c.set, True, "oops")
        self.assertRaises(TypeError, c.set, True, ["value_value"])
        self.assertRaises(ItemNotFoundError, c.set, False, "oops")
        self.assertRaises(TypeError, c.set, False, ["value_value"])

        self.assert_(str(c) == "<CheckboxControl("
                     "name_value=[*value_value, value_value2])>")
        c.disabled = True
        self.assertRaises(AttributeError, set_value, ["value_value"])
        self.assert_(str(c) == "<CheckboxControl("
                     "name_value=[*value_value, value_value2]) "
                     "(disabled)>")
        self.assert_(c.value == ["value_value"])
        self.assert_(c.pairs() == [])
        c.readonly = True
        self.assertRaises(AttributeError, set_value, ["value_value"])
        self.assert_(str(c) == "<CheckboxControl("
                     "name_value=[*value_value, value_value2]) "
                     "(disabled, readonly)>")
        self.assert_(c.value == ["value_value"])
        self.assert_(c.pairs() == [])
        c.disabled = False
        self.assert_(str(c) == "<CheckboxControl("
                     "name_value=[*value_value, value_value2]) "
                     "(readonly)>")
        self.assertRaises(AttributeError, set_value, ["value_value"])
        self.assert_(c.value == ["value_value"])
        self.assert_(c.pairs() == [("name_value", "value_value")])
        c.readonly = False
        c.value = []
        self.assert_(c.value == [])

    def testSelectControlMultiple(self):
        import copy
        attrs = {"type": "this is ignored",
                 "name": "name_value",
                 "value": "value_value",
                 "alt": "some string",
                 "label": "contents_value",
                 "contents": "contents_value",
                 "__select": {"type": "this is ignored",
                              "name": "select_name",
                              "multiple": "",
                              "alt": "alt_text"}}
        # with Netscape / IE default selection...
        c = ClientForm.SelectControl("select", "select_name", attrs)
        c.fixup()
        self.assert_(c.type == "select")
        self.assert_(c.name == "select_name")
        self.assert_(c.value == [])
        self.assert_(c.possible_items() == ["value_value"])
        self.assert_(c.attrs.has_key("name"))
        self.assert_(c.attrs.has_key("type"))
        self.assert_(c.attrs["alt"] == "alt_text")
        # ... and with RFC 1866 default selection
        c = ClientForm.SelectControl("select", "select_name", attrs, select_default=True)
        c.fixup()
        self.assert_(c.value == ["value_value"])

        # construct larger list from length-1 lists
        c = ClientForm.SelectControl("select", "select_name", attrs)
        attrs2 = attrs.copy()
        attrs2["value"] = "value_value2"
        c2 = ClientForm.SelectControl("select", "select_name", attrs2)
        c.merge_control(c2)
        c.fixup()
        self.assert_(str(c) == "<SelectControl("
                     "select_name=[value_value, value_value2])>")
        self.assert_(c.possible_items() == ["value_value", "value_value2"])

        # get_item_attrs
        attrs3 = c.get_item_attrs("value_value")
        self.assert_(attrs3.has_key("alt"))
        self.assert_(not attrs3.has_key("multiple"))
        # HTML attributes dictionary should have been copied by ListControl
        # constructor.
        attrs["new_attr"] = "new"
        attrs2["new_attr2"] = "new2"
        for key in ("new_attr", "new_attr2"):
            self.assert_(not attrs3.has_key(key))
        self.assertRaises(ItemNotFoundError, c.get_item_attrs, "oops")

        c.value = ["value_value", "value_value2"]
        self.assert_(c.value == ["value_value", "value_value2"])
        c.value = ["value_value"]
        self.assert_(c.value == ["value_value"])
        def set_value(value, c=c): c.value = value
        self.assertRaises(ItemNotFoundError, set_value, ["oops"])
        self.assertRaises(TypeError, set_value, "value_value")
        self.assertRaises(TypeError, set_value, None)
        c.value = ["value_value2"]
        self.assert_(c.value == ["value_value2"])
        c.toggle("value_value")
        self.assert_(c.value == ["value_value", "value_value2"])
        c.toggle("value_value2")
        self.assert_(c.value == ["value_value"])
        self.assertRaises(ItemNotFoundError, c.toggle, "oops")
        self.assert_(c.value == ["value_value"])
        # test ordering of items
        c.value = ["value_value2", "value_value"]
        self.assert_(c.value == ["value_value", "value_value2"])
        # set
        c.set(True, "value_value")
        self.assert_(c.value == ["value_value", "value_value2"])
        c.set(True, "value_value2")
        self.assert_(c.value == ["value_value", "value_value2"])
        c.set(False, "value_value")
        self.assert_(c.value == ["value_value2"])
        c.set(False, "value_value")
        self.assert_(c.value == ["value_value2"])
        self.assertRaises(ItemNotFoundError, c.set, True, "oops")
        self.assertRaises(TypeError, c.set, True, ["value_value"])
        self.assertRaises(ItemNotFoundError, c.set, False, "oops")
        self.assertRaises(TypeError, c.set, False, ["value_value"])
        c.value = []
        self.assert_(c.value == [])

    def testSelectControlMultiple_label(self):
        import ClientForm
##         <SELECT name=year>
##          <OPTION value=0 label="2002">current year</OPTION>
##          <OPTION value=1>2001</OPTION>
##          <OPTION>2000</OPTION>
##         </SELECT>
        attrs = {"type": "ignored",
                 "name": "year",
                 "value": "0",
                 "label": "2002",
                 "contents": "current year",
                 "__select": {"type": "this is ignored",
                              "name": "select_name",
                              "multiple": ""}}
        attrs2 = {"type": "ignored",
                  "name": "year",
                  "value": "1",
                  "label": "2001",  # label defaults to contents
                  "contents": "2001",
                 "__select": {"type": "this is ignored",
                              "name": "select_name",
                              "multiple": ""}}
        attrs3 = {"type": "ignored",
                  "name": "year",
                  "value": "2000",  # value defaults to contents
                  "label": "2000",  # label defaults to contents
                  "contents": "2000",
                 "__select": {"type": "this is ignored",
                              "name": "select_name",
                              "multiple": ""}}
        c = ClientForm.SelectControl("select", "select_name", attrs)
        c2 = ClientForm.SelectControl("select", "select_name", attrs2)
        c3 = ClientForm.SelectControl("select", "select_name", attrs3)
        c.merge_control(c2)
        c.merge_control(c3)
        c.fixup()

        self.assert_(c.possible_items() == ["0", "1", "2000"])
        self.assert_(c.possible_items(by_label=True) ==
                     ["2002", "2001", "2000"])

        self.assert_(c.value == [])
        c.toggle("2002", by_label=True)
        self.assert_(c.value == ["0"])
        c.toggle("0")
        self.assert_(c.value == [])
        c.toggle("0")
        self.assert_(c.value == ["0"])
        self.assert_(c.get_value_by_label() == ["2002"])
        c.toggle("2002", by_label=True)
        self.assertRaises(ItemNotFoundError, c.toggle, "blah", by_label=True)
        self.assert_(c.value == [])
        c.toggle("2000")
        self.assert_(c.value == ["2000"])
        self.assert_(c.get_value_by_label() == ["2000"])

        def set_value(value, c=c): c.value = value
        self.assertRaises(ItemNotFoundError, set_value, ["2002"])
        self.assertRaises(TypeError, set_value, "1")
        self.assertRaises(TypeError, set_value, None)
        self.assert_(c.value == ["2000"])
        c.value = ["0"]
        self.assert_(c.value == ["0"])
        c.value = []
        self.assertRaises(TypeError, c.set_value_by_label, "2002")
        c.set_value_by_label(["2002"])
        self.assert_(c.value == ["0"])
        self.assert_(c.get_value_by_label() == ["2002"])
        c.set_value_by_label(["2000"])
        self.assert_(c.value == ["2000"])
        self.assert_(c.get_value_by_label() == ["2000"])
        c.set_value_by_label(["2000", "2002"])
        self.assert_(c.value == ["0", "2000"])
        self.assert_(c.get_value_by_label() == ["2002", "2000"])

        c.set(False, "2002", by_label=True)
        self.assert_(c.get_value_by_label() == c.value == ["2000"])
        c.set(False, "2002", by_label=True)
        self.assert_(c.get_value_by_label() == c.value == ["2000"])
        c.set(True, "2002", by_label=True)
        self.assert_(c.get_value_by_label() == ["2002", "2000"])
        self.assert_(c.value == ["0", "2000"])
        c.set(False, "2000", by_label=True)
        self.assert_(c.get_value_by_label() == ["2002"])
        self.assert_(c.value == ["0"])
        c.set(True, "2001", by_label=True)
        self.assert_(c.get_value_by_label() == ["2002", "2001"])
        self.assert_(c.value == ["0", "1"])
        self.assertRaises(ItemNotFoundError, c.set, True, "blah",
                          by_label=True)
        self.assertRaises(ItemNotFoundError, c.set,
                          False, "blah", by_label=True)

    def testSelectControlSingle_label(self):
        import ClientForm
##         <SELECT name=year>
##          <OPTION value=0 label="2002">current year</OPTION>
##          <OPTION value=1>2001</OPTION>
##          <OPTION>2000</OPTION>
##         </SELECT>
        attrs = {"type": "ignored",
                 "name": "year",
                 "value": "0",
                 "label": "2002",
                 "contents": "current year",
                 "__select": {"type": "this is ignored",
                              "name": "select_name"}}
        attrs2 = {"type": "ignored",
                  "name": "year",
                  "value": "1",
                  "label": "2001",  # label defaults to contents
                  "contents": "2001",
                 "__select": {"type": "this is ignored",
                              "name": "select_name"}}
        attrs3 = {"type": "ignored",
                  "name": "year",
                  "value": "2000",  # value defaults to contents
                  "label": "2000",  # label defaults to contents
                  "contents": "2000",
                 "__select": {"type": "this is ignored",
                              "name": "select_name"}}
        c = ClientForm.SelectControl("select", "select_name", attrs)
        c2 = ClientForm.SelectControl("select", "select_name", attrs2)
        c3 = ClientForm.SelectControl("select", "select_name", attrs3)
        c.merge_control(c2)
        c.merge_control(c3)
        c.fixup()

        self.assert_(c.possible_items() == ["0", "1", "2000"])
        self.assert_(c.possible_items(by_label=True) ==
                     ["2002", "2001", "2000"])

        def set_value(value, c=c): c.value = value
        self.assertRaises(ItemNotFoundError, set_value, ["2002"])
        self.assertRaises(TypeError, set_value, "1")
        self.assertRaises(TypeError, set_value, None)
        self.assert_(c.value == ["0"])
        c.value = []
        self.assert_(c.value == [])
        c.value = ["0"]
        self.assert_(c.value == ["0"])
        c.value = []
        self.assertRaises(TypeError, c.set_value_by_label, "2002")
        self.assertRaises(ItemNotFoundError, c.set_value_by_label, ["foo"])
        c.set_value_by_label(["2002"])
        self.assert_(c.value == ["0"])
        self.assert_(c.get_value_by_label() == ["2002"])
        c.set_value_by_label(["2000"])
        self.assert_(c.value == ["2000"])
        self.assert_(c.get_value_by_label() == ["2000"])

    def testSelectControlSingle(self):
        attrs = {"type": "this is ignored",
                 "name": "name_value",
                 "value": "value_value",
                 "label": "contents_value",
                 "contents": "contents_value",
                 "__select": {"type": "this is ignored",
                              "name": "select_name",
                              "alt": "alt_text"}}
        # Netscape and IE behaviour...
        c = ClientForm.SelectControl("select", "select_name", attrs)
        c.fixup()
        self.assert_(c.type == "select")
        self.assert_(c.name == "select_name")
        self.assert_(c.value == ["value_value"])
        self.assert_(c.possible_items() == ["value_value"])
        self.assert_(c.attrs.has_key("name"))
        self.assert_(c.attrs.has_key("type"))
        self.assert_(c.attrs["alt"] == "alt_text")
        # ...and RFC 1866 behaviour are identical (unlike multiple SELECT).
        c = ClientForm.SelectControl("select", "select_name", attrs,
                                     select_default=1)
        c.fixup()
        self.assert_(c.value == ["value_value"])

        # construct larger list from length-1 lists
        c = ClientForm.SelectControl("select", "select_name", attrs)
        attrs2 = attrs.copy()
        attrs2["value"] = "value_value2"
        c2 = ClientForm.SelectControl("select", "select_name", attrs2)
        c.merge_control(c2)
        c.fixup()
        self.assert_(str(c) == "<SelectControl("
                     "select_name=[*value_value, value_value2])>")
        c.value = []
        self.assert_(c.value == [])
        self.assert_(str(c) == "<SelectControl("
                     "select_name=[value_value, value_value2])>")
        c.value = ["value_value"]
        self.assert_(c.value == ["value_value"])
        self.assert_(str(c) == "<SelectControl("
                     "select_name=[*value_value, value_value2])>")
        self.assert_(c.possible_items() == ["value_value", "value_value2"])

        def set_value(value, c=c): c.value = value
        self.assertRaises(ItemCountError, set_value,
                          ["value_value", "value_value2"])
        self.assertRaises(TypeError, set_value, "value_value")
        self.assertRaises(TypeError, set_value, None)
        c.value = ["value_value2"]
        self.assert_(c.value == ["value_value2"])
        c.value = ["value_value"]
        self.assert_(c.value == ["value_value"])
        self.assertRaises(ItemNotFoundError, set_value, ["oops"])
        self.assert_(c.value == ["value_value"])
        c.toggle("value_value")
        self.assertRaises(ItemNotFoundError, c.toggle, "oops")
        self.assertRaises(TypeError, c.toggle, ["oops"])
        self.assert_(c.value == [])
        c.value = ["value_value"]
        self.assert_(c.value == ["value_value"])
        # nothing selected is allowed
        c.value = []
        self.assert_(c.value == [])
        # set
        c.set(True, "value_value")
        self.assert_(c.value == ["value_value"])
        c.set(True, "value_value")
        self.assert_(c.value == ["value_value"])
        c.set(True, "value_value2")
        self.assert_(c.value == ["value_value2"])
        c.set(False, "value_value")
        self.assert_("value_value2")
        c.set(False, "value_value2")
        self.assert_(c.value == [])
        c.set(False, "value_value2")
        self.assert_(c.value == [])
        self.assertRaises(ItemNotFoundError, c.set, True, "oops")
        self.assertRaises(TypeError, c.set, True, ["value_value"])
        self.assertRaises(ItemNotFoundError, c.set, False, "oops")
        self.assertRaises(TypeError, c.set, False, ["value_value"])

    def testRadioControl(self):
        attrs = {"type": "this is ignored",
                 "name": "name_value",
                 "value": "value_value",
                 "id": "blah"}
        # Netscape and IE behaviour...
        c = ClientForm.RadioControl("radio", "name_value", attrs)
        c.fixup()
        self.assert_(c.type == "radio")
        self.assert_(c.name == "name_value")
        self.assert_(c.id == "blah")
        self.assert_(c.value == [])
        self.assert_(c.possible_items() == ["value_value"])
        # ...and RFC 1866 behaviour
        c = ClientForm.RadioControl("radio", "name_value", attrs,
                                    select_default=True)
        c.fixup()
        self.assert_(c.value == ["value_value"])

        # construct larger list from length-1 lists
        c = ClientForm.RadioControl("radio", "name_value", attrs,
                                    select_default=True)
        attrs2 = attrs.copy()
        attrs2["value"] = "value_value2"
        c2 = ClientForm.RadioControl("radio", "name_value", attrs2,
                                     select_default=True)
        c.merge_control(c2)
        c.fixup()
        self.assert_(str(c) == "<RadioControl("
                     "name_value=[*value_value, value_value2])>")
        self.assert_(c.possible_items() == ["value_value", "value_value2"])

        def set_value(value, c=c): c.value = value
        self.assertRaises(ItemCountError, set_value,
                          ["value_value", "value_value2"])
        self.assertRaises(TypeError, set_value, "value_value")
        self.assert_(c.value == ["value_value"])
        c.value = ["value_value2"]
        self.assert_(c.value == ["value_value2"])
        c.value = ["value_value"]
        self.assert_(c.value == ["value_value"])
        self.assertRaises(ItemNotFoundError, set_value, ["oops"])
        self.assert_(c.value == ["value_value"])
        c.toggle("value_value")
        self.assert_(c.value == [])
        c.toggle("value_value")
        self.assert_(c.value == ["value_value"])
        self.assertRaises(TypeError, c.toggle, ["value_value"])
        self.assert_(c.value == ["value_value"])
        # nothing selected is allowed
        c.value = []
        self.assert_(c.value == [])
        # set
        c.set(True, "value_value")
        self.assert_(c.value == ["value_value"])
        c.set(True, "value_value")
        self.assert_(c.value == ["value_value"])
        c.set(True, "value_value2")
        self.assert_(c.value == ["value_value2"])
        c.set(False, "value_value")
        self.assert_("value_value2")
        c.set(False, "value_value2")
        self.assert_(c.value == [])
        c.set(False, "value_value2")
        self.assert_(c.value == [])
        self.assertRaises(ItemNotFoundError, c.set, True, "oops")
        self.assertRaises(TypeError, c.set, True, ["value_value"])
        self.assertRaises(ItemNotFoundError, c.set, False, "oops")
        self.assertRaises(TypeError, c.set, False, ["value_value"])


class FormTests(TestCase):
    base_uri = "http://auth.athensams.net/"
    def test_click(self):
        file = StringIO(
"""<form action="abc" name="myform">

<input type="submit" name="foo"></input>
<input type="submit" name="bar"></input>
</form>
""")
        form = ClientForm.ParseFile(file, "http://blah/")[0]
        self.assertRaises(ControlNotFoundError, form.click, nr=2)
        self.assert_(form.click().get_full_url() == "http://blah/abc?foo=")
        self.assert_(form.click(name="bar").get_full_url() == "http://blah/abc?bar=")

        # XXX POST, ?, and #
        for method in ["GET", "POST"]:
            file = StringIO(
"""<form method="%s" action="abc?bang=whizz#doh" name="myform">

<input type="submit" name="foo"></input>
</form>
""" % method)
            # " (this line is here for emacs)
            form = ClientForm.ParseFile(file, "http://blah/")[0]
            if method == "GET":
                url = "http://blah/abc?foo="
            else:
                url = "http://blah/abc?bang=whizz"
            self.assert_(form.click().get_full_url() == url)

    def testAuth(self):
        file = open("./testdata/Auth.html", "r")
        forms = ClientForm.ParseFile(file, self.base_uri)
        self.assert_(len(forms) == 1)
        form = forms[0]
        self.assert_(form.action ==
                     "http://auth.athensams.net/"
                     "?ath_returl=%22http%3A%2F%2Ftame.mimas.ac.uk%2Fisicgi"
                     "%2FWOS-login.cgi%22&ath_dspid=MIMAS.WOS")

        self.assertRaises(ControlNotFoundError,
                          lambda form=form: form.toggle("d'oh", "oops"))
        self.assertRaises(ControlNotFoundError, lambda form=form: form["oops"])
        def bad_assign(form=form): form["oops"] = ["d'oh"]
        self.assertRaises(ControlNotFoundError, bad_assign)

        self.assertRaises(ValueError, form.find_control)

        keys = ["ath_uname", "ath_passwd"]
        values = ["", ""]
        types = ["text", "password"]
        for i in range(len(keys)):
            key = keys[i]
            c = form.find_control(key)
            self.assert_(c.value == values[i])
            self.assert_(c.type == types[i])
        c = form.find_control(type="image")
        self.assert_(c.name is None)
        self.assert_(c.value == "")
        self.assert_(c.type == "image")

        form["ath_uname"] = "jbloggs"
        form["ath_passwd"] = "foobar"

        self.assert_(form.click_pairs() ==
                     [("ath_uname", "jbloggs"),
                      ("ath_passwd", "foobar")])

    def testSearchType(self):
        file = open("./testdata/SearchType.html", "r")
        forms = ClientForm.ParseFile(file, self.base_uri)
        self.assert_(len(forms) == 1)
        form = forms[0]

        keys = ["SID", "SESSION_DIR", "Full Search", "Easy Search",
                "New Session", "Log off", "Form", "JavaScript"]
        values = ["PMrU0IJYy4MAAELSXic_E2011300_PMrU0IJYy4MAAELSXic-0",
                  "", "", "", "", "", "Welcome", "No"]
        types = ["hidden", "hidden", "image", "image", "image", "image",
                 "hidden", "hidden"]
        for i in range(len(keys)):
            key = keys[i]
            self.assert_(form.find_control(key).value == values[i])
            self.assert_(form.find_control(key).type == types[i])

        pairs = form.click_pairs("Full Search")
        self.assert_(pairs == [
            ("SID", "PMrU0IJYy4MAAELSXic_E2011300_PMrU0IJYy4MAAELSXic-0"),
            ("SESSION_DIR", ""), ("Full Search.x", "1"), ("Full Search.y", "1"),
            ("Form", "Welcome"), ("JavaScript", "No")])

    def testFullSearch(self):
        pass  # XXX

    def testGeneralSearch(self):
        file = open("./testdata/GeneralSearch.html", "r")
        forms = ClientForm.ParseFile(file, self.base_uri)
        self.assert_(len(forms) == 1)
        form = forms[0]

        keys = ["SID", "SESSION_DIR",
                "Home", "Date & Database Limits", "Cited Ref Search",
                "Log off", "Search",
                "topic", "titleonly", "author", "journal", "address",
                "Search", "Save query", "Clear",
                "languagetype", "doctype", "Sort",
                "Form", "Func"]
        values = ["PMrU0IJYy4MAAELSXic_E2011300_PMrU0IJYy4MAAELSXic-0", "",
                  "", "", "", "", "",
                  "", [], "", "", "",
                  "", "", "",
                  ["All languages"], ["All document types"], ["Latest date"],
                  "General", "Search"]
        types = ["hidden", "hidden",
                 "image", "image", "image", "image", "image",
                 "text", "checkbox", "text", "text", "text",
                 "image", "image", "image",
                 "select", "select", "select",
                 "hidden", "hidden"]
        fc = form.find_control
        for i in range(len(keys)):
            name = keys[i]
            type = types[i]
            self.assert_(fc(name).value == form.get_value(name) == values[i])
            self.assert_(fc(name).type == type)
            self.assert_(fc(name, type).name == name)
        self.assert_(fc(type="hidden").name == "SID")
        self.assert_(fc(type="image").name == "Home")
        self.assert_(fc(nr=6).name == "Search")
        self.assertRaises(ControlNotFoundError, fc, nr=50)
        self.assertRaises(ValueError, fc, nr=-1)
        self.assert_(fc("Search", "image").name == "Search")
        self.assertRaises(ControlNotFoundError, fc, "Search", "hidden")
        s0 = fc("Search", "image", nr=0)
        s0b = fc("Search", "image", nr=0)
        s1 = fc("Search", "image", nr=1)
        self.assert_(s0.name == s1.name == "Search")
        self.assert_(s0 is s0b)
        self.assert_(s0 is not s1)
        self.assertRaises(ControlNotFoundError, fc, "Search", "image", nr=2)
        self.assert_(fc(type="text", nr=2).name == "journal")
        self.assert_(fc("Search", nr=0) is not fc("Search", nr=1))

        form["topic"] = "foo"
        self.assert_(form["topic"] == "foo")
        form["author"] = "bar"
        form["journal"] = ""
        form["address"] = "baz"
        form["languagetype"] = ["English", "Catalan"]
        self.assert_(form["languagetype"] == ["English", "Catalan"])
        form["titleonly"] = ["on"]
        self.assert_(form["titleonly"] == ["on"])
        pairs = form.click_pairs("Search")
        self.assert_(pairs == [
            ("SID", "PMrU0IJYy4MAAELSXic_E2011300_PMrU0IJYy4MAAELSXic-0"),
            ("SESSION_DIR", ""),
            ("Search.x", "1"), ("Search.y", "1"),
            ("topic", "foo"),
            ("titleonly", "on"),
            ("author", "bar"),
            ("journal", ""), ("address", "baz"),
            ("languagetype", "English"), ("languagetype", "Catalan"),
            ("doctype", "All document types"), ("Sort", "Latest date"),
            ("Form", "General"), ("Func", "Search")])

        pvs = form.possible_items("languagetype")
        self.assert_(pvs[0] == "All languages")
        self.assert_(len(pvs) == 47)

        self.assertRaises(
            ItemNotFoundError,
            lambda form=form: form.toggle("d'oh", "languagetype"))
        form.toggle("English", "languagetype")
        self.assert_(form["languagetype"] == ["Catalan"])
        self.assertRaises(TypeError, form.toggle, ["Catalan"], "languagetype")
        self.assertRaises(TypeError, form.toggle, "Catalan", ["languagetype"])

        # XXX type, nr, by_label args

        self.assertRaises(ControlNotFoundError, form.set, True, "blah", "SID")

        # multiple select
        form["languagetype"] = []
        self.assert_(form["languagetype"] == [])
        form.set(True, "Catalan", "languagetype")
        self.assert_(form["languagetype"] == ["Catalan"])
        form.set(True, "English", "languagetype")
        self.assert_(form["languagetype"] == ["English", "Catalan"])
        form.set(False, "English", "languagetype")
        self.assert_(form["languagetype"] == ["Catalan"])
        form.set(False, "Catalan", "languagetype")
        self.assert_(form["languagetype"] == [])
        self.assertRaises(ItemNotFoundError, form.set, True, "doh", "languagetype")
        self.assertRaises(ItemNotFoundError, form.set, False, "doh", "languagetype")
        self.assertRaises(ControlNotFoundError, form.set, True, "blah", "oops")
        self.assertRaises(TypeError, form.set, True, ["Catalan"], "languagetype")
        self.assertRaises(TypeError, form.set, False, ["Catalan"], "languagetype")
        self.assertRaises(TypeError, form.set, True, "Catalan", ["languagetype"])
        self.assertRaises(TypeError, form.set, False, "Catalan", ["languagetype"])

        def setitem(name, value, form=form): form[name] = value
        form["languagetype"] = ["Catalan"]
        self.assert_(form["languagetype"] == ["Catalan"])
        self.assertRaises(ItemNotFoundError,
                          setitem, "languagetype", ["doh"])
        self.assertRaises(ControlNotFoundError, setitem, "oops", ["blah"])
        self.assertRaises(TypeError, setitem, ["languagetype"], "Catalan")

        # single select
        form["Sort"] = []
        self.assert_(form["Sort"] == [])
        form.set(True, "Relevance", "Sort")
        self.assert_(form["Sort"] == ["Relevance"])
        form.set(True, "Times Cited", "Sort")
        self.assert_(form["Sort"] == ["Times Cited"])
        form.set(False, "Times Cited", "Sort")
        self.assert_(form["Sort"] == [])
        self.assertRaises(ItemNotFoundError, form.set, True, "doh", "Sort")
        self.assertRaises(ItemNotFoundError, form.set, False, "doh", "Sort")
        self.assertRaises(ControlNotFoundError, form.set, True, "blah", "oops")
        self.assertRaises(TypeError, form.set, True, ["Relevance"], "Sort")
        self.assertRaises(TypeError, form.set, False, ["Relevance"], "Sort")
        self.assertRaises(TypeError, form.set, True, "Relevance", ["Sort"])
        self.assertRaises(TypeError, form.set, False, "Relevance", ["Sort"])

        form["Sort"] = ["Relevance"]
        self.assert_(form["Sort"] == ["Relevance"])
        self.assertRaises(ItemNotFoundError,
                          setitem, "Sort", ["doh"])
        self.assertRaises(ControlNotFoundError, setitem, "oops", ["blah"])
        self.assertRaises(TypeError, setitem, ["Sort"], ["Relevance"])

    def testResults(self):
        file = open("./testdata/Results.html", "r")
        forms = ClientForm.ParseFile(file, self.base_uri)
        self.assert_(len(forms) == 1)
        form = forms[0]

        pvs = form.possible_items("marked_list_candidates")
        self.assert_(pvs == [
            "000174872000059/1", "000174858300003/2", "000174827900006/3"])
        def bad_setitem(form=form):
            form["marked_list_candidates"] = ["blah"]
        self.assertRaises(ItemNotFoundError, bad_setitem)
        form["marked_list_candidates"] = [pvs[0]]

        # I've removed most of the INPUT elements from this page, and
        # corrected an HTML error
        keys = ["Add marked records to list",
                "Add records on page to list",
                "Add all records retrieved to list",
                "marked_list_candidates",
                "Add marked records to list",
                "Add records on page to list",
                "Add all records retrieved to list"
                ]
        types = ["image", "image", "image",
                 "checkbox",
                 "image", "image", "image"]
        values = ["", "", "",
                  [pvs[0]],
                  "", "", "",
                 ]

        for i in range(len(keys)):
            key = keys[i]
            control = form.find_control(key)
            self.assert_(control.value == values[i])
            self.assert_(control.type == types[i])

        pairs = form.click_pairs("Add all records retrieved to list")
        self.assert_(pairs == [
            ("Add all records retrieved to list.x", "1"),
            ("Add all records retrieved to list.y", "1"),
            ("marked_list_candidates", pvs[0])])

    def testMarkedResults(self):
        file = open("./testdata/MarkedResults.html", "r")
        forms = ClientForm.ParseFile(file, self.base_uri)
        self.assert_(len(forms) == 1)
        form = forms[0]

        pairs = form.click_pairs()
        # I've removed most of the INPUT elements from this page, and
        # corrected an HTML error
        self.assert_(pairs == [
            ("Add marked records to list.x", "1"),
            ("Add marked records to list.y", "1"),
            ("marked_list_candidates", "000174872000059/1"),
            ("marked_list_candidates", "000174858300003/2"),
            ("marked_list_candidates", "000174827900006/3")
            ])

    def testMarkedRecords(self):
        pass  # XXX


class MoreFormTests(TestCase):
    def make_form(self):
        f = StringIO("""\
<form blah="nonsense" name="formname">
  <input type="checkbox" name="a" value="1" id="1a" blah="spam"></input>
  <input type="checkbox" name="a" value="2" blah="eggs"></input>
  <input type="checkbox" name="a" value="3" id="3a"></input>

  <input type="radio" name="b" value="1"></input>
  <input type="radio" name="b" value="2" id="2"></input>
  <input type="radio" name="b" value="3" id="3"></input>

  <select name="c" id="cselect" blah="foo">
    <option id="coption1" blah="bar">1</option>
    <option selected blah="baz">2</option>
    <option id="coption3">3</option>
  </select>

  <select name="d" multiple>
    <option value="v1">l1</option>
    <option value="v2">l2</option>
    <option blah="fee" rhubarb="fi" value="v3">l3</option>
  </select>

  <input type="checkbox" name="e" value="1"></input>
</form>
""")
        return ClientForm.ParseFile(f, "http://blah/")[0]

    def test_value(self):
        form = self.make_form()

        form.set_value(["v3"], type="select", kind="multilist")
        self.assert_(form.get_value("d") == ["v3"])
        form.set_value(["l2"], type="select", kind="multilist", by_label=True)
        self.assert_(form.get_value("d", by_label=True) == ["l2"])

        self.assert_(form.get_value(
            "b", "radio", "singlelist", None, 0, False) == [])
        self.assertRaises(NotImplementedError,
                          form.set_value, ["1"], "b", by_label=True)

    def test_id(self):
        form = self.make_form()

        self.assert_(form.find_control("c").id == "cselect")
        self.assert_(form.find_control("a").id == "1a")
        self.assert_(form.find_control("b").id is None)

        self.assert_(form.find_control(id="cselect").id == "cselect")
        self.assertRaises(ControlNotFoundError, form.find_control,
                          id="coption1")
        self.assert_(form.find_control(id="1a").id == "1a")
        self.assertRaises(ControlNotFoundError, form.find_control, id="1")

    def test_single(self):
        form = self.make_form()

        self.assertRaises(ItemCountError, form.set_single, True, "d")

        self.assertRaises(NotImplementedError,
                          form.set_single, True, "e", by_label=True)
        form.toggle_single("e", "checkbox", "list", nr=0)
        self.assert_("1" in form.get_value("e"))
        form.set_single(False, "e", "checkbox", "list", nr=0)
        self.assert_("1" not in form.get_value("e"))
        form.set_single(True, "e", "checkbox", "list", nr=0)
        self.assert_("1" in form.get_value("e"))

    def test_possible_items(self):
        form = self.make_form()

        self.assert_(form.possible_items("c") == ["1", "2", "3"])
        self.assert_(form.possible_items("d", by_label=True) ==
                     ["l1", "l2", "l3"])

        self.assert_(form.possible_items("a") == ["1", "2", "3"])
        self.assertRaises(NotImplementedError,
                          form.possible_items, "a", by_label=True)

    def test_set_all_readonly(self):
        form = self.make_form()

        form.set_all_readonly(True)
        for c in form.controls:
            self.assert_(c.readonly)
        form.set_all_readonly(False)
        for c in form.controls:
            self.assert_(not c.readonly)

    def test_attrs(self):
        form = self.make_form()

        self.assert_(form.attrs["blah"] == "nonsense")
        self.assert_(form.attrs["name"] == "formname")

        a = form.find_control("a")
        self.assert_(not hasattr(a, "attrs"))
        self.assert_(a.get_item_attrs("1")["blah"] == "spam")
        self.assert_(a.get_item_attrs("2")["blah"] == "eggs")
        self.assert_(not a.get_item_attrs("3").has_key("blah"))

        c = form.find_control("c")
        self.assert_(c.attrs["blah"] == "foo")
        self.assert_(c.get_item_attrs("1")["blah"] == "bar")
        self.assert_(c.get_item_attrs("2")["blah"] == "baz")
        self.assert_(not c.get_item_attrs("3").has_key("blah"))


def startswith(string, initial):
    if len(initial) > len(string): return False
    return string[:len(initial)] == initial

class CaseInsensitiveDict:
    def __init__(self, dict):
        self._dict = {}
        for key, val in dict.items():
            self._dict[string.lower(key)] = val

    def __getitem__(self, key): return self._dict[key]

    def __getattr__(self, name): return getattr(self._dict, name)


class UploadTests(TestCase):
    def make_form(self):
        html = """\
<form action="/cgi-bin/upload.cgi" method="POST" enctype="multipart/form-data">
<input type="file" name="data">
<input type="text" name="user" value="nobody">
<br>
<input type="submit">
</form>
"""

        return ClientForm.ParseFile(StringIO(html),
                                    "http://localhost/cgi-bin/upload.cgi")[0]

    def test_file_request(self):
        import cgi

        # fill in a file upload form...
        form = self.make_form()
        form["user"] = "john"
        data_control = form.find_control("data")
        data = "blah\nbaz\n"
        data_control.add_file(StringIO(data))
        #print "data_control._upload_data", data_control._upload_data
        req = form.click()
        self.assert_(startswith(req.headers["Content-type"],
                                'multipart/form-data; boundary='))

        #print "req.get_data()\n>>%s<<" % req.get_data()

        # ...and check the resulting request is understood by cgi module
        fs = cgi.FieldStorage(StringIO(req.get_data()),
                              CaseInsensitiveDict(req.headers),
                              environ={"REQUEST_METHOD": "POST"})
        self.assert_(fs["user"].value == "john")
        self.assert_(fs["data"].value == data)
        self.assert_(fs["data"].filename is None)

    def test_file_request_with_filename(self):
        import cgi

        # fill in a file upload form...
        form = self.make_form()
        form["user"] = "john"
        data_control = form.find_control("data")
        data = "blah\nbaz\n"
        data_control.add_file(StringIO(data), filename="afilename")
        req = form.click()
        self.assert_(startswith(req.headers["Content-type"],
                                'multipart/form-data; boundary='))

        # ...and check the resulting request is understood by cgi module
        fs = cgi.FieldStorage(StringIO(req.get_data()),
                              CaseInsensitiveDict(req.headers),
                              environ={"REQUEST_METHOD": "POST"})
        self.assert_(fs["user"].value == "john")
        self.assert_(fs["data"].value == data)
        self.assert_(fs["data"].filename == "afilename")

    def test_multipart_file_request(self):
        import cgi

        # fill in a file upload form...
        form = self.make_form()
        form["user"] = "john"
        data_control = form.find_control("data")
        data = "blah\nbaz\n"
        data_control.add_file(StringIO(data), filename="filenamea")
        more_data = "rhubarb\nrhubarb\n"
        data_control.add_file(StringIO(more_data), filename="filenameb")
        yet_more_data = "rheum\nrhaponicum\n"
        data_control.add_file(StringIO(yet_more_data), filename="filenamec")
        req = form.click()
        self.assert_(startswith(req.headers["Content-type"],
                                'multipart/form-data; boundary='))

        #print "req.get_data()\n>>%s<<" % req.get_data()

        # ...and check the resulting request is understood by cgi module
        fs = cgi.FieldStorage(StringIO(req.get_data()),
                              CaseInsensitiveDict(req.headers),
                              environ={"REQUEST_METHOD": "POST"})
        self.assert_(fs["user"].value == "john")

        fss = fs["data"][None]
        filenames = "filenamea", "filenameb", "filenamec"
        datas = data, more_data, yet_more_data
        for i in range(len(fss)):
            fs = fss[i]
            filename = filenames[i]
            data = datas[i]
            self.assert_(fs.filename == filename)
            self.assert_(fs.value == data)

    def test_upload_data(self):
        form = self.make_form()
        data = form.click().get_data()
        self.assert_(startswith(data, "--"))

    def test_empty_upload(self):
        # no controls except for INPUT/SUBMIT
        forms = ClientForm.ParseFile(StringIO("""<html>
<form method="POST" action="./weird.html" enctype="multipart/form-data">
<input type="submit" name="submit"></input>
</form></html>"""), ".")
        form = forms[0]
        data = form.click().get_data()
        lines = string.split(data, "\r\n")
        self.assert_(startswith(lines[0], "--"))
        self.assert_(lines[1] == 
                     'Content-disposition: form-data; name="submit"')
        self.assert_(lines[2] == lines[3] == "")
        self.assert_(startswith(lines[4], "--"))


if __name__ == "__main__":
    unittest.main()
