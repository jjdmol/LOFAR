

%global declarations
%global ch1;
%global ch2;
%global x1;
%global x2;
%global e;
%global nb_spectre;
%global size_spectre;
global echantillonnage;
global freq;
global numero;
global DSP;
global MyData2;
global numero_donnee;
global Fig_VarDonnee;
global range;
%local declarations
MEGA = 1e6;

%initialisation of size_spectre for all sub-functions
size_spectre = 8192;
resultat_Activation1=[];
resultat_Activation=[];
%Activation en histogramme


%changement de la variable DSP-----------------------------------------------------
%Recherche des neurones Activés pour les afficher sur l'histogramme

G=colormap(hsv(length(range(:,3))));
%Jcolor=colormap(hot(length(resultat_Activation(:,3))));    
    
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
%Xcb3 = New_codebook(:,:,3);
%Xcb4 = New_codebook(:,:,4);
%Xcb5 = New_codebook(:,:,5);

Xcbt1=Xcb1';
Xcbt2=Xcb2';
%Xcbt3=Xcb3';
%Xcbt4=Xcb4';
%Xcbt5=Xcb5';

%_________________________________________________________________________________________
%Donnees projetes dans les axes de l'ACP
new_don = donnee * PC;

Xdon1 = new_don(:,arg_acpplan1);
Xdon2 = new_don(:,arg_acpplan2);
%Xdon3 = new_don(:,3);
%Xdon4 = new_don(:,4);
%Xdon5 = new_don(:,5);
       
if (arg_acpplan1<cmy & arg_acpplan2<=cmy & arg_acpplan2>arg_acpplan1)

subplot (HCarteFig); 
hold on
plot(Xcb1,Xcb2,Xcbt1,Xcbt2,'b')
plot(Xdon1,Xdon2,'*b');

for i=1:(taille_carte)
       couleur=MyData3(i);
       text(Xcb1(i)+0.005,Xcb2(i)+0.005,[' ' num2str(i) '-' num2str(couleur) ''],'Color',U(couleur,:))
       plot(Xcb1(i),Xcb2(i),'ob')     
end

for i=1:(length(range(:,3)))
       plot(Xcb1(range(i,2)),Xcb2(range(i,2)),'o','MarkerFaceColor',G(i,:))     
end
text(Xdon1(numero_donnee)+0.005,Xdon2(numero_donnee)+0.005,num2str(numero_donnee*echantillonnage),'Color','r'),   
plot(Xdon1(numero_donnee),Xdon2(numero_donnee),'*r');
set(gcf,'windowbuttondownfcn','ecris_freq');  


else message='Entrer des valeurs valables pour les plans de l''ACP';
    title='Attention';
    msgbox(message,title,'warn')

end

subplot (HistoFig);

hold on
for i=1:length(range(:,3))
bar(range(i,2),range(i,3))     
h = findobj(gca,'Type','patch')
end
h=flipud(h);
for i=1:length(range(:,3))
set(h(i),'FaceColor',G(i,:));
end
hold off

subplot (Fig_VarDonnee);

hold on
ranger=[];
ranger=[range(:,2),MyVar(range(:,2)),range(:,3)]
[ranger]=sortrows(ranger,3);

for i=1:length(ranger(:,1))
bar(ranger(i,1),ranger(i,2))     
hV = findobj(gca,'Type','patch')
end
%hV=flipud(hV);
for i=1:length(range(:,2))
set(hV(i),'FaceColor',G(i,:));
end
hold off

subplot (HTimeFig_Donnee);
plot(10*log10(DSP(numero_donnee,:)))
