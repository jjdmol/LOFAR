
 load('data/Path_info.mat')

[Filename_load,Pathname]=uigetfile([Path_dat '*.dat'],'Load The File ');

 fid=fopen([Path_dat Filename_load],'r');
 count_signal=0;
 count_file=count_file+1;
stop=0;
car=0;
feature_mod=[];
while(stop==0)   
  line=fgetl(fid);
  resp=strfind(line,'Fsampling');
  if length(resp)>0
  resp1=strfind(line,'Total');
  resp2=strfind(line,'Fs_signal');
  fs=str2num(line(resp(1)+11:resp1(1)-1));
  if length(resp1)>0&length(resp2)==0
    message='You must load a signal generated and not a modulated one';
    title='Warning';
    msgbox(message,title,'Warn')
    stop=1;
    car=1;
  else 
  time=str2num(line(resp1(1)+12:resp2(1)-1));
  if count_file>1
  fsHold=fs;
  end
  line1=line;
  end
  end
  resp2=strfind(line,'Data');
  if length(resp2)~=0
      stop=1;
  end
end

if car~=1
        if (count_file>1 & fsHold~=fs) 
            message='You have load a file with different sampling frequency';
             title='Warning';
             msgbox(message,title,'Warn')
        end
        answer{count_file}=Filename_load;
        set(Visu_Fs,'string',fs);
        set(Visu_Time,'string',time);
        set(Load_listbox,'string',answer);
        mat=[];
        fsHold=fs;
        mat=fscanf(fid,'%e');
        set(Visu_Size,'string',length(mat)*8);
        if (count_signal==0 & count_file==1)
             Signal_modulation=zeros(length(mat),1); 
        end
        clear resampler;
        fclose(fid);
end