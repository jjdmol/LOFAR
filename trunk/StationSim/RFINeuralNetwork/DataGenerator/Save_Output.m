if (length(I)==0)
   message='You must compute the reference matrix';
    title='Warning';
    msgbox(message,title,'warn')
else

[Filename_Output,Pathname]=uiputfile('*.txt','Save The File ');
fid1=fopen(Filename_Output,'w+');

fprintf(fid1,'Reference file :\n',Filename_reference);
fprintf(fid1,'Fsampling : %s\n',fs);
fprintf(fid1,'Waveform File :\n');
if Filename_load2==[]
    Filename_load2='No waveform available';
end
fprintf(fid1,'%s\n',Filename_load2);
Filename_load2==[];
if indice ==2
    chain='Polyphase Filter';
elseif indice==1
    chain='FFT';
else chain='Empty';
end
fprintf(fid1,'Time Frequency Plane with : %s\n',chain);
fprintf(fid1,'Freq resolution :%f Integration Time :%f Total Time : %f \n',str2num(get(real_fr,'String')),str2num(get(real_time,'String')),str2num(get(Total_time,'String')));
fprintf(fid1,'Threshold(lin) : %f\n',Threshold_modu);

if indice==1
fprintf(fid1,'\nSize:%f %f \n',size(DSP_1inst));
fprintf(fid1,'\nData :\n');
fprintf(fid1,'\nData :\n');
fprintf(fid1,'\nReal:\n');
fprintf(fid1,'%f\n',real(DSP_1inst));
fprintf(fid1,'\nImag:\n');
fprintf(fid1,'%f\n',imag(DSP_1inst));

else
fprintf(fid1,'\nSize:%f %f \n',size(OutputSignal5));
fprintf(fid1,'\nData :\n');
fprintf(fid1,'\nReal:\n');
fprintf(fid1,'%f\n',real(OutputSignal5));
fprintf(fid1,'\nImag:\n');
fprintf(fid1,'%f\n',imag(OutputSignal5));
end
fclose(fid1);
end