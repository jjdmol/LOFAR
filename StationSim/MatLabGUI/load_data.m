fid=fopen('signal_example.txt','r');  
stop1=0;
while(stop1==0) 
  line=fgetl(fid);
  resp=strfind(line,'Data');
  if length(resp)>0
   stop1=1;
  end
end
data=fscanf(fid,'%e');