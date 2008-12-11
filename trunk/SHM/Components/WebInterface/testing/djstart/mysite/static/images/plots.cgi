import sys
sys.path.append("/home/lofartest/SHM/Components/JobScripts")
import scipy, numpy, time, logging
import xcstats_classifier_max
import rcu_settings_masks
import lofar.shm.db
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

def fetch_recent(db, start_epoch=datetime.datetime.now(), max_batch_size=1, si_id = 1010):

    #construct the query
    query = "SELECT si_id, subband, time, rcu_settings, acm_data, classification FROM Lofar.AntennaCorrelationMatrices WHERE "\
            "(si_id = %d) and (time < %s) ORDER BY time LIMIT %d;"%\
            (si_id, start_epoch.strftime("'%Y-%m-%d %H:%M:%S+00'"), max_batch_size)

    results = db.perform_query(query)

    batch_size = len(results)

    for inst in results:
        #logging.debug("si_id          <%(si_id)s>" % vars(inst))
        #logging.debug("subband        <%(subband)s>"     % vars(inst))
        #logging.debug("time           <%(time)s>"       % vars(inst))

        #N is the number of crossed dipoles = 1/2 the number of rcus
        N = int(scipy.sqrt(len(inst.acm_data)/2.)/2.)
        assert N in [16,48,96]
        
        xcm = xcstats_classifier_max.unflatten(inst.acm_data,N)
        rspstatus = rcu_settings_masks.rcu_status(inst.rcu_settings)

        #print len(xcm)
        
        #(classification, mean_power, peak_power) = sbstats_classifier_max.classify(spectrum)
        classification = xcstats_classifier_max.classify(xcm, rspstatus)
        
        #logging.debug("classification <%(classification)s>"%vars())
        
        #make an annotated plot

        # When I get all my site-lib modules working under python2.5 I can use datetime
        # to munge dates. until then use string fcns
        #tmptim = datetime.strptime(time,"%Y-%m-%d %H:%M:%S+00:00")
        #time_id = tmptim.strftime("%Y-%m-%d-%H:%M:%S")

        fig = Figure()
        ax = fig.add_axes([0.1, 0.1, 0.8, 0.8])
        ax.imshow(scipy.log(1.0e-3 + abs(xcm)), interpolation='nearest', origin='lower', cmap=cm.gray)
        ax.set_title("Station %s / Subband #%03d \n %s"% (inst.si_id, inst.subband[0], inst.time),family='sans-serif')
        #ax.set_ylabel('signal power (ADU)')
        #ax.set_xticks( (0, 127, 255, 383, 511) )
        #ax.set_xlim(0,511)
        #ax.set_ylim(1e4,1e12)
        if (classification == 'NOMINAL'):
            classcolor = 'green'
        else:
            classcolor = 'red'
        
        ax.text(0.5,0.5,"%s"%classification,
                fontsize=80,
                transform=ax.transAxes,
                horizontalalignment='center',
                verticalalignment='center',
                rotation='30',
                color=classcolor,
                alpha=0.5)
        #
        canvas = FigureCanvasAgg(fig)

        save_stdout = sys.stdout
        sys.stdout = cStringIO.StringIO()
        print "Content-type: image/png\n\n"
        canvas.print_png(sys.stdout, dpi=150)
        result = sys.stdout.getvalue()
        sys.stdout.close()
        sys.stdout = save_stdout
        return result 

db = lofar.shm.db.SysHealthDatabase()
db.open()

#print "entering"
fetch_recent(db)
print "foo!"