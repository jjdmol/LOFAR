% Read in a data file
global R;
[file_name,path_name] = uigetfile('*.map;*.res;*.xMap;*.txt','Choix d''un fichier de donnee',100,220);
count=count+1;
if (file_name == 0)
   errordlg('Fichier non trouvé.','File Error');
else
   file_name = [path_name file_name];
end
read=file_name(length(path_name)+1:length(file_name));
R{count}=read
%get file extension
size1 = size(file_name);
size2 = size1(2);
ext = file_name(size2-3:size2);
%open file 
if (ext == '.map')
  lectrurefic1;
elseif (ext == '.txt')
    lecture;
elseif (ext== '.res');
   if length(strfind(file_name,'Acti'))>0
       lectrurefic4;
   elseif length(strfind(file_name,'Var'))>0 
       lectrurefic5;
   elseif length(strfind(file_name,'Proba'))>0 
       lectrurefic6;    
   else 
   lectrurefic2;
   end
elseif (ext== 'xMap');
lectrurefic3;
end
