[Filename_matrix2,Pathname]=uigetfile('*.dat','Save The File ');
fid=fopen(Filename_matrix2,'r');
stop=0;
stop1=0;
stop2=0;
Size=[];
line=[];
line1=[];
line5=[];
count_file3=count_file3+1;
while(stop==0)   
  line=fgetl(fid);
  resp=strfind(line,'Fsampling');
  if length(resp)>0
  line1=line;
  Fsampl=str2num(line(resp(1)+11:length(line)));
  set(fs2,'String',Fsampl)
  end
  resp_test1=strfind(line,'Freq resolution :');
  if length(resp_test1)>0
  resp_test2=strfind(line,'Integration');
  resp_test3=strfind(line,'Total Time');
  Res_freq=str2num(line(resp_test1(1)+17:resp_test2(1)-1));
  Res_Int=str2num(line(resp_test2(1)+18:resp_test3(1)-1));
  Res_Tot=str2num(line(resp_test3(1)+12:length(line)));
  set(real_fr,'String',Res_freq);
  set(real_time,'string',Res_Int);
  set(Total_time,'String',Res_Tot);
  end
  resp_thres=strfind(line,'Noise');
  if length(resp_thres)>0
     line1=line;
  Noise_reference=str2num(line(resp_thres(1)+15:length(line)));
  set(NoisePower,'String',num2str(Noise_reference),'min',Threshold_reference);
  end
  resp_cal=strfind(line,'Calibration');
  if length(resp_cal)>0
  line5=line;
  calibration=str2num(line(resp_cal(1)+13:length(line)))
  end
  resp_size=strfind(line,'Size');
  if length(resp_size)>0
  Size=str2num(line(resp_size(1)+5:length(line)))
  marqueur=1; 
  end
  resp2=strfind(line,'Data');
  if length(resp2)~=0
      stop=1;
  end
end

matrix_real=[];
while(stop1==0) 
line1=fgetl(fid);
if length(strfind(line1,'Real'))>0
matrix_real=fscanf(fid,'%e',[Size(1)*Size(2) 1]);
stop1=1
end
end

matrix_imag=[];
while(stop2==0) 
line2=fgetl(fid);
if length(strfind(line2,'Imag'))>0
matrix_imag=fscanf(fid,'%e');
stop2=1
end
end

matrix_noise=complex(matrix_real,matrix_imag);

if marqueur==1 
matrix_noise=reshape(matrix_noise,Size(1),Size(2));
set(0,'CurrentFigure',HMainFig_gene)
subplot(PolyphasePlot)
imagesc(20*log10(abs(matrix_noise)));
colormap(1-gray);
else
message='Here is Time Series';
title='Warning';
msgbox(message,title,'warn')
end
answer3{count_file3}=Filename_matrix2;
set(listbox2_load,'String',answer3);
clear matrix_real;
clear matrix_imag;
fclose(fid);