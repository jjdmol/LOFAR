[Filename_reference,Pathname]=uiputfile('*.txt','Save The File ');
fid=fopen(Filename_reference,'w+');
Filename_load2==[];
fprintf(fid,'Fsampling : %s\n',fs);
chain='Polyphase Filter';
fprintf(fid,'Time Frequency Plane with : %s\n',chain);
fprintf(fid,'Freq resolution :%f Integration Time :%f Total Time : %f \n',str2num(get(real_fr,'String')),str2num(get(real_time,'String')),str2num(get(Total_time,'String')));

for i=1:length(Modulated_signal)
fprintf(fid,'Modulated Signal Filename\n');   
fprintf(fid,'%s\n', Modulated_signal(i).Modula_signal)
fprintf(fid,'Modulation_name   Carrier_Frequency   Amplitude  modulation_feature \n'); 
for j=1:length(Modulated_signal(i).feature_modulat)  
fprintf(fid,'     %s              %f    %f    %f \n',Modulated_signal(i).feature_modulat(j).name,Modulated_signal(i).feature_modulat(j).Carrier_freq,Modulated_signal(i).feature_modulat(j).modulation_Ampl,Modulated_signal(i).feature_modulat(j).modulation_param)
end
end
fprintf(fid,'Threshold applied : %f\n',str2num(get(Noise_minValue,'String')));
if indice==1
fprintf(fid,'\nSize:%f %f \n',size(reference));
fprintf(fid,'\nData :\n');
fprintf(fid,'%f\n',DSP_1inst);
else
fprintf(fid,'\nSize:%f %f \n',size(reference));
fprintf(fid,'\nData :\n');
fprintf(fid,'%f\n',reference);
end
fclose(fid);