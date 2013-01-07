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
"""
import matplotlib
matplotlib.use("Agg")
import numpy as np
from pylab import *
from datetime import *

# Load data
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
    xlabel_str = "5 Minute samples"
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



def create_images(data_path_1day, data_path_7day,
                  target_path, metric_str):
    """
    Creation of the 4 images for a metric
    """
    print "Creating {0} images".format(metric_str)
    #one day images
    target_path_format = "{0}locus_{1}_1day".format(target_path, metric_str)
    duration_str = "last 24 Hrs."
    calculate_and_write_metric(data_path_1day, target_path_format, metric_str, duration_str)

    # 7 day images
    target_path_format = "{0}locus_{1}_7day".format(target_path, metric_str)
    duration_str = "7 days."
    calculate_and_write_metric(data_path_7day, target_path_format, metric_str, duration_str)


if __name__ == '__main__':
    target_path = "/home/klijn/temp/"

    metric_str = "CPU"
    create_images("/tmp/locus_usage_1day", "/tmp/locus_usage_7days",
                  target_path, metric_str)

    metric_str = "diskIO"
    create_images("/tmp/locus_diskio_1day", "/tmp/locus_diskio_7days",
                  target_path, metric_str)

    metric_str = "swapfile"
    create_images("/tmp/locus_swapusage_1day", "/tmp/locus_swapusage_7days",
                  target_path, metric_str)

    #show() # comment out the line matplotlib.use("Agg") at start of file

