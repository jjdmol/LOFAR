D=sortie;
[lig,coll]=size(D);
index=0;
selection=1;
index=ceil(lig/10)-1;
select=[];

%pour 1s de données on selectionne 50 section temporelles  
for i=1:index
    selection=10*i;
    select(i,:)=D(selection,:);
end
dim=coll;
%fichier=ceil(index/4);
%Ds=reshape(select,,4);
%evite une dynamique trop forte entre les points
select((find(select(:,5)>200)),5)=200;%mean
select((find(select(:,6)>200)),6)=200;%median0
select((find(select(:,7)>200)),7)=200;%median1
select((find(select(:,9)>200)),9)=200;%maximum0
select((find(select(:,10)>200)),10)=200;%maximum1

%Passage au log;
select(:,5)=10*log10(select(:,5));%mean
select(:,6)=10*log10(select(:,6));%median0
select(:,7)=10*log10(select(:,7));%median1
select(:,9)=10*log10(select(:,9));%max0
select(:,10)=10*log10(select(:,10));%max1

select2=[];
for i=1:coll
    select2(:,i)=(select(:,i)-mean(select(:,i))/(max(select(:,i))-min(select(:,i))));
    %select2(:,i)=select2(:,i)-min(select2(:,i));
end

%select=centree_reduit(select);

%Ds=Ds(1:100,:);
%taille de la carte en colonne
%tc=input('quelle taille de la carte en colonne ?\n');
%taille de la carte en ligne 
%tl=input('quelle taille de la carte en ligne ?\n');
%dimension de la carte 
[liselect,coselect]=size(select);
taille=liselect*coselect;
%fid=fopen('sauvegarde6.txt','w');
fid1=fopen('savecontmed2.txt','w');
%fprintf(fid,'# setDim=%d setSize=%d setStat={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0}\n',dim,liselect);
fprintf(fid1,'# setDim=%d setSize=%d setStat={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0}\n',dim,liselect);
%fprintf(fid,'%6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f 6.9f\n',select');
fprintf(fid1,'%6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f %6.9f\n',select2');
%status=fclose(fid);
status1=fclose(fid1);
    