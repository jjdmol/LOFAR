
global MyData;
global MyData2;
global MyData3;

fid=fopen(file_name,'r');
tline = fgetl(fid);
dimens=str2num(tline(findstr(tline,'setDim')+7:findstr(tline,'setSize')-1))
taille=str2num(tline(findstr(tline,'setSize')+8:findstr(tline,'setStat')-1))
donnee=fscanf(fid,'%e');
fclose(fid);
donnee=reshape(donnee,dimens,taille);
donnee=donnee';

%load des données de la carte etr des classes obtenues après CAH

%Création d'une structure de données pour la carte
nino_varnames=cell(5,1);
nino_varnames{1}=['moy0/std0'];
nino_varnames{2}=['moy/std'];
nino_varnames{3}=['skewness'];
nino_varnames{4}=['kutorsis'];
nino_varnames{5}=['10*log10(moy)'];
nino_varnames{6}=['minClip'];
nino_varnames{7}=['min'];
nino_varnames{8}=['10*log10(MaxClip)'];
nino_varnames{9}=['10*log10(Max)'];
nino_varnames{10}=['sample0'];
nino_varnames{11}=['sample1'];
nino_varnames{12}=['sample2'];
nino_varnames{13}=['sample3'];
nino_varnames{14}=['sample4'];
nino_varnames{15}=['sample5'];
nino_varnames{16}=['sample6'];
nino_varnames{17}=['sample7'];
nino_varnames{18}=['sample8'];
nino_varnames{19}=['sample9'];

donnee_smap=cell(dimens,1);

for i=1:dimens
        donnee_smap{i}=nino_varnames{i};
end
    
labels=[];%Pas de label des données 
sData = som_data_struct(donnee,'Deca',labels,donnee_smap);


%Initialisation de la structure de la carte avec le données issues de Spoutnik
msize     = [sizeMap(1) sizeMap(2)];
init_type = 'random';   %% 'random' ou 'linear'
lattice   = 'hexa';     %% 'hexa' ou 'rect'
shape     = 'rect';     %% 'rect', 'cyl', ou 'toroid'
sMap  = som_init(sData, msize, init_type, lattice, shape);

for i=1:dimens 
sMap.codebook(:,:,i)=reshape(MyData(:,i),sizeMap(1),sizeMap(2));
end


