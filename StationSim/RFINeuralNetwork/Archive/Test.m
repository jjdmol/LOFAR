classe=[];
for i=1:length(MatClass)
   [ligne,colonne]=find(MatClass(i,:)== max(MatClass(i,:)));
   if colonne==1
      sMap.labels{i}=['non']; 
      classe=[classe 1];
   else sMap.labels{i}=['OUI'];
      classe=[classe 2];
   end
end

%sMap = som_autolabel(sMap,sData,'vote');
figure
som_show(sMap,1);
%sMap=som_label(sMap,[1:49],lab[1:49])
som_addlabels(sMap,1,'all',[],'black')

%création d'une nouvelle structure de données à partir de Nino2,Nino3,Nino4 
t_fenetre=3;
var_nino_lab=2;
seuil_norm = 1;
alpha_norm=1;

%les valeurs de Température_Nino2
var_nino=3;
var_nino_lab=3;
[nino_don2, nino_labels2, nino_num_labels2, nino_varnames2] = load_nino_class(var_nino, var_nino_lab, t_fenetre, alpha_norm, seuil_norm);
sData2=som_data_struct(nino_don2,'nino2',nino_labels2, nino_varnames2);

%les valeurs de Température_Nino3
var_nino=4;
var_nino_lab=4;
[nino_don3, nino_labels3, nino_num_labels3, nino_varnames3] = load_nino_class(var_nino, var_nino_lab, t_fenetre, alpha_norm, seuil_norm);
sData3=som_data_struct(nino_don3,'nino3',nino_labels3, nino_varnames3);

%les valeurs de Température_Nino3
var_nino_lab=5;
var_nino=5;
[nino_don4, nino_labels4, nino_num_labels4, nino_varnames4] = load_nino_class(var_nino, var_nino_lab, t_fenetre, alpha_norm, seuil_norm);
sData4=som_data_struct(nino_don4,'nino4',nino_labels4, nino_varnames4);


% Matrice de confusion


[Bmus , Qerrors] = som_bmus(sMap , sData);
[Bmus2 , Qerrors2] = som_bmus(sMap , sData2);
[Bmus3 , Qerrors3] = som_bmus(sMap , sData3);
[Bmus4 , Qerrors4] = som_bmus(sMap , sData4);

% On rajoute 1 aux nino_nums_labels pour s'en servir comme indice de matrice de confusion.

nino_num_labels=nino_num_labels+1;
nino_num_labels2=nino_num_labels2+1;
nino_num_labels3=nino_num_labels3+1;
nino_num_labels4=nino_num_labels4+1;


Confusion1=0*ones(2,2);
Confusion2=0*ones(2,2);
Confusion3=0*ones(2,2);
Confusion4=0*ones(2,2);

for i=1:length(sData.data)
   
   classeInterm1=classe(Bmus(i));             %Classe du neurone activé par les points de Bmus
   classeInterm2=classe(Bmus2(i));
   classeInterm3=classe(Bmus3(i));
   classeInterm4=classe(Bmus4(i));
   
   classeVrai1=nino_num_labels(i);
   classeVrai2=nino_num_labels2(i);
   classeVrai3=nino_num_labels3(i);
   classeVrai4=nino_num_labels4(i);
  
   %Comptabilisation des erreurs faites par la carte
   
   Confusion1(classeVrai1 , classeInterm1)=Confusion1(classeVrai1 , classeInterm1)+1;
   Confusion2(classeVrai2 , classeInterm2)=Confusion2(classeVrai2 , classeInterm2)+1;
   Confusion3(classeVrai3 , classeInterm3)=Confusion3(classeVrai3 , classeInterm3)+1;
   Confusion4(classeVrai4 , classeInterm4)=Confusion4(classeVrai4 , classeInterm4)+1;
end

Confusion1
Confusion2
Confusion3
Confusion4

x1=sMap.codebook(:,:,1);
x2=sMap.codebook(:,:,2);
x3=sMap.codebook(:,:,3);

trans1=sMap.codebook(:,:,1)';
trans2=sMap.codebook(:,:,2)';
trans3=sMap.codebook(:,:,3)';


%Traçage de la visualistion de l'erreur sur les deux première coordonnées pour Nino1.


figure
hold on
axis on

for i=1:(length(classe))
   
   if classe(i)==1
      text(x1(i)+0.05,x2(i)+0.05,x3(i)+0.05,'non')
      plot3(x1(i),x2(i),x3(i),'ob')
      
   else 
      text(x1(i)+0.05,x2(i)+0.05,x3(i)+0.058,'OUI')
      plot3(x1(i),x2(i),x3(i),'om')
   end
end

%Visualisaton 3D des points

for i=1:length(sData.data)

classeInterm1=classe(Bmus(i));
classeVrai1=nino_num_labels(i);

if (classeVrai1==classeInterm1)
      plot3(sData.data(i,1),sData.data(i,2),sData.data(i,3),'*b')
   else
      plot3(sData.data(i,1),sData.data(i,2),sData.data(i,3),'*r')
   end
end
plot3(x1,x2,x3,'b',trans1,trans2,trans3,'b')
rotate3d on
hold off
title('Visualisation 3D de la carte et des données de Température nino1 (en bleu : les données bien classées) (en rouge : les données mal classées)');
%Traçage de la visualistion 3D avec en rouge points mal classés pour Nino2.
figure
hold on
axis on

for i=1:(length(classe))
   
   if classe(i)==1
      text(x1(i)+0.05,x2(i)+0.05,x3(i)+0.05,'non')
      plot3(x1(i),x2(i),x3(i),'ob')
      
   else 
      text(x1(i)+0.05,x2(i)+0.05,x3(i)+0.058,'OUI')
      plot3(x1(i),x2(i),x3(i),'om')
   end
end

%Visualisaton 3D des points

for i=1:length(sData2.data)

classeInterm2=classe(Bmus2(i));
classeVrai2=nino_num_labels2(i);

if (classeVrai2==classeInterm2)
      plot3(sData2.data(i,1),sData2.data(i,2),sData2.data(i,3),'*b')
   else
      plot3(sData2.data(i,1),sData2.data(i,2),sData2.data(i,3),'*r')
   end
end
plot3(x1,x2,x3,'b',trans1,trans2,trans3,'b')
rotate3d on
hold off
title('Visualisation 3D de la carte et des données de Température nino2(en bleu : les données bien classées) (en rouge : les données mal classées)');


%Traçage de la visualistion 3D avec en rouge points mal classés pour Nino3.


figure
hold on
axis on

for i=1:(length(classe))
   
   if classe(i)==1
      text(x1(i)+0.05,x2(i)+0.05,x3(i)+0.05,'non')
      plot3(x1(i),x2(i),x3(i),'ob')
      
   else 
      text(x1(i)+0.05,x2(i)+0.05,x3(i)+0.058,'OUI')
      plot3(x1(i),x2(i),x3(i),'om')
   end
end

%Visualisaton 3D des points

for i=1:length(sData3.data)

classeInterm3=classe(Bmus3(i));
classeVrai3=nino_num_labels3(i);

if (classeVrai3==classeInterm3)
      plot3(sData3.data(i,1),sData3.data(i,2),sData3.data(i,3),'*b')
   else
      plot3(sData3.data(i,1),sData3.data(i,2),sData3.data(i,3),'*r')
   end
end
plot3(x1,x2,x3,'b',trans1,trans2,trans3,'b')
rotate3d on
hold off
title('Visualisation 3D de la carte et des données de Température nino3(en bleu : les données bien classées) (en rouge : les données mal classées)');


%Traçage de la visualistion 3D avec en rouge points mal classés pour Nino4.
figure
hold on
axis on

for i=1:(length(classe))
   
   if classe(i)==1
      text(x1(i)+0.05,x2(i)+0.05,x3(i)+0.05,'non')
      plot3(x1(i),x2(i),x3(i),'ob')
      
   else 
      text(x1(i)+0.05,x2(i)+0.05,x3(i)+0.058,'OUI')
      plot3(x1(i),x2(i),x3(i),'om')
   end
end

%Visualisaton 3D des points

for i=1:length(sData4.data)

classeInterm4=classe(Bmus4(i));
classeVrai4=nino_num_labels4(i);

if (classeVrai4==classeInterm4)
      plot3(sData4.data(i,1),sData4.data(i,2),sData4.data(i,3),'*b')
   else
      plot3(sData4.data(i,1),sData4.data(i,2),sData4.data(i,3),'*r')
   end
end
plot3(x1,x2,x3,'b',trans1,trans2,trans3,'b')
rotate3d on
hold off
title('Visualisation 3D de la carte et des données de Température nino4(en bleu : les données bien classées) (en rouge : les données mal classées)');



% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Projection de la carte et des donnees a l'aide de
%   l'ACP Matlab: PRINCOMP
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

N = size(sMap.codebook,3);
X = reshape(sMap.codebook,prod(sMap.msize),N);
[PC, SCORE, LATENT, TSQUARE] = princomp(X);
%Calcule les nouveaux codebooks projetes dans les axes de l'ACP
newX = X * PC;

New_codebook = reshape(newX, [sMap.msize, N]);

Xcb1 = New_codebook(:,:,1);
Xcb2 = New_codebook(:,:,2);
Xcb3 = New_codebook(:,:,3);

Xcbt1=Xcb1';
Xcbt2=Xcb2';
Xcbt3=Xcb3';

%_________________________________________________________________________________________
%Donnees projetes dans les axes de l'ACP
new_nino_don = nino_don * PC;

Xdon1 = new_nino_don(:,1);
Xdon2 = new_nino_don(:,2);
Xdon3 = new_nino_don(:,3);

figure
hold on
for i=1:(length(classe))
   
   if classe(i)==1
      text(Xcb1(i)+0.05,Xcb2(i)+0.05,'non')
      plot(Xcb1(i),Xcb2(i),'ob')
      
   else 
      set(text,'Color','m')
      text(Xcb1(i)+0.05,Xcb2(i)+0.05,'OUI')
      plot(Xcb1(i),Xcb2(i),'om')
      set(text,'Color','black')
   end
end
plot(Xcb1,Xcb2,'b',Xcbt1,Xcbt2,'b')
      

for i=1:length(sData3.data)
   
classeInterm1=classe(Bmus(i));
classeVrai1=nino_num_labels(i);
if (classeVrai1==classeInterm1)
      plot(Xdon1(i),Xdon2(i),'*b')
   else
      plot(Xdon1(i),Xdon2(i),'*r')
   end
end
%set(gca,'YDir','reverse')
%axis image
hold off
title('Visualisation de la carte (plan 1/2)et des données de Température nino1 après ACP(en bleu : les données bien classées) (en rouge : les données mal classées)');new_nino_don = nino_don * PC;

%_______________________________________________________________________________________________

new_nino_don = nino_don * PC;

Xdon1 = new_nino_don(:,1);
Xdon2 = new_nino_don(:,2);
Xdon3 = new_nino_don(:,3);

figure
hold on
for i=1:(length(classe))
   
   if classe(i)==1
      text(Xcb1(i)+0.05,Xcb3(i)+0.05,'non')
      plot(Xcb1(i),Xcb3(i),'ob')
      
   else 
      set(text,'Color','m')
      text(Xcb1(i)+0.05,Xcb3(i)+0.05,'OUI')
      plot(Xcb1(i),Xcb3(i),'om')
      set(text,'Color','black')
   end
end
plot(Xcb1,Xcb3,'b',Xcbt1,Xcbt3,'b')
      

for i=1:length(sData3.data)
   
classeInterm1=classe(Bmus(i));
classeVrai1=nino_num_labels(i);
if (classeVrai1==classeInterm1)
      plot(Xdon1(i),Xdon3(i),'*b')
   else
      plot(Xdon1(i),Xdon3(i),'*r')
   end
end
%set(gca,'YDir','reverse')
%axis image
hold off
title('Visualisation (plan 1/3)de la carte et des données de Température nino1 après ACP(en bleu : les données bien classées) (en rouge : les données mal classées)');

%__________________________________________________________________________________________________________

Xdon1 = new_nino_don(:,1);
Xdon2 = new_nino_don(:,2);
Xdon3 = new_nino_don(:,3);

figure
hold on
for i=1:(length(classe))
   
   if classe(i)==1
      text(Xcb2(i)+0.05,Xcb3(i)+0.05,'non')
      plot(Xcb2(i),Xcb3(i),'ob')
      
   else 
      set(text,'Color','m')
      text(Xcb2(i)+0.05,Xcb3(i)+0.05,'OUI')
      plot(Xcb2(i),Xcb3(i),'om')
      set(text,'Color','black')
   end
end
plot(Xcb2,Xcb3,'b',Xcbt2,Xcbt3,'b')
      

for i=1:length(sData3.data)
   
classeInterm1=classe(Bmus(i));
classeVrai1=nino_num_labels(i);
if (classeVrai1==classeInterm1)
      plot(Xdon2(i),Xdon3(i),'*b')
   else
      plot(Xdon2(i),Xdon3(i),'*r')
   end
end
%set(gca,'YDir','reverse')
%axis image
hold off
title('Visualisation de la carte et des données de Température nino1 après ACP(en bleu : les données bien classées) (en rouge : les données mal classées)');
%_______________________________________________________________________________________________________________________________

%Projection des données pour l'ACP

new_nino_don = nino_don2 * PC;

Xdon1 = new_nino_don(:,1);
Xdon2 = new_nino_don(:,2);
Xdon3 = new_nino_don(:,3);

figure
hold on
for i=1:(length(classe))
   
   if classe(i)==1
      text(Xcb1(i)+0.05,Xcb2(i)+0.05,'non')
      plot(Xcb1(i),Xcb2(i),'ob')
      
   else 
      set(text,'Color','m')
      text(Xcb1(i)+0.05,Xcb2(i)+0.05,'OUI')
      plot(Xcb1(i),Xcb2(i),'om')
      set(text,'Color','black')
   end
end
plot(Xcb1,Xcb2,'b',Xcbt1,Xcbt2,'b')

for i=1:length(sData3.data)
   
classeInterm2=classe(Bmus2(i));
classeVrai2=nino_num_labels2(i);
if (classeVrai2==classeInterm2)
      plot(Xdon1(i),Xdon2(i),'*b')
   else
      plot(Xdon1(i),Xdon2(i),'*r')
   end
end

%set(gca,'YDir','reverse')
%axis image
hold off
title('Visualisation de la carte et des données de Température nino2 après ACP(en bleu : les données bien classées) (en rouge : les données mal classées)');


%Donnees projetes dans les axes de l'ACP
new_nino_don = nino_don3 * PC;

Xdon1 = new_nino_don(:,1);
Xdon2 = new_nino_don(:,2);
Xdon3 = new_nino_don(:,3);

figure
hold on
for i=1:(length(classe))
   
   if classe(i)==1
      text(Xcb1(i)+0.05,Xcb2(i)+0.05,'non')
      plot(Xcb1(i),Xcb2(i),'ob')
      
   else 
      text(Xcb1(i)+0.05,Xcb2(i)+0.05,'OUI')
      plot(Xcb1(i),Xcb2(i),'om')
   end
end
plot(Xcb1,Xcb2,'b',Xcbt1,Xcbt2,'b')

for i=1:length(sData3.data)
   
classeInterm3=classe(Bmus3(i));
classeVrai3=nino_num_labels3(i);
if (classeVrai3==classeInterm3)
      plot(Xdon1(i),Xdon2(i),'*b')
   else
      plot(Xdon1(i),Xdon2(i),'*r')
   end
end

%set(gca,'YDir','reverse')
%axis image
hold off
title('Visualisation de la carte et des données de Température nino3 après ACP(en bleu : les données bien classées) (en rouge : les données mal classées)');



%Donnees projetes dans les axes de l'ACP
new_nino_don = nino_don4 * PC;

Xdon1 = new_nino_don(:,1);
Xdon2 = new_nino_don(:,2);
Xdon3 = new_nino_don(:,3);

figure
hold on
for i=1:(length(classe))
   
   if classe(i)==1
      text(Xcb1(i)+0.05,Xcb2(i)+0.05,'non')
      plot(Xcb1(i),Xcb2(i),'ob')
      
   else 
      set(text,'Color','m')
      text(Xcb1(i)+0.05,Xcb2(i)+0.05,'OUI')
      plot(Xcb1(i),Xcb2(i),'om')
      set(text,'Color','black')
   end
end
plot(Xcb1,Xcb2,'b',Xcbt1,Xcbt2,'b')

for i=1:length(sData4.data)
   
classeInterm4=classe(Bmus4(i));
classeVrai4=nino_num_labels4(i);
if (classeVrai4==classeInterm4)
      plot(Xdon1(i),Xdon2(i),'*b')
   else
      plot(Xdon1(i),Xdon2(i),'*r')
   end
end
%set(gca,'YDir','reverse')
%axis image
hold off
title('Visualisation de la carte et des données de Température nino4 après ACP(en bleu : les données bien classées) (en rouge : les données mal classées)');


%Erreur de quantification

qerror = som_quality(sMap,'qe',sData)
topog_error = som_quality(sMap,'topog',sData)
energy_error = som_quality(sMap,'energy',sData,3)     %voir voisinage


qerror = som_quality(sMap,'qe',sData2)
topog_error = som_quality(sMap,'topog',sData2)
energy_error = som_quality(sMap,'energy',sData2,3)

qerror = som_quality(sMap,'qe',sData3)
topog_error = som_quality(sMap,'topog',sData3)
energy_error = som_quality(sMap,'energy',sData3,3)

qerror = som_quality(sMap,'qe',sData4)
topog_error = som_quality(sMap,'topog',sData4)
energy_error = som_quality(sMap,'energy',sData4,3)