global MyData;
global sizeMap;
fid=fopen(file_name,'r');
MyData=[];
disp(['here: ' file_name ' ']);
tline1=fgetl(fid);
cmy=str2num(tline1(1:findstr(tline1,'rect')-1))+1%+1 pour la colonne des cardinalites
esp2=strfind(tline1,' ');
type_ap=tline1(esp2(1):esp2(2));
type_fenetre=tline1(esp2(4):length(tline1));
sizeMap(1)=str2num(tline1(esp2(2):esp2(3)));
sizeMap(2)=str2num(tline1(esp2(3):esp2(4)));
%str=finstr(tline1,'gaussian')
MyData=fscanf(fid,'%e');
MyData=reshape(MyData,cmy,length(MyData)/cmy);
MyData=MyData';
fclose(fid);