global proba;
global Contrl_Classe5;
global Contrl_seuil5;
global U;
global HTimeFig5;
global Vecteur_choisi;
global Contrl_donnee5;
global n_choice;
choix_proba=get(Contrl_Classe5,'string');
seuil=get(Contrl_seuil5,'string');


choix_proba=str2num(choix_proba);
seuil=str2num(seuil);

%[ligne,colonne]=size(DSP);
%moy=mean(DSP(visua(selected,2),:));
   %ecart=std(DSP(visua(selected,2),:));
   %disp('moy = 'num2str(moy) ' ');
   %disp('ecart='num2str(ecart) ' ');
  % trimbal=ones(8192,1);
  %trimbal(visua(:,2))=DSP(visua(:,2),colonne/2);
 
 %subplot(HDspFig);
 %plot(10*log10(trimbal));
Vecteur_choisi=[];
Vecteur_choisi1=[];
Vecteur_choisi2=[];
Vecteur_choix=[];
hop=4;
for i=0:n_choice 
t=i*2+2;
voici=find(proba(:,t)==choix_proba);
[Vecteur_choisi1]=proba(voici,1);
if length(voici)~=0
Vecteur_choisi2=[Vecteur_choisi1 proba(voici,hop-1)];
Vecteur_choix=[Vecteur_choix;Vecteur_choisi2];
end
hop=hop+2;
end
Vecteur_choix=sortrows(Vecteur_choix,2);

Vecteur_choisi=Vecteur_choix(find(Vecteur_choix(:,2)>=seuil),:);

Id=1:length(Vecteur_choisi(:,1));
%couleur_proba=colormap(hsv(length(Vecteur_choisi(:,2))));
set(Contrl_Actives5,'String',num2str([Id' Vecteur_choisi(:,1)*echantillonnage Vecteur_choisi(:,2)]));

%Pour la carte 

%=====================================================
% Read in a data file

[ligcarte,colcarte]=size(sMap.codebook(:,:,3));
taille_carte=colcarte*ligcarte;

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

Xcb1 = New_codebook(:,:,arg_acpplan1);
Xcb2 = New_codebook(:,:,arg_acpplan2);
Xcb3 = zeros(prod(sMap.msize),1);
Xcb3=reshape(Xcb3,msize(1),msize(2));
%Xcb4 = New_codebook(:,:,4);
%Xcb5 = New_codebook(:,:,5);

 
Xcbt1=Xcb1';
Xcbt2=Xcb2';
Xcbt3=Xcb3';
%Xcbt4=Xcb4';
%Xcbt5=Xcb5';

%_________________________________________________________________________________________
%Donnees projetes dans les axes de l'ACP
new_don = donnee * PC;

Xdon1 = new_don(:,arg_acpplan1);
Xdon2 = new_don(:,arg_acpplan2);
Xdon3 = zeros(length(MyData2),1);
Xdon3(Vecteur_choix(:,1))=Vecteur_choix(:,2);
%Xdon4 = new_don(:,4);
%Xdon5 = new_don(:,5);
       
if (arg_acpplan1<cmy & arg_acpplan2<=cmy & arg_acpplan2>arg_acpplan1)

subplot (HCarteFig5); 
hold on
plot3(Xcb1,Xcb2,Xcb3,Xcbt1,Xcbt2,Xcbt3,'b')
plot3(Xdon1,Xdon2,Xdon3,'*b');

for i=1:(taille_carte)
       couleur=MyData3(i);
       text(Xcb1(i)+0.005,Xcb2(i)+0.005,Xcb3(i)+0.005,[' ' num2str(i) '-' num2str(couleur) ''],'Color',U(couleur,:))   
       plot3(Xcb1(i),Xcb2(i),Xcb3(i),'o','MarkerFaceColor',U(couleur,:))     
end
for i=1:length(Vecteur_choisi)
    interm=Vecteur_choisi(i,1);
%text(Xdon1(interm)+0.005,Xdon2(interm)+0.005,Xdon3(interm)+0.005,num2str(interm*echantillonnage),'Color',couleur_proba(i,:)),   
plot3(Xdon1(interm),Xdon2(interm),Xdon3(interm),'*','MarkerEdgeColor',[Vecteur_choisi(i,2) 0 0]);
end

end


%log10(find(Vecteur_choisi(:,2)<1))/max(log10(find(Vecteur_choisi(:,2)<1))-0)
%Pour le specgram
subplot (HSpecgramFig5);
imagesc(DSP(Vecteur_choisi(:,1)*echantillonnage,:));
colormap(1-gray)
set(gcf,'windowbuttondownfcn','reader5_choix');  
%subplot(HThreeDFig)
%cla
%l3=line([0 1],[1 1]);%Car pas d'intégration.
%l4=line([0 1],[ecart/moy ecart/moy],'EraseMode','xor','Color',[1 0 0]);
   %axis ([0 77 0 max(max(todraw))]);
   %axis ([0 77 0 0.00005]);
   %else