[Filename_Matrix,Pathname]=uiputfile('*.dat','Save The File ');
fid=fopen([Filename_Matrix '.dat'],'w+');

fprintf(fid,'Reference file : %s\n',Filename_reference);
fprintf(fid,'Fsampling : %s\n',fs);
fprintf(fid,'Waveform File :\n');
if Filename_load2==[]
    Filename_load2='No waveform available';
end
fprintf(fid,'%s\n',Filename_load2);
Filename_load2==[];
if indice ==2
    chain='Polyphase Filter';
elseif indice==1
    chain='FFT';
else chain='Empty';
end
fprintf(fid,'Time Frequency Plane with : %s\n',chain);
fprintf(fid,'Freq resolution :%f Integration Time :%f Total Time : %f \n',str2num(get(real_fr,'String')),str2num(get(real_time,'String')),str2num(get(Total_time,'String')));
fprintf(fid,'Noise(level) : %e\n',power);

if indice==1
fprintf(fid,'Calibration : %f',1);
fprintf(fid,'\nSize:%f %f \n',size(OutputFFT));
fprintf(fid,'\nData :\n');
fprintf(fid,'\nReal:\n');
fprintf(fid,'%f\n',real(OutputFFT));
fprintf(fid,'\nImag:\n');
fprintf(fid,'%f\n',imag(OutputFFT));
else
fprintf(fid,'Calibration : %f',20*log10(Factor));
fprintf(fid,'\nSize:%f %f \n',size(OutputSignal3));
fprintf(fid,'\nData :\n');
fprintf(fid,'\nReal:\n');
fprintf(fid,'%f\n',real(OutputSignal3));
fprintf(fid,'\nImag:\n');
fprintf(fid,'%f\n',imag(OutputSignal3));
end
fclose(fid);