function [out2]=trajectory_reader(file_name)
fid=fopen(file_name,'r');
out=fscanf(fid,'%f');
out2=reshape(out,3,floor(length(out))/3).';
fclose(fid);