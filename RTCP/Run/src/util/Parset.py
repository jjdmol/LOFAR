from Hosts import ropen
import shlex

def isnumeric( x ):
  """ Returns whether x is numerical. """
  try:
    float(x)
  except ValueError:
    return False
  except TypeError:
    return False

  return True

def decode(s):
  """ decode a string s into an array of values (possibly one). """

  # (xxx) = xxx
  if s[0] == "(" and s[-1] == ")":
    s = s[1:-1]

  # x;x;x = x,x,x
  if ";" in s:
    # warning: not safe in case of () recursion
    return map(decode, s.split(";"))

  # 2*xxx = [xxx,xxx]
  if "*" in s:
    num,s = s.split("*",1)
    return decode(s) * int(num)

  # 1..3 = [1,2,3]
  if ".." in s:  
    a,b = s.split("..",1)
    if isnumeric(a) and isnumeric(b):
      return range(int(a),int(b)+1)

  # a single value
  return [s] 

def encode( v ):
  """ Compress a value v: if v is an array, try using ... or * to shorten ranges. """

  def strfy( x ):
    if isnumeric( x ) or type(x) != str:
      return str(x)
    elif reduce( lambda x,y: x or y, map( lambda y: y in x, """ []'",""" ), False ):
      # if some characters given are present, quote the string
      if "'" not in x:
        return "'%s'" % (x,)
      elif '"' not in x:
        return '"%s"' % (x,)
      else:
        # both single and double quotes: use single quotes and escape
        # existing single quotes with '"'"'.
        return "'%s'" % (str.replace( "'", """'"'"'""", x ))
    else:
      return x

  if type(v) != list:
    # can only compress lists
    return strfy(v)

  if len(v) < 3:
    # length 0, 1 or 2 is not worth analysing
    return "[%s]" % (",".join(map( strfy, v )),)

  i = 0
  dst = []
  while i < len(v):
    # make sure each value is processed only once
    written = False

    # compress equal values
    for j in xrange(i,len(v)+1):
      if j == len(v) or str(v[j]) != str(v[i]):
        if j-i-1 > 1:
          dst.append( "%s*%s" % (j-i,strfy(v[i])) )
          i = j
          written = True
        break
    
    if written:
      continue

    # compress ranges
    for j in xrange(i,len(v)+1):
      if j == len(v) or not isnumeric(v[j]) or float(v[j]) != float(v[i])+j-i:
        if j-i-1 > 2:
          dst.append( "%s..%s" % (str(v[i]),str(v[i]+j-i-1) ) )
          i = j
          written = True
        break

    if written:
      continue

    dst.append( strfy(v[i]) )
    i += 1

  return "[%s]" % (",".join(dst),)


class Parset(dict):
    def __init__(self, defaults = dict() ):
      self.update ( defaults.copy() )

    def readFile(self, filename):
      """ Read a parset to disk, and merge it with the current settings. """

      f = ropen( filename, "r" )

      lexer = shlex.shlex( f, filename )
      basic_wordchars = lexer.wordchars + ".:+-!@$%^&*/{}"
      key_wordchars   = basic_wordchars + "[]()"
      value_wordchars = basic_wordchars + "="
      token = lexer.get_token

      errormsg = lambda t: "%s%s"% (lexer.error_leader(),t)

      def peek():
        t = token()
        if not t: return t

        lexer.push_token(t)
        return t

      lexer.wordchars = key_wordchars
      for t in lexer:
        # read a KEY = VALUE pair
        key = t
        assert token() == "=",errormsg(t)

        # read the value
        lexer.wordchars = value_wordchars
        t = token()
        if t == "[":
          # read array
          value = []

          if peek() != "]":
            # non-empty array

            # accumulate tokens as a single value,
            # and keep track of [] and () pairs as
            # part of a value
            cur = ""
            parentheses, brackets = 0, 0
            for t in lexer:
              if t == "(":
                assert parentheses == 0, errormsg("nested parentheses not supported")
                parentheses += 1
              elif t == ")":
                assert parentheses > 0, errormsg("unmatched parentheses")
                parentheses -= 1
              if t == "[":
                assert parentheses == 0, errormsg("cannot put brackets inside parentheses")
                brackets += 1
              elif t == "]":
                if not brackets:
                  # end of array
                  value.extend(decode(cur))
                  break

                brackets -= 1
              elif t == ",":
                if not brackets and not parentheses:
                  value.extend(decode(cur))
                  cur = ""
                  continue

              cur += t
            else:
              assert False,errormsg("unterminated array")

        else:
          # t was no array
          value = t

        # store the key,value pair
        self[key] = value

        lexer.wordchars = key_wordchars

    def writeFile(self, filename):
      """ Write the parset to disk. """

      outf = ropen(filename, 'w')

      # construct strings from key,value pairs
      lines = ["%s = %s" % (str(k),encode(v)) for k,v in self.iteritems()]

      # sort them for easy lookup
      lines.sort()

      # write them to file
      outf.write( "\n".join( lines ) )

    def __delitem__(self, key):
      # avoid KeyErrors
      if key in self: dict.__delitem__(self,key)

    def getBool(self, key):
      return self[key] in ["T","t","1","Y","y",1,True]

    def getString(self, key):
      return str(self[key])

    def getInt32(self, key):
      return int(self[key])

    def getFloat(self, key):
      return float(self[key])

    def getStringVector(self, key):
      return map(str,self[key])

    def getInt32Vector(self, key):
      return map(int,self[key])

    def getFloatVector(self, key):
      return map(float,self[key])
