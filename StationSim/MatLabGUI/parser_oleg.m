load('data/antenna_signals.mat');
[li,col]=size(AntennaSignals);
fid=fopen('C:\Documents and Settings\broekema\Desktop\phased.txt','r');
stop2=0;
stop1=0;
tic
while(stop1==0) 
line1=fgetl(fid);
if length(strfind(line1,'Antenna 1'))>0
matrix_antenna1=fscanf(fid,'%e',[li col]);
stop1=1
end
end
matrix_final1=complex(matrix_antenna1(1,:),matrix_antenna1(2,:));
toc

t=0;
while(stop2==0) 

line2=fgetl(fid);
if (length(strfind(line2,'Blaat'))>0)
    t=t+1
    matrix_antenna2=fscanf(fid,'%e',[li col])
    stop2=1
end
end
matrix_final2=complex(matrix_antenna2(1,:),matrix_antenna2(2,:));
fclose(fid)