"""
A simple script collecting usage data from the locus usage files.
Parse and cleaning it. create averaged data taking in account nodes not in 
production. Data is then used to create visualization and writing them
as png to disk.

2013
Wouter Klijn (klijn@astron.nl)
"""

import subprocess
from gauge import create_and_save_gauge

def get_latest_usage_time_slot_from_disk():
    disk_io = subprocess.Popen(["tail", "/tmp/locus_diskio_1day", "-n", "1"], stdout=subprocess.PIPE).communicate()[0]
    swap = subprocess.Popen(["tail", "/tmp/locus_swapusage_1day", "-n", "1"], stdout=subprocess.PIPE).communicate()[0]
    cpu_usage = subprocess.Popen(["tail", "/tmp/locus_usage_1day", "-n", "1"], stdout=subprocess.PIPE).communicate()[0]

    return disk_io, swap, cpu_usage

def convert_to_float_list(data):
    float_list = []
    for entry in data:
        float_list.append(float(entry))

    return float_list

def strip_timestamps_and_unmonitored_nodes(data, ignore_above_idx=93):
    # ignore_above_idx+2 -> locus095 and higher are unmanaged and
    # should not be included in the usage average  
    data = data[2:]
    data = data[:ignore_above_idx]
    return data


if __name__ == '__main__':
    # Get the raw data
    disk_io, swap, cpu_usage = get_latest_usage_time_slot_from_disk()

    # convert to float lists with the unmaged nodes removed
    disk_io_floats = convert_to_float_list(
        strip_timestamps_and_unmonitored_nodes(disk_io.split()))
    swap_floats = convert_to_float_list(
        strip_timestamps_and_unmonitored_nodes(swap.split()))
    cpu_usage_floats = convert_to_float_list(
        strip_timestamps_and_unmonitored_nodes(cpu_usage.split()))


    # Number of nodes (needed for average)
    n_nodes = len(disk_io_floats) * 1.0

    draw = True
    # First create a disk load image    
    disk_io_average = sum(disk_io_floats) / n_nodes
    file_name = 'gauge_disk_io.png'
    attribute_name = "% disk io usage\n (5 Min. Av.)"
    value = 80.0
    create_and_save_gauge(
            disk_io_average, file_name, attribute_name, draw=False)

    # The cpu usage
    cpu_usage_average = sum(cpu_usage_floats) / n_nodes
    file_name = 'gauge_cpu.png'
    attribute_name = "% CPU usage\n (5 Min. Av.)"
    create_and_save_gauge(
            cpu_usage_average, file_name, attribute_name, draw=False)

    # The maximum swap usage
    swap_max = max(swap_floats)
    attribute_name = "MAX swap usage\n (5 Min. Av.)"
    max_swap_usage = 10.0
    swapping_nodes = False
    if swap_max > max_swap_usage:
        swapping_nodes = True
        swapping_list = [i + 1 for i, j in enumerate(swap_floats) if j > max_swap_usage]
        attribute_name = \
           "MAX swap usage\n (5 Min. Av.)\nNODES:\n{0}".format(
                            str(swapping_list).strip("[]"))

    file_name = 'gauge_swap.png'
    create_and_save_gauge(
            swap_max, file_name, attribute_name, draw=draw,
                limits=[0.0, 7.0, 10.0, 100.0])
