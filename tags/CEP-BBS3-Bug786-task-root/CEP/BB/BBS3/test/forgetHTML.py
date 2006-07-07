# $Id$
#
# (c) Stian Soiland <stain@nvg.org> 2001-2003
# Licence: LGPL
#

"""A totally object-oriented way of writing HTML. Each
element is a object which may contain other HTML
elements (ie. objects). Only those elements allowed
in HTML to contain each other are allowed to be
inserted.

Print or str() or .output() the top (root) object
to get the final HTML."""

# Last changes:
# $Log$
# Revision 1.8  2004/09/13 00:48:42  stain
# radiobutton is 'radio' in <input type='radio'>, not 'radiobutton'.
# The class is still 'Radiobutton' though, as this isn't a radio, but a
# radio button.
#
# Revision 1.7  2004/04/25 22:14:00  stain
# code style fixup
#
# Revision 1.6  2004/04/15 22:50:25  stain
# Use class_ instead of _class - this is the preferred Pythonic way
# to have a variable named "class" according to PEP 0009.
#
# (the old _class still works, for compatibility)
#
# Revision 1.5  2003/11/07 10:47:21  stain
# Calling a HTML element returns the HTML.. this is usefull with
# template engines like Cheetah.
# 

# Note:
#  + Only a few elements are supported at this time.
#  + Currently all possible attributes are allowed
#   (modify element.attributes or simply include them
#    as keywords in constructor call)

import exceptions, types

indention = ' ' * 2  # Use two spaces for indention in HTML output

class HTMLError(exceptions.Exception):
    """General HTML error"""
    def __str__(self):
        args = exceptions.Exception.__str__(self) # Get our arguments
        return self.__doc__ + ': ' + args

class ElementNotAllowedError(HTMLError):
    """Element not allowed"""
    pass

class AbstractClassError(HTMLError):
    """Abstract class"""
    pass

class HeaderLevelOutOfRangeError(HTMLError):
    """Header level out of range (1-6)"""
    pass

class Element:
    tag = None # this would be 'p' for <p>, etc.
    
    def __init__(self, *add, **attributes):
        self._content = []
        self.attributes = attributes # might be {} :=)
        self.allowed_content = self._define_allowed()
        self.default_child = self._define_default()
        self.extend(add)

        # class_ means the class attribute. We need to 
        # 'escape' this to avoid the Python keyword class
        if(self.attributes.has_key('class_')):
            # Rename it!
            self['class'] = self['class_']
            del self.attributes['class_']
        # compatibility mode, class_ is the preferred python style    
        elif(self.attributes.has_key('_class')):
            # Rename it!
            self['class'] = self['_class']
            del self.attributes['_class']
            
    def __cmp__(self, other):
        # Sort (recursive) by content
        if isinstance(other, Element):
                return cmp(self._content, other._content)
        else: 
                return cmp(self._content, other)
                
    def _define_allowed(self):
        """Override this method to specify allowed content"""
        return [Flow]         # list of classes of elements 
        # The reason for using a seperate function for this
        # is to make it possible to allow classes that have
        # not yet been declared at the point of parsing
        
    def _define_default(self):
        """Override this method to specify an element
        class wrapper to be tried if not matched by isallowed""" 
        return None
        
    def _check_abstract(self):     
        """Checks if our class is capable of producing HTML"""
        # If self.tag is not set, this is not a real
        # HTML-element for use by append or __str__
        if(self.tag == None): # we allow '' as a tag (used by Text)
            raise AbstractClassError, self.__class__.__name__        

    def __str__(self):
        return self.output()

    def __call__(self):
        return self.output()
        
    def output(self, indent=0):
        # returns the HTML-formatted output, nicely indented
        self._check_abstract()
        result = indention * indent + '<' + self.tag
        # Each attribute in the form key="value"
        count = -1
        for (attribute,value) in self.attributes.items():
            count += 1
            if(not count % 2 and count): # Not the first time!
                result = result + '\n' + indention * indent + ' ' * (len(self.tag) + 1)
            result = result + ' %s="%s"' % (attribute,value)
        if(not self._content):
            result = result + ' />\n' # No content!
        else:
            result = result + '>\n'
            for element in self._content:
                result = result + element.output(indent+1) # increased indent level
                # end tag
            result = result + indention * indent + '</' + self.tag + '>\n'
        return result

    def get(self, key):
        """Finds a sub-element by it's id. Fun part: recursive! """
        if(key == self.attributes.get('id')):
            return self # simplest case
        for item in self._content:
            match = item.get(key)
            if(match):
                return match
        return None # No match
 
    def __getitem__(self, key):
        return self.attributes.__getitem__(key)

    def __setitem__(self, key, value):
        self.attributes.__setitem__(key, value)
        
    def isallowed(self,element):
        """Checks if the given given element is allowed within ourself"""
        for allowed in self.allowed_content:
            if(isinstance(element,allowed)):
                return 1
        return 0 # None of those allowed matched

    def extend(self, list):
        """Appends each element in the given list"""
        for element in list:
            self.append(element)
        
    def append(self, element):        
        """Appends the given element within this element"""
        self._check_abstract()
        if(type(element) in (types.StringType, types.UnicodeType, types.LongType, types.IntType, types.FloatType)):
             # For easy adding of text and numbers, we make a text
             #    element
             element = Text(element)
        if(not self.isallowed(element)):
            if(type(self.default_child) == types.ClassType):
                element = self.default_child(element) # No further checks
            else:
                raise ElementNotAllowedError, repr(element)
        self._content.append(element)
    
    def set(self, element):
        """Replaces the content with this new element, if it's allowed"""
        old = self._content
        self._content = []
        try:
            self.append(element)
        except Exception, e:
            self_content = old # Restore!
            raise e

class Document(Element):
    tag = 'html'

    def _define_allowed(self):
        return [Head, Body]

    def output(self, indent=0):
        result = """<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
                    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
"""
        return result + Element.output(self, indent)


class Head(Element):
    tag = 'head'
    
    def _define_allowed(self):
        return [HeadStuff]

class HeadStuff(Element):
    pass

class Meta(HeadStuff):
    tag = 'meta'

    def _define_allowed(self):
        return []

class Link(HeadStuff):
    tag = 'link'

    def _define_allowed(self):
        return []

class NonIndentMixin:
    
    def output(self, indent=0):
        indent = -999
        return Element.output(self, indent)

# <link href="/stylesheet.css" rel="stylesheet" type="text/css" />

class Stylesheet(Link):
    
    def __init__(self, href, media=None):
        if(media):
            Link.__init__(self,
                            rel='stylesheet',
                            type='text/css',
                            href=href,
                            media=media)
        else:
            Link.__init__(self,
                            rel='stylesheet',
                            type='text/css',
                            href=href)

class Title(HeadStuff):
    tag = 'title'

    def _define_allowed(self):
        return [Text]

class Body(Element):
    tag = 'body'

class Flow(Element):
    pass

class Block(Flow):
    pass

class Inline(Flow):

    def _define_allowed(self):
        return [Inline]

class Text(Inline):
    """Special class for plain text"""
    tag = ''

    def _define_allowed(self):
        return []

    def isallowed(self,element):
        # Only strings and numbers are allowed. Unicode not supported (yet)
        if(type(element) in (types.StringType, types.LongType, types.IntType, types.FloatType)):
            return 1
        else:
            return 0
            
    def get(self, key):
        """Finds a sub-element by it's id. Fun part: recursive! """
        if(key == self.attributes.get('id')):
            return self # simplest case
        else:
            return None
        # We don't want to search our _content

    def append(self, element):
        # Overriding this method is very important,
        # otherwise Text('blapp') would loop forever :)
        if(not self.isallowed(element)):
            raise ElementNotAllowedError, element
        self._content.append(element)
        
    def output(self, indent=0):
        result = ''
        for line in self._content:
            result = result + indention * indent + str(line) + '\n'
            # Question: What would be the right thing to do here?
            # Is it acceptable to allow several entries in self._content?
            # Shoult they be seperated by ' ', '\n' or '' in output? 
        return result

class List(Block):
    
    def _define_allowed(self):
        return [ListItem]

    def _define_default(self):
        return ListItem
    
class UnorderedList(List):
    tag = 'ul'

class OrderedList(List):
    tag = 'ol'

class ListItem(Element):
    tag = 'li'

class Table(Block):
    tag = 'table'
    
    def _define_allowed(self):
        return [TableRow]
        
    def _define_default(self):
        return TableRow
        
    def extend2(self, lists):
        """Inserts each array in lists as a row in the table"""
        for list in lists:
            row = TableRow()
            row.extend(list)
            self.append(row)

class TableRow(Element):
    tag = 'tr'
    def _define_allowed(self):
        return [TableCell]
    def _define_default(self):
        return TableCell

class TableCell(Element):
    tag = 'td'

class TableHeader(TableCell):
    tag = 'th'

class Division(Block):
    tag = 'div'

class Span(Inline):
    tag = 'span'

class Strong(Inline):
    tag = 'strong'

class Emphasis(Inline):
    tag = 'em'

class Small(Inline):
    tag = 'small'

class Big(Inline):
    tag = 'big'


class Header(Block):
    def _define_allowed(self):
        return [Inline] 

    def __init__(self, add=None, level=1, **kargs):
        self.set_level(level)
        if(add):
            Block.__init__(self, add, **kargs)
        else:
            Block.__init__(self, **kargs)

    def set_level(self, level):
        if((level >= 1) and (level <= 6)):
            self.level = level
            self.tag = 'h' + str(self.level)
        else:
            raise HeaderLevelOutOfRangeError, level

class Paragraph(Block): 
    tag = 'p'
    def _define_allowed(self):
        return [Inline]


class Pre(NonIndentMixin, Block): 
    tag = 'pre'
    def _define_allowed(self):
        return [Inline]


class Break(Inline):
    tag = 'br'
    def _define_allowed(self):
        return []


class Image(Inline):
    tag = 'img'
    def _define_allowed(self):
        return []


class Anchor(Inline):
    tag = 'a'
    def _define_allowed(self):
        return [Inline]
        

class Ruler(Block):
    tag = 'hr'
    def _define_allowed(self):
        return []


class Form(Block):
    tag = 'form'
    def _define_allowed(self):
        return [Flow]

class Input(Inline):
    tag = 'input'
    def _define_allowed(self):
        return []


class Select(Inline):
    tag = 'select'
    def _define_allowed(self):
        return [OptGroup, Option]


class OptGroup(Element):
    tag = 'optgroup'
    def _define_allowed(self):
        return [Option]


class Option(Element):
    tag = 'option'
    def _define_allowed(self):
        return [Text]


class Submit(Input):
    def __init__(self, value=None, *args, **kargs):
        if(value <> None):
            Input.__init__(self, type='submit', value=value, *args, **kargs)
        else:
            Input.__init__(self, type='submit', *args, **kargs)


class Checkbox(Input):
    def __init__(self, *args, **kargs):
        Input.__init__(self, type='checkbox',    *args, **kargs)


class Radiobutton(Input):
    def __init__(self, *args, **kargs):
        Input.__init__(self, type='radio',    *args, **kargs)


class Textfield(Input):
    def __init__(self, *args, **kargs):
        Input.__init__(self, type='text',    *args, **kargs)


class Hidden(Input):
    def __init__(self, *args, **kargs):
        Input.__init__(self, type='hidden',    *args, **kargs)


class Textarea(Inline, NonIndentMixin):
    tag = 'textarea'


class Label(Inline):
    tag = 'label'
    def _define_allowed(self):
        return [Inline]


class SimpleDocument(Document):
    """A handy default document with a title and
    optionally a specified external stylesheet. You
    may change the title or stylesheet later on using
    the methods setTitle() and setStylesheet()."""
    
    def __init__(self, title=None, stylesheet=None):
        Document.__init__(self)
        self.head = Head()
        self.body = Body()
        self.append(self.head)
        self.append(self.body)
        self.title = None
        self.header = None
        self.stylesheet = None
        if(title):
            self.setTitle(title)
        if(stylesheet):
            self.setStylesheet(stylesheet)

    def setTitle(self, title):     
        if not self.title:
            self.title = Title()
            self.head.append(self.title)
        if not self.header:
            self.header = Header()
            self.body.append(self.header)
        self.title.set(title)
        self.header.set(title)

    def setStylesheet(self, url):
        if not self.stylesheet:
            self.stylesheet = Stylesheet(url)
            self.head.append(self.stylesheet)
        self.stylesheet['href'] = url    


class SimpleForm(Form):
    """A simple way to create an easy form. 

    form = SimpleForm()
    form.addText('firstname', 'First name', 'John')
    form.addText('lastname', 'Last name')
    choices = [
        ('red', 'Red color'),
        ('blue', 'Blue color'),
        ('green', 'Greenish color')
    ]
    form.addChoices('color', choices, 'Select a color',
                                    'red')
    """
    
    def __init__(self, **kwargs):
        Form.__init__(self, **kwargs)

    def addText(self, id, description=None, value=None):
        """Adds a textfield, optionallly with a label."""
        div = Division()
        self.append(div)
        if(description):
            label = Label(description)
            label['for'] = id
            div.append(label)
        if(value):
            div.append(Textfield(id=id, name=id, value=value))
        else:    
            div.append(Textfield(id=id, name=id))

    def addChoices(self, id, choices, description=None, 
                   default=None):
        """Adds a selection box for the given choices.
        Note that the choices parameter must be a sequence of 
        bi-tupples (key, description).
        """
        div = Division()
        self.append(div)
        label = Label(description)
        label['for'] = id
        select = Select(id=id, name=id)
        for (key, desc) in choices:
            option = Option(desc, id=key, value=key)
            if(key == default):
                option['selected'] = 'selected'
            select.append(option)
        div.append(label)
        div.append(select)


class SimpleTable(Table):
    def __init__(self, header="column", *args, **kwargs):
        """header -- "column" or "row"
        if column, the first column will be indexed (TableHeader), 
        if "row", the first row will be indexed.
        if "both", both the first row and column will be indexed.
        Anything else (like None) - no index"""
        Table.__init__(self, *args, **kwargs)
        self._items = []
        self._width=0
        self.header = header

    def add(self, *args, **kwargs):
        self._content = [] # reset html version because we changed..!
        row = (kwargs,) + args
        self._width = max(self._width, self._rowWidth(row))
        rowWidth = self._rowWidth(row)
        self._items.append(row)

    def _getColSpan(self, cell):
        # Eh.. we need to substract colspans.. 
        if isinstance(cell, TableCell):
            try:
                # *cough* *cough*
                colspan = cell['colspan']
                colspan = int(colspan)
                return colspan
            except KeyError:
                return 1
            except ValueError:
                return 1
        return 1           

    def _rowWidth(self, row):
        rowWidth = 0
        for cell in row[1:]:
            rowWidth += self._getColSpan(cell)
        return rowWidth    

    def _extendRow(self, row):
        """Extends a row if needed, but skip the kwargs in pos 0"""
        rowWidth = self._rowWidth(row)
        extendWidth = self._width - rowWidth
        # skip first element, and extend
        return list(row[1:]) + [''] * extendWidth
        
    def _generateContent(self):    
        if not self._items:
            pass
        items = self._items
        if self.header in ("row", "both"):
            row=items.pop(0)
            kwargs = row[0]
            row = self._extendRow(row)
            tablerow = TableRow(**kwargs)
            self.append(tablerow)
            pos = 0
            for column in row:
                cssClass = "col%s" % pos
                if not isinstance(column, TableCell):
                    column = TableHeader(column)
                try:
                    column['class'] = column['class'] + ' ' + cssClass
                except KeyError:
                    column['class'] = cssClass
                tablerow.append(column)
                pos += self._getColSpan(column)
        even = True         
        for row in items:
            even = not even #ooo, flipz colorstylz

            kwargs = row[0]
            row = self._extendRow(row)
            tablerow = TableRow(**kwargs)
            try:
                class_ = tablerow['class'] + " "
            except KeyError:
                class_ = ""
            tablerow['class'] = class_ + (even and 'even' or 'odd')
            self.append(tablerow)
            pos = 0
            cssClass = "col%s" % pos
            column = row[0]
            if not isinstance(column, TableCell):
                if self.header in ("column", "both"):
                    column = TableHeader(column)
                else:
                    column = TableCell(column)
            try:
                column['class'] = column['class'] + ' ' + cssClass
            except KeyError:   
                column['class'] = cssClass
            tablerow.append(column)    
            pos += self._getColSpan(column)
            for column in row[1:]:
                cssClass = "col%s" % pos
                if not isinstance(column, TableCell):
                    column = TableCell(column)
                try:
                    column['class'] = column['class'] + ' ' + cssClass
                except KeyError:   
                    column['class'] = cssClass
                tablerow.append(column)
                pos += self._getColSpan(column)

    def output(self, *args, **kwargs):
        if not self._content:
            self._generateContent()
        return Table.output(self, *args, **kwargs)
 
