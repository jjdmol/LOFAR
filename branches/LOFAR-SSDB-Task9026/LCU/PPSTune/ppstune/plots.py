r'''
Matplotlib-based plotting tools for ppstune log file analysis.
'''

from matplotlib.figure import Figure
from matplotlib.backends.backend_agg import FigureCanvasAgg
from pylab import array, floor, ceil, sqrt, figure
import os



class DiffFailureReport(object):
    def __init__(self, report_lines):
        self.header  = report_lines[0]
        self.station = self.header.split()[0].split('@')[1].strip()
        self.date    = self.header.split()[1]
        self.time    = self.header.split()[2]

        diff_failures = [[int(num)
                          for num in line.strip().split(':')[1].split()]
                         for line in report_lines[2:]]
        self.diff_failures = array(diff_failures)

        
    def __repr__(self):
        return 'DiffFailureReport %s %s %s\n%r' % (self.station, self.date, self.time, self.diff_failures)


    def plot(self, ax):
        r'''
        '''

        ax.set_title('%s (%s %s)' % (self.station, self.date, self.time))
        ax.set_xlabel('Antenna Processor ID')
        ax.set_ylabel('Delay step')
        ax.imshow(self.diff_failures, vmin = 0, interpolation='nearest')
        ax.grid()
        
        


def extract_diff_failure_reports(log_file_name):
    r'''
    '''

    lines   = open(log_file_name, 'r').readlines()
    reports = []

    collecting   = False
    report_lines = []
    for line in lines:
        if '*** Failure report' in line:
            collecting = True
            report_lines = [line]
        elif 'Delay/AP' in line and collecting:
            pass
        elif collecting and not 'ppstune' in line:
            report_lines.append(line)
        elif collecting:
            reports.append(report_lines)
            report_lines = []
            collecting   = False
        else:
            pass 
    return [DiffFailureReport(report) for report in reports]




def get_log_files(directory, exclude = []):
    r'''
    Return the list of pps-tuning log files stored in
    ``directory``. Exclude stations in the ``exclude`` list. Log files
    are assumed to be named ``pps-tuning-CS013.log``, etc.

    **Parameters**

    directory : string
        The directory in which to look for log files.

    exclude : list of strings
        Example ['cs001', 'rs508', 'fr606'].

    **Returns**

    A list of strings
    '''
    return [os.path.join(directory, file_name)
            for file_name in os.listdir(directory)
            if file_name[0:10] == 'pps-tuning' and file_name[-3:] == 'log' and
            file_name[11:16].lower() not in exclude]


def latest_result_page(log_file_names, index = -1, output_name = None):
    dpi = 50
    if output_name is None:
        fig = figure(figsize=(38, 24), dpi=dpi)
    else:
        fig = Figure(figsize=(38, 24), dpi=dpi)
    
    reports = [extract_diff_failure_reports(log_file)
               for log_file in sorted(log_file_names)]
    last_reports = [report[index] for report in reports if len(report)> (1+ abs(1+2*index))/2]
    columns = int(floor(sqrt(len(last_reports))*1.5))
    rows = int(ceil(len(last_reports)/float(columns)))

    for number, report in enumerate(last_reports):
        ax = fig.add_subplot(rows, columns, number+1)
        report.plot(ax)
    fig.subplots_adjust(left = 0.02, right  = 0.98,
                        top  = 0.98, bottom = 0.02,
                        hspace = 0.2)
    if output_name is not None:
        canvas = FigureCanvasAgg(fig)
        canvas.print_figure(output_name, dpi = dpi)
    


def row_plot(report_list, output_name = None):
    cols = len(report_list)
    dpi  = 50
    if output_name is None:
        fig = figure(figsize=(38, 24), dpi=dpi)
    else:
        fig = Figure(figsize=(38, 24), dpi=dpi)
    
    for i, report in enumerate(report_list):
        ax = fig.add_subplot(1, cols, 1+i)
        report.plot(ax)

    fig.subplots_adjust(left = 0.02, right  = 0.98,
                        top  = 0.98, bottom = 0.02,
                        hspace=0.2)
    if output_name is not None:
        canvas = FigureCanvasAgg(fig)
        canvas.print_figure(output_name, dpi = dpi)
    
