pragma include once

# defines a set of standard debug methods inside an object
const define_debug_methods := function (ref self,ref public,initverbose=1)
{
  self.verbose := initverbose;
  # prints debug message if level is <= current verbosity level
  const public.dprint := function (level,...)
  {
    wider self;
    if( level <= self.verbose )
      print spaste('[== ',self.appid,' ==] ',...);
  }
  const public.dprintf := function (level,format,...)
  {
    wider self;
    if( level <= self.verbose )
      print spaste('[== ',self.appid,' ==] ',sprintf(format,...));
  }
  # private versions for convenience
  const self.dprint := function (level,...)
  {
    wider public;
    public.dprint(level,...);
  }
  const self.dprintf := function (level,format,...)
  {
    wider public;
    public.dprintf(level,format,...);
  }
  # sets the verbosity level for the dprint methods
  const public.setverbose := function (level)
  {
    wider self;
    self.verbose := level;
    return level;
  }
  return T;
}
