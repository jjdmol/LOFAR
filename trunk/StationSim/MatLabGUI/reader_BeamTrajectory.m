function [traject]=reader_BeamTrajectory(Filename_beam)
stop=0;
fid=fopen(Filename_beam,'r');
t=0;
while(stop==0)   
line=fgetl(fid);
t=t+1;
resp0=strfind(line,'npoints :');
if length(resp0)>0
    lig=str2num(line(resp0+9:length(line)));
end
resp=strfind(line,'nsubbands :');
if length(resp)>0
   nsubbands=str2num(line(resp+11:length(line)));
end
if t==3
    stop=1;
end
end
traject=fscanf(fid,'%g',[3 lig]);
fclose(fid);