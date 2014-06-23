import unittest
from testconfig import config
from src.updater import _refactor_update

class UpdaterTest(unittest.TestCase):
    def test1(self):
        sql = """
update runningcatalog
   set value1 = t1.value1,
       value2 = t1.value2
  from table1 t1
 where t1.runcat_id = runningcatalog.runcatid
   and t1.condition = runningcatalog.condition;
"""
        result = _refactor_update(sql)
        self.assertEquals(len(result), 2)
        self.assertEquals(result[0].replace('\n', ''),
        'update runningcatalog set  value1  = (select  t1.value1 from runningcatalog x,  table1 t1  where  t1.runcat_id = x.runcatid   and t1.condition = runningcatalog.condition and runningcatalog.runcatid = x.runcatid)    where exists (select 1 from  table1 t1 where t1.runcat_id = runningcatalog.runcatid   and t1.condition = runningcatalog.condition);')
        self.assertEquals(result[1].replace('\n', ''),
        'update runningcatalog set        value2  = (select  t1.value2   from runningcatalog x,  table1 t1  where  t1.runcat_id = x.runcatid   and t1.condition = runningcatalog.condition and runningcatalog.runcatid = x.runcatid)    where exists (select 1 from  table1 t1 where t1.runcat_id = runningcatalog.runcatid   and t1.condition = runningcatalog.condition);')
