# plot_MeqPeel.g: Plots to illustrate MeqPeel issues

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
print 'include plot_MeqPeel.g    h17may/d19may/h19may/d21may2003';

include 'plot_template.g';

plot_MeqPeel := function () {
    private := [=];
    public := [=];
    plot_common (private=private, public=public, includefile='plot_MeqPeel.g',
		 title='Plots to illustrate \n MeqPeeling issues');
    print '\n\n********************************************************** plot_MeqPeel():';

    private.init := function () {
	wider private;
	# private.yapi := yapi();
	private.gui();
	# Execute one of the plots (convenient during development):
	if (T) {
	    dypl.clear();
	    # public.plot('patch');
	    public.plot('phase_track');
	    # public.plot('TID');
	    # public.plot('fiduc');
	    # public.plot('SNR');
	    # public.plot('visib');
	    # public.plot('noise');
	    # public.plot('newnoise');
	    # public.plot('cosfac_N');
	    # public.plot('chisq');
	    # public.plot('chisq_noise');
	    # public.plot('chisq_contam');
	}
	return T;
    }

    
#================================================================================

    private.inarg_Bt := function (ref pp=F, BW_MHz=[1,4,10]) {
	private.yapi.inarg (pp,'inttime_sec', [1,10,100,1000,10000], vector=T,
			    help='integration time (sec)');
	private.yapi.inarg (pp,'BW_MHz', BW_MHz, vector=T,
			    help='observing bandwidth (MHz)');
	return T;
    }

    private.inarg_freq := function (ref pp=F) {
	private.yapi.inarg (pp,'freq_MHz', [100,10,20,40,80,160,320],
			    help='observing frequency (MHz)');
    }


#=======================================================================
# Various plots of quantities that depend on sqrt(Bt):

    private.plot.SNR := function (pp=T, trace=F) {
	funcname := 'plot.SNR()';
	#--------------------------------------------------------
	# Standard entry module for pp=controlled functions:
	yapi := private.yapi;                    # convenience
	pp := yapi.on_entry(pp, funcname, trace=trace);
	if (yapi.do_check(pp)) {
	    yapi.itsov (pp, prompt=T, help='overall function help', killonOK=F);
	    # private.inarg_Bt(pp, BW_MHz=[1,4,10]);
	    yapi.inarg (pp,'BW_MHz', [1,4,30], vector=T,
			help='observing bandwidth (MHz)');
	    yapi.inarg (pp,'flux_Jy', [40000,1], 
			help='source flux (Jy)');
	    yapi.inarg (pp,'sky_noise_kJy', [8000], 
			help='sky-noise');
	    yapi.inarg (pp,'nelem1', [100, 57, 81, 144, 2500], 
			help='nr of dipoles of 1st station');
	    yapi.inarg (pp,'nelem2', [100, 57, 81, 144, 2500], 
			help='nr of dipoles of 2nd station');
	    private.inarg_plotcontrol(pp, funcname, legend='tlc', 
				      margin=[0.2, 0.1, 0.0, 0.0]);
	}
	if (!yapi.do_execute(pp)) return pp;
	#--------------------------------------------------------
	rr := private.on_entry (pp, funcname);

	# Make S/N plots for various bandwidths:
	q := pp.flux_Jy/(pp.sky_noise_kJy*1000);
	for (n in pp.nelem1) {
	    private.SNR_vs_inttime (q=n*q, BW_MHz=pp.BW_MHz);
	}

	# Legend and labels:
	private.pp2legend (pp, exclude="inttime_sec BW_MHz", trace=F);
	dypl.labels(title=funcname);
	return private.on_exit(pp, rr);
    }

# Make S/N plot as a function of integration time:

    private.SNR_vs_inttime := function (q=1.0, label=F, BW_MHz=[4.0], 
					color=F, style='solid', size=1,
					tt_sec=[1,10,100,1000,10000]) {
	colors := dypl.bw_colors (n=len(BW_MHz));
	for (i in ind(BW_MHz)) {
	    qq := q * sqrt(tt_sec*BW_MHz[i]*1e6);
	    s := spaste('BW=',BW_MHz[i],'MHz');
	    if (is_string(label)) s := label;
	    c := colors[i];
	    if (is_string(color)) c := color;
	    r := dypl.plotxy (log(tt_sec), log(qq), label=s, labelpos='right',
			      color=c, style=style, size=size);
	    if (is_fail(r)) print r;
	}
	dypl.labels(xlabel='integration time (s)', ylabel='S/N', axis='xylog');
	dypl.margin(rmargin=0.2);
	return T;
    }

#=======================================================================
# Receptor noise:

    private.plot.newnoise := function (pp=T, trace=F) {
	funcname := 'plot.newnoise()';
	#--------------------------------------------------------
	# Standard entry module for pp=controlled functions:
	yapi := private.yapi;                    # convenience
	pp := yapi.on_entry(pp, funcname, trace=trace);
	if (yapi.do_check(pp)) {
	    yapi.itsov (pp, prompt=T, help='overall function help', killonOK=F);
	    yapi.inarg(pp,'freq_MHz', [74,10,20,30, 40,80,160,320],  
		       help='observing freq');
	    yapi.inarg(pp,'bw_kHz', [4000,1.0,5], 
		       help='total bandwidth (kHz)');
	    yapi.inarg(pp,'tau_sec', [1000,10,1.0,60,240,3600,36000,360000], hide=T,
	     	       help='total integration time');
	    yapi.inarg(pp,'Tsky_index', [-2.6,-2.7], 
		       help='Tsky spectral index');
	    yapi.inarg(pp,'SNR', [3.0, 1.0], 
		       help='required S/N per chi-squared');
	    private.inarg_plotcontrol(pp, funcname, legend='blc',
				      margin=[0.0, 0.3, 0.0, 0.0]);
	}
	if (!yapi.do_execute(pp)) return pp;
	#--------------------------------------------------------
	rr := private.on_entry (pp, funcname);

	tt_sec := [0.1,1000];
	nelem1 := [2500,144,81,57,25,10];
	colors := dypl.bw_colors (n=len(nelem1));
	for (j in ind(nelem1)) {
	    rr.nifr := 100;               # one station with all others
	    rr.nelem1 := nelem1[j];
	    rr.nelem2 := rr.nelem1;
	    if (rr.nelem1>1000) {         # core
		rr.nelem2 := 100;
		rr.nifr := 1;
	    } else if (rr.nelem1<100) {   # inner stations (core ant extended core)
		rr.nifr := 100;           # one station with all(?) others
	    } else {                      # remote stations
		rr.nifr := 10;            # one station with 10 close ones
	    }
	    # print '\n',j,rr.nelem1,rr.nelem2;
	    for (i in ind(tt_sec)) {
		rr.tau_sec := tt_sec[i];
		rr := private.noise(rr);
		yy[i] := pp.SNR*rr.Schisq_Jy;
		# print '-',i,rr.tau_sec,'->',yy[i];
	    }
	    dypl.plotxy (log(tt_sec), log(yy), color=colors[j],
			 label=rr.selem, labelpos='right');
	}

	# Indicate the 'old' (Station-Core) requirement:
	if (T) {
	    color := 'magenta';
	    color := colors[1];
	    rr.tau_sec := 10;                                 # 10 s
	    rr.nifr := 1;                                     # individial ifr
	    rr.nelem1 := 2500;                                # core
	    rr.nelem2 := 100;                                 # station
	    rr := private.noise(rr);
	    s := '   old Station-Core requirement: S/N>3 in 10s (per vis)';
	    dypl.plotxy (log(rr.tau_sec), log(pp.SNR*rr.Nvis_Jy), 
			 style='star', color=color, size=5,
			 label=s, labelpos='right');
	    s := paste('old Station-Core: 2500 x 100 (nifr=1, corr=1)');
	    dypl.legend(s, color=color);
	    dypl.plotxy (log([10,10]), color=color, size=1, style='dashed');
	}

	# Indicate the VLA situation:
	if (rr.freq_MHz==74) {
	    color := 'blue';
	    rr.bw_kHz := 1000                             # 10 s
	    rr.nifr := 27;                                # 
	    rr.nelem1 := 25;                              # 100 m2
	    rr.nelem2 := 25;                              # 100 m2
	    tt_sec := [10.0,100];
	    for (i in ind(tt_sec)) {
		rr.tau_sec := tt_sec[i];
		rr := private.noise(rr);
		yy[i] := rr.Schisq_Jy;
	    }
	    s := spaste('VLA 74 MHz: bw=1MHz (nifr=',rr.nifr,' corr=4)');
	    dypl.plotxy (log(tt_sec), log(pp.SNR*yy), color=color, size=5,
			 label=s, labelpos='right');
	    s := paste('VLA antenna @100 m2 = 25 LOFAR dipoles @4m2');
	    dypl.legend(s, color=color);
	}

	# Legend and labels:
	dypl.legend (rr.legend, color='default');
	private.pp2legend (pp, exclude="tau_sec", clear=F);
	s := 'noise bias: divide by pi (Jy)';
	# dypl.legend(s,color='blue');
	dypl.labels(title='required minimum calibrator flux for LOFAR',
		    xlabel='integration time (s)', axis='xylog',
		    ylabel='required minimum flux (Jy)');
	return private.on_exit(pp, rr);
    }

#=======================================================================
# Receptor noise:

    private.plot.noise := function (pp=T, trace=F) {
	funcname := 'plot.noise()';
	#--------------------------------------------------------
	# Standard entry module for pp=controlled functions:
	yapi := private.yapi;                    # convenience
	pp := yapi.on_entry(pp, funcname, trace=trace);
	if (yapi.do_check(pp)) {
	    yapi.itsov (pp, prompt=T, help='overall function help', killonOK=F);
	    yapi.inarg(pp,'freq_MHz', [74,10,20,40,80,160,320],  
		       help='observing freq');
	    yapi.inarg(pp,'bw_kHz', [4000,1.0,5], 
		       help='total bandwidth (kHz)');
	    yapi.inarg(pp,'tau_sec', [1000,10,1.0,60,240,3600,36000,360000], hide=T,
	     	       help='total integration time');
	    yapi.inarg(pp,'Tsky_index', [-2.6,-2.7], 
		       help='Tsky spectral index');
	    yapi.inarg(pp,'useall_ifrs', tf=T, 
		       help='if T, use all available ifrs');
	    yapi.inarg(pp,'SNR', [3.0, 1.0], 
		       help='required S/N per chi-squared');
	    private.inarg_plotcontrol(pp, funcname, legend='blc',
				      margin=[0.0, 0.3, 0.0, 0.0]);
	}
	if (!yapi.do_execute(pp)) return pp;
	#--------------------------------------------------------
	rr := private.on_entry (pp, funcname);

	tt_sec := [0.1,1000];
	nelem1 := [2500,144,81,57];
	colors := dypl.bw_colors (n=len(nelem1));
	for (j in ind(nelem1)) {
	    rr.nelem1 := nelem1[j];
	    rr.nelem2 := rr.nelem1;
	    if (rr.nelem1>1000) {                        # core
		rr.nelem2 := 100;
		rr.nifr := 1;                            # ...?
		if (pp.useall_ifrs) rr.nifr := 100; 
	    } else if (rr.nelem1<100) {                  # inner stations
		rr.nifr := rr.nelem1;                    # ...?
		if (pp.useall_ifrs) rr.nifr := ceil(0.5*rr.nelem1*rr.nelem1);
	    } else {                                     # remote stations
		rr.nifr := 10;
		if (pp.useall_ifrs) rr.nifr := 100; 
	    }
	    # print '\n',j,rr.nelem1,rr.nelem2;
	    for (i in ind(tt_sec)) {
		rr.tau_sec := tt_sec[i];
		rr := private.noise(rr);
		yy[i] := pp.SNR*rr.Schisq_Jy;
		# print '-',i,rr.tau_sec,'->',yy[i];
	    }
	    dypl.plotxy (log(tt_sec), log(yy), color=colors[j],
			 label=rr.selem, labelpos='right');
	}

	# Indicate the 'old' (Station-Core) requirement:
	color := 'magenta';
	color := colors[1];
	rr.tau_sec := 10;                                 # 10 s
	rr.nifr := 1;                                     # individial ifr
	rr.nelem1 := 2500;                                # core
	rr.nelem2 := 100;                                 # station
	rr := private.noise(rr);
	s := '   old Station-Core requirement: S/N>3 in 10s (per vis)';
	dypl.plotxy (log(rr.tau_sec), log(pp.SNR*rr.Nvis_Jy), 
		     style='star', color=color, size=5,
		     label=s, labelpos='right');
	s := paste('old Station-Core: 2500 x 100 (nifr=1, corr=1)');
	dypl.legend(s, color=color);

	# Indicate the VLA situation:
	if (rr.freq_MHz==74) {
	    color := 'blue';
	    rr.bw_kHz := 1000                             # 10 s
	    rr.nifr := 27;                                # 
	    if (pp.useall_ifrs) rr.nifr := 351; 
	    rr.nelem1 := 25;                              # 100 m2
	    rr.nelem2 := 25;                              # 100 m2
	    tt_sec := [10.0,100];
	    for (i in ind(tt_sec)) {
		rr.tau_sec := tt_sec[i];
		rr := private.noise(rr);
		yy[i] := rr.Schisq_Jy;
	    }
	    s := spaste('VLA 74 MHz: bw=1MHz (nifr=',rr.nifr,' corr=4)');
	    dypl.plotxy (log(tt_sec), log(pp.SNR*yy), color=color, size=5,
			 label=s, labelpos='right');
	    s := paste('VLA antenna @100 m2 = 25 LOFAR dipoles @4m2');
	    dypl.legend(s, color=color);
	}

	# Legend and labels:
	dypl.legend (rr.legend, color='default');
	private.pp2legend (pp, exclude="tau_sec", clear=F);
	s := 'noise bias: divide by pi (Jy)';
	# dypl.legend(s,color='blue');
	dypl.labels(title='required minimum calibrator flux for LOFAR',
		    xlabel='integration time (s)', axis='xylog',
		    ylabel='required minimum flux (Jy)');
	return private.on_exit(pp, rr);
    }

# Do the actual noise calculations:

    private.noise := function (rr=F, trace=F) {
	if (!is_record(rr)) rr := [=];
	private.default (rr, 'freq_MHz', 100);
	private.default (rr, 'bw_kHz', 4000);
	private.default (rr, 'tau_sec', 10);
	private.default (rr, 'Tsky_index', -2.6);
	private.default (rr, 'Trec_K', 75);
	private.default (rr, 'nelem1', 100);
	private.default (rr, 'nelem2', 100);
	private.default (rr, 'nifr', 100);
	ss := "";                                  # legend-string

	rr.HB := (rr.freq_MHz>100);                # T if High Band (>100 MHz)
	rr.SC := (rr.nelem1>300 || rr.nelem2>300); # T if station-core

	rr.lambda_m := 300/rr.freq_MHz;
	rr.k_Boltzmann := 1.4*10e-23;              # (J/Hz.K)
	rr.k_Jy := rr.k_Boltzmann/10e-26;          # (Jy/Hz.K)

	# Calculate the Tsys of a dipole:
	if (F) {
	    # From JDB graph:
	    rr.Tsky := 'Tsky = 350*((160/freq_MHz)^2.7)';
	    rr.Tsky_K := 350*((160/rr.freq_MHz)^2.7);
	    rr.Trec_K := 0;
	} else {
	    # AGB definition (NB: Tsky_index=-2.6)!!!!!
	    rr.Tsky := spaste('Tsky = 50*((freq_MHz/300)^',rr.Tsky_index,') K');
	    ss := [ss,rr.Tsky];
	    rr.Tsky_K := 50*((rr.freq_MHz/300)^rr.Tsky_index);
	    rr.Trec_K := rr.Trec_K;      # ~ 75 K
	}
	for (fname in "Tsky_K Trec_K") {
	    rr[fname] := as_integer(rr[fname]);
	}
	rr.Tsys_K := rr.Trec_K + rr.Tsky_K;
	s := spaste('Tsys = Tsky(=',rr.Tsky_K,')');
	rr.Tsys := spaste(s,' + Trec(=',rr.Trec_K,') K');
	ss := [ss,rr.Tsys];

	# The effective area of a dipole is lambda^2/omega_sterad
	# For an isotropic radiator: omega=4*pi
	# For a dipole above a reflecting plate: omega=3;
	# In LOFAR case: use omega=4 (JDB), and separation lambda/2
	# NB: The noise does NOT depend on dipole length...
	#     as long as it is short w.r.t. the wavelength.
	rr.omega_sterad := 4.0;
	rr.Arcp_m2 := ((rr.lambda_m)^2)/rr.omega_sterad;
	rr.Aeff_m2 := rr.Arcp_m2;              # total effective area
	rr.Aeff_m2::print.precision := 2;
	rr.Aeff := paste('dipole: Aeff = (lambda^2)/omega =',rr.Aeff_m2,'m2');
	ss := [ss,rr.Aeff];
	if (T) {
	    rr.Nrcp_Jy := 2*rr.k_Jy*rr.Tsys_K/rr.Aeff_m2; 
	    rr.Nrcp := spaste('Nrcp = 2 k Tsys / Aeff Jy');
	} else {
	    rr.Nrcp_Jy := 3e6;                 # 3 MJy
	    rr.Nrcp := spaste('Nrcp = 3 MJy');
	    if (F) {
		# Correct for wavelength? (JDB?)
		rr.Nrcp_Jy /:= rr.lambda_m;
		rr.Nrcp := spaste(rr.Nrcp_Jy,'/ lambda=',rr.lambda_m);
	    }
	}
	rr.Nrcp := paste('dipole:',rr.Nrcp,'=',ceil(rr.Nrcp_Jy*1e-3),'kJy/s/Hz');
	ss := [ss,rr.Nrcp];

	# Correct for bandwidth and integration time:
	rr.JypK := 2*rr.k_Jy/rr.Aeff_m2;       # Jy/K (see below)
	rr.KpJy := 1/rr.JypK;                  # K/Jy
	ss := [ss,paste('dipole:',ceil(rr.JypK),'Jy/K')];
	# ss := [ss,paste(rr.KpJy,'K/Jy')];

	# NB: The factor 2 comes from full polarisation (JDB)
	q := sqrt(2 * 1000 * rr.bw_kHz * rr.tau_sec);
	rr.Noise_Jy := rr.Nrcp_Jy/q;           # corrected for sqrt(B.t)
	# ss := [ss,paste('dipole: noise (B,t):',ceil(rr.Noise_Jy),'Jy')];
	if (rr.HB) {
	    rr.Noise_Jy /:= 16;
	    ss := [ss,paste('compound: noise (B,t):',ceil(rr.Noise_Jy),'Jy')];
	}

	# Noise per visibility:
	# Use the geometric mean of two stations.
	rr.Nvis_Jy := rr.Noise_Jy/sqrt(rr.nelem1*rr.nelem2);
	rr.selem := paste(rr.nelem1,'x',rr.nelem2);
	if (rr.HB) rr.selem := paste(rr.selem,'x 16');
	rr.selem := spaste(rr.selem,' (nifr=',rr.nifr,', corr=4)');

	# The noise N_Jy in the chi-squared calculations:
	# Assume that there are 4 correlations per ifr:
	rr.Schisq_Jy := rr.Nvis_Jy/sqrt(4*rr.nifr);

	# The under-estimation of dM (noise-bias):
	rr.err_dM_Jy := rr.Schisq_Jy/pi;

	rr.legend := ss;                       # attach legend-string
    return rr;
}


#===============================================================================
# Sky-related functions (from calofar_calc.g):
#===============================================================================
#-------------------------------------------------------------------
# Make a jenmath, if necessary (moved to plot_common()):

    private.check_jenmath_old := function () {
	wider private;
	if (!has_field(private,'jnm')) private.jnm := F;
	if (!is_record(private.jnm)) {
	    include 'jenmath.g';
	    private.jnm := jenmath();
	    # print 'jnm=',private.jnm;
	}
    }

#====================================================================
# Set the various sky simulation parameters:

    private.skysim_init := function () {
	wider private;
	rr := [type='skysim', descr='sky simulation parms'];
	rr.spectral_index := -0.8;

	# Determine polynomil coeff for flux-distr (NgtS) at 1415 MHz:
	rr.flux_refreq_MHz := 1415; # reference freq
	logNgtS := []; logS := []; n := 0;
	logS[n+:=1] := log(3.0e-4); logNgtS[n] := log(4.0e5);
	logS[n+:=1] := log(1.0e-3); logNgtS[n] := log(2.5e5);
	for (i in [1:4]) {
	    flux_index := -0.7;                # faint end <1 mJy 
	    logStep := -1.0;                   # steps of -log(10)
	    n +:= 1;
	    logS[n] := logS[n-1]+logStep;
	    logNgtS[n] := logNgtS[n-1]+logStep*flux_index; 
	}
	logS[n+:=1] := log(1.0e-2); logNgtS[n] := log(6.0e4);
	logS[n+:=1] := log(1.0e-1); logNgtS[n] := log(5.5e3);
	logS[n+:=1] := log(1.0); logNgtS[n] := log(1.8e2);
	for (i in [1:4]) {
	    flux_index := -1.2;                # bright end >1 Jy 
	    logStep := +1.0;                   # steps of -log(10)
	    n +:= 1;
	    logS[n] := logS[n-1]+logStep;
	    logNgtS[n] := logNgtS[n-1]+logStep*flux_index; 
	}
	private.check_jenmath();               # -> private.jnm
	ii := private.jnm.sort (logS, desc=F, index=T, trace=F);
	cc := private.jnm.fit_poly(yy=logNgtS[ii], xx=logS[ii], ndeg=4, 
				   eval=T, xscale=F, trace=T);
	cc := private.yapi.print_precision(cc);
	if (F) {
	    print 'skysim.fit_poly(): cc=\n',cc;
	    r := private.yapi.show_record(cc,'skysim.fit_poly()');
	    print 'skysim.fit_poly(): -> r=\n',r;
	}
	rr.flux_coeff := cc.coeff;             # used in .NgtS()

	# Attach jenmath.fit_poly result for later scrutiny:
	rr.flux_polfit := cc; 

	# Finally, attach to private
	rr := private.yapi.print_precision(rr);
	private.skysim := rr;
	return T;
    }

    public.skysim := function () {return private.skysim};    # access function


# Helper function: calculate the number of sources per steradian > S_Jy:
# The number depends on the baseline length L_km:
# NB: S_Jy can be a vector
# Combine with .logNgtS() above?

    public.NgtS := function (S_Jy=1.0, freq_MHz=30.0, L_km=0.0, trace=F) {
	funcname := 'calc.NgtS()';
	s1 := paste(range(S_Jy),' freq=',freq_MHz,' L=',L_km);
	if (trace) print funcname,s1;

	# For extended sources, the apparent flux is reduced for long 
	# baselines. This means that we see fewer brighter sources:
	Sin_Jy := S_Jy;                      # keep input for debugging
	if (L_km>0) {
	    rr := public.sizeS (S_Jy, freq_MHz, L_km, spectral_index=-0.8)
	    S_Jy /:= rr.exp;                 # Increase effective S_Jy values
	    S_Jy[S_Jy>1000] := 1000;         # limit to reasonable values ...
	}

	# Calculate the log(NgtS) from the polynomial coeff:
	# Per sterad, at 1412 MHz
	logNgtS := rep(0.0,len(S_Jy));
	logS := log(S_Jy);
	logSS := rep(1.0,len(S_Jy));
	for (i in ind(private.skysim.flux_coeff)) {
	    logNgtS +:= private.skysim.flux_coeff[i]*logSS;
	    logSS *:= logS;
	}

	# Apply freq-dependence (e.g. f^-0.8)
	fratio := freq_MHz[1]/private.skysim.flux_refreq_MHz;
	if (trace) print 'fratio=',fratio;
	logNgtS +:= (private.skysim.spectral_index * log(fratio));

	NgtS := 10^logNgtS;                  # convert for output

	# Debugging:
	if (any(logNgtS>10)) {
	    print funcname,s1,'(logNgtS>10):';
	    for (i in ind(S_Jy)) {
		print i,':',Sin_Jy[i],S_Jy[i],'->',logNgtS[i],NgtS[i];
	    }
	}
	
	return NgtS;
    }

# Helper function: calculate the size of sources, as a function
# of flux S, and observing frequency.
# The median size is a function of source flux (S_Jy) 
# (see thesis of Marc Oort, 1986)
# These sizes do not depend on frequency, because the size
# is usually determined by the distance of two hot-spots.
# Also the resulting attenuation for a given baseline length.

    public.sizeS := function (S_Jy=1.0, freq_MHz=30.0, L_km=0.0,
			      spectral_index=-0.8) {
	funcname := 'calc.sizeS()';
	rr := [type='sizeS', descr='source sizes'];
	rr.freq_MHz := freq_MHz[1];          # just in case
	rr.lambda := 300/rr.freq_MHz;
	rr.spectral_index := spectral_index;
	rr.L_km := L_km;
	rr.S_Jy := S_Jy;                     # may be vector

	if (L_km>0) {
	    # The values of Oort are at 1412 MHz:
	    q := (1412/rr.freq_MHz)^rr.spectral_index;
	    rr.fwhm_arcsec := 20*((q*rr.S_Jy)^0.4); # from Oort pp 140
	    fwhm_rad := rr.fwhm_arcsec*private.arcsec2rad; 
	    # Case 1: top-hat source:
	    # aa := 2*pi*(1000*rr.L_km/rr.lambda)*(fwhm_rad/2);  # argument
	    # rr.sinc := abs(public.sinc(aa)); # main lobe only?
	    # Case 2: gaussian source:
	    bb := pi*(1000*rr.L_km/rr.lambda)*(fwhm_rad/2);    # argument
	    rr.exp := exp(-(bb^2));
	    # rr.ratio := rr.sinc/rr.exp;      # temporary
	}

	if (F) private.yapi.show_record(rr, recurse=F);
	return rr;
    }

    

# Helper function to calculate sinc=sin(x)/x:

    public.sinc := function (x=0) {
	s := sin(x)/x;
	s[abs(x)<=0.001] := 1.0;
	return s;
    }


#===============================================================================
# Show a cross-section of a station beam, with the role of
# fiducial points (cat I sources) etc.

    private.plot.fiduc := function (pp=T, trace=F) {
	funcname := 'plot.fiduc()';
	#--------------------------------------------------------
	# Standard entry module for pp=controlled functions:
	yapi := private.yapi;                    # convenience
	pp := yapi.on_entry(pp, funcname, trace=trace);
	if (yapi.do_check(pp)) {
	    yapi.itsov (pp, prompt=T, help='overall function help', killonOK=F);
	    # private.yapi.inarg (pp, C=T, dM_well=F);
	    private.inarg_plotcontrol(pp, funcname, legend='tlc', axes=F, 
				      margin=[0.0, 0.1, 0.1, 0.0]);
	}
	if (!yapi.do_execute(pp)) return pp;
	#--------------------------------------------------------
	rr := private.on_entry (pp, funcname);

	# Make an array of beam sampling points:
	rr.FWHM_rad := 0.1;                  # beamwidth
	rr.na := 150;                        # nr of points
	rr.theta_rad := 0.5;                 # pointing zentih angle 
	rr.astep_rad := 0.002;               # angle step     
	rr.da := rr.astep_rad*[-rr.na:rr.na];   # relative angles 
	rr.aa := rr.theta_rad + rr.da;          # absolute angles
	rr.i0 := 1+rr.na;                    # index of centre 

	# Make the station-beamshape:
	bb := 2*pi*rr.da/rr.FWHM_rad;           
	rr.gg := sin(bb)/bb;                 # sinc
	rr.gg[rr.i0] := 1.0;                 # centre of sinc
	# Distort the sinc with a polynomial:
	rr.gg +:= 0.15*rr.da - 0.2*rr.da*rr.da; 
	dypl.plotxy(rr.aa, rr.gg, style='dashed', color='blue',
		    label='station beam', labelpos='right');
	dypl.plotxy(y=0, color='default');   # x-axis
	# dypl.plotxy(rep(rr.theta_rad,2), [0,1], 
	# 	    style='dashed',color='blue');

	# Ionospheric phases:
	rr.pp := 0.5 - 1.0*rr.da - 0.9*(rr.da^2) + 8.0*(rr.da^3); 
	dypl.plotxy(rr.aa, rr.pp, style='dashed', color='red', 
		    label='ionos.phase', labelpos='right');

	# The main lobe is special:
	sv := [rr.gg>0.3];                   # selection vector
	aa1 := rr.aa[sv];
	gg1 := rr.gg[sv];
	pp1 := rr.pp[sv];
	# dypl.plotxy(aa1, gg1, color='blue');
	# dypl.plotxy(aa1, pp1, color='red');
	dypl.plotxy(rr.aa[rr.i0], rr.gg[rr.i0], 
		    style='dot', color='blue',
		    label='main lobe', labelpos='top');

	# Random cat I source positions and fluxes (all-sky):
	rr.acomp := [];
	rr.gcomp := [];
	rr.pcomp := [];
	rr.fcomp := [];
	nml := 0;                            # nr in main lobe
	rr.nmlmax := 10;                     # max in main lobe
	nc := 0;
	rr.ncmax := 100;
	iiml := [];
	while (T) {
	    k := random(1,len(rr.gg));
	    gabs := abs(rr.gg[k]);
	    if (gabs<0.05) next;             # in beam 'zero';
	    nc +:= 1;
	    if (sv[k]) {                     # in main lobe
		nml +:= 1;                   # counter
		iiml[nml] := nc;             # indices in rr.acomp etc
	    }
	    if (gabs>0.6) icmax := nc;       # index of brightest source                 
	    rr.acomp[nc] := rr.aa[k];        # position
	    rr.gcomp[nc] := rr.gg[k];        # beam gain
	    rr.pcomp[nc] := rr.pp[k];        # ionos.phase
	    rr.fcomp[nc] := 1.0;
	    if (nml>=rr.nmlmax) break;
	    if (nc>=rr.ncmax) break;
	}
	dypl.plotxy(rr.acomp[iiml], 0*rr.gcomp[iiml], 
		    style='star', color='default', size=2);
	dypl.plotxy(rr.acomp, rr.gcomp, style='star', color='blue', size=2);
	dypl.plotxy(rr.acomp, rr.pcomp, style='star', color='red', size=2);
	dypl.legend('(*): cat I sources (peeled)');

	# Indicate the brightest source(s):
	if (F) {
	    dypl.plotxy(rr.acomp[icmax], rr.gcomp[icmax], 
			style='star', color='blue', size=4,
			label='brightest cat I source', labelpos='right');
	    dypl.plotxy(rr.acomp[icmax], rr.pcomp[icmax], 
			style='star', color='red', size=4);
	    dypl.plotxy(rr.acomp[icmax], 0*rr.gcomp[icmax], 
			style='star', color='default', size=4);
	}

	# The estimated (smooth) and the actual values coincide
	# in the 'fiducial' points, i.e. the bright (cat I) sources
	# that are used for peeling:
	yy1 := rep(1.0,len(aa1));
	for (i in [1,iiml]) {
	    yy1 *:= (aa1-rr.acomp[i]);
	}
	yy1 *:= (aa1-aa1[1]);
	yy1 *:= (aa1-aa1[len(aa1)]);
	yy1 *:= 0.1/max(abs(yy1));
	dypl.plotxy(aa1, yy1, color='red');
	dypl.plotxy(aa1, yy1+pp1, color='red');

	# Random cat II source positions and fluxes (main lobe):
	rr.acomp2 := [];
	rr.gcomp2 := [];
	rr.pcomp2 := [];
	rr.fcomp2 := [];
	yy2 := [];
	nc2 := 0;
	rr.nc2max := 30;
	while (T) {
	    k := random(1,len(gg1));
	    nc2 +:= 1;
	    rr.acomp2[nc2] := aa1[k];        # position
	    rr.gcomp2[nc2] := gg1[k];        # beam gain
	    rr.pcomp2[nc2] := pp1[k];        # ionos.phase
	    yy2[nc2] := yy1[k];
	    rr.fcomp2[nc2] := 1.0;
	    if (nc2>=rr.nc2max) break;
	}
	dypl.plotxy(rr.acomp2, rr.gcomp2, style='cross', color='green', size=2);
	dypl.plotxy(rr.acomp2, 0*rr.gcomp2, style='cross', color='green', size=2);
	# dypl.plotxy(rr.acomp2, yy2, style='cross', color='green', size=2);
	# dypl.plotxy(rr.acomp2, rr.pcomp2+yy2, style='cross', color='green', size=2);
	dypl.plotxy(rr.acomp2, rr.pcomp2, style='cross', color='green', size=2);
	dypl.legend('(x): cat II sources (just subtr)', color='green');

	# Finishing touches:
	# private.pp2legend (pp, exclude="", clear=F);
	dypl.labels(title='peeling in a nutshell', 
		    xlabel='zenith angle (rad)', 
		    ylabel='voltage gain (station beam) or rad (ionos.phase)');
	return private.on_exit(pp, rr);
    }


#===============================================================================
# Illustrate ionospheric phase-tracking  

    private.plot.phase_track := function (pp=T, trace=F) {
	funcname := 'plot.phase_track()';
	#--------------------------------------------------------
	# Standard entry module for pp=controlled functions:
	yapi := private.yapi;                    # convenience
	pp := yapi.on_entry(pp, funcname, trace=trace);
	if (yapi.do_check(pp)) {
	    yapi.itsov (pp, prompt=T, help='overall function help', killonOK=F);
	    private.inarg_ionos(pp);
	    private.inarg_station(pp);
	    private.inarg_plotcontrol(pp, funcname, legend='tlc', axes=F, 
				      margin=[0.0, 0.2, 0.1, 0.0]);
	}
	if (!yapi.do_execute(pp)) return pp;
	#--------------------------------------------------------
	rr := private.on_entry (pp, funcname);
	private.check_jenmath();                 # -> private.jnm
	private.TID_pierce(rr);

	# Time-vector:
	v := rr.TID_veloc_kph/3600;              # -> km/s
	T_s := rr.TID_wvl_km/v;                  # time for one wavelength          
	dtt := T_s/10;                           # sample separation (sec)
	ntt := ceil(min(30*T_s,3600)/dtt);       # at least 30 periods
	# ntt := ceil(3600/dtt);                   # one hour
	rr.tt := dtt*[0:ntt];                    # time vector (sec)

	# Calculate phase(t) for each piercing point:
	rr.ff := [=];
	for (k in ind(rr.ps.xp)) {               # see .TID_pierce
	    isrc := rr.ps.isrc[k];               # source nr
	    istat := rr.ps.istat[k];             # station nr
	    xp := rr.ps.xp[k];                   # piercing pos
	    rr.ff[k] := private.TID_phase(rr, x=xp, t=rr.tt);
	    if (F) {
		color := rr.color[isrc];
		s := paste('istat=',istat);
		dypl.plotxy(rr.tt, rr.ff[k], color=color, label=s, labelpos='right');
	    }
	}

	# Calculate phase differences as a function of time:
	n := len(rr.ps.xp);
	for (k in [1:(n-1)]) {
	    isrc := rr.ps.isrc[k];               # source nr
	    istat := rr.ps.istat[k];             # station nr
	    xp := rr.ps.xp[k];                   # piercing pos
	    color := rr.color[isrc];
	    for (m in [(k+1):n]) { 
		if (istat>1) next;                     # avoid repeats
		if (rr.ps.istat[m]==istat) {           # same station
		    if (istat>1) next;                 # avoid repeats
		    dff := rr.ff[m]-rr.ff[k];
		    jsrc := rr.ps.isrc[m];     
		    ionolen := as_integer(abs(xp - rr.ps.xp[m]));
		    s := spaste(istat,':');
		    s := paste(s,ionolen,'km');        # distance in ionosphere
		    d1D := private.jnm.diff1D (dff, rr.tt);
		    dmax := ceil(max(abs(d1D))*180/pi);
		    s := spaste(s,' (<',dmax,'deg/s)');
		    dypl.plotxy(rr.tt, dff, color='faint', style='dotted',
				label=s, labelpos='right');
		} else if (rr.ps.isrc[m]==isrc) {      # same source
		    if (isrc>1) next;                  # avoid repeats
		    dff := rr.ff[m]-rr.ff[k];
		    jstat :=  rr.ps.istat[m];
		    s := spaste(istat,'-',jstat,':');
		    ionolen := abs(rr.xpos[istat]-rr.xpos[jstat]);
		    s := paste(s,ionolen,'km'); # distance = baseline length
		    d1D := private.jnm.diff1D (dff, rr.tt);
		    dmax := ceil(max(abs(d1D))*180/pi);
		    s := spaste(s,' (<',dmax,'deg/s)');
		    dypl.plotxy(rr.tt, dff, label=s, labelpos='right');
		    dypl.legend(paste('same source:',s), 
				color=dypl.get_dataset().color) 
		}
	    }
	}
	dypl.legend('between different sources in the same field', color='faint') 

	# For each station, estimate the phase at a random position 
	# from the 'measured' phases in the direction of the bright
	# calibrater sources.
	n := len(rr.ps.xp);
	for (k in [1:(n-1)]) {
	    isrc := rr.ps.isrc[k];               # source nr
	    istat := rr.ps.istat[k];             # station nr
	    xp := rr.ps.xp[k];                   # piercing pos
	    color := rr.color[isrc];
	    for (m in [(k+1):n]) { 
		if (rr.ps.istat[m]==istat) {           # same station
		    # dff := rr.ff[m]-rr.ff[k];
		    jsrc := rr.ps.isrc[m];     
		    ionolen := as_integer(abs(xp - rr.ps.xp[m]));
		    # s := paste(ionolen,'km');         # distance in ionosphere
		    # dypl.plotxy(rr.tt, dff, color='red', label=s, labelpos='right');
		}
	    }
	    # cc := private.jnm.fit_poly(yy=logNgtS[ii], xx=logS[ii], ndeg=4, 
	    # 			   eval=T, xscale=F, trace=T);
	}	

	private.pp2legend (pp, exclude="", clear=F);
	dypl.labels(title='ionospheric phase tracking', aspect=unset,
		    xlabel='time (sec)', 
		    ylabel='phase (rad)');
	return private.on_exit(pp, rr);
    }

#------------------------------------------------------
# Plot the TID and its piercing points:

    private.plot.TID := function (pp=T, trace=F) {
	funcname := 'plot.TID()';
	#--------------------------------------------------------
	# Standard entry module for pp=controlled functions:
	yapi := private.yapi;                    # convenience
	pp := yapi.on_entry(pp, funcname, trace=trace);
	if (yapi.do_check(pp)) {
	    yapi.itsov (pp, prompt=T, help='overall function help', killonOK=F);
	    private.inarg_ionos(pp);
	    private.inarg_station(pp);
	    private.inarg_plotcontrol(pp, funcname, legend='tlc', axes=F, 
				      margin=[0.0, 0.1, 0.1, 0.0]);
	}
	if (!yapi.do_execute(pp)) return pp;
	#--------------------------------------------------------
	rr := private.on_entry (pp, funcname);
	private.TID_pierce(rr);

	# Ionospheric phase wave:
	dypl.plotxy(rr.xx, rr.cc, color='red', 
		    label='TID phase', labelpos='right');
	dypl.plotxy(range(rr.xx), range(rr.altid), color='magenta', style='dotted',
		    label='TID screen altitude', labelpos='right');
	s := paste(rr.TID_veloc_kph,'km/h');
	a := rr.altid + 0.7 * rr.TID_peak_rad;
	dypl.arrow([rr.xrange[1] + (rr.xrange[2]-rr.xrange[1])/3, a], 
		   [rr.xrange[1] + 2*(rr.xrange[2]-rr.xrange[1])/3, a], 
	 	   color='red', label=s, labelpos='right');

	# Stations and their beams:
	dypl.plotxy(rr.xpos, 0*rr.xpos, style='star', color='blue');
	dypl.plotxy(range(rr.xpos), [0,0], style='dashed', color='default',
		    label='LOFAR', labelpos='right');
	for (x in rr.xpos) {
	    dypl.plotxy(x+rr.beam.xx, rr.beam.yy, style='dotted', color='faint');
	}

	# Rays and piercing points:
	for (k in ind(rr.ps.xp)) {               # see .TID_pierce
	    isrc := rr.ps.isrc[k];               # source nr
	    istat := rr.ps.istat[k];             # station nr
	    xs := rr.xpos[istat];                # station pos
	    xp := rr.ps.xp[k];                   # piercing pos
	    ip :=  rr.ps.ixxp[k];                # index in xx
	    color := rr.color[isrc];
	    dypl.plotxy([xs,xp], [0,rr.altid], color=color, style='dashed');
	    dypl.plotxy([xp], [rr.altid], color=color, style='cross');
	    dypl.plotxy([xp,xp], [rr.altid,rr.cc[ip]], color=color, style='dotted');
	    dypl.plotxy([xp], [rr.cc[ip]], color=color, style='cross');
	}
	
	private.pp2legend (pp, exclude="", clear=F);
	dypl.labels(title='TID piercing points', aspect=unset,
		    xlabel='ground position (km)', 
		    ylabel='altitude (km) or phase (rad)');
	return private.on_exit(pp, rr);
    }


# Calculate the positions where the rays from the stations to the
# calibrator sources pierce the TID phase screen:

    private.TID_pierce := function (ref rr=F) {
	rr.altid := rr.TID_alt_km;                  # convenience
	rr.xpos := rr.station_xpos_km;              # convenience

        # Make up zenith angles of 3 calibrator sources: 
	rr.zaa := rr.pointing_rad + 0.5*rr.beam_fwhm_rad * [-1,0,0.6];
	rr.color := "blue green cyan";              # their colors

	# Station beam-shape:
	private.beamshape (rr, nb=50, trace=F);
	
	# Calculate the pierce positions:
	k := 0;
	rr.ps := [isrc=[], istat=[], xp=[], ixxp=[]];                  
	for (isrc in ind(rr.zaa)) {                 # source zenith angles
	    dx := rr.altid * tan(rr.zaa[isrc]);
	    for (istat in ind(rr.xpos)) {           # station positions
		k +:= 1;
		rr.ps.isrc[k] := isrc;              # source index
		rr.ps.istat[k] := istat;            # station index
		rr.ps.xp[k] := rr.xpos[istat] + dx; # pos of ray piercing point
	    }
	}

	# Also calculate the TID phase at t=0 for a 'suitable' vector rr.xx:
	xrange := range(rr.ps.xp);                  # all piercing points
	xmean := sum(xrange)/2;
	rr.xrange := xmean + (xrange-xmean)*1.2;       # and some extra
	nx := 100;
	rr.xx := rr.xrange[1] + (rr.xrange[2]-rr.xrange[1])*[0:nx]/nx;
	rr.cc := rr.altid + private.TID_phase(rr, x=rr.xx);
	for (k in ind(rr.ps.xp)) {
	    dxp := abs(rr.xx-rr.ps.xp[k]); 
	    ip := ind(dxp)[dxp==min(dxp)];
	    rr.ps.ixxp[k] := ip[1];                 # index in xx etc
	}
	# print '\n** rr.ps=\n',rr.ps,'\n';
	return T;
    }

# Helper function to calculate TID phase(s) as a function of 
# position (x) and time (t). One of these may be a vector:
# NB: Take elevation and Earth curvature into account....

    private.TID_phase := function (rr=F, x=0, z=F, t=0, trace=F) {
	if ((len(x)>1 || len(z)>1) && len(t)>1) {
	    print '** TID_phase(): either x/z or t may be a vector!';
	    return 0;
	} else if (!is_boolean(z)) {                # zenith angle specified
	    x := rr.TID_alt_km * tan(z);            # convert to x
	}
	v := rr.TID_veloc_kph/3600;                 # -> km/s
	aa := 2*pi*(x-v*t)/rr.TID_wvl_km;
	return rr.TID_peak_rad * sin(aa);
    }

# Helper function to calculate the beamshape from rr:

    private.beamshape := function (ref rr=F, nb=50, trace=F) {
	rr.beam := [fwhm_rad=rr.beam_fwhm_rad,
		    pointing_rad=rr.pointing_rad];
	zrel := rr.beam.fwhm_rad * [-nb:nb]/nb;
	rr.beam.zaa := rr.beam.pointing_rad + zrel;
	rr.beam.gain := cos(0.5*pi*zrel/rr.beam.fwhm_rad);
	q := 1.5 * rr.TID_alt_km;
	rr.beam.xx := q * rr.beam.gain * sin(rr.beam.zaa);
	rr.beam.yy := q * rr.beam.gain * cos(rr.beam.zaa);
	return beam;
    }


#------------------------------------------------------
# Plot patches in the FOV:

    private.plot.patch := function (pp=T, trace=F) {
	funcname := 'plot.patch()';
	#--------------------------------------------------------
	# Standard entry module for pp=controlled functions:
	yapi := private.yapi;                    # convenience
	pp := yapi.on_entry(pp, funcname, trace=trace);
	if (yapi.do_check(pp)) {
	    yapi.itsov (pp, prompt=T, help='overall function help', killonOK=F);
	    private.inarg_ionos(pp);
	    private.inarg_station(pp);
	    private.inarg_plotcontrol(pp, funcname, legend='tlc', axes=F, 
				      margin=[0.0, 0.1, 0.1, 0.0]);
	}
	if (!yapi.do_execute(pp)) return pp;
	#--------------------------------------------------------
	rr := private.on_entry (pp, funcname);
	private.TID_patch(rr);
	# inspect(rr.patch,'patch');

	dypl.plotxy(rr.patch.zaa, rr.patch.phase, color='blue',
		    label='TID phase', labelpos='right');
	pmean := sum(rr.patch.phase)/len(rr.patch.phase);
	zr := range(rr.patch.zaa);
	n := ceil((zr[2]-zr[1])/rr.patch.size_rad);
	zz := [1:n]*rr.patch.size_rad;
	dypl.plotxy(zz, rep(0,n), style='dot', color='red',
		    label='patches', labelpos='right');
	
	dypl.legend(paste('patch size =',rr.patch.size_arcmin,'arcmin'));
	dypl.legend(paste('nr of patches/FOV (fwhm) =',rr.patch.nr));
	
	private.pp2legend (pp, exclude="", clear=F);
	dypl.labels(title='patch imaging', aspect=unset,
		    xlabel='zenith angle(rad)', 
		    ylabel='phase (rad)');
	return private.on_exit(pp, rr);
    }

# Helper function to calculate the patch size (1 rad):

    private.TID_patch := function (ref rr=F, trace=F) {
	private.beamshape (rr, nb=50, trace=trace);
	rr.patch := [zaa=rr.beam.zaa,               # zenith angles
		     fov=rr.beam.fwhm_rad];         # Field of View
	rr.patch.phase := private.TID_phase (rr, z=rr.patch.zaa);
	private.check_jenmath();                 # -> private.jnm
	d1D := private.jnm.diff1D (rr.patch.phase, rr.patch.zaa);
	rr.patch.size_rad := 1.0/max(d1D);         
	rr.patch.size_deg := rr.patch.size_rad*180/pi;         
	rr.patch.size_arcmin := ceil(rr.patch.size_deg*60);         
	q := (rr.beam.fwhm_rad/rr.patch.size_rad)^2;
	rr.patch.nr := ceil(q);                     # nr of patches
	return patch;
    }

#----------------------------------------------------------------------
# Ionospheric parameters:
# NB: The default values are consistent with the 74MHz Virgo A obs

    private.inarg_ionos := function (ref pp=F) {
	private.yapi.inarg (pp,'TID_alt_km', [300],
			    help='altitude of phase-screen');
	private.yapi.inarg (pp,'TID_peak_rad', [20,2,1.0],
			    help='peak value of TID wave');
	private.yapi.inarg (pp,'TID_wvl_km', [130,90,3.2,10.0,30,100,300,1000],
			    help='wavelength value of TID wave');
	private.yapi.inarg (pp,'TID_veloc_kph', [500,750,1000],
			    help='velocity of TID wave');
	return T;
    }


    private.inarg_station := function (ref pp=F) {
	private.yapi.inarg (pp,'station_xpos_km', [0.0,10,100], 
			    [0.0,30], [0.0,10,30], 
			    help='station positions (1D)');
	private.yapi.inarg (pp,'beam_fwhm_rad', [0.1],  
			    help='station beamwidth');
	private.yapi.inarg (pp,'pointing_rad', [0.1],  
			    help='pointing direction (w.r.t. zenith)');
	return T;
    }


#===============================================================================
# The optimal cosine-factor to calculate  

    private.plot.cosfac_N := function (pp=T, trace=F) {
	funcname := 'plot.cosfac_N()';
	#--------------------------------------------------------
	# Standard entry module for pp=controlled functions:
	yapi := private.yapi;                    # convenience
	pp := yapi.on_entry(pp, funcname, trace=trace);
	if (yapi.do_check(pp)) {
	    yapi.itsov (pp, prompt=T, help='overall function help', killonOK=F);
	    private.inarg_chisq_terms(pp, C=T, dM_well=F);
	    private.inarg_plotcontrol(pp, funcname, legend='tlc', axes=T, 
				      margin=[0.0, 0.0, 0.1, 0.0]);
	}
	if (!yapi.do_execute(pp)) return pp;
	#--------------------------------------------------------
	rr := private.on_entry (pp, funcname);

	n2 := 20;
	xx := 2*pi*[0:n2]/n2;
	yy := cos(xx);
	dypl.plotxy(xx, yy, style='dashed', color='blue');
	yy[yy>0] := 0;
	dypl.plotxy(xx, yy, style='solid', size=10, color='red');
	dypl.plotxy(y=-1/pi, style='dashed', label='-1/pi', labelpos='right');
	dypl.legend('under-estimation of dM = N_Jy * cos(phi)', charsize=1);
	dypl.legend('(1/2pi) int(pi/2,3pi/2) cos(x) dx -> -1/pi');

	# private.pp2legend (pp, exclude="", clear=F);
	dypl.labels(title='under-estimation of dM (Jy)', 
		    xlabel='phi (rad)', 
		    ylabel='cos(phi)');
	return private.on_exit(pp, rr);
    }

#===============================================================================
# Illustrate the various visibility vectors

    private.plot.visib := function (pp=T, trace=F) {
	funcname := 'plot.visib()';
	#--------------------------------------------------------
	# Standard entry module for pp=controlled functions:
	yapi := private.yapi;                    # convenience
	pp := yapi.on_entry(pp, funcname, trace=trace);
	if (yapi.do_check(pp)) {
	    yapi.itsov (pp, prompt=T, help='overall function help', killonOK=F);
	    r := private.inarg_chisq_terms(pp, C=T, dM_well=F, CSratio=0.3);
	    if (is_fail(r)) print r;
	    private.inarg_plotcontrol(pp, funcname, legend='tlc', axes=2,
				      margin=[0.1, 0.0, 0.0, 0.0]);
	}
	if (!yapi.do_execute(pp)) return pp;
	#--------------------------------------------------------
	rr := private.on_entry (pp, funcname);

	rr := [xy1=[=], xy2=[=], color=[=], label=[=], descr=[=], flux=[=]];

	rr.flux.N := pp.Source_Jy * (pp.NSratio) / sqrt(pp.nvis);
	rr.flux.C := pp.Source_Jy * (pp.CSratio) / sqrt(pp.nvis);
	rr.flux.M := pp.Source_Jy * (1-pp.dMratio);
	rr.flux.dM := pp.Source_Jy * pp.dMratio;
	rr.arg := [M=0.0*pi, dM=0.4*pi, C=0.7*pi, N=0.4*pi];
	# rr.arg.N *:= sqrt(pp.nvis);           # 'randomise'
	# rr.arg.C *:= sqrt(pp.nvis);           # 'randomise'

	# Model of peeling source:
	rr.xy1.M := [0,0];
	rr.xy2.M := rr.xy1.M + rr.flux.M * [cos(rr.arg.M),sin(rr.arg.M)];
	# Error in M:
	rr.xy1.dM := rr.xy2.M;
	rr.xy2.dM := rr.xy1.dM + rr.flux.dM * [cos(rr.arg.dM),sin(rr.arg.dM)];

	# Contaminating source:
	rr.xy1.C := rr.xy2.dM;
	rr.xy2.C := rr.xy1.C + rr.flux.C * [cos(rr.arg.C),sin(rr.arg.C)];

	# Noise:
	rr.xy1.N := rr.xy2.C;
	rr.xy2.N := rr.xy1.N + rr.flux.N * [cos(rr.arg.N),sin(rr.arg.N)];

	# Some resulting vectors:
	rr.xy1.A := [0,0];
	rr.xy2.A := rr.xy2.dM;
	rr.xy1.V := [0,0];
	rr.xy2.V := rr.xy2.N;

	rr.color := [M='default', dM='magenta', C='blue', N='green', V='red', A='cyan'];
	rr.label := [M='Mi', dM='dMi', C='Ci', N='Ni', V='Vi', A='Mi+dMi'];
	if (pp.nvis>1) {
	    for (fname in field_names(rr.label)) {
		rr.label[fname] := spaste('<',rr.label[fname],'>');
	    }
	}
	rr.descr.M := paste('predicted for peeling source');
	rr.descr.dM := paste('error in',rr.label.M,'(to be minimised)');
	rr.descr.A := paste('actual contr. of peeling source to V');
	rr.descr.C := paste('contaminating source(s)');
	rr.descr.N := paste('rms noise');
	rr.descr.V := paste('measured visibility');

	for (fname in field_names(rr.descr)) {
	    s := spaste(rr.label[fname],': ',rr.descr[fname]);
	    dypl.legend(s,color=rr.color[fname]);
	    if (fname=='C' && rr.flux.C==0) next;
	    if (any(fname=="A")) next;

	    labelpos := 'right';
	    style := 'solid';
	    size := 1;
	    if (any(fname=="A")) style := 'dashed';
	    if (any(fname=="V M dM")) size := 10;
	    # if (any(fname=="V A")) next;
	    # if (fname=='V') labelpos := 'left';
	    if (any(fname=="N C")) {
		# Indicate variable contribution by a circle:
		dypl.circle (x0=rr.xy1[fname][1], y0=rr.xy1[fname][2], 
			     radius=rr.flux[fname], 
			     label=rr.label[fname], labelpos=F, descr=F,
			     style='dotted', color=rr.color[fname], size=1, 
			     trace=F);
		if (pp.nvis>1) {
		    # Emphsize the half-circle of the 'diode effect':
		    arange := rr.arg.dM + [1,3]*pi/2;
		    dypl.ellipse (x0=rr.xy1[fname][1], y0=rr.xy1[fname][2], 
				  rx=rr.flux[fname], ry=F, 
				  label='diode', labelpos=F,
				  pa=0, arange=arange, axes=F, 
				  pp=F, trace=F, adjust=T,
				  color=rr.color[fname], 
				  style='dashed', size=6);
		    s1 := paste(rr.label[fname],': diode-effect half-circle emphasized');
		    dypl.legend(s1, color=rr.color[fname]);
		}
	    } 
	    dypl.arrow (xy1=rr.xy1[fname], xy2=rr.xy2[fname], 
			sah=3, angle=45, vent=0.3, hsize=1,
			label=rr.label[fname], labelpos=labelpos, 
			charsize=2, descr=F,
			style=style, color=rr.color[fname], size=size, 
			sparrow=F, trace=F);
	    if (fname=='M') {
		s1 := paste(rr.label.M,': phase centre moved to peeling source');
		dypl.legend(s1, color=rr.color.M);
	    }
	}

	private.pp2legend (pp, exclude="cosfac_N", clear=F);
	dypl.labels(title='the various visibility contributions', 
		    xlabel='real (Jy)', ylabel='imag (Jy)', aspect=T);
	return private.on_exit(pp, rr);
    }

#===============================================================================
# The error in M as a function of S/N ratio

    private.plot.errM := function (pp=T, trace=F) {
	funcname := 'plot.errM()';
	#--------------------------------------------------------
	# Standard entry module for pp=controlled functions:
	yapi := private.yapi;                    # convenience
	pp := yapi.on_entry(pp, funcname, trace=trace);
	if (yapi.do_check(pp)) {
	    yapi.itsov (pp, prompt=T, help='overall function help', killonOK=F);
	    private.inarg_chisq_terms(pp, C=F);
	    private.inarg_plotcontrol(pp, funcname, legend='tlc', axes=T,
				      margin=[0.0, 0.1, 0.0, 0.0]);
	}
	if (!yapi.do_execute(pp)) return pp;
	#--------------------------------------------------------
	rr := private.on_entry (pp, funcname);

	# Calculate the various terms of chisq as functions of M.E.parameter(s):
	NSR := [1:10]/10;
	pp.cosfac_N := -1; 
	for (d_well in [5:10]/10) {
	    pp.d_well := d_well;
	    print '\n',s := spaste('d_well=',pp.d_well);
	    xx := [];
	    yy := [];
	    for (i in ind(NSR)) {
		pp.NSratio := NSR[i]; 
		xx[i] := 1/pp.NSratio;
		rr := private.chisq_terms (pp=pp);
		yy[i] := rr.chisq_errM_pct;
		# private.plot_chisq(rr);
		print '-',i,':',NSR[i],xx[i],'->',yy[i],'%';
	    } 
	    dypl.plotxy(xx, yy, label=s, labelpos='right');
	}

	private.pp2legend (pp, exclude="d_well NSratio", clear=F);
	dypl.labels(title=funcname, 
		    xlabel='S/N ratio', 
		    ylabel='chisq_errM_pct');
	return private.on_exit(pp, rr);
    }

#===============================================================================
# The various contributions to chisq:

    private.plot.chisq := function (pp=T, trace=F) {
	funcname := 'plot.chisq()';
	#--------------------------------------------------------
	# Standard entry module for pp=controlled functions:
	yapi := private.yapi;                    # convenience
	pp := yapi.on_entry(pp, funcname, trace=trace);
	if (yapi.do_check(pp)) {
	    yapi.itsov (pp, prompt=T, help='overall function help', killonOK=F);
	    private.inarg_chisq_terms(pp);
	    private.inarg_plotcontrol(pp, funcname, legend='tlc', axes=5,
				      margin=[0.5, 0.1, 0.2, 0.0]);
	}
	if (!yapi.do_execute(pp)) return pp;
	#--------------------------------------------------------
	rr := private.on_entry (pp, funcname);

	# Calculate the various terms of chisq as functions of M.E.parameter(s):
	rr := private.chisq_terms (pp=pp); 
	private.plot_chisq(rr);
	private.pp2legend (pp, exclude="", clear=F);
	return private.on_exit(pp, rr);
    }

#---------------------------------------------------------------------------------
# Helper function to plot chisq or its terms:

    private.plot_chisq := function (rr=F, name='full', label=T, legend=F, 
				    envelope=F, negate=F, minimum=F,
				    color=F, style='solid', size=1, trace=F) {
	s := paste('** .plot_chisq(',name,label,color,style,size,'):');
	labelpos := 'none';
	if (any(name==field_names(rr.color))) {
	    c := rr.color[name];
	    if (is_boolean(label) && label) labelpos := 'right';
	    if (is_string(label)) labelpos :=  'right';
	    if (!is_string(label)) label := rr.label[name];
	    if (is_boolean(color)) color := rr.color[name];
	    if (name=='chisq') size := 8;        # special case
	    if (name=='dM2') size := 5;          # special case
	    if (trace) print s,legend;
	    yy := rr[name];
	    if (negate) yy *:= -1;
	    if (len(yy)!=len(rr.xx)) yy := rep(yy[1],len(rr.xx));     # adjust yaploi?
	    dypl.plotxy (x=rr.xx, y=yy, label=label, labelpos=labelpos, 
			 color=color, style=style, size=size);
	    cc := dypl.get_dataset();
	    if (is_string(legend)) dypl.legend (legend, color=cc.color);
	    if (envelope) {                      # Plot the envelope, if required
		ename := spaste(name,'_max');
		private.plot_chisq(rr, ename, negate=T, style='dashed',
				   label=paste('min: <cos>=-1'));
	    }
	    return cc.color;

	} else if (name=='term_dM') {
	    s := paste(rr.label.dM2,': term that should be minimised');
	    return private.plot_chisq(rr,'dM2', legend=s);
	} else if (name=='term_chisq') {
	    s := paste(rr.label.chisq,': error in M =',rr.chisq_errM_pct,'%');
	    color := private.plot_chisq(rr,'chisq', legend=s);
	    dypl.plotxy (rr.xx[rr.chisq_imin], label=s, labelpos='none', 
			 style='dotted', color=color);
	    dypl.plotxy (rr.xx[rr.chisq_imin], rr.chisq_min,
			 style='star', color=color, size=5);
	    dypl.plotxy (rr.xx[rr.ix0], rr.chisq_min_nom, 
			 style='cross', color=color, size=5);
	    return color;
	} else if (name=='terms_N') {
	    if (rr.NSR==0) return F;
	    s := 'terms related to noise (N)';
	    private.plot_chisq(rr,'N2', legend=s);
	    return private.plot_chisq(rr,'dMN', envelope=T);
	} else if (name=='terms_C') {
	    if (rr.CSR==0) return F;
	    s := 'terms related to contaminating sources (C)';
	    private.plot_chisq(rr,'C2', legend=s);
	    return private.plot_chisq(rr,'dMC', envelope=T);
	} else if (name=='terms_NC') {
	    if ((rr.NSR*rr.CSR)==0) return F;
	    s := 'cross-term (N*C)';
	    return private.plot_chisq(rr,'NC', legend=s, envelope=T);
	} else if (name=='legend') {
	    if (!has_field(rr,'legend')) return F;
	    if (!is_string(rr.legend)) return F;
	    for (s in rr.legend) dypl.legend (s);
	} else if (name=='full') {
	    private.plot_chisq(rr,'legend');           # context-sensitive overall legend
	    private.plot_chisq(rr,'terms_N');          # Noise-related terms
	    private.plot_chisq(rr,'terms_NC');         # Cross-term (N*C)
	    private.plot_chisq(rr,'terms_C');          # Contaminating sources
	    private.plot_chisq(rr,'term_dM');          # The term that should be minimised
	    private.plot_chisq(rr,'term_chisq');       # Chi-squared (actually minimised)
	    dypl.labels(title=rr.title, xlabel=rr.xlabel, ylabel=rr.ylabel);
	} else {
	    print s,'name not recognised...??'; 
	    return F;
	}
	return T;
    }

# Input arguments for private.chisq_terms():

    private.inarg_chisq_terms := function (ref pp, hide_S=T, 
					   dM_well=T, dMratio=unset, 
					   N=T, NSratio=unset, 
					   C=T, CSratio=unset,
					   trace=F) {
	yapi := private.yapi;                    # convenience
	yapi.inarg (pp,'Source_Jy', [1.0], hide=hide_S,
		    help='flux of brightest source (to be peeled)');
	yapi.inarg (pp,'dMratio', [0.2], default=dMratio,
		    help='dM/Mratio (w.r.t. Source_Jy)');
	yapi.inarg (pp,'nvis', [1,10,100,1000,10000], 
		    help='nr of visibilities used in solution');
	if (dM_well) {
	    # Include arguements related to dM well-shape:
	    yapi.inarg (pp,'x_well', [0.0,0.1,0.2,0.3],
			help='position of the minimum in |dM(x)|');
	    yapi.inarg (pp,'d_well', [1.0,0.9,0.8,0.5],
			help='depth of the minimum in |dM(x)|');
	    yapi.inarg (pp,'w_well', [0.3,0.1,0.2,0.5,1,2],
			help='width of the minimum (dip) in |dM(x)|');
	}
	if (N) {
	    # Include noise-related arguements:
	    yapi.inarg (pp,'NSratio', [0.5,0.1,0.01,0.001,0.0,0.5,1,2,5],
			default=NSratio,
			help='noise/signal ratio (w.r.t. Source_Jy)');
	    yapi.inarg (pp,'cosfac_N', [-0.32,0,-0.1,-0.5,-0.9,-1,1],
			help='cosine-factor of cross-term dM*N (default=1/pi)');
	    yapi.inarg (pp,'noise', "gaussian log(gauss)",
			help='noise-statistics (supersedes cosfac_N)');
	}
	if (C) {
	    # Include contam-related arguements:
	    yapi.inarg (pp,'CSratio', [0,0.2,0.02], default=CSratio,
			help='relative flux of contaminating source (w.r.t. Source_Jy)');
	    yapi.inarg (pp,'cosfac_C', [0,-0.1,-0.5,-0.9,-1,1],
			help='cosine-factor of cross-term dM*C');
	}
	return T;
   }

# Helper function to calculate the various terms of chi-squared as a function of 
# the M.E. parameter value(s). These are combined into a single parameter x. 
# S_Jy is the actual flux of the 'peeling' source (M+dM=S)

# Nomenclature: small for complex, capitals for modulus:
# dM = |dm(x)|
#  C = |c|
#  N = |n|

    private.chisq_terms := function (S_Jy=1.0, 
				     N_Jy=0, cosfac_N=0, noise='gaussian',
				     C_Jy=0, cosfac_C=0,
				     x_well=0, d_well=1.0, w_well=0.3, 
				     nx2=50, ref pp=F, trace=F) {
	# Input arguments can be superseded by an argument record(pp):
	if (is_record(pp)) {
	    if (has_field(pp,'S_Jy')) S_Jy := pp.S_Jy;
	    if (has_field(pp,'NSratio')) N_Jy := pp.NSratio*S_Jy;
	    if (has_field(pp,'CSratio')) C_Jy := pp.CSratio*S_Jy;
	    if (has_field(pp,'N_Jy')) N_Jy := pp.N_Jy;
	    if (has_field(pp,'C_Jy')) C_Jy := pp.C_Jy;
	    if (has_field(pp,'noise')) noise := pp.noise;
	    if (has_field(pp,'cosfac_N')) cosfac_N := pp.cosfac_N;
	    if (has_field(pp,'cosfac_C')) cosfac_C := pp.cosfac_C;
	    if (has_field(pp,'x_well')) x_well := pp.x_well;
	    if (has_field(pp,'d_well')) d_well := pp.d_well;
	    if (has_field(pp,'w_well')) w_well := pp.w_well;
	    if (x_well!=0 && d_well==1) {
		# If the position of the minimum is not at x=0, 
		# its value should be >0: so modify d_well.
		d_well := (1-x_well);        # reduce the depth of the well 
		pp.d_well := d_well;         # make sure it gets into the legend
	    }
	    if (has_field(pp,'trace')) trace := pp.trace;
	}
	# Output record:
	rr := [S_Jy=S_Jy, 
	       N_Jy=N_Jy, NSR=N_Jy/S_Jy, cosfac_N=cosfac_N, noise=noise, 
	       C_Jy=C_Jy, CSR=C_Jy/S_Jy, cosfac_C=cosfac_C,
	       x_well=x_well, d_well=d_well, w_well=w_well, nx2=nx2];
	if (trace) print '** .chisq_terms(): input=',rr;

	# Calculate the shape y(x) of the dip-function |dM(x)| = yy(x) * S_Jy. 
        # The values of the M.E. parameter value(s) are 'correct' at x=0.
	# Assume that the model M of the brightest (peeling) source S is roughly correct,
	# i.e. within 10% or so. This means that the 'missing part' |dM| will approach
	# the actual flux S_Jy far from the optimum ('correct') M.E. parameter values.
	xx := ([-nx2:nx2]/nx2)^3;                 # (-1 <= x <= 1), denser sampling in the centre
	rr.ix0 := nx2+1;                          # index of x=0
	# print 'ix0=',rr.ix0,xx[rr.ix0];

	# If we are solving for the correct subset of M.E. parameters, 
	# the dip-function |dM(x)| will have a minimum of zero at x=0.
	# If we are solving for the wrong subset, the minimum value will be 
	# greater than zero (0<d_well<1), and the minimum will lie at x_well#0.
	yy := 1 - rr.d_well * exp(-((xx-rr.x_well)/rr.w_well)^2);

	# Make a secondary minimum to the right:
	x_secmin := 0.7;                          # position of secondary minimum 
	d_secmin := 0.2;                          # relative depth of sec.min.
	w_secmin := 0.2;                          # relative width of sec.min.
	yy -:= d_secmin*exp(-((xx-x_secmin)/w_secmin)^2);
	rr.ymin := min(yy);                       
	rr.iymin := ind(yy)[yy==rr.ymin][1];      # (first) index of ymin  

	# Make the function assymmetric to the left:
	ii := [1:rr.iymin];                       # left of the minimum
	yy[ii] := 1 - rr.d_well * exp(-((xx[ii]-rr.x_well)/(1.5*rr.w_well))^2);

	# Calculate the various terms of chi-squared:
	# All are averages <y> over all visibilities, i.e. <y> = sumi(yi)/nvis
	rr.N2 := rr.N_Jy^2;                       # <N*N>: real, positive, independent of x
	rr.C2 := rr.C_Jy^2;                       # <C*C>: real, positive, 'independent' of x
	#............................................................................
	if (trace) print '.chisq_terms():',rr;    # print before vectors are attached
	#............................................................................
	rr.xx := xx;
	rr.dM := rr.S_Jy * yy;                    # <|dm(x)|> = <dM(x)> real
	rr.M := rr.S_Jy - rr.dM;                  # <|m(x)|> = M(x)  real (M + dM = constant = S_Jy) 
	rr.dM2 := rr.dM * rr.dM;                  # <dM*dM>: f(x), real, positive, to be minimised

	# The following cross-terms are outer envelopes: they should be multiplied 
	# with a real cos-factor between -1 and 1 (see also private.calc_chisq() below);
	# NB: a*b+b*a = 2|a||b|cosfac, cosfac=cos(a,b), i.e. the angle between a and b
	rr.dMC_max := 2 * rr.dM * rr.C_Jy;        # outer envelope of <dM*C+C*dM>: real
	rr.dMN_max := 2 * rr.dM * rr.N_Jy;        # outer envelope of <dM*N+N*dM>: real
	rr.NC_max := 2 * rr.N_Jy * rr.C_Jy;       # <N*C+C*N>: real, independent of x

	# Plotting:
	rr.unit := 'Jy^2';
	rr.title := paste('The terms of chi-squared');
	rr.xlabel := paste('error (multi-dimensional) in the estimated M.E. parameter value(s)'); 
	rr.ylabel := spaste('chi-squared and its terms (',rr.unit,')');
	rr.color := [dM2='magenta', N2='green', NC='cyan', NC_max='cyan', C2='blue', 
		     chisq='red', chisq_min='red', chisq_min_nom='red',
		     dMC='blue', dMC_max='blue', dMN='green', dMN_max='green'];
	rr.label := [dM2='<dM*dM>', N2='<N*N>', NC='<N*C+C*N>', 
		     NC_max='NC_max',  C2='<C*C>', chisq='<chisq>',
		     chisq_min='minimum', chisq_min_nom='expected minimum',
		     dMC='<dM*C+C*dM>', dMC_max='dMC_max', 
		     dMN='<dM*N+N*dM>', dMN_max='dMN_max']; 

	# Calculate chisq itself from its terms:
	return private.calc_chisq(rr);
    }

#----------------------------------------------------------------------
# Calculation and analysis of chisq:
#----------------------------------------------------------------------
#
#   chisq := sumi(N*N + C*C + N*C + dM*dM + dM*N + dM*C);

#   The solution (M.E.parameter values) is affected only if the 
#   position of the minimum of chisq is shifted.
#   This is only possible if: 
#      sumi(dM*dM + 2*dM*(N|C)*cosi) <= 0
#   sumi is sum over nvis participating visibilities (i)
#   (N|C) is N or C. Both are real and positive
#   dM(x)=|dm(x)| is real (and positive) and can be divided out: 
#      sumi(dM) + 2*sumi((N|C)*cosi)) <= 0
#   also divide by nvis, we get the condition: 
#      dM(x) + 2*(N|C)*<cos>) <= 0
#   in which  <cos> = sumi(cosi)/nvis
#   and  <dM(x)> = sumi(dM(x))/nvis ~ dM(x), assuming dM is the same for all vis 
#   since dM and N and C are positive, ONLY a negative <cos> 
#   can cause the solution to be affected!!

# chisq has an extreme at (chain rule):
#   (2*dM(x) + 2*(N|C)*<cos>)*(dM(x)/dx) = 0 
# obviously, there will be a 'primary' min/max at (dM(x)/dx)=0
# but also wherever:  
#   dM(x) = - (N|C)*<cos>
# since dM, N and C are positive, there can only be a new
# minimum when <cos> is negative (!).
# Fill this into the chisq expression to find the value of the minimum.
# Take the sqrt() to get the under-estimation (Jy) of dM, 
# which is leads to the over-estimation of (the model) M.
#   the new minimum is lower by = (N|C)<cos> 
# This leads to (negative?) flux scattered through the map (??)
# Note that the value (but not the position) is independent of 
# the shape of M(x).

# So: given a certain noise distribution, what will be the
# scattered flux in the map after selfcal? In any case there
# will be a 'skew' effect, because only if <cos> negative
# Q: Discuss the special case of peeling! 

# Q: What can we say about the position of the new minimum?
# A: Nothing specific. However, the corresponding errors in
#    M.E. parameter values lead to overestimation of flux,
#    which is what affects the image (both in terms of too
#    much subtracted, as in under-estimation of the beam
#    gain that is used for predicting cat II sources.

# Q: What happens at very high noise levels?
# A: The dip disappears. So: keep NSR<1

# Q: Are the current stations calibratable? 
# A: station-core: 1 Jy source: SNR=3/vis in 10s and 4kHz
#    station-station (same source):
#    SNR *:= sqrt(1000/10)*sqrt(4/4)*sqrt(20000/1)*sqrt(1/25)
#    - 1000s (solving for polynomial coeff)
#    - 4 MHz (same)
#    - 20000 visibilities (100 stations)
#    - station-station (core is 25 stations)
#    So: SNR = (3/5) * sqrt(2) * 1000 = 1000/chisq
#    NB: The resulting over-estimation of M is INDEPENDENT of 
#        the nr of M.E. parameters that we try to solve for!   
#    So the original requirement (SNR=3/vis) was much too severe:
#    - for all vis (win sqrt(20000))
#    - for much longer times (win sqrt(100))
#    The latter relies on smoothness in time, and solving for coeff!

# Q: Tricky remaining problem: x_well#0 and/or d_well<1
# Q: Is x_well#0 possible at all? Under what conditions?
# Q: Happens if not solving for the correct subset of parms
# Q: NB: Is the 1D well-analysis valid for each M.E. parameter
# Q:     (dimension in MEP-space) independently? Cannot be right. 

# The effect of noise (N=Nrms=|n|): 
#   N = |n|: the rms noise flux of the chisq value
#   Ni: the rms noise flux of a single visibility (1s, 1kHz)
#   N := Ni / (sqrt(sec)*sqrt(kHz)*sqrt(nvis))
#   the error in the estimation of dM is N <cos_N> if cos<0
#   and zero if cos >=0. Since phi will vary randomly between
#   0 and 2pi, the average EFFECTIVE value of cos is -1/pi.
#   So: the under-estimation of dM due to noise is -N/pi.  
#   NB: for gaussian noise (always the case if visibilities)
#   non-gaussian if non-linear conversion, like amplitude (noise bias) 

# The effect of contaminating source(s) (C): 
#   C = |c| = C_Jy: the flux of a contaminating (point) source
#   Analysis is general: can be extended to a sum of point souces
#   solution affected if: 
#      dM(x) <= 2 C <cos_C>
#   <cos_C> is the synthesised beam response sbCM from C at M
#   sbCM = <cos_C> = sumi(cos(2pi(ui(lC-lM)+vi(mC-mM))))/nvis
#   NB: the solution is affected ONLY if sbCM is negative!
#----------------------------------------------------------------------

# Calculate chisq itself from its terms (in rr), and attach to rr:

    private.calc_chisq := function (rr=F, trace=F) {
	if (!is_record(rr)) rr := private.chisq_terms();  # just in case

	# The plot-legend (leg) and title (tit) are context-sensitive:
	leg := 'All terms are <averages> over nvis visibilities';
	tit := 'chisq:';

	# Calculate the actual values of dMN, dMC and NC by multiplying
	# with the relevant phase-factors.
	# For C, the phase factor depends on (u(lC-lM)+v(mC-mM)),
	# i.e. the projected distance between C and M.
	if (abs(rr.cosfac_C)>1) rr.cosfac_C := 0;         # just in case, message?
	rr.dMC := rr.cosfac_C * rr.dMC_max;
	if (abs(rr.cosfac_C)>0) {
	    rr.cosfac_C::print.precision := 2;
	    leg := [leg,paste(rr.label.dMC,': <cos_C> =',rr.cosfac_C)];
	}
	
	# For N, the phase factor is random if gaussian noise.
	# (non-gaussian if non-linear conversion like log): 
	if (abs(rr.cosfac_N)>1) rr.cosfac_N := 0;         # just in case, message?
	if (rr.noise=='log(gauss)') {
	    # <cos> = int(log(x)dx) from (1-a) to (1+a), |a|<1
	    a := min(0.99,rr.NSR);                        # what if N/S>1 ?? 
	    rr.cosfac_N := -2*a/(1-a*a);
	    if (rr.cosfac_N<-1) rr.cosfac_N := -1;        # just in case, message?
	    leg := [leg,paste(rr.noise,': N/S ratio =',rr.NSR,' (',a,')')];
	    tit := paste(tit,'non-gaussian noise');
	}
	rr.dMN := rr.cosfac_N * rr.dMN_max;
	if (abs(rr.cosfac_N)>0) {
	    rr.cosfac_N::print.precision := 2;
	    leg := [leg,paste(rr.label.dMN,': <cos_N> =',rr.cosfac_N)];
	}

	# Cross-term: cosfac_NC is derived from the other two: 
	# cos(a-b) = cos(a)cos(b)+sin(a)sin(b)
	sinfac_N := sqrt(1-(rr.cosfac_N^2));
	sinfac_C := sqrt(1-(rr.cosfac_C^2));
	rr.cosfac_NC := rr.cosfac_N*rr.cosfac_C + sinfac_N*sinfac_C;                 
	rr.NC := rr.cosfac_NC * rr.NC_max; 

	# The total chi-squared is the sum of the various terms:
	# NB: Positive, even though some terms can be negative.
	rr.chisq := rr.N2 + rr.C2 + rr.NC + rr.dM2 + rr.dMN + rr.dMC;
	rr.chisq_min := min(rr.chisq);              # minimum value
	rr.chisq_min::print.precision := 3;
	rr.chisq_imin := ind(rr.chisq)[rr.chisq==rr.chisq_min][1]; # index of minimum
	rr.solOK := (rr.chisq_imin==rr.ix0);          # T if solution OK 
	s := paste(rr.label.chisq,'=',rr.label.dM2);
	for (fname in "N2 C2 NC dMN dMC") {
	    if (sum(abs(rr[fname]))>0) s := paste (s,'+',rr.label[fname]);
	}
	leg := [leg,s];                               # append to legend
	s := paste(rr.label.chisq,': minimum =',rr.chisq_min);
	# s := paste(s,' (@ x =',rr.chisq_imin,rr.ix0,')');
	s := paste(s,' (@x =',rr.xx[rr.chisq_imin],')');
	leg := [leg,s];                               # append to legend

	# The nominal (expected) minimum value of chisq is <N*N>:
	rr.chisq_min_nom := rr.N2;
	rr.chisq_min_nom::print.precision := 3;
	s := paste(rr.label.chisq_min_nom,'=',rr.label.N2);
	s := paste(s,'=',rr.chisq_min_nom);
	if (rr.solOK) s := paste(s,'(OK)');
	leg := [leg,s];                               # append to legend

	# The solution is OK if the actual minimum of chisq lies at x=0 (?)
	# (but what if it does not have the nominal minimum value?)
	rr.chisq_errM_Jy := sqrt(abs(rr.chisq_min_nom - rr.chisq_min));
	# rr.chisq_errM_Jy +:= (1-rr.d_well)*rr.S_Jy;
	rr.chisq_errM_pct := 100*rr.chisq_errM_Jy/rr.S_Jy;
	rr.chisq_errM_pct::print.precision := 3;

	# Transfer the context-sensitive title and legend strings to rr:
	rr.legend := leg;
	rr.title := tit;
	# inspect(rr,'rr');
	return rr;
    }

#===============================================================================
# Processing requirements:

    public.calc_proc := function (trace=T) {
	rr := [=];
	rr.nstat := 100;                                  # nr of stations
	rr.nifr := 4 * 0.5 * rr.nstat*(rr.nstat+1);       # nr if ifrs
	rr.npeel := 20;                                   # nr of peeling sources
	rr.ncell := 100;                                  # nr of cells/domain
	rr.niter := 5;                                    # nr of iterations/solution
	rr.nequ := rr.ncell * rr.nifr;                    # nr of equations/solution
	rr.mqp := [=];                                    # nr of meqparms
	rr.mqp.source := 10;                              # - per peeling source
	rr.mqp.station := 10;                             # - per station/ per source
	rr.uk := rr.nstat * 2 + rr.nmq.source;            # nr of parms per source
	return rr;
    }


#=======================================================================
# Finished:

    private.init();
    return public;
} 

#************************************
# Run the script:
plot_MeqPeel();
#************************************



