
""" script for testing PRBS data in dir.
This script can be used for testing data from the TBB, it will be used by TBB test scripts.

Started by Gijs, 16 dec 07
Modified by Gijs on March 17 2009:
-PRBS test bug fixed, when data is all 0 error did't count.
-CRC test on files with RRBS errors. When a PRBS error and no CRC, error in RCU-to-RSP communications, when both has errors, error between RSP-to-TBB communication
Modified by Menno on Sept 21 2009:
-Removed Samples Checked because sometime 10238 or 10239


"""
# INIT

import array
import operator
import os
import time
import commands

# Look for files to test	
def open_dir() :
	files = os.listdir('./prbs/.')
	files.sort()
	#print files
        return files

# Open de file for testing
def open_file(files, file_nr) :
        file_name = './prbs/' + files[file_nr][:]
 	if files[file_nr][-3:] == 'dat':
                fileinfo = os.stat(file_name)
		size = int(fileinfo.st_size)
		f=open(file_name,'rb')
		max_frames = size/(88 + 1024*2 + 4)
		frames_to_proces=max_frames
	else :
		frames_to_proces=0
		f=open(file_name,'rb')
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
		string_info =  'Frame nr %(FR)d RSP %(RSP)d  RCU %(RCU)d  Sample rate %(S)d MHz'%\
				{"FR": frame_nr,"RSP": station_info[1], "RCU": station_info[2], "S": station_info[3]}


#	print string_info
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
                elif data_list[i]  == data_list[i-1]: 
                        cur = data_list[i]
                        samples_chk = samples_chk + 1
                        prbs_err = prbs_err + 1
                        prev = data_list[i] & 0x07FF
		else :
			cur = data_list[i] & 0x0FFE
			samples_chk = samples_chk + 1
			if cur != 2*prev :
				prbs_err = prbs_err + 1
#				print(str(i) + ' ' + hex(2*prev) + ' ' + hex(cur))
			prev = data_list[i] & 0x07FF
	return samples_chk, prbs_err, prev

# Function for testing CRC of header
def CRC16_check(buf) :
        CRC=0
        CRC_poly=0x18005
        bits=16
        data=0
        CRCDIV = (CRC_poly & 0x7fffffff) * 32768 # << 15
        data = (buf[0] & 0x7fffffff) << 16
        len_buf = len(buf)
        for cnt in range(1,len_buf) :
                data = data + buf[cnt]
                for  cnt in range(bits) :
                        if data & 0x80000000 :
                                data = data ^ CRCDIV
                        data = data & 0x7fffffff
                        data = data * 2 # << 1
        CRC = data >> 16
        return CRC

# Function for testing CRC of data
def CRC32_check(buf) :
        CRC=0
        CRC_poly=0x104C11DB7    # 1 0000 0100 1100 0001 0001 1101 1011 0111 
        bits=16
        data=0
        CRCDIV = (CRC_poly & 0x7fffffffffff) * 32768 #<< 15
        data = buf[0]
        data = data & 0x7fffffffffff
        data = data << 16
        data = data + buf[1]
        data = data & 0x7fffffffffff
        data = data << 16
        len_buf = len(buf)
        for cnt in range(2,len_buf) :
                data = data + buf[cnt]
                for  cnt in range(bits) :
                        if data & 0x800000000000 :
                                data = data ^ CRCDIV
                        data = data & 0x7fffffffffff
                        data = data * 2 # << 1
        CRC = int(data >> 16)
        return CRC

#Function for testing CRC of complete frame (header and data)

def crc_frame(f, info_plot, frame_nr,f_log):
        CRC_ERROR=0
        header = array.array('H')
        data_in = array.array('H')
        data_crc = array.array('H')

        # READING HEADER INFORMATION
        header.fromfile(f,44)       # Bytes 0..88
        # remove SEQNR from header, this data is added after CRC calculations   
        header[2]=0
        header[3]=0
        if CRC16_check(header) :
                str_info = 'CRC ERROR IN HEADER '
#                f_log.write(str_info )
                CRC_ERROR=1

        Station_id = header[0] & 0xFF
        RSP_id = header[0] >> 8
        RCU_id = header[1] &0xFF
        Sample_rate = header[1] >> 8
        Time = float((header[5] * 65536) + header[4])
        Sample_nr = (header[7] * 65536) + header[6]
        Samples = header[8]
        if (info_plot) :
                time_string = time.ctime(Time)

#                str_info =  'Frame nr %(FR)d Station %(ST)d  RSP %(RSP)d  RCU %(RCU)d  Sample rate %(S)d MHz  time of data  %(ti_D)s and %(SN)00.6f seconds'%\
#                                {"FR": frame_nr, "ST": Station_id ,"RSP": RSP_id, "RCU": RCU_id, "S": Sample_rate, "ti_D": time_string,"SN": float(Sample_nr)/float(200000000)}


#               print string_info
#                f_log.write(str_info + '\n')

        del(header)
        # READ DATA SAMPLES
        data_in.fromfile(f,1024)
        data_crc.fromfile(f,2)
        data_list = data_in.tolist()
        for cnt in range(len(data_in)):
                data_in[cnt] = (data_in[cnt] & 0x0FFF)
        data_in.append(data_crc[1])
        data_in.append(data_crc[0])
        if CRC32_check(data_in):
                str_info = 'CRC ERROR IN DATA, '
#                f_log.write(str_info )
                CRC_ERROR=1
        return CRC_ERROR

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
		if frames_to_proces >0 :
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
#			print 'PRBS errors: ' + str(prbs_err)
			f_log.write('PRBS errors: ' + str(prbs_err) + '\n')
		f.close
		if prbs_err > 0:
			(f, frames_to_proces) = open_file(files, file_cnt)
	                if frames_to_proces >0 :
				crc_err=0
        	                for frame_cnt in range(frames_to_proces):
                	                crc_err = crc_err + crc_frame(f, (frame_cnt==0), frame_cnt, f_log)
#                       print 'PRBS errors: ' + str(prbs_err)
                        f_log.write('Number of frames with CRC  errors: ' + str(crc_err) + '\n')

		f.close
	f_log.close

if __name__ == "__main__":
	main()
