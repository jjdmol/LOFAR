pragma include once
include 'app_proxy.g'

solver := function (appid='Solver',
    server='./applauncher',options="-d0 -meq:M:M:Solver",
    suspend=F,
    verbose=1)
{
  self := [=];
  public := [=];
  # init app proxy
  ret := define_app_proxy(self,public,appid,server,options,suspend,verbose);
  if( is_fail(ret) )
    fail;
  # define solver-specific methods
  const public.solve := function (solverec)
  {
    wider public;
    public.command("Solver.Add.Solution",solverec);
  }
  const public.endsolve := function (endrec=[=])
  {
    wider public;
    public.command("Solver.End.Solution",endrec);
  }
  const public.nextdomain := function ()
  {
    wider public;
    public.command("Solver.Next.Domain");
  }
  return ref public;
}

