c  main program
      common /cstak/ ds
      double precision ds(1000)
      external handle, bc, af, postd
      integer ndx, k, is(1000), nu, nv, nmesh
      real errpar(2), u(100), v(1), mesh(100), dt, rs(1000)
      real ws(1000), tstop
      logical ls(1000)
      complex cs(500)
      equivalence (ds(1), cs(1), ws(1), rs(1), is(1), ls(1))
c to test  post on
c      u sub t = u sub xx + f      on (0,1)
c where f is chosen so that the solution is
c      u(x,t) = exp(xt).
c the port library stack and its aliases.
c initialize the port library stack length.
      call istkin(1000, 4)
      nu = 1
      nv = 0
      errpar(1) = 0
c absolute error.
      errpar(2) = 1e-2
      tstop = 1
      dt = 1e-2
      k = 4
c ndx uniform mesh points on (0,1).
      ndx = 4
      call umb(0e0, 1e0, ndx, k, mesh, nmesh)
c initial conditions for u.
      call setr(nmesh-k, 1e0, u)
      call post(u, nu, k, mesh, nmesh, v, nv, 0e0, tstop, dt, af, bc, 
     1   postd, errpar, handle)
c check for errors and stack usage statistics.
      call wrapup
      stop 
      end
      subroutine af(t, x, nx, u, ux, ut, utx, nu, v, vt, nv, a, 
     1   au, aux, aut, autx, av, avt, f, fu, fux, fut, futx, fv, fvt)
      integer nu, nx
      integer nv
      real t, x(nx), u(nx, nu), ux(nx, nu), ut(nx, nu), utx(nx, nu)
      real v(1), vt(1), a(nx, nu), au(nx, nu, nu), aux(nx, nu, nu), aut(
     1   nx, nu, nu)
      real autx(nx, nu, nu), av(1), avt(1), f(nx, nu), fu(nx, nu, nu), 
     1   fux(nx, nu, nu)
      real fut(nx, nu, nu), futx(nx, nu, nu), fv(1), fvt(1)
      integer i
      real exp
      do  1 i = 1, nx
         a(i, 1) = -ux(i, 1)
         aux(i, 1, 1) = -1
         f(i, 1) = (x(i)-t**2)*exp(x(i)*t)-ut(i, 1)
         fut(i, 1, 1) = -1
   1     continue
      return
      end
      subroutine bc(t, l, r, u, ux, ut, utx, nu, v, vt, nv, b, bu,
     1   bux, but, butx, bv, bvt)
      integer nu
      integer nv
      real t, l, r, u(nu, 2), ux(nu, 2), ut(nu, 2)
      real utx(nu, 2), v(1), vt(1), b(nu, 2), bu(nu, nu, 2), bux(nu, nu,
     1   2)
      real but(nu, nu, 2), butx(nu, nu, 2), bv(1), bvt(1)
      real exp
      b(1, 1) = u(1, 1)-1.
      b(1, 2) = u(1, 2)-exp(t)
      bu(1, 1, 1) = 1
      bu(1, 1, 2) = 1
      return
      end
      subroutine handle(t0, u0, v0, t, u, v, nu, nxmk, nv, k, x, 
     1   nx, dt, tstop)
      integer nxmk, nu, nx
      integer nv, k
      real t0, u0(nxmk, nu), v0(1), t, u(nxmk, nu), v(1)
      real x(nx), dt, tstop
      common /time/ tt
      real tt
      external uofx
      integer i1mach
      real eu, eesff
      integer temp
c output and checking routine.
      if (t0 .eq. t) return
c uofx needs time.
      tt = t
      eu = eesff(k, x, nx, u, uofx)
      temp = i1mach(2)
      write (temp,  1) t, eu
   1  format (14h error in u(x,, 1pe10.2, 4h ) =, 1pe10.2)
      return
      end
      subroutine uofx(x, nx, u, w)
      integer nx
      real x(nx), u(nx), w(nx)
      common /time/ t
      real t
      integer i
      real exp
      do  1 i = 1, nx
         u(i) = exp(x(i)*t)
   1     continue
      return
      end
