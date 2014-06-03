from config import Config
from listener import Listener

cfg = Config()
cfg.add_item('parentpid',1234)
lst = Listener(cfg)

lst.start()