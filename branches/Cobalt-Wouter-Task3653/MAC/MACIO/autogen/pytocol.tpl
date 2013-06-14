[+ AutoGen5 template py +]
#!/usr/bin/env python
#coding: iso-8859-15
[+ (dne "#  ") +][+ (out-push-add "/dev/null") +]
[+ (out-pop) +]
#
#  [+ (base-name) +].[+ (suffix) +]: [+ description +]
#
#  Copyright (C) 2003-2013
#  ASTRON (Netherlands Foundation for Research in Astronomy)
#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
#
[+ IF (== autogen-version "5.12") +]
#  [+ (lgpl "This program" "ASTRON" "#  ") +]
[+ ELSE "5.10 is fine, 5.12 forgets the 1st // (build err) and 5.12 and 5.16 append 2 chars (don't care)." +]
[+ (lgpl "This program" "ASTRON" "#  ") +]
[+ ENDIF +]
#
#  $Id: pytocol.tpl 23417 2012-12-20 14:06:29Z loose $
#
[+ DEFINE prefix_cap +][+ IF (exist? "prefix") +][+ (get "prefix") +][+ ENDIF +][+ ENDDEF +]
[+ DEFINE prefix_ucase +][+ IF (exist? "prefix") +][+ (string-upcase (get "prefix")) +][+ ENDIF +][+ ENDDEF +]
[+ DEFINE protocol_id +][+ IF (exist? "id") +][+ (get "id") +][+ ENDIF +][+ ENDDEF +]
[+ DEFINE signal_name +][+ prefix_ucase +]_[+ (get "signal") +][+ ENDDEF +]
[+ DEFINE signal_id +][+ signal_name +]_ID[+ ENDDEF +]
[+ DEFINE cap_signal +][+ (string-substitute (string-capitalize! (get "signal")) '( "_" )' ( "" )) +][+ ENDDEF +]
[+ DEFINE event_class_name +][+ prefix_cap +][+ cap_signal +]Event[+ ENDDEF +]
[+ DEFINE event_class_decl +][+ event_class_name +](GCFEvent):[+ ENDDEF +]
[+ DEFINE protocol_name +][+ (string-upcase (base-name)) +][+ ENDDEF +]
[+ DEFINE event_class_member_type +][+ IF (*== (get "type") "]") +][+ (substring (get "type") 0 (string-index (get "type") #\[)) +][+ ELSE +][+ (get "type") +][+ ENDIF +][+ ENDDEF +]
[+ DEFINE event_class_member +][+ event_class_member_type +][+ IF (*== (get "type") "[]") +]*[+ ENDIF +] [+ (get "name") +][+ IF (and (*== (get "type") "]") (not (*== (get "type") "[]"))) +][+ (substring (get "type") (string-index (get "type") #\[) (string-length (get "type"))) +][+ ENDIF +][+ ENDDEF +]
[+ DEFINE event_param_init +][+ CASE (get "type") +][+ == string +]""[+ == char +]'\0'[+ ==* char +]""[+ *== int64 +]0L[+ *=* int +]0[+ == float +]0.0[+ == double +]0.0[+ == bool +]False[+ ESAC +][+ ENDDEF +]
[+ DEFINE event_param_format1 +][+ CASE (get "type") +][+ == string +]%s[+ == char +]%c[+ *== "]" +]%s[+ *== int64 +]%ld[+ *=* int +]%d[+ == float +]%f[+ == double +]%f[+ == bool +]%r[+ ESAC +][+ ENDDEF +]
[+ DEFINE event_param_format2 +][+ CASE (get "type") +][+ ==* char +]c[+ ==* uint16 +]H[+ ==* int16 +]h[+ ==* uint32 +]I[+ ==* int32 +]i[+ ==* uint64 +]Q[+ ==* int64 +]q[+ ==* float +]f[+ ==* double +]d[+ == bool +]b[+ ESAC +][+ ENDDEF +]
[+ DEFINE event_param_size +][+ CASE (get "type") +][+ == string +]XXX[+ * +]struct.calcsize("=[+ event_param_format2 +]")[+ ESAC +][+ ENDDEF +]
[+ DEFINE protocolnr +][+ IF (*=* (get "id") "LOFAR::MACIO::") +][+ (string-substitute (get "id") '("LOFAR::MACIO::")' ("")) +][+ ELSE +][+ (get "id") +][+ ENDIF +][+ ENDDEF +]
[+ DEFINE array_size +][+ IF (*== (get "type") "]") +][+ (substring (get "type") (1+(string-index (get "type") #\[)) (string-index (get "type") #\])) +][+ ENDIF +][+ ENDDEF +]
[+ DEFINE array_type +][+ CASE (get "type") +][+ ==* float +]Float32[+ ==* double +]Float[+ ==* int +][+ (string-capitalize (substring (get "type") 0 (string-index (get "type") #\[))) +][+ ==* uint +][+ (string-capitalize (substring((get "type") 1))) +][+ ESAC +][+ ENDDEF +]

from MACIO import *
[+ (define array-import "from Numeric import *") +][+ FOR event "" +][+ FOR param "" +][+ IF (*== (get "type") "]") +][+ (. array-import) +][+ (define array-import "") +][+ BREAK +][+ ENDIF +][+ ENDFOR param +][+ ENDFOR event +]

#
# Define protocol ID
#
[+ protocol_name +] = [+ protocolnr +]

#
# Define error numbers and names
#
[+ FOR error "" +][+ prefix_ucase +]_[+ (get "id") +]_ERR = F_ERROR([+ protocol_name +],[+ (for-index) +])
[+ ENDFOR error +]

#
# Define protocol message types
#
[+ FOR event "" +][+ signal_id +] = [+ (for-index) +]+1
[+ ENDFOR event +]

[+ FOR event "" +][+ prefix_ucase +]_[+ (get "signal") +] = F_SIGNAL([+ protocol_name +], [+ prefix_ucase +]_[+ (get "signal") +]_ID, F_[+ (get "dir")+])
[+ ENDFOR event +]

[+ protocol_name +]_signalnames = [
  "[+ protocol_name +]: invalid signal",[+ FOR event "," +]
  "[+ signal_name +]"[+ ENDFOR event +]
]

[+ protocol_name +]_errornames = [ [+ FOR error "," +]
  "[+ (get "msg") +]"[+ ENDFOR error +]
]

[+ FOR event "" +]
class [+ event_class_decl +]
  def __init__(self):
    GCFEvent.__init__(self, signal=[+ prefix_ucase +]_[+ (get "signal") +])[+ FOR param "" +]
    self.[+ (get "name") +] = [+ IF (and (*== (get "type") "]") (not (==* (get "type") "char["))) +]zeros([+ array_size +], [+ array_type +])[+ ELSE +][+ event_param_init +][+ ENDIF +][+ ENDFOR param +]

  def __str__(self):
    return "{%s}[+ FOR param "," +][+ (get "name") +]=[+ event_param_format1 +][+ ENDFOR param +]" % (GCFEvent.__str__(self),[+ FOR param "," +] self.[+ (get "name") +][+ ENDFOR param +])

  def pack(self): [+ FOR param "" +]
    self.buffer += [+ CASE (get "type") +][+ == "string" +]packString([+ *== "]" +]packArray([+ * +]struct.pack("=[+ event_param_format2 +]", [+ ESAC +]self.[+ (get "name") +][+ CASE (get "type") +][+ ==* "char[" +], [+ array_size +][+ *== "]" +].tostring()[+ ESAC +])[+ ENDFOR param +]
    GCFEvent.pack(self)

  def unpack(self, somebuffer):
    offset = GCFEvent.unpack(self, somebuffer)[+ FOR param +]
    [+ CASE (get "type") +][+ == "string" +](self.[+ (get "name") +], vLen)=unpackString(self.buffer[offset:])[+ == bool +]self.[+ (get "name") +]=bool(struct.unpack("=[+ event_param_format2 +]", self.buffer[offset:offset+1])[0])[+ ==* "char[" +]self.[+ (get "name") +]=unpackArray(self.buffer[offset:], [+ event_param_size +], [+ array_size +])[+ *== "]" +]self.[+ (get "name") +]=fromstring(unpackArray(self.buffer[offset:], [+ event_param_size +], [+ array_size +]), [+ array_type +])[+ * +]vLen=[+ event_param_size +]
    self.[+ (get "name") +]=struct.unpack("=[+ event_param_format2 +]", self.buffer[offset:offset+vLen])[0][+ ESAC +]
    offset += [+ CASE (get "type") +][+ *== "]" +][+ event_param_size +] * [+ array_size +][+ == bool +]1[+ * +]vLen[+ ESAC +][+ ENDFOR param +]

[+ ENDFOR event +]

if __name__ == '__main__':
[+ FOR event "" +]  print "Testing [+ event_class_name +]..."
  obj[+ (for-index) +] = [+ event_class_name +]()
  print obj[+ (for-index) +]
  obj[+ (for-index) +].pack()
  print obj[+ (for-index) +]
  objX[+ (for-index) +] = [+ event_class_name +]()
  objX[+ (for-index) +].unpack(obj[+ (for-index) +].buffer)
  print objX[+ (for-index) +]

[+ ENDFOR event +]

