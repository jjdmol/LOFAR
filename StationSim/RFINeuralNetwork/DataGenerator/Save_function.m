global bton_save
global s;
global features;
global Res_Fr;
global resampler;

[Filename,Pathname]=uiputfile('*.txt','Save The File ');
fid=fopen(Filename,'w+');
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