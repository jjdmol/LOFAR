clear Simulation_
count_file2=count_file2+1;
[Filename_load2,Pathname]=uigetfile('*.txt','Save The File ');
fid=fopen(Filename_load2,'r');
stop=0;
while(stop==0)   
  line=fgetl(fid);
  resp_f=strfind(line,'Fsampling :');
  if  length(resp)~=0
      resp_f=strfind(line,'Total')
      fs_2=str2num(line(resp(1)+11:resp1(1)-1));
  end
  line=fgetl(fid);
  resp=strfind(line,'FreqSample');
  if length(resp)>0
  Size=str2num(line(resp(1)+3:length(line)));
  end
  resp1=strfind(line,'Size');
  if length(resp1)>0
  Size=str2num(line(resp1(1)+5:length(line)));
  end
  resp_thres=strfind(line,'Threshold');
  if length(resp_thres)>0
  Threshold_reference=str2num(line(resp_thres(1)+20:length(line)));
  set(Noise_minValue,'String',num2str(Threshold_reference));
  end
  resp2=strfind(line,'Data');
  if length(resp2)~=0
      stop=1;
  resp2=strfind(line,'Data');
  if length(resp2)~=0
      stop=1;
  end
end


set(fs2,'String',fs_2)

mat=[];
mat=fscanf(fid,'%e');
answer2{count_file2}=Filename_load2;
set(listbox_load,'String',answer2);
fclose(fid);