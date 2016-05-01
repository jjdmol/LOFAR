

class ParameterSet(object):
    def __init__(self, data=None):
        self.parset = {}
        if data:
            if type(data) == dict:
                self.parset = data
            elif type(data) == str:
                self.import_string(data)

    def clear(self):
        self.parset = {}

    def get_set(self):
        return self.parset

    def import_file(self, filename):
        fd = open(filename, 'r')
        data = fd.readlines()
        fd.close()
        self.import_string(data)

    def import_string(self, data):
        if isinstance(data, basestring):
            data = data.split('\n')

        for line in data:
            if line.strip() == '' or line.strip()[0] == '#':
                continue
            ps = self.parset
            #key, value = line.strip().split('=')
            items = [x.strip() for x in line.strip().split('=')]
            if len(items) != 2:
                continue
            key, value = items[0], items[1]
            key_list = key.strip().split('.')
            #print key_list, key_list[-1]

            last_key = key_list[-1]
            for k in key_list[:-1]:
                if k in ps:
                    ps = ps[k]
                else:
                    ps[k] = {}
                    ps = ps[k]
            ps[key_list[-1]] = value.strip()

    def replace(self, key, value):
        self.parset[key] = value

    def make_subset(self, subset):
        keys = subset.strip().split('.')
        ps = self.parset
        for key in keys:
            ps = ps[key]
        return ps

    def get(self, key, default_val=None):
        ps = self.parset
        keys = key.split('.')
        try:
            for k in keys:
                ps = ps[k]
        except:
            ps = default_val
        return ps


