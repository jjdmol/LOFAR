global fichier;
fid=fopen(file_name,'r');
disp(['here: ' file_name ' ']);
tline4=fgetl(fid);
alors=strfind(tline4,'mapSize');
nombre_point=str2num(tline4(alors+8:alors+10));
fichier=cell(819,1);

for i=1:nombre_point;
    fichier{i}=fgetl(fid);
end

fclose(fid);