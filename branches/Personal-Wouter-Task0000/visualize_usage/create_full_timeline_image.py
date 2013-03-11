"""
This script calculates the cpu averaged for each day
and displays this in an image
"""

import create_detailed_usage_images

#matplotlib.use("Agg")
import numpy as np
from pylab import *
from datetime import *
from compiler.ast import Break


def create_day_average(cleaned_data):
    n_nodes = len(cleaned_data[0])
    steps_in_day = (60 * 24) / 5
    days = len(cleaned_data) / steps_in_day

    daily_average_data = []
    # day stride in the data
    for idx_day in range(days):
        first_time_step_idx = idx_day * steps_in_day
        past_last_time_step_idx = first_time_step_idx + steps_in_day
        node_summed_list = [0.0] * n_nodes
        # step the 5 min datapoints
        for idx_timestep in range(first_time_step_idx, past_last_time_step_idx):
            # Add all the timesteps
            node_summed_list = [ i + j for i, j in zip(node_summed_list,
                                               cleaned_data[idx_timestep])]
        # create mean using the n_steps
        node_average_list = [i / steps_in_day for i in node_summed_list]
        daily_average_data.append(node_average_list)

    return daily_average_data




def calculate_and_write_metric_full_time_range(data_path, target_path, metric_str, duration_str,
                               n_nodes=100, dpi=120,
                               largest_managed_idx_in_data=94): # this means that 0 untill 94 are summed
    """
    Main functional unit:
    load data
    Clean and convert
    Call the imaging functions
    
    This is a adapted copy paste from create_detailed_usage_images.calculate_and_write_metric
    """
    data = create_detailed_usage_images.load_data(data_path)
    cleaned_data, n_datapoints, n_nodes_data = create_detailed_usage_images.clean_data(data, n_nodes)

    # convert the cleaned data to a dayly average
    remainder_of_day = len(cleaned_data) % 288 #
    cleaned_data = cleaned_data[remainder_of_day:]
    cleaned_data = create_day_average(cleaned_data)
    print len(cleaned_data)
    print len(cleaned_data[0])
    n_datapoints = len(cleaned_data)
    z_data = create_detailed_usage_images.fill_z_array(cleaned_data, n_datapoints, n_nodes_data)
    print z_data
    #return
    print "creating daily averages:"
    averaged_values = create_detailed_usage_images.calculate_average_on_managed_nodes(
        cleaned_data, n_datapoints, n_nodes_data, largest_managed_idx_in_data)

    print "creating plots"
    path = target_path + "_" + metric_str
    fig_size = (12, 6)
    title_str = "{0} usage Locus nodes in % for {1}\nCreated at UTC ".format(
                            metric_str, duration_str)
    xlabel_str = "date/time"
    ylabel_str = "locus node number"
    cb_label = " % {0} usage".format(metric_str)
    path = target_path + "_heatmap.png"
    create_detailed_usage_images.write_an_overview_image_to_file(z_data, path,
                                   n_datapoints, n_nodes_data, fig_size,
                                   title_str, xlabel_str, ylabel_str, cb_label,
                                   dpi=dpi)

    title_str = "Averaged {0} usage Locus nodes in % for {1}\nCreated at UTC ".format(
                        metric_str, duration_str)
    ylabel_str = "Percentage"
    fig_size = (12, 5)
    path = target_path + "_averaged.png"
    create_detailed_usage_images.write_an_average_image_to_file(averaged_values, path,
                                  n_datapoints, n_nodes_data, fig_size,
                                  title_str, xlabel_str, ylabel_str,
                                  dpi=120)

    # return the mean of the average value for additional printing of target
    # visualization
    return mean(averaged_values)

if __name__ == '__main__':
    target_path = "/tmp/"

    data_path = "/tmp/locus_usage"
    metric_str = "daily CPU"
    duration_str = "uptime"
        #one day images
    target_path_format = target_path + "cpu_full" # /tmp/cpu_full.png

    day_average = calculate_and_write_metric_full_time_range(
                "/tmp/locus_usage", target_path_format, metric_str, duration_str)

    show()
