global NbClasse;
fid = fopen(file_name,'r');
tline6=fgetl(fid);
proba=zeros(length(MyData2),NbClasse+1);
for i=1:length(MyData2)
    tempo=str2num(fgetl(fid));
    [m,n]=size(tempo);
    if n==0
    proba(i,:)=[i zeros(1,NbClasse)];
else
    proba(i,1:n+1)=[i tempo(1,:)];
end
end

proba=sortrows(proba,2);