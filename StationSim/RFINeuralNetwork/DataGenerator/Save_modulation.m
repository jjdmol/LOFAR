[Filename,Pathname]=uiputfile('*.txt','Save The File ');
fid=fopen(Filename,'w+');
moy=0;
   
fprintf(fid,'Features :\n');
fprintf(fid,'Fsampling :%f Total Time : %f  %f ',fs,length(Signal_modulation_noisy)*1/fs);
fprintf(fid,'\nNoise Mean :%f Var : %f\n',moy,var)
for i=1:length(Modulated_signal)
fprintf(fid,'Modulated Signal Filename\n');   
fprintf(fid,'%s\n', Modulated_signal(i).Modula_signal)
fprintf(fid,'Modulation_name   Carrier_Frequency   Amplitude  modulation_feature \n'); 
for j=1:length(Modulated_signal(i).feature_modulat)  
fprintf(fid,'     %s              %f    %f    %f \n',Modulated_signal(i).feature_modulat(j).name,Modulated_signal(i).feature_modulat(j).Carrier_freq,Modulated_signal(i).feature_modulat(j).modulation_Ampl,Modulated_signal(i).feature_modulat(j).modulation_param)
end
end
clear Signal_modulation
clear mat;
fprintf(fid,'\nData :\n');
fprintf(fid,'%f\n',Signal_modulation_noisy);
fclose(fid);
