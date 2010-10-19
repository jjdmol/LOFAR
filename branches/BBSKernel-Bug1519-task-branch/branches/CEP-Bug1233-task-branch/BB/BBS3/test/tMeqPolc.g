f11:=function(nx,ny,sx,ex,sy,ey)
{
    return 2.;
}

f21:=function(nx,ny,sx,ex,sy,ey)
{
    dx:=(ex-sx)/nx;
    dy:=(ey-sy)/ny;
    res:=array(0,nx*ny);
    y:=sy+dy/2;
    for (i in [1:ny]) {
	x:=sx+dx/2;
	for (j in [1:nx]) {
	    res[j+(i-1)*nx] := 2 + 2*x;
	    x +:= dx;
	}
	y +:= dy;
    }
    return res;
}

f12:=function(nx,ny,sx,ex,sy,ey)
{
    dx:=(ex-sx)/nx;
    dy:=(ey-sy)/ny;
    res:=array(0,nx*ny);
    y:=sy+dy/2;
    for (i in [1:ny]) {
	x:=sx+dx/2;
	for (j in [1:nx]) {
	    res[j+(i-1)*nx] := 2 + 2*y;
	    x +:= dx;
	}
	y +:= dy;
    }
    return res;
}

f33:=function(nx,ny,sx,ex,sy,ey)
{
    dx:=(ex-sx)/nx;
    dy:=(ey-sy)/ny;
    res:=array(0,nx*ny);
    y:=sy+dy/2;
    for (i in [1:ny]) {
	x:=sx+dx/2;
	for (j in [1:nx]) {
	    res[j+(i-1)*nx] := 2 + 2*x + 2*x*x + 2*y + 2*y*x + 2*y*x*x + 2*y*y + 2*y*y*x + 2*y*y*x*x;
	    x +:= dx;
	}
	y +:= dy;
    }
    return res;
}

f34:=function(nx,ny,sx,ex,sy,ey)
{
    dx:=(ex-sx)/nx;
    dy:=(ey-sy)/ny;
    res:=array(0,nx*ny);
    y:=sy+dy/2;
    for (i in [1:ny]) {
	x:=sx+dx/2;
	for (j in [1:nx]) {
	    res[j+(i-1)*nx] := 1.5 + 2.1*x - 0.3*x*x - 2*y + 1.45*y*x - 2.3*y*x*x + 0.34*y*y + 1.7*y*y*x + 5*y*y*x*x + 1*y*y*y + 0*y*y*y*x - 1*y*y*y*x*x;
	    x +:= dx;
	}
	y +:= dy;
    }
    return res;
}

f43:=function(nx,ny,sx,ex,sy,ey)
{
    dx:=(ex-sx)/nx;
    dy:=(ey-sy)/ny;
    res:=array(0,nx*ny);
    y:=sy+dy/2;
    for (i in [1:ny]) {
	x:=sx+dx/2;
	for (j in [1:nx]) {
	    res[j+(i-1)*nx] := 1.5 + 2.1*x - 0.3*x*x - 2*x*x*x + 1.45*y - 2.3*y*x + 0.34*y*x*x + 1.7*y*x*x*x + 5*y*y + 1*y*y*x + 0*y*y*x*x - 1*y*y*x*x*x;
	    x +:= dx; 
	}
	y +:= dy;
    }
    return res;
}

f26:=function(nx,ny,sx,ex,sy,ey)
{
    dx:=(ex-sx)/nx;
    dy:=(ey-sy)/ny;
    res:=array(0,nx*ny);
    y:=sy+dy/2;
    for (i in [1:ny]) {
	x:=sx+dx/2;
	for (j in [1:nx]) {
	    res[j+(i-1)*nx] := 1.5 + 2.1*x - 0.3*y - 2*y*x + 1.45*y*y - 2.3*y*y*x + 0.34*y*y*y + 1.7*y*y*y*x + 5*y*y*y*y + 1*y*y*y*y*x + 0*y*y*y*y*y - 1*y*y*y*y*y*x;
	    x +:= dx;
	}
	y +:= dy;
    }
    return res;
}

f62:=function(nx,ny,sx,ex,sy,ey)
{
    dx:=(ex-sx)/nx;
    dy:=(ey-sy)/ny;
    res:=array(0,nx*ny);
    y:=sy+dy/2;
    for (i in [1:ny]) {
	x:=sx+dx/2;
	for (j in [1:nx]) {
	    res[j+(i-1)*nx] := 1.5 + 2.1*x - 0.3*x*x - 2*x*x*x + 1.45*x*x*x*x - 2.3*x*x*x*x*x + 0.34*y + 1.7*x*y + 5*x*x*y + 1*x*x*x*y + 0*x*x*x*x*y - 1*x*x*x*x*x*y;
	    x +:= dx;
	}
	y +:= dy;
    }
    return res;
}

doit := function()
{
    print f11(1,1,0,1,0,1);
    print f11(2,2,0,1,0,1);
    print f11(8,1,0,1,0,1);
    print f11(1,1, -0.5,0.5,0.25,0.5);
    print f11(2,2, -0.5,0.5,0.25,0.5);
    print f11(8,1, -0.5,0.5,0.25,0.5);

    print f21(1,1,0,1,0,1);
    print f21(2,2,0,1,0,1);
    print f21(8,1,0,1,0,1);
    print f21(1,1, -0.5,0.5,0.25,0.5);
    print f21(2,2, -0.5,0.5,0.25,0.5);
    print f21(8,1, -0.5,0.5,0.25,0.5);

    print f12(1,1,0,1,0,1);
    print f12(2,2,0,1,0,1);
    print f12(8,1,0,1,0,1);
    print f12(1,1, -0.5,0.5,0.25,0.5);
    print f12(2,2, -0.5,0.5,0.25,0.5);
    print f12(8,1, -0.5,0.5,0.25,0.5);

    print f33(1,1,0,1,0,1);
    print f33(2,2,0,1,0,1);
    print f33(8,1,0,1,0,1);
    print f33(1,1, -0.5,0.5,0.25,0.5);
    print f33(2,2, -0.5,0.5,0.25,0.5);
    print f33(8,1, -0.5,0.5,0.25,0.5);

    print f34(1,1,0,1,0,1);
    print f34(2,2,0,1,0,1);
    print f34(8,1,0,1,0,1);
    print f34(1,1, -0.5,0.5,0.25,0.5);
    print f34(2,2, -0.5,0.5,0.25,0.5);
    print f34(8,1, -0.5,0.5,0.25,0.5);

    print f43(1,1,0,1,0,1);
    print f43(2,2,0,1,0,1);
    print f43(8,1,0,1,0,1);
    print f43(1,1, -0.5,0.5,0.25,0.5);
    print f43(2,2, -0.5,0.5,0.25,0.5);
    print f43(8,1, -0.5,0.5,0.25,0.5);

    print f62(1,1,0,1,0,1);
    print f62(2,2,0,1,0,1);
    print f62(8,1,0,1,0,1);
    print f62(1,1, -0.5,0.5,0.25,0.5);
    print f62(2,2, -0.5,0.5,0.25,0.5);
    print f62(8,1, -0.5,0.5,0.25,0.5);

    print f26(1,1,0,1,0,1);
    print f26(2,2,0,1,0,1);
    print f26(8,1,0,1,0,1);
    print f26(1,1, -0.5,0.5,0.25,0.5);
    print f26(2,2, -0.5,0.5,0.25,0.5);
    print f26(8,1, -0.5,0.5,0.25,0.5);
}

doit();
exit;
