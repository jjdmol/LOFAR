%function [select,select2,interm]=Selection3(sortie,DSP);
D=sortie;
[lig,coll]=size(D);
[lig2,col2]=size(DSP);
pas0=10;
pas1=10;
index=ceil(lig/pas0)-1;
index2=ceil(col2/pas1)-1;
select=[];

%pour 1s de données on selectionne 50 section temporelles  
selection=1:index;
selection=10*selection;
select=D(selection,:);


selection2=1:index2;
selection2=10*selection2;
interm=DSP(selection,:);
interm=interm(:,selection2);
interm=10*log10(interm);


dim=19;
%fichier=ceil(index/4);
%Ds=reshape(select,,4);
%evite une dynamique trop forte entre les points
%select((find(select(:,5)>200)),5)=200;%mean
%select((find(select(:,8)>200)),8)=200;%maximum0
%select((find(select(:,9)>200)),9)=200;%maximum1

%select(:,5)=10*log10(select(:,5));
%select(:,8)=10*log10(select(:,8));
%select(:,9)=10*log10(select(:,9));

%select2=[];
%for i=1:9
%    select2(:,i)=(select(:,i)-mean(select(:,i)))/(max(select(:,i))-min(select(:,i)));
%end

for i=1:index
interm(i,:)=(interm(i,:)-mean(interm(i,:))/(max(interm(i,:))-min(interm(i,:))));
%interm(i,:)=interm(i,:)-min(interm(i,:));
end
%select=centree_reduit(select);

%select3=[select2 interm];

%Ds=Ds(1:100,:);
%taille de la carte en colonne
%tc=input('quelle taille de la carte en colonne ?\n');
%taille de la carte en ligne 
%tl=input('quelle taille de la carte en ligne ?\n');
%dimension de la carte 
%[liselect,coselect]=size(select);
%taille=liselect*coselect;
%fid=fopen('sauvegarde64.txt','w');
fid1=fopen('savecontsample.txt','w');
%fprintf(fid,'# setDim=%d setSize=%d setStat={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0}\n',dim,liselect);
fprintf(fid1,'# setDim=%d setSize=%d setStat={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0}\n',dim,liselect);
%fprintf(fid,'%6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f\n',select');
fprintf(fid1,'%6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f%6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f%6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f%6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f%6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f%6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f%6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f%6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f%6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f%6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f\n',interm');
%status=fclose(fid);
%status1=fclose(fid1);
    