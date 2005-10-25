# plot_ionophase.g: Template for simple plot-scripts

# Copyright (C) 1996,1997,1998
# Associated Universities, Inc. Washington DC, USA.
#
# This library is free software; you can redistribute it and/or modify it
# under the terms of the GNU Library General Public License as published by
# the Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.
#
# This library is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
# License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with this library; if not, write to the Free Software Foundation,
# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
#
# Correspondence concerning AIPS++ should be addressed as follows:
#        Internet email: aips2-request@nrao.edu.
#        Postal address: AIPS++ Project Office
#                        National Radio Astronomy Observatory
#                        520 Edgemont Road
#                        Charlottesville, VA 22903-2475 USA
#
# $Id$

#---------------------------------------------------------

# pragma include once
print '\n\n\n\n\n\n=================================================';
print 'include plot_ionophase.g    w21oct/h28oct/d28oct2002';

include 'unset.g';
include 'yaploi.g';
include 'inspect.g';
include 'menubar.g';

plot_ionophase := function () {
    private := [=];
    public := [=];
    print '\n\n********************************************************** plot_ionophase():';

    create_dypl(new=F);
    if (T) dypl.clear();

#-----------------------------------------------------------------
# Some helper functions:


#-----------------------------------------------------------------
# The generic parameter interface:

    private.gui := function () {
	wider private;
	private.frame := frame(title='plot_ionophase');
	whenever private.frame->killed do private.dismiss();
	private.mbr := menubar(private.frame);
	plot_menu := private.mbr.makemenu('plot'); 
	private.item_xxx(plot_menu);
	# private.item_yyy(plot_menu);
	defrec := private.mbr.defrecinit('plot_ionophase.g', menu='reinclude');
	private.mbr.makemenuitem(defrec, private.reinclude);
	return T;
    }

# Re-include the present .g file (e.g. after modification):

    private.reinclude := function (trace=T) {
	wider private;
	if (trace) print '** private.reinclude():';
	private.dismiss();
	include 'plot_ionophase.g';
	return T;
    }

    private.dismiss := function (trace=T) {
	wider private;
	if (trace) print '** private.dismiss():';
	r := private.mbr.done();
	if (trace) print 'menubar.done() ->',r;
	private.frame := F;
	return T;
    }

#----------------------------------------------------------------
# The specific parameter interface:

    private.item_xxx := function (menu='plot') {
	name := 'plot_xxx';
	defrec := private.mbr.defrecinit(name, menu=menu, killonOK=F);
	defrec.prompt := paste('prompt for',name);
	defrec.help := paste('help for',name);
	private.mbr.inarg (defrec, 'lambda_m', [4,1,2,3],
		   help='observing wavelength');
	private.mbr.inarg (defrec, 'shift_deg', [0.5,0.1,-0.2],
		   help='field shift');
	private.inarg_plotctrl(defrec);            # clear/mosaic/etc
	private.mbr.makemenuitem(defrec, private[name]);
	return T;
    }

    private.item_yyy := function (menu='plot') {
	name := 'plot_yyy';
	defrec := private.mbr.defrecinit(name, menu=menu, killonOK=F);
	defrec.prompt := paste('prompt for',name);
	defrec.help := paste('help for',name);
	private.inarg_plotctrl(defrec);            # clear/mosaic/etc
	private.mbr.makemenuitem(defrec, private[name]);
	return T;
    }

# Some standard plot-control arguments (clear, mosaic, etc):

    private.inarg_plotctrl := function (ref defrec=F) {
	private.mbr.inarg (defrec, 'clear', tf=F,
		   help='if T, clear the plotter first');
	private.mbr.inarg (defrec, 'mosaic', tf=T,
		   help='if T, plot a mosaic of all plots');
	return T;
    }

#-----------------------------------------------------------------
# The specific plotting function itself:

    private.plot_xxx := function (pp=F, trace=T) {
	if (trace) print 'private.plot_xxx(): pp=\n',pp;
	if (pp.clear) dypl.clear();     # clear the plotter
	dypl.begin('plot_xxx');

	# Some specific calculations:
	shift_rad := pp.shift_deg * (pi/180);
	xx_km := [[0:300]/100,[4:100]];
	dd_m := xx_km * 1000 * shift_rad;
	yy_rad := (dd_m/pp.lambda_m) * 2 * pi;
	q := 100;
	yy_rad_2nd := (yy_rad[2]/xx_km[2]) * q * sin(xx_km/q);
	sinc := sin(yy_rad)/yy_rad;
	sinc[1] := 1;
	sinc *:= 30;
	
	# Plot the data
	dypl.plotxy(xx_km, yy_rad, label='gradient', labelpos='right');
	cc := dypl.get_dataset();         # last dataset
	# inspect(cc,'cc');
	dypl.axes();                      # plot (x,y) axes
	xy := dypl.datamark (x=0.1, cc=cc, ylabel='station', xproj=T);
	xy := dypl.datamark (x=2, cc=cc, ylabel='core', xproj=T);

	dypl.plotxy(xx_km, yy_rad_2nd, style='dashed', label='2nd order', labelpos='right');
	dypl.plotxy(xx_km, sinc);
	
	# Labels:
	title := paste('ionospheric phase');
	xlabel := 'baseline (km)';
	ylabel := 'phase (rad)';
	dypl.labels(title, xlabel, ylabel);
	
	# Legend:
	dypl.legend(refpos='tlc', charsize=2);
	dypl.legend(paste('observing wavelength =',pp.lambda_m,'m'));
	dypl.legend(paste('field shift =',pp.shift_deg,'deg'));
	
	# Finished:
	dypl.end();
	if (pp.mosaic) dypl.mosaic();  
	return T;
    }

#-----------------------------------------------------------------
# Execute:

    private.gui();
    return public;
} 

#************************************
# Run the script:
plot_ionophase();
#************************************



