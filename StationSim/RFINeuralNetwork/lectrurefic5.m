global MyVar;
fid=fopen(file_name,'r');
MyVar=[];
disp(['here: ' file_name ' ']);
tline5=fgetl(fid);
MyVar=fscanf(fid,'%e');
fclose(fid);
