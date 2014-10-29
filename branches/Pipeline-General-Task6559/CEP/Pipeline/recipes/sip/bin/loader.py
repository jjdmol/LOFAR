import sys


def load_plugin(name, path=None):
    if path:
        sys.path.append(path)
    mod = __import__("PipelineStep_" + name)
    return mod


def call_plugin(path, name, *args, **kwargs):
    plugin = load_plugin(path, name)
    plugin.plugin_main(*args, **kwargs)

#call_plugin("example", 1234)