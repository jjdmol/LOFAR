import sys
sys.path.append("/home/avruch/work/lofar/shm/versions/Components/JobScripts")
import scipy, numpy, numarray, time
import xcstats_classifier_max_v2
import station_geom
#import rcu_settings_masks
import rcu_settings_masks_v2
import sbstats_classifier_max
import datetime
import mx.DateTime
# have to set backaeng to non0interactive before importing pylab
import  matplotlib
matplotlib.use('Agg')
#import pylab 
from matplotlib import rc
from matplotlib.backends.backend_agg import FigureCanvasAgg
from matplotlib.figure import Figure
from matplotlib.cbook import iterable
from matplotlib.colors import Colormap, normalize
from pylab import cm
import cStringIO
import ephem
import re
import lofar.shm.db

#######################################
# These two fcns should become db look-ups, or something else.
def name_to_si_id(name):
    db = lofar.shm.db.SysHealthDatabase()
    db.open()
    
    query = "SELECT * FROM Lofar.MacInformationServers WHERE si_name = '" + name.upper() + "';"
    results = db.perform_query(query)
    db.close()     

    assert len(results) == 1
    return results[0].si_id
    #return int(name[-3:])+1000

def si_id_to_name(id):
    db = lofar.shm.db.SysHealthDatabase()
    db.open()
    
    query = "SELECT * FROM Lofar.MacInformationServers WHERE si_id = " + str(id) + ";"
    results = db.perform_query(query)
    db.close()     

    assert len(results) == 1
    return results[0].si_name.upper()
    #return "CS%03d" % (id-1000)
########################################

def ang_separation(ra1,dec1,ra2,dec2):
    '''angular sep of coords in ra,dec
       in radians.  see Meeus p111'''
    hav_d = (1.0-cos(dec1-dec2))/2.0 + \
            cos(dec1)*cos(dec2) * (1.0-cos(ra1-ra2))
    return 2.0*arcsin(sqrt(hav_d))

def xst(dbinst, thumb=True):
    
    #N is the number of crossed dipoles = 1/2 the number of rcus
    N = int(scipy.sqrt(len(dbinst.acm_data)/2.)/2.)
    assert N in [16,32,48,96]
        
    xcm = xcstats_classifier_max_v2.unflatten(dbinst.acm_data,N)
    rspstatus = rcu_settings_masks_v2.rcu_status(dbinst.rcu_settings)

    on_rcus = rcu_settings_masks_v2.on_off_rcus(dbinst.rcu_settings)
    #check: zero data of OFF rcus
    #for i in range(len(on_rcus)):
    #    if not on_rcus[i]:
    #        xcm[i,:] = 0.0
    #        xcm[:,i] = 0.0

    #classification = xcstats_classifier_max.classify(xcm, rspstatus)
    # classification_text: just give OFF_NOMINAL RCUs
    if (type(dbinst.classification) == type([])):
        if dbinst.classification == ['NOMINAL'] * len(dbinst.classification):
            classification_text = 'NOMINAL'
        elif dbinst.classification == ['INVALID'] * len(dbinst.classification):
            classification_text = 'INVALID'
        elif dbinst.classification == ['DISABLED'] * len(dbinst.classification):
            classification_text = 'DISABLED'
        else:
            bad_rcus = []
            for i in range(len(dbinst.classification)):
                if dbinst.classification[i] != 'NOMINAL':
                    bad_rcus.append(i)
        
            classification_text = 'OFF_NOMINAL RCUs:\n'+ str.join(",", [("%d" % b) for b in bad_rcus])
    else:
        classification_text = "UNKNOWN"
        
    if thumb:
        fig = Figure(frameon=False, dpi=24)
    else:
        fig = Figure(frameon=False, dpi=96)


    ax = fig.add_axes([0.1, 0.1, 0.8, 0.8])

    #a bit of fooling around with labels
    tick_interval = N / 4
    tick_indices = range(tick_interval-1,2*N,tick_interval)
    tick_labels = [("%d"%(2*d))   for d in tick_indices[:len(tick_indices)/2]] + \
                  [("%d"%(2*(d-N)+1)) for d in tick_indices[len(tick_indices)/2:]]
    tick_indices = [0] + tick_indices
    tick_labels = ['0'] + tick_labels
    
    ax.set_xticks(tick_indices)
    ax.set_xticklabels(tick_labels)
    ax.set_yticks(tick_indices)
    ax.set_yticklabels(tick_labels)
    
    ax.imshow(scipy.log(1.0e-3 + abs(xcm)), interpolation='nearest', origin='lower', cmap=cm.gray)
    ax.set_title("Station %s / Subband #%03d \n %s"% (si_id_to_name(dbinst.si_id), dbinst.subband[0], dbinst.time),family='sans-serif')

    ax.set_xlabel("RCU")
    ax.set_ylabel("RCU")
    if (classification_text == 'NOMINAL'):
        classcolor = 'green'
    else:
        classcolor = 'red'

    ax.text(0.5,0.5,"%s"%classification_text,
            fontsize=40,
            transform=ax.transAxes,
            horizontalalignment='center',
            verticalalignment='center',
            rotation='30',
            color=classcolor,
            alpha=0.5)
    canvas = FigureCanvasAgg(fig)
    output = cStringIO.StringIO()
    #response['Content-Disposition'] = 'attachment; filename=somefilename.pdf'
    
    #output.write("Content-Type: image/png\n\n")
    canvas.print_png(output, dpi=150)
    
    png = output.getvalue()
    output.close()
    return png

def sky(dbinst, thumb=True):

    status = rcu_settings_masks_v2.rcu_status(dbinst.rcu_settings)
    #KLUDGE this to force LBA antenna positions
    #only the first one (elem 0) is checked in array_geom
    #(all db entries are in HBA mode?)
    #status[0]["BANDSEL"] = 1

    #N is the number of crossed dipoles = 1/2 the number of rcus
    N = int(scipy.sqrt(len(dbinst.acm_data)/2.)/2.)
    assert N in [16,32,48,96]
        
    xcm = xcstats_classifier_max_v2.unflatten(dbinst.acm_data,N)
    #[polar,cart] = xcstats_classifier_max_v2.array_geom(dbinst.si_id, status, dbinst.time)
    if ((len(dbinst.geo_loc) == 1) or (len(dbinst.ant_coord) == 1)):
        [polar,cart] = xcstats_classifier_max_v2.array_geom(dbinst.si_id, dbinst.rcu_settings, dbinst.time)
    else:
        [polar,cart] = station_geom.Locations(dbinst.geo_loc, dbinst.ant_coord)
        
    on_rcus = rcu_settings_masks_v2.on_off_rcus(dbinst.rcu_settings)

    #now I can grid the data onto u,v meters
    # First , I have to convert from coords measured for LOFAR field to coords in geo equatorial plane
    # Acc. to (my understanding of) discussion with Stefan Wijnholds, station correlators center phase
    # at zenith in both LBA & HBA modes. Thus, HourAngle = 0, declination = station latitude
    # and w term is 0.
    dec = polar[1]
    loc_to_geocen = scipy.asarray([ [   0.0 , -numpy.sin(dec), numpy.cos(dec)],
                                    [   1.0 ,    0.0   ,   0.0   ],
                                    [   0.0 ,  numpy.cos(dec), numpy.sin(dec)] ])
    #A is rot from Geocentric to uvw
    A = scipy.asarray([ [      0.0,  1.0,      0.0],
                        [-numpy.sin(dec),  0.0, numpy.cos(dec)],
                        [ numpy.cos(dec),  0.0, numpy.sin(dec)] ])

    #B is for baselines
    B = scipy.zeros((xcm.shape[0],xcm.shape[1],2),dtype=float)

    #these are masks for polarizations
    XXidx = scipy.zeros((xcm.shape[0],xcm.shape[1]),dtype=float)
    YYidx = scipy.zeros((xcm.shape[0],xcm.shape[1]),dtype=float)
    XYidx = scipy.zeros((xcm.shape[0],xcm.shape[1]),dtype=float)
    YXidx = scipy.zeros((xcm.shape[0],xcm.shape[1]),dtype=float)

    for row in range(0,N):
        for col in range(0,N):
            #XX-pol correlation
            B[row,col,0] = cart[row][0] - cart[col][0]
            B[row,col,1] = cart[row][1] - cart[col][1]
            XXidx[row,col] = 1
        
            #YY-pol
            B[row+N,col+N,0] = cart[row][3] - cart[col][3]
            B[row+N,col+N,1] = cart[row][4] - cart[col][4]
            YYidx[row+N,col+N] = 1
            
            #XY-pol
            B[row,col+N,0] = cart[row][0] - cart[col][3]
            B[row,col+N,1] = cart[row][1] - cart[col][4]
            XYidx[row,col+N] = 1
        
            #YX-pol
            B[row+N,col,0] = cart[row][3] - cart[col][0]
            B[row+N,col,1] = cart[row][4] - cart[col][1]
            YXidx[row+N,col] = 1

    #getting obs freq: I'm dubious of this method.
    # if in any of rcumodes 1-4 => NYQ zone 1, clock 200MHz (though it would be nice to have clock in rcustats)
    # mask for those modes is 0x17900 | 0x57900 | 0x37a00 | 0x77a00 = 0x77b00

    #if in mode 5, NYQ 2, clk 200MHz
    # mask is 0x7a400

    #if in mode 6, NYQ 3 clk 160
    #mask is 0x79400

    #if in mode 7, NYQ 3 clk 200
    #mask 0x78400

    for i in range(2*N):
        freq = xcstats_classifier_max_v2.rcu_freq(status[i],dbinst.subband[0])
        if (freq == None):
            # serious error
            # assert False
            break
        else:
            if (freq == 'NA'):
                continue
            else:
                break

    if ((freq == 'NA') or (freq == None)):
        # make a blank skymap with annotations
        if (freq == 'NA'):
            classification_text = 'All RCUs and Antennae OFF'
        else:
            classification_text = 'ERROR: unknown obs. frequency'
        dang = 2.0/128.0
        M = scipy.arange(-1.0+dang/2.0 , 1.0 ,dang)
        L = scipy.arange(-1.0+dang/2.0 , 1.0 ,dang)
        sky = scipy.zeros((L.shape[0],M.shape[0]),dtype=float)        
    else:
        assert freq
        wavelen = 2.9979245e8/(freq*1.0e6)

        # now transform to uvw
        # I expect all w's to be zero (they are numerically small)
        UVW = scipy.zeros((xcm.shape[0],xcm.shape[1],3),dtype=float)
        for row in range(0,UVW.shape[0]):
            for col in range(0,UVW.shape[1]):
                XYZ = scipy.dot(loc_to_geocen, [B[row,col,0],B[row,col,1], 0.0])
                # uvw[row,col] = scipy.dot(A,[B[row,col,0],B[row,col,1],0])
                UVW[row,col] = scipy.dot(A,[XYZ[0],XYZ[1],XYZ[2]])

        b = UVW[scipy.where(XXidx)]
        bx = b[:,0]/wavelen
        by = b[:,1]/wavelen

        #check: zero data of OFF rcus
        for i in range(len(on_rcus)):
            if not on_rcus[i]:
                xcm[i,:] = 0.0
                xcm[:,i] = 0.0
        
        # remove zero-spacing fluxA
        xcm[range(xcm.shape[0]),range(xcm.shape[0])] = complex(0)
        vXX = xcm[scipy.where(XXidx)]
        vYY = xcm[scipy.where(YYidx)]
    
        # pick sky resolution based on array geometry; about 3 pix per synth beam
        longest_baseline = scipy.sqrt(bx*bx + by*by).max()
        da = 1.0 / 3.0 / longest_baseline
        # now pick for closest power-of-two mapsize
        # in case I want to do an FFT someday
        Ns = 2.0/da
        if Ns < 64:
            Ns = 64
        else:
            if Ns < 128:
                Ns = 128
            else:
                Ns = 128
    #            Ns = 256
    #            if Ns < 256:
    #                Ns = 256
    #            else:
    #                Ns = 512
        dang = 2.0/Ns
    
        # grid the sky
        M = scipy.arange(-1.0+dang/2.0 , 1.0 ,dang)
        L = scipy.arange(-1.0+dang/2.0 , 1.0 ,dang)
    
        # make the sky map
        IXXd = scipy.zeros((L.shape[0],M.shape[0]),dtype=complex)
        IYYd = scipy.zeros((L.shape[0],M.shape[0]),dtype=complex)
        k = -2.0*scipy.pi

        # kludge for spectral inversion; assume it's on
        if (freq >= 100.)  and (freq <= 200.):
            k = -k
            
        wx = scipy.exp(complex(0,k)*scipy.outer(bx,L))
        wy = scipy.exp(complex(0,k)*scipy.outer(by,M))
        for lidx in range(len(L)):
            for midx in range(len(M)):
                if (L[lidx]**2 + M[midx]**2) < 1:
                    wt = wx[:,lidx] * wy[:,midx]
                    IXXd[lidx,midx] = scipy.inner(vXX,wt) * scipy.sqrt(1.0 - L[lidx]*L[lidx] - M[midx]*M[midx])
                    IYYd[lidx,midx] = scipy.inner(vYY,wt) * scipy.sqrt(1.0 - L[lidx]*L[lidx] - M[midx]*M[midx])
        # for the time being, return only one image, sum of the two parallel pols
        sky = scipy.fliplr(scipy.rot90(IYYd.real+IXXd.real))      
    
        #classification text
        if (type(dbinst.classification) == type([])):
            if dbinst.classification == ['NOMINAL'] * len(dbinst.classification):
                classification_text = 'NOMINAL'
            elif dbinst.classification == ['INVALID'] * len(dbinst.classification):
                classification_text = 'INVALID'
            else:
                bad_rcus = []
                for i in range(len(dbinst.classification)):
                    if dbinst.classification[i] != 'NOMINAL':
                        bad_rcus.append(i)
            
                classification_text = 'OFF_NOMINAL RCUs:\n'+ str.join(",", [("%d" % b) for b in bad_rcus])
        else:
            classification_text = "UNKNOWN"

    # some ephemeris objects for plot annotation
    # observer (=station) location
    observer = ephem.Observer()
    observer.long = polar[0]*scipy.pi/180.
    observer.lat = polar[1]*scipy.pi/180.
    observer.elev = polar[2]
    #observer.date = datetime.datetime.strftime(dbinst.time,"%Y/%m/%d %H:%M:%S")
    #kludge -- pypehem wants all dates in UTC --
    observer.date = dbinst.time - dbinst.time.utcoffset()
    
    # interesting objects
    cas_a = ephem.readdb("Cas-A,f|J,23:23:26,58:48:00,2000")
    cyg_a = ephem.readdb("Cyg-A,f|J,19:59:28.4,40:44:02.1,2000")
    tau_a = ephem.readdb("Tau-A,f|J,05:34:31.97,22:00:52.1,2000")
    sun = ephem.Sun()
    moon = ephem.Moon()
    jup = ephem.Jupiter()
    
    cas_a.compute(observer)
    cyg_a.compute(observer)
    tau_a.compute(observer)
    sun.compute(observer)
    moon.compute(observer)
    jup.compute(observer)
    
    m_cas = numpy.cos(cas_a.alt)*numpy.cos(cas_a.az)
    l_cas = numpy.cos(cas_a.alt)*numpy.sin(cas_a.az)
    m_cyg = numpy.cos(cyg_a.alt)*numpy.cos(cyg_a.az)
    l_cyg = numpy.cos(cyg_a.alt)*numpy.sin(cyg_a.az)
    m_tau = numpy.cos(tau_a.alt)*numpy.cos(tau_a.az)
    l_tau = numpy.cos(tau_a.alt)*numpy.sin(tau_a.az)
    m_sun = numpy.cos(sun.alt)*numpy.cos(sun.az)
    l_sun = numpy.cos(sun.alt)*numpy.sin(sun.az)
    m_moon = numpy.cos(moon.alt)*numpy.cos(moon.az)
    l_moon = numpy.cos(moon.alt)*numpy.sin(moon.az)
    m_jup = numpy.cos(jup.alt)*numpy.cos(jup.az)
    l_jup = numpy.cos(jup.alt)*numpy.sin(jup.az)

    # Galactic plane locus
    m_gal = []
    l_gal = []
    for gallat in range(0,360,3):
        anticen = ephem.Galactic(str(gallat),'0')
        galplan = ephem.readdb("GP,f,%s,%s,2000" % (str(anticen.to_radec()[0]), str(anticen.to_radec()[1])))
        galplan.compute(observer)
        # check if above point is above horizon
        if galplan.alt > 0:
            m_gal.append(numpy.cos(galplan.alt)*numpy.cos(galplan.az))
            l_gal.append(numpy.cos(galplan.alt)*numpy.sin(galplan.az))

    if (classification_text == 'NOMINAL'):
        classcolor = 'green'
    else:
        classcolor = 'red'


    # make the plot
    if thumb:
        fig = Figure(frameon=False, dpi=24)
    else:
        fig = Figure(frameon=False, dpi=96)

    ax = fig.add_axes([0.1, 0.1, 0.8, 0.8])
    ax.imshow(sky,
              interpolation='nearest',
              cmap=cm.gray,
              extent=[L[-1],L[0],M[0],M[-1]])
    if (type(freq) == float):
        ax.set_title("Station %s / Subband #%03d (%6.2f MHz)\n %s"% (si_id_to_name(dbinst.si_id), dbinst.subband[0], freq, dbinst.time),
                     family='sans-serif')
    else:
        ax.set_title("Station %s / Subband #%03d\n %s"% (si_id_to_name(dbinst.si_id), dbinst.subband[0], dbinst.time), family='sans-serif')
    # annotate
    if sun.alt > 0:
        #ax.plot([l_sun],[m_sun],'yo')
        #ax.text(l_sun,m_sun,'SUN',horizontalalignment = 'center',verticalalignment='bottom',color='yellow')
        ax.text(l_sun,m_sun,'SUN',horizontalalignment = 'center',verticalalignment='center',color='yellow',fontsize=8)
    if jup.alt > 0:
        #ax.plot([l_sun],[m_sun],'yo')
        #ax.text(l_sun,m_sun,'SUN',horizontalalignment = 'center',verticalalignment='bottom',color='yellow')
        ax.text(l_jup,m_jup,'JUP',horizontalalignment = 'center',verticalalignment='center',color='yellow',fontsize=6)
    if moon.alt > 0:
        #ax.plot([l_sun],[m_sun],'yo')
        #ax.text(l_sun,m_sun,'SUN',horizontalalignment = 'center',verticalalignment='bottom',color='yellow')
        ax.text(l_moon,m_moon,'MOON',horizontalalignment = 'center',verticalalignment='center',color='yellow',fontsize=6)
    if cas_a.alt > 0:
        #ax.plot([l_cas],[m_cas],'co')
        #ax.text(l_cas,m_cas,'CAS A',horizontalalignment = 'left',verticalalignment='top',color='cyan')
        ax.text(l_cas,m_cas,'CAS A',horizontalalignment = 'center',verticalalignment='center',color='cyan',fontsize=10)
    if cyg_a.alt > 0:
        #ax.plot([l_cyg],[m_cyg],'co')
        #ax.text(l_cyg,m_cyg,'CYG A',horizontalalignment = 'left',verticalalignment='top',color='cyan')
        ax.text(l_cyg,m_cyg,'CYG A',horizontalalignment = 'center',verticalalignment='center',color='cyan',fontsize=10)
    if tau_a.alt > 0:
        ax.text(l_tau,m_tau,'TAU A',horizontalalignment = 'center',verticalalignment='center',color='cyan',fontsize=8)

    ax.plot(l_gal, m_gal, 'c,')
    ax.set_xlabel('$l$', fontsize=16)
    ax.set_ylabel('$m$', fontsize=16, rotation=0)
    ax.text(-0.65,-0.9,"E",
            fontsize=16,
            horizontalalignment='center',
            verticalalignment='center',
            color='white')
    ax.text(-0.9,-0.63,"N",
            fontsize=16,
            horizontalalignment='center',
            verticalalignment='center',
            color='white')
    ax.text(0.0,0.0,"%s"% classification_text,
            fontsize=40,
            horizontalalignment='center',
            verticalalignment='center',
            rotation='30',
            color=classcolor,
            alpha=0.3)
    ax.quiver([-0.9,-0.9],[-0.9,-0.9],[-.090,0],[0,.090],
              units='width',
              width=0.005,
              scale = 1.0,
              pivot='tail',
              color='white')

    canvas = FigureCanvasAgg(fig)
    output = cStringIO.StringIO()
    canvas.print_png(output, dpi=150)
    png = output.getvalue()
    output.close()
    return png

def sst(dbinst, thumb=True):

    output = cStringIO.StringIO()

    # spectrum = numarray.array(map(float, dbinst.spectrum))
    spectrum = numpy.asarray(dbinst.spectrum,dtype=float)
    #(classification, mean_power, peak_power, med_chan) = sbstats_classifier_max.classify(spectrum)
    #classification_string = '\'{' + str.join(",", classification) + '}\''
    
    #make an annotated plot
    #set rcparams as I like them
    #matplotlib.rcParams['interactive'] = 'False'
    #matplotlib.rcParams['axes_hold']   = 'False'
    #matplotlib.rcParams['backend']     = 'Agg'
    #matplotlib.rcParams['font.family'] = 'sans-serif'
    
    # When I get all my site-lib modules working under python2.5 I can use datetime
    # to munge dates. until then use string fcns
    #tmptim = datetime.strptime(time,"%Y-%m-%d %H:%M:%S+00:00")
    #time_id = tmptim.strftime("%Y-%m-%d-%H:%M:%S")
        
    if thumb:
        fig = Figure(frameon=False, dpi=24)
    else:
        fig = Figure(frameon=False, dpi=96)

    ax = fig.add_axes([0.1, 0.1, 0.8, 0.8])
    ax.set_title("Station %s / RCU #%02d \n %s"%(si_id_to_name(dbinst.si_id), dbinst.rcu_id, dbinst.time))
    ax.set_ylabel('signal power (ADU)')
    if numpy.any(numpy.where(spectrum <= 0)):
        line,  = ax.plot(range(0,512), spectrum, 'b-', linewidth=0.5)
        ax.set_xticks( (0, 127, 255, 383, 511) )
        ax.set_xlim(0,511)
        yspan = numpy.array([spectrum.max() - spectrum.min(), 10.0]).max()
        ax.set_ylim(spectrum.min() - 0.1*yspan,spectrum.max() + 0.1*yspan)
    else:
        line,  = ax.semilogy(range(0,512), spectrum, 'b-', linewidth=0.5)
        ax.set_xticks( (0, 127, 255, 383, 511) )
        ax.set_xlim(0,511)
        ax.set_ylim(1e4,1e12)
    #kludgey
    if (type(dbinst.classification) != type([])):
        classification_text = "UNKNOWN"
        classification_color = 'black'
    else:
        if ((re.match("NOMINAL",dbinst.classification[0])) or
            (re.match("ANTENNA_OFF",dbinst.classification[0]))):
            classification_color = 'green'
        else:
            classification_color = 'red'
        classification_text = "%s" % ",".join(dbinst.classification)
            
    #ax.text(0.5,0.5,"%s" % ",".join(dbinst.classification),
    ax.text(0.5,0.5, classification_text,
            fontsize=24,
            transform=ax.transAxes,
            horizontalalignment='center',
            verticalalignment='center',
            rotation='30',
            color=classification_color,
            alpha=0.5)
                
    #for those who care: the RCU config byte
    ax.text(0.5,0.08, "x%08x"%dbinst.rcu_settings,
            fontsize=24,
            transform=ax.transAxes,
            horizontalalignment='center',
            verticalalignment='center',
            rotation='0',
            color='black',
            alpha=1.0)
    
    if thumb:
        ax.text(0.5,0.8,"RCU %02d" % dbinst.rcu_id,
                fontsize=112, 
                transform=ax.transAxes,
                horizontalalignment='center',
                verticalalignment='center',
                rotation='0',
                color='blue',
                alpha=0.3)

    canvas = FigureCanvasAgg(fig)
    #canvas.print_figure(filename, dpi=150)
    canvas.print_png(output)
    
    png = output.getvalue()
    output.close()
    return png

def rsp(dbinst):
    """
    present rspstatus values formatted in HTML, ready for display
    """
    
    output = cStringIO.StringIO()
#    output.write('<table class="rspstat">\n')
#    output.write('<tr>')
#    output.write('<td>%s  RSP#%02d  %s</td>' % (si_id_to_name(dbinst.si_id),
#                                                dbinst.rsp_id, dbinst.time))
#    output.write('</tr>')
#    if dbinst.classification:
#        for problem in dbinst.classification:
#            output.write('<tr><td>classification: %s</td></tr>\n' % problem)
#    
#    output.write('<tr>\n')
#    output.write('<td>1.2V: %4.2f</td><td>2.5V: %4.2f</td><td>3.3V: %4.2f</td>\n' %
#                 (dbinst.rsp_board_volts[0], dbinst.rsp_board_volts[1], dbinst.rsp_board_volts[2]))
#    output.write('</tr>\n')
#    output.write('</table>')

    output.write('RSP#%02d %s\n' % (dbinst.rsp_id, dbinst.time))
    if dbinst.classification:
        output.write('\nclassification: ')
        for problem in dbinst.classification:
            output.write('%s, ' % problem)
        output.write('\n')
    output.write('\nVolts:  1.2V: %4.2f   2.5V: %4.2f   3.3V: %4.2f\n' %
                 (dbinst.rsp_board_volts[0], dbinst.rsp_board_volts[1], dbinst.rsp_board_volts[2]))
    output.write('Temps:  PCB:%3d   BP:%3d   AP0:%3d   AP1:%3d   AP2:%3d   AP3:%3d\n' %
                 (dbinst.rsp_board_temps[0], dbinst.rsp_board_temps[1], dbinst.rsp_board_temps[2],
                  dbinst.rsp_board_temps[3], dbinst.rsp_board_temps[4], dbinst.rsp_board_temps[5]))
    
    output.write('RCU PLL  X: %d %d %d %d\n' % (dbinst.rcu_pllx[0],
                                                dbinst.rcu_pllx[1],
                                                dbinst.rcu_pllx[2],
                                                dbinst.rcu_pllx[3]))
    output.write('RCU PLL  Y: %d %d %d %d\n' % (dbinst.rcu_plly[0],
                                                dbinst.rcu_plly[1],
                                                dbinst.rcu_plly[2],
                                                dbinst.rcu_plly[3]))
    output.write('RCU OVFL X: %d %d %d %d\n' % (dbinst.rcu_num_overflow_x[0],
                                                dbinst.rcu_num_overflow_x[1],
                                                dbinst.rcu_num_overflow_x[2],
                                                dbinst.rcu_num_overflow_x[3]))
    output.write('RCU OVFL Y: %d %d %d %d\n' % (dbinst.rcu_num_overflow_y[0],
                                                dbinst.rcu_num_overflow_y[1],
                                                dbinst.rcu_num_overflow_y[2],
                                                dbinst.rcu_num_overflow_y[3]))
    output.write('ADC OFFS X: %+d %+d %+d %+d\n' % (dbinst.ado_adc_offset_x[0],
                                                    dbinst.ado_adc_offset_x[1],
                                                    dbinst.ado_adc_offset_x[2],
                                                    dbinst.ado_adc_offset_x[3]))
    output.write('ADC OFFS Y: %+d %+d %+d %+d\n' % (dbinst.ado_adc_offset_y[0],
                                                    dbinst.ado_adc_offset_y[1],
                                                    dbinst.ado_adc_offset_y[2],
                                                    dbinst.ado_adc_offset_y[3]))

    result = output.getvalue()
    output.close()
    return result

