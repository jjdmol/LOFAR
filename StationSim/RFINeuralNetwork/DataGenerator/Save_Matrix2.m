[Filename_Matrix,Pathname]=uiputfile('*.txt','Save The File ');
fid=fopen(Filename_Matrix,'w+');

fprintf(fid,'Final File :\n');
fprintf(fid,'Fsampling : %s\n',Fsampl);
fprintf(fid,'Waveform File :\n');
if Filename_load2==[]
    Filename_load2='No waveform available';
end
fprintf(fid,'Reference Signal: %s\n',Filename_reference);
fprintf(fid,'%s\n',Filename_matrix);
fprintf(fid,'Reference Noise :\n');
fprintf(fid,'%s\n',Filename_matrix2);

fprintf(fid,'Time Frequency Plane :');
fprintf(fid,'Freq resolution :%f Integration Time :%f Total Time : %f \n',str2num(get(real_fr,'String')),str2num(get(real_time,'String')),str2num(get(Total_time,'String')));
fprintf(fid,'Noise(level) : %f\n',str2num(get(NoisePower,'String')));

fprintf(fid,'\nSize:%f %f \n',size(final_matrix));
fprintf(fid,'\nData :\n');
fprintf(fid,'\nReal:\n');
fprintf(fid,'%f\n',real(final_matrix));
fprintf(fid,'\nImag:\n');
fprintf(fid,'%f\n',imag(final_matrix));
fclose(fid);