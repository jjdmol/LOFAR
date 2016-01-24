import SimpleXMLRPCServer
s = SimpleXMLRPCServer.SimpleXMLRPCServer(('10.178.1.2', 2009))
def boo():
  return "hello"
s.register_introspection_functions()
s.register_function(boo, 'boo')
s.serve_forever()
