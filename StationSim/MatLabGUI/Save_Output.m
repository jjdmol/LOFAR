if (exist('I')==0)
   message='You must compute the reference matrix';
    title='Warning';
    msgbox(message,title,'warn')
else

[Filename_Output,Pathname]=uiputfile('*.dat','Save The File ');
if strfind(Filename_Output,'.dat')>0
Filename_Output=Filename_Output(1:strfind(Filename_Output,'.dat')-1);
end

fid1=fopen([Filename_Output '.dat'],'w+');

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
fprintf(fid1,'\nSize:%f %f \n',size(OutputFFT_ref));
fprintf(fid1,'\nData :\n');
fprintf(fid1,'\nReal:\n');
fprintf(fid1,'%f\n',real(OutputFFT_ref));
fprintf(fid1,'\nImag:\n');
fprintf(fid1,'%f\n',imag(OutputFFT_ref));
subplot(PolyphasePlot)
imagesc(20*log10(abs(OutputFFT_ref)));
colormap(1-gray)

else
fprintf(fid1,'\nSize:%f %f \n',size(OutputSignal5));
fprintf(fid1,'\nData :\n');
fprintf(fid1,'\nReal:\n');
fprintf(fid1,'%f\n',real(OutputSignal5));
fprintf(fid1,'\nImag:\n');
fprintf(fid1,'%f\n',imag(OutputSignal5));
subplot(PolyphasePlot)
imagesc(20*log10(abs(OutputSignal5)));
colormap(1-gray)
end
fclose(fid1);
end
DSP_1inst=[];
OutputSignal=[];
OutputSignal2=[];
OutputSignal3=[];