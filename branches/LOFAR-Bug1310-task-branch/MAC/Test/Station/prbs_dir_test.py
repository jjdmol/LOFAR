
""" script for testing PRBS data in dir.
Gijs, 16 dec 07
"""
# INIT

import array
import operator
import os
import time
import commands

# Open file for processing	
def open_dir() :
#	os.chdir('c:/test')
	files = os.listdir('.')
	files.sort()
	return files

def open_file(files, file_nr) :
	if files[file_nr][-3:] == 'dat':
		fileinfo = os.stat(files[file_nr])
		size = int(fileinfo.st_size)
		f=open(files[file_nr],'rb')
		max_frames = size/(88 + 1024*2 + 4)
		frames_to_proces=max_frames
	else :
		frames_to_proces=0
		f=open(files[file_nr],'rb')
	return f, frames_to_proces	


# Read single frame from file	
def read_frame(f, info_plot, frame_nr,f_log):
	station_info = array.array('B')	  	
	station_info.fromfile(f,4)       # Bytes 0..3
	time_info = array.array('L')
	time_info.fromfile(f,3)          # Bytes 4..15
	if (info_plot) :
		time_string = time.ctime(time_info[1])
#		string_info =  'Frame nr %(FR)d Station %(ST)d  RSP %(RSP)d  RCU %(RCU)d  Sample rate %(S)d MHz  time of data  %(ti_D)s and %(SN)00.6f seconds'%\
#				{"FR": frame_nr, "ST": station_info[0] ,"RSP": station_info[1], "RCU": station_info[2], "S": station_info[3], "ti_D": time_string,"SN": float(time_info[2])/float(200000000)}
		string_info =  'Frame nr %(FR)d Station %(ST)d  RSP %(RSP)d  RCU %(RCU)d  Sample rate %(S)d MHz'%\
				{"FR": frame_nr, "ST": station_info[0] ,"RSP": station_info[1], "RCU": station_info[2], "S": station_info[3]}
#		print string_info
		f_log.write(string_info + '\n')
	div_info = array.array('H')
	div_info.fromfile(f,36)          # Bytes 16..87
	
	# READ DATA SAMPLES
	data_in = array.array('H')
	samples = int(div_info[0])
	data_in.fromfile(f,samples)
	data_list = data_in.tolist()
	
	data_crc = array.array('l')
	data_crc.fromfile(f,1)
	return data_list, time_info[1], time_info[2]


# Function for testing PRBS data
def PRBS_CHECK(data_list, prev):
	samples_chk=0
	prbs_err=0
 	for i in range(0,len(data_list)) :
		if prev == 0x0FFF :
			prev = data_list[i] & 0x07FF
		elif data_list[i] == 0xFFFF :
			prbs_err = prbs_err + 1
		else :
			cur = data_list[i] & 0x0FFE
			samples_chk = samples_chk + 1
			if cur != 2*prev :
				prbs_err = prbs_err + 1
#				print(str(i) + ' ' + hex(2*prev) + ' ' + hex(cur))
			prev = data_list[i] & 0x07FF
	return samples_chk, prbs_err, prev


# Main loop
def main() :
	files = open_dir()
	f_log = file('prbs_dir_test.log', 'w')
	f_log.write('\n \n PRSB test \n \n')
	for file_cnt in range(len(files)) :
		prev = 0x0FFF;
		samples_chk=0
		prbs_err=0
		o_ta=0
		o_tb=0
		(f, frames_to_proces) = open_file(files, file_cnt)
		if frames_to_proces >1 :
			for frame_cnt in range(frames_to_proces):
				data_list, ta, tb = read_frame(f, (frame_cnt==0), frame_cnt, f_log)
                                if (((ta==o_ta) and tb==(o_tb+1024)) or (ta == (o_ta+1))) :
#                                if (tb==(o_tb+1)) :
                                	prev = prev
                                else:
                                	prev=0x0FFF
				r_samples_chk, r_prbs_err, prev = PRBS_CHECK(data_list, prev)
				samples_chk = samples_chk + r_samples_chk
				prbs_err = prbs_err + r_prbs_err
				o_ta = ta
				o_tb = tb				
			# plot results		
#			print 'Samples checked : ' + str(samples_chk) + ' PRBS errors: ' + str(prbs_err)
			f_log.write('Samples checked : ' + str(samples_chk) + ' PRBS errors: ' + str(prbs_err) + '\n')
		
		f.close
	f_log.close

if __name__ == "__main__":
	main()
