poid=[];

%poid=MyData(:,1:5);

for i=1:NbClasse
        neurone_classe=find(MyData3==i);
        donnee_selection=[];
        for j=1:length(neurone_classe)       
        donnee_select=find(MyData2==neurone_classe(j)-1);
        donnee_selection=[donnee_selection;donnee_select];
        end
        donnee_ben{i}=donnee_selection;
    end
[lid,cod]=size(donnee);

label=zeros(length(MyData2),1);
for i=1:length(donnee_ben)
    label(donnee_ben{i})=i;
end

sortie_lab=[];
sortie_lab=[donnee label];

sortie_neurone=[];
sortie_neurone=[MyData(:,1:9) MyData3];

dim=2;

%fid=fopen('label.res','w');
%fprintf(fid,'# mapDim=%d mapSize={8,8} pointDim=%d mapMin={%d,%d,%d,%d,%d} mapMax={%d,%d,%d,%d,%d}\n',...
%    dim,cod,min(donnee(:,1)),min(donnee(:,1)),min(donnee(:,2)),min(donnee(:,3)),min(donnee(:,4)),min(donnee(:,5)),...
%    max(donnee(:,1)),max(donnee(:,2)),max(donnee(:,3)),max(donnee(:,4)),max(donnee(:,5)));
%fprintf(fid,'%f %f %f %f %f %f\n',sortie_lab');
%status=fclose(fid);

fid1=fopen('label.res','w');
fprintf(fid1,'# mapDim=%d mapSize={%d,%d} pointDim=%d mapMin={%d,%d,%d,%d,%d,%d,%d,%d,%d} mapMax={%d,%d,%d,%d,%d,%d,%d,%d,%d}\n',dim,msize(1),msize(2),cod,min(donnee(:,1)),min(donnee(:,2)),min(donnee(:,3)),min(donnee(:,4)),min(donnee(:,5)),min(donnee(:,6)),min(donnee(:,7)),min(donnee(:,8)),min(donnee(:,9)),max(donnee(:,1)),max(donnee(:,2)),max(donnee(:,3)),max(donnee(:,4)),max(donnee(:,5)),max(donnee(:,6)),max(donnee(:,7)),max(donnee(:,8)),max(donnee(:,9)));
fprintf(fid1,'%f\n',label');
status=fclose(fid1);

fid2=fopen('prholcont2_IndexMap.map','w');
fprintf(fid2,'# LabelMap mapDim=%d mapSize={%d,%d} pointDim=%d mapMin={%d,%d,%d,%d,%d,%d,%d,%d,%d} mapMax={%d,%d,%d,%d,%d,%d,%d,%d,%d}\n',dim,msize(1),msize(2),cod,min(donnee(:,1)),min(donnee(:,2)),min(donnee(:,3)),min(donnee(:,4)),min(donnee(:,5)),min(donnee(:,6)),min(donnee(:,7)),min(donnee(:,8)),min(donnee(:,9)),max(donnee(:,1)),max(donnee(:,2)),max(donnee(:,3)),max(donnee(:,4)),max(donnee(:,5)),max(donnee(:,6)),max(donnee(:,7)),max(donnee(:,8)),max(donnee(:,9)));
fprintf(fid2,'%f %f %f %f %f %f %f %f %f %f\n',sortie_neurone');
status=fclose(fid2);
