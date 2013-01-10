#!/usr/bin/env python
"""
This script creates usage visualization for the CEP2 Cluster
It depends on statistics calculated by teun Grit.

It read datafiles in the /tmp directory. Converts and cleans the data.
It then writes images containing a heatmap and average plot for the last day
and the last 7 days.

The images are created for the CPU, the diskIO and the swapfile usages.

2013
Wouter Klijn (klijn@astron.nl)
original implementation 
Teun Grit  (grit@astron.nl)

13-01-2013: Incorrect datapoint (eg. missing entrie for single node) led to
a incorrect number of time points: The number not being a multiple of a day 
length. Add the average of the remaining data as best effort estimation in that
timeslice: Leads to correct positioning of the axis labels
Add support from printing images of day averages
"""
import matplotlib
matplotlib.use("Agg")
import numpy as np
from pylab import *
from datetime import *

# A global current time variable to allow the same axis for all figures!
current_time = datetime.utcnow().replace(second=0, microsecond=0)

def load_data(file):
    """
    Open file at supplied path and return the lines a list of strings
    """
    f = open(file, 'r')
    data = f.readlines()
    return data

def clean_data(data, n_nodes=100):
    """
    converts the list of raw lines containing the usage data to a list
    of float list containing the converted data, minus the timestamp
    Return the cleaned data and the dimensions
    """

    cleaned_data = []
    for line in data:
        # split in 'words'
        line_entries = line.split()

        # len should be n_nodes plus two timestamps
        if len(line_entries) == n_nodes + 2:
            clean_line = []

            # skip the timestamps convert to float with max 100.0
            for entrie in line_entries[2:]:
                clean_line.append(min(float(entrie), 100.0))


            # add to data list
            cleaned_data.append(clean_line)
        else:

            if len(line_entries) < 3:
                continue
            print "Incorrectly formatted line found. Adding average of known data"
            print "time stamp missing data: " + str(line_entries[0])
            clean_line = []
            for entrie in line_entries[2:]:
                try:
                    clean_line.append(min(float(entrie), 100.0))
                except:
                    pass # skip parsing this line item


            mean_data_found = mean(clean_line)
            mean_line = [mean_data_found] * 100
            cleaned_data.append(mean_line)

    n_datapoints = len(cleaned_data)        # rows
    n_nodes_data = len(cleaned_data[0])     # cols  
    return cleaned_data, n_datapoints, n_nodes_data

def fill_z_array(clean_data, n_datapoints, n_nodes_data):
    """
    Create a ndarray from the cleaned data ready for entering it in an figure
    """
    z_array = np.ndarray((n_nodes_data, n_datapoints))
    # loop the datapoint
    for idx_data_point in range(n_datapoints):
        # loop the nodes
        for idx_node in range(n_nodes_data):
            # Swap the data indexes 
            # TODO: use z_array = matrix.transpose(clean_data) ?
            z_array[idx_node, idx_data_point] = \
               clean_data[idx_data_point][idx_node]

    return z_array

def calculate_average_on_managed_nodes(clean_data, n_datapoints, n_nodes_data,
        largest_managed_idx_in_data):
    """
    calute for each timestep the average load on the managed nodes
    Return a list of values for the whole time range
    """
    # loop the datapoint
    averaged_values = []
    for idx_data_point in range(n_datapoints):
        # loop the nodes
        average_for_node = 0.0
        for idx_node in range(n_nodes_data):
            # Swap the data indexes 
            if idx_node <= largest_managed_idx_in_data:
                average_for_node += clean_data[idx_data_point][idx_node]
        averaged_values.append(average_for_node /
                               (largest_managed_idx_in_data + 1))

    return averaged_values

def get_tick_labels_from_ntimesteps(n_rows):
    # each row is a 5 min step, If nrow > as 12 * 24 = 288
    tick_location = []
    tick_labels = []
    if n_rows == 288: # If the input is a day
        # Add the current date and time minus 24 hours
        tick_location.append(0)
        tick_labels.append(
            str((current_time - timedelta(days=1)).time())[:-3])
        #for each two hours (5 minutes ticks)
        for idx_hour in range(1, n_rows / 24):
            tick_location.append(idx_hour * 24)  # add the tick location
            tick_labels.append(str(# str representation
                (current_time - # of the current time
                 timedelta(minutes=(24 - idx_hour * 2) * 60)).time()
                                   )[:-3]
                               )   # Minus the time from the end of the range
        # add the current time
        tick_location.append(n_rows)
        tick_labels.append("       " + str(current_time)[5:-3])
    elif n_rows == 2016: # 7day plot
        # Add the current date and time minus 7 days
        n_ticks_from_midnight = (current_time.time().minute +
                                   current_time.time().hour * 60) / 5
        tick_location.append(0)
        tick_labels.append(str((current_time - timedelta(days=7)).time())[:-3])
        #for each day
        for idx_day in range(1, n_rows / 288):
            tick_location.append(idx_day * 288 -
                n_ticks_from_midnight)  # add the tick location at midnight location
            tick_labels.append(str(# str representation
                (current_time - # of the current time
                timedelta(days=7 - idx_day)
                                   ).date())[5:]
                               )   # Minus the time from the end of the range
        # add the current time
        tick_location.append(n_rows)
        tick_labels.append("      " + str(current_time)[5:-3])
    else: # all time plot
        # Add the current date and time minus 7 days
        start_date = current_time - timedelta(days=n_rows)
        start_day = n_rows
        n_label_steps = 5 # adapt this when 
        label_step = floor(n_rows / n_label_steps)
        tick_location.append(0)
        tick_labels.append(str((start_date).date()))
        #for each day
        for idx in range(1, n_label_steps):
            step_index = idx * label_step
            tick_location.append(step_index)  # add the tick location at midnight location
            tick_labels.append(str(# str representation
                (start_date + timedelta(days=idx * label_step)).date())[5:])
                                  # Minus the time from the end of the range
        # add the current time
        tick_location.append(n_rows)
        tick_labels.append(str((current_time).date()))
    return tick_location, tick_labels

def write_an_overview_image_to_file(z_data, path,
                                   n_rows, n_cols, fig_size,
                                   title_str, xlabel_str, ylabel_str, cb_label,
                                   dpi=120):
    """
    create and write a fully formatted map of the whole cluster usage to file
    """
    figure(figsize=fig_size, dpi=dpi)
    ax = gca()
    p = pcolor(z_data)
    ax.yaxis.set_major_locator(MaxNLocator(20))
    axis([0, n_rows, 0, n_cols])
    date = str(datetime.utcnow())[:16]
    title(title_str + date)
    xlabel(xlabel_str)
    ylabel(ylabel_str)
    tick_location, tick_labels = get_tick_labels_from_ntimesteps(n_rows)
    xticks(tick_location, tick_labels)
    # Change the width of the tickmarks
    for l in ax.get_xticklines():
        l.set_markersize(6)
        l.set_markeredgewidth(1.2)
        l.set_color("white")


    cb = colorbar()
    cb.set_label(cb_label)
    savefig(path, format='png')


def write_an_average_image_to_file(averaged_data_range, path,
                                  n_rows, n_cols, fig_size,
                                  title_str, xlabel_str, ylabel_str,
                                  dpi=120

                                  ):
    """
    Create and save an image
    """
    fig = figure(figsize=fig_size, dpi=dpi)
    ax = fig.add_subplot(111)


    ax.plot(averaged_data_range)
    ax.plot([mean(averaged_data_range)] * len(averaged_data_range))
    ax.legend(("5 Min average", "Mean time range"), loc=0)
    ax.set_xlim([0, n_rows])
    ax.set_ylim([0, max(averaged_data_range) + 2])
    date_str = datetime.utcnow()
    title(title_str + str(date_str)[0:16])
    tick_location, tick_labels = get_tick_labels_from_ntimesteps(n_rows)
    xticks(tick_location, tick_labels)
    xlabel(xlabel_str)
    ylabel(ylabel_str)

    savefig(path, format='png')


def calculate_and_write_metric(data_path, target_path, metric_str, duration_str,
                               n_nodes=100, dpi=120,
                               largest_managed_idx_in_data=94): # this means that 0 untill 94 are summed
    """
    Main functional unit:
    load data
    Clean and convert
    Call the imaging functions
    """
    data = load_data(data_path)
    cleaned_data, n_datapoints, n_nodes_data = clean_data(data, n_nodes)
    z_data = fill_z_array(cleaned_data, n_datapoints, n_nodes_data)
    averaged_values = calculate_average_on_managed_nodes(
        cleaned_data, n_datapoints, n_nodes_data, largest_managed_idx_in_data)


    path = target_path + "_" + metric_str
    fig_size = (12, 6)
    title_str = "{0} usage Locus nodes in % for {1}\nCreated at UTC ".format(
                            metric_str, duration_str)
    xlabel_str = "date/time"
    ylabel_str = "locus node number"
    cb_label = " % {0} usage".format(metric_str)
    path = target_path + "_heatmap.png"
    write_an_overview_image_to_file(z_data, path,
                                   n_datapoints, n_nodes_data, fig_size,
                                   title_str, xlabel_str, ylabel_str, cb_label,
                                   dpi=dpi)

    title_str = "Averaged {0} usage Locus nodes in % for {1}\nCreated at UTC ".format(
                        metric_str, duration_str)
    ylabel_str = "Percentage"
    fig_size = (12, 5)
    path = target_path + "_averaged.png"
    write_an_average_image_to_file(averaged_values, path,
                                  n_datapoints, n_nodes_data, fig_size,
                                  title_str, xlabel_str, ylabel_str,
                                  dpi=120)

    # return the mean of the average value for additional printing of target
    # visualization
    return mean(averaged_values)



def create_images(data_path_1day, data_path_7day,
                  target_path, metric_str):
    """
    Creation of the 4 images for a metric
    """
    print "Creating {0} images".format(metric_str)
    #one day images
    target_path_format = "{0}locus_{1}_1day".format(target_path, metric_str)
    duration_str = "last 24 Hrs."
    day_average = calculate_and_write_metric(
                data_path_1day, target_path_format, metric_str, duration_str)

    # 7 day images
    target_path_format = "{0}locus_{1}_7day".format(target_path, metric_str)
    duration_str = "7 days."
    week_average = calculate_and_write_metric(
            data_path_7day, target_path_format, metric_str, duration_str)

    return int(round(day_average)), int(round(week_average))

def create_gross_usage_image(target_path, cpu_averages, diskio_averages, swap_averages):
    fig = figure(figsize=(10, 2), dpi=120)
    ax = fig.add_subplot(111)
    ax.set_axis_off()
    # First metric is the cpu usage
    text(0.1, 0.3, "CPU usage %\n7 days(24 Hrs)", size=10,
         ha="center", va="center")

    if cpu_averages[1] < 50:
        color_day = "red"
    else:
        color_day = "green"
    text(0.01, 0.7, "{0}".format(cpu_averages[1]), size=50,
         ha="center", va="center", color=color_day)

    if cpu_averages[0] < 50:
        color_week = "red"
    else:
        color_week = "green"
    text(0.15, 0.7, "({0})".format(cpu_averages[0]), size=30,
         ha="center", va="center", color=color_week)

    # second metric is the disk io
    text(0.5, 0.3, "diskio %\n7 days(24 Hrs)", size=10,
         ha="center", va="center")

    if diskio_averages[1] < 90:
        color_day = "green"
    else:
        color_day = "red"
    text(0.41, 0.7, "{0}".format(diskio_averages[1]), size=50,
         ha="center", va="center", color=color_day)

    if diskio_averages[0] < 90:
        color_week = "green"
    else:
        color_week = "red"
    text(0.55, 0.7, "({0})".format(diskio_averages[0]), size=30,
         ha="center", va="center", color=color_week)

    # second metric is the swap usage
    text(0.9, 0.3, "swap space %\n7 days(24 Hrs)", size=10,
         ha="center", va="center")

    if swap_averages[1] < 10:
        color_day = "green"
    else:
        color_day = "red"
    text(0.81, 0.7, "{0}".format(swap_averages[1]), size=50,
         ha="center", va="center", color=color_day)

    if swap_averages[0] < 10:
        color_week = "green"
    else:
        color_week = "red"
    text(0.95, 0.7, "({0})".format(swap_averages[0]), size=30,
         ha="center", va="center", color=color_week)

    savefig(target_path, format='png')


if __name__ == '__main__':
    target_path = "/tmp/"

    metric_str = "CPU"
    cpu_averages = create_images("/tmp/locus_usage_1day",
                                 "/tmp/locus_usage_7days",
                                 target_path, metric_str)

    metric_str = "diskIO"
    diskio_averages = create_images("/tmp/locus_diskio_1day",
                                     "/tmp/locus_diskio_7days",
                                     target_path, metric_str)

    metric_str = "swapfile"
    swap_averages = create_images("/tmp/locus_swapusage_1day",
                                  "/tmp/locus_swapusage_7days",
                                  target_path, metric_str)

    create_gross_usage_image(
        target_path + "gross_numbers.png", cpu_averages, diskio_averages,
        swap_averages)


    #show() # comment out the line matplotlib.use("Agg") at start of file

