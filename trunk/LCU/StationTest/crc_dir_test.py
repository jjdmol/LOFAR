
import array
import operator
import os
import time

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
	 
# Open file for processing
def open_dir() :
        files = os.listdir('./prbs/.')
        files.sort()
        return files

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


def read_frame(f, info_plot, frame_nr,f_log):
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
		str_info = 'CRC ERROR HEADER '
		f_log.write(str_info + '\n')
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

                str_info =  'Frame nr %(FR)d Station %(ST)d  RSP %(RSP)d  RCU %(RCU)d  Sample rate %(S)d MHz  time of data  %(ti_D)s and %(SN)00.6f seconds'%\
                                {"FR": frame_nr, "ST": Station_id ,"RSP": RSP_id, "RCU": RCU_id, "S": Sample_rate, "ti_D": time_string,"SN": float(Sample_nr)/float(200000000)}


#               print string_info
                f_log.write(str_info + '\n')
 
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
		str_info = 'CRC ERROR DATA'
		f_log.write(str_info + '\n')
		CRC_ERROR=1
	return data_list,CRC_ERROR

def PRBS_CHECK(data_list, prev):
	samples_chk=0
	prbs_err=0
 	for i in range(0,len(data_list)) :
		if prev == 0x0FFF :
			prev = data_list[i] & 0x07FF
		elif data_list[i] == 0xFFFF :
			prbs_err = prbs_err + 1
                elif data_list[i] == data_list[i-1] :
                        prbs_err = prbs_err + 1
                        samples_chk = samples_chk + 1
                        prev = data_list[i] & 0x07FF
		else :
			cur = data_list[i] & 0x0FFE
			samples_chk = samples_chk + 1
			if cur != 2*prev :
				prbs_err = prbs_err + 1
#				print(str(i) + ' ' + hex(2*prev) + ' ' + hex(cur))
			prev = data_list[i] & 0x07FF
	return samples_chk, prbs_err, prev


	
def main() :
        files = open_dir()
        f_log = file('crc_dir_test.log', 'w')
        f_log.write('\n \n PRSB test \n \n')
        for file_cnt in range(len(files)) :
		prev = 0x0FFF;
		samples_chk=0
		prbs_err=0
                (f, frames_to_proces) = open_file(files, file_cnt)
		plot_info=1
                if frames_to_proces >0 :
                        for frame_cnt in range(frames_to_proces):
				data_list,CRC_ERROR = read_frame(f,(frame_cnt==0),frame_cnt,f_log)
				if CRC_ERROR :
					break
				r_samples_chk, r_prbs_err, prev = PRBS_CHECK(data_list, prev)
				samples_chk = samples_chk + r_samples_chk
				prbs_err = prbs_err + r_prbs_err
		# plot results		
#			print 'Samples checked : ' + str(samples_chk) + ' PRBS errors: ' + str(prbs_err)
                	f_log.write('Samples checked : ' + str(samples_chk) + ' PRBS errors: ' + str(prbs_err) + '\n')
                f.close
        f_log.close

main()
