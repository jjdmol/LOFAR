function [out]=phase_reader(file_name)
fid=fopen(file_name,'r');
out=fscanf(fid,'%f');
fclose(fid);