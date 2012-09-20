class parameterset():
    def __init__(self, test):
        print "Muck parameterset, parameter retrieved:"
        print test
        self.function_calls = []

    def replace(self, key, value):
        self.function_calls.append(('replace', key, value))

    def writeFile(self, output):
        fp = open(output, 'w')
        fp.write(str(self.function_calls))
        fp.close()
        self.function_calls.append(('writeFile', output))
