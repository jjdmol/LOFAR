 load('data/Path_info.mat')

[Filename,Pathname]=uiputfile([Path_dat '*.dat'],'Save The File ');
if strfind(Filename,'.dat')>0
Filename=Filename(1:strfind(Filename,'.dat')-1);
end 
fid=fopen([Path_config Filename '.config'],'w+');
fprintf(fid,'fs :%f  \n',fs);
t=0;
for i=1:length(Modulated_signal)
for j=1:length(Modulated_signal(i).feature_modulat) 
    t=t+1;
end
end
%path= '/home/alex/gerdes/DG_input_log/';
fprintf(fid,'nsignals : %f  \n\n',t);
for i=1:length(Modulated_signal)
Filename_record_dat=Modulated_signal(i).Modula_signal(1:strfind(Modulated_signal(i).Modula_signal,'.dat')-1);
fprintf(fid,['SignalFilename : %s\n'], [Filename_record_dat '.config']);   
fprintf(fid,'Modulation_name   Carrier_Frequency   Amplitude  modulation_feature \n'); 
for j=1:length(Modulated_signal(i).feature_modulat)  
fprintf(fid,'     %s              %f    %f    %f \n',Modulated_signal(i).feature_modulat(j).name,Modulated_signal(i).feature_modulat(j).Carrier_freq,Modulated_signal(i).feature_modulat(j).modulation_Ampl,Modulated_signal(i).feature_modulat(j).modulation_param);
end
end
clear Signal_modulation
clear mat;
fclose(fid);

fid=fopen([Path_dat Filename '.dat'],'w+');
fprintf(fid,'Features :\n');
fprintf(fid,'Fsampling : %f Total Time : %f  %f ',fs,length(Signal_modulation_noisy)*1/fs);
fprintf(fid,'\nNoise Mean :%f Var : %f\n',0,var)
for i=1:length(Modulated_signal)
fprintf(fid,'Modulated Signal Filename\n');   
Filename_record_dat=Modulated_signal(i).Modula_signal(1:strfind(Modulated_signal(i).Modula_signal,'.dat')-1);
fprintf(fid,['SignalFilename : %s\n'], [Filename_record_dat '.dat']);   
fprintf(fid,'Modulation_name   Carrier_Frequency   Amplitude  modulation_feature \n'); 
for j=1:length(Modulated_signal(i).feature_modulat)  
fprintf(fid,'     %s              %f    %f    %f \n',Modulated_signal(i).feature_modulat(j).name,Modulated_signal(i).feature_modulat(j).Carrier_freq,Modulated_signal(i).feature_modulat(j).modulation_Ampl,Modulated_signal(i).feature_modulat(j).modulation_param);
end
end
fprintf(fid,'\nData :\n');
fprintf(fid,'%f\n',Signal_modulation_noisy);
fclose(fid);

fid=fopen([ Path_info Filename '.info'],'w+');
fprintf(fid,'Features :\n');
fprintf(fid,'Fsampling :%f Total Time : %f  %f ',fs,length(Signal_modulation_noisy)*1/fs);
fprintf(fid,'\nNoise Mean :%f Var : %f\n',moy,var)
for i=1:length(Modulated_signal)
fprintf(fid,'Modulated Signal Filename\n'); 
Filename_record_inf=Modulated_signal(i).Modula_signal(1:strfind(Modulated_signal(i).Modula_signal,'.dat')-1);
fprintf(fid,'%s\n', [Filename_record_inf '.info']);
fprintf(fid,'Modulation_name   Carrier_Frequency   Amplitude  modulation_feature \n'); 
for j=1:length(Modulated_signal(i).feature_modulat)  
fprintf(fid,'     %s              %f    %f    %f \n',Modulated_signal(i).feature_modulat(j).name,Modulated_signal(i).feature_modulat(j).Carrier_freq,Modulated_signal(i).feature_modulat(j).modulation_Ampl,Modulated_signal(i).feature_modulat(j).modulation_param);
end
end
fclose(fid);