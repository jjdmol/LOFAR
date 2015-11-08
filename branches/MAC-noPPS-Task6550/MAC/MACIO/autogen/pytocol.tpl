[+ AutoGen5 template py +]
#!/usr/bin/env python
#coding: iso-8859-15
[+ (dne "#  ") +][+ (out-push-add "/dev/null") +]
[+ (out-pop) +]
#
#  [+ (base-name) +].[+ (suffix) +]: [+ description +]
#
#  Copyright (C) 2003-2015
#  ASTRON (Netherlands Foundation for Research in Astronomy)
#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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
[+ DEFINE protocolnr +][+ IF (*=* (get "id") "LOFAR::MACIO::") +][+ (string-substitute (get "id") '("LOFAR::MACIO::")' ("")) +][+ ELSE +][+ (get "id") +][+ ENDIF +][+ ENDDEF +]
[+ DEFINE lib_name +][+ IF (exist? "lib") +][+ (get "lib") +][+ ENDIF +][+ ENDDEF +]
[+ DEFINE lib_prefix +][+ IF (exist? "lib") +][+ (get "lib") +].[+ ENDIF +][+ ENDDEF +]
[+ DEFINE signal_name +][+ prefix_ucase +]_[+ (get "signal") +][+ ENDDEF +]
[+ DEFINE signal_id +][+ signal_name +]_ID[+ ENDDEF +]
[+ DEFINE cap_signal +][+ (string-substitute (string-capitalize! (get "signal")) '( "_" )' ( "" )) +][+ ENDDEF +]
[+ DEFINE event_class_name +][+ prefix_cap +][+ cap_signal +]Event[+ ENDDEF +]
[+ DEFINE event_class_decl +][+ event_class_name +](GCFEvent):[+ ENDDEF +]
[+ DEFINE protocol_name +][+ (string-upcase (base-name)) +][+ ENDDEF +]
[+ DEFINE event_class_member_type +][+ IF (*== (get "type") "]") +][+ (substring (get "type") 0 (string-index (get "type") #\[)) +][+ ELSE +][+ IF (*== (get "type") ">") +][+ (substring (get "type") (1+(string-index (get "type") #\<)) (string-index (get "type") #\>)) +][+ ELSE +][+ (get "type") +][+ ENDIF +][+ ENDIF +][+ ENDDEF +]
[+ DEFINE event_class_member +][+ event_class_member_type +][+ IF (and (*== (get "type") "]") (not (*== (get "type") "[]"))) +][+ (substring (get "type") (string-index (get "type") #\[) (string-length (get "type"))) +][+ ENDIF +][+ ENDDEF +]
[+ DEFINE event_param_init +][+ CASE (get "type") +][+ == string +]""[+ == char +]'\0'[+ ==* char +]""[+ *== int64 +]0L[+ *=* int +]0[+ == float +]0.0[+ == double +]0.0[+ == bool +]False[+ * +][+ lib_prefix +][+ (get "type") +]()[+ ESAC +][+ ENDDEF +]
[+ DEFINE event_param_format1 +][+ CASE (get "type") +][+ == string +]%s[+ == char +]%c[+ *== "]" +]%s[+ ==* vector +]%s[+ ==* map +]%s[+ *== int64 +]%ld[+ *=* int +]%d[+ == float +]%f[+ == double +]%f[+ == bool +]%r[+ * +]%s[+ ESAC +][+ ENDDEF +]
[+ DEFINE event_param_format2 +][+ CASE (get "type") +][+ ==* char +]c[+ ==* uint16 +]H[+ ==* int16 +]h[+ ==* uint32 +]I[+ ==* int32 +]i[+ ==* uint64 +]Q[+ ==* int64 +]q[+ ==* float +]f[+ ==* double +]d[+ == bool +]b[+ ESAC +][+ ENDDEF +]
[+ DEFINE event_param_size +][+ CASE (get "type") +][+ == string +]XXX[+ * +]struct.calcsize("=[+ event_param_format2 +]")[+ ESAC +][+ ENDDEF +]
[+ DEFINE array_size +][+ IF (*== (get "type") "]") +][+ (substring (get "type") (1+(string-index (get "type") #\[)) (string-index (get "type") #\])) +][+ ENDIF +][+ ENDDEF +]
[+ DEFINE array_type +][+ CASE (get "type") +][+ ==* float +]np.float32[+ ==* double +]np.float[+ ==* int +]np.[+ (substring (get "type") 0 (string-index (get "type") #\[)) +][+ ==* uint +]np.[+ (substring((get "type") 1)) +][+ ESAC +][+ ENDDEF +]

from macio import *
[+ (define array-import "import numpy as np") +][+ FOR event "" +][+ FOR param "" +][+ IF (*== (get "type") "]") +][+ (. array-import) +][+ (define array-import "") +][+ BREAK +][+ ENDIF +][+ ENDFOR param +][+ ENDFOR event +]
[+ IF (exist? "lib") +]import [+ lib_name +] as [+ lib_name +][+ ENDIF +]

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
    self.[+ (get "name") +] = [+ IF (and (*== (get "type") "]") (not (==* (get "type") "char["))) +]np.zeros([+ array_size +], [+ array_type +])[+ ELSE +][+ IF (and (*== (get "type") ">") (==* (get "type") "vector")) +][][+ ELSE +][+ IF (and (*== (get "type") ">") (==* (get "type") "map")) +]{}[+ ELSE +][+ event_param_init +][+ ENDIF +][+ ENDIF +][+ ENDIF +][+ ENDFOR param +]

  def __str__(self):
    return "{%s}[+ FOR param "," +][+ (get "name") +]=[+ event_param_format1 +][+ ENDFOR param +]" % (GCFEvent.__str__(self),[+ FOR param "," +] self.[+ (get "name") +][+ ENDFOR param +])

  def pack(self):[+ FOR param "" +]
    [+ IF (exist? "userdefined") +]offset = len(self.buffer)
    self.buffer += "".zfill([+ lib_prefix +]MSH_size(self.[+ (get "name") +]))
    [+ lib_prefix +]MSH_pack(self.buffer, offset, self.[+ (get "name") +])[+ ELSE +]self.buffer += packCdefinedVariable(self.[+ (get "name") +], "[+ (get "type") +]")[+ ENDIF +][+ ENDFOR param +]
    GCFEvent.pack(self)

  def unpack(self, somebuffer):
    offset = GCFEvent.unpack(self, somebuffer)[+ FOR param +]
    [+ IF (exist? "userdefined") +]self.[+ (get "name") +]=[+ lib_prefix +][+ (get "type") +]()
    offset = [+ lib_prefix +]MSH_unpack(self.buffer, offset, self.[+ (get "name") +])[+ ELSE +][+ CASE (get "type") +][+ == "string" +](self.[+ (get "name") +], vLen)=unpackString(self.buffer[offset:])[+ == bool +]self.[+ (get "name") +]=bool(struct.unpack("=[+ event_param_format2 +]", self.buffer[offset:offset+1])[0])[+ ==* "char[" +]self.[+ (get "name") +]=unpackArray(self.buffer[offset:], [+ event_param_size +], [+ array_size +])[+ ==* vector +](newValue, vLen)=unpackVector(self.buffer[offset:],"[+ event_class_member +]")
    self.[+ (get "name") +]=newValue[+ *== "]" +]self.[+ (get "name") +]=np.fromstring(unpackArray(self.buffer[offset:], [+ event_param_size +], [+ array_size +]), [+ array_type +])[+ ==* map +](newValue, vLen)=unpackMap(self.buffer[offset:],"[+ event_class_member +]")
    self.[+ (get "name") +]=newValue[+ * +]vLen=[+ event_param_size +]
    self.[+ (get "name") +]=struct.unpack("=[+ event_param_format2 +]", self.buffer[offset:offset+vLen])[0][+ ESAC +]
    offset += [+ CASE (get "type") +][+ *== "]" +][+ event_param_size +] * [+ array_size +][+ == bool +]1[+ * +]vLen[+ ESAC +][+ ENDIF +][+ ENDFOR param +]

[+ ENDFOR event +]

if __name__ == '__main__':
[+ FOR event "" +]  print "Testing [+ event_class_name +]..."
  obj[+ (for-index) +] = [+ event_class_name +]()[+ FOR param "" +]
  obj[+ (for-index "event") +].[+ (get "name") +]=[+ IF (exist? "testvalue") +][+ lib_prefix +][+ (get "type") +][+ (get "testvalue") +][+ ELSE +]testValue("[+ (get "type") +]")[+ ENDIF +][+ ENDFOR param +]
  print obj[+ (for-index "event") +]
  obj[+ (for-index "event") +].pack()
  print obj[+ (for-index "event") +]
  objX[+ (for-index "event") +] = [+ event_class_name +]()
  objX[+ (for-index "event") +].unpack(obj[+ (for-index "event") +].buffer)
  print objX[+ (for-index "event") +]

[+ ENDFOR event +]

