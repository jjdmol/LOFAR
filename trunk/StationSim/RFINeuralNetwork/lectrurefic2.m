global MyData2;
fid=fopen(file_name,'r');
MyData2=[];
disp(['here: ' file_name ' ']);
tline=fgetl(fid);
MyData2=fscanf(fid,'%e');
fclose(fid);
