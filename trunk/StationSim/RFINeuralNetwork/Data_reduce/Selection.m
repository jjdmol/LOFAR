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
dim=5;
%fichier=ceil(index/4);
%Ds=reshape(select,,4);
select((find(select(:,5)>200)),5)=200;
select(:,5)=10*log10(select(:,5));
select=centree_reduit(select);

%Ds=Ds(1:100,:);
%taille de la carte en colonne
%tc=input('quelle taille de la carte en colonne ?\n');
%taille de la carte en ligne 
%tl=input('quelle taille de la carte en ligne ?\n');
%dimension de la carte 
[liselect,coselect]=size(select);
taille=liselect*coselect;
fid=fopen('sauvegarde6.txt','w');
fprintf(fid,'# setDim=%d setSize=%d setStat={0.0,0.0,0.0,0.0,0.0}\n',dim,liselect);
fprintf(fid,'%6.9f %6.9f %6.9f %6.9f %6.9f\n',select');
status=fclose(fid);

    