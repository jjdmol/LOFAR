global bton_save
global s;
global features;
global Res_Fr;
global resampler;

 load('data/Path_info.mat')

if length(resampler)==0
    message='You must apply a resampling on your signal';
    title='Warning';
    msgbox(message,title,'Warn')
else

[Filename,Pathname]=uiputfile([Path_dat '*.dat'],'Save The File ');
if strfind(Filename,'.dat')>0
Filename=Filename(1:strfind(Filename,'.dat')-1);
end
fid=fopen([Path_dat Filename '.dat'],'w+');
%features=struct('Sampling_Freq_signal',{},'total_time',{});
fprintf(fid,'Features :\n');
fprintf(fid,'Fsampling :%f Total Time : %f Fs_signal: %f \n',Res_Fr,features(1).total_time,features(1).Sampling_Freq_signal);

fprintf(fid,'Signal_type       Ampl        Frequency       Time        TimeWidth        TimeOffset       Phase \n'); 
for i=1:length(s)
fprintf(fid,'  %s         %f     %f      %f       %f          %f      %f\n',s(i).Signal_type,s(i).Ampl,s(i).Frequency,s(i).Time,s(i).Timewidth,s(i).OffsetTime,s(i).phase );
end

fprintf(fid,'\nData :\n');
fprintf(fid,'%f\n',resampler);
fclose(fid);

fid=fopen([Path_config Filename '.config'],'w+');
%features=struct('Sampling_Freq_signal',{},'total_time',{});
fprintf(fid,'Features :\n');
fprintf(fid,'Fsampling :%f Total Time : %f Fs_signal: %f \n',Res_Fr,features(1).total_time,features(1).Sampling_Freq_signal);

fprintf(fid,'Signal_type       Ampl        Frequency       Time        TimeWidth        TimeOffset       Phase \n'); 
for i=1:length(s)
fprintf(fid,'  %s         %f     %f      %f       %f          %f      %f\n',s(i).Signal_type,s(i).Ampl,s(i).Frequency,s(i).Time,s(i).Timewidth,s(i).OffsetTime,s(i).phase );
end

fprintf(fid,'\nData :\n');
fprintf(fid,'%f\n',resampler);
fclose(fid);

fid=fopen([Path_info Filename '.info'],'w+');
%features=struct('Sampling_Freq_signal',{},'total_time',{});
fprintf(fid,'Features :\n');
fprintf(fid,'Fsampling :%f Total Time : %f Fs_signal: %f \n',Res_Fr,features(1).total_time,features(1).Sampling_Freq_signal);

fprintf(fid,'Signal_type       Ampl        Frequency       Time        TimeWidth        TimeOffset       Phase \n'); 
for i=1:length(s)
fprintf(fid,'  %s         %f     %f      %f       %f          %f      %f\n',s(i).Signal_type,s(i).Ampl,s(i).Frequency,s(i).Time,s(i).Timewidth,s(i).OffsetTime,s(i).phase );
end
fclose(fid)
end