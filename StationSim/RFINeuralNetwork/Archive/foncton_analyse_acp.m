function [ACP]=foncton_analyse_acp(x)% Script effectuant l'analyse en composantes principales d'une matrice 
% contenant sur chaque ligne les differents individus et sur chaque colonne
% les differents parametres.
%
% Les variables de sortie sont :
% ACP     : matrice dont chaque ligne contient une valeur propre (classées
%           dans l'ordre décroissant) et le vecteur propre correspondant
% CP      : coordonnées des individus dans les plans principaux
% Contrib : contribution des individus à chaque axe
% QR	  : qualité de représentation des individus par chaque axe
% VEPV	  : coordonnées des paramètres dans les plans principaux
%
% Les sorties graphiques sont :
% - inertie et inertie cumulée de chacune des composantes principales
% - coordonnées des individus dans le plan principal (1,2)
% - coordonnées des paramètres dans le plan principal (1,2)

%_______________________________________________________________________________



	% Lecture des données et calcul de l'ACP
	%_______________________________________________________________________


% Nettoyage de variables
%-----------------------
%clear X ACP Contrib QR VEPU VEPV inertie CP dist 

%Entree des donnees
%------------------

%donnees=input('matrice des données : ','s');
%eval(['x=' donnees ';']);

%donnees centrees
%----------------
X=centree(x);

%Calcul de l'ACP
%---------------
ACP=acp(X);
nb=length(ACP(:,1));



	% Calcul des variables de sortie
	%________________________________________________________________________


% Inertie et inertie cumulée des valeurs propres
%-----------------------------------------------
inertie(:,1) = (ACP(:,1)/sum(ACP(:,1)))*100;
inertie(:,2) = cumsum(inertie(:,1));


% Coordonnées(i,j) des individus(i) dans les plans principaux(j) 
% --------------------------------------------------------------
VEPU=ACP(:,2:nb+1);
VEPU=VEPU';
CP=X*VEPU;

% Contribution(i,j) d'un individu(i) à la fabrication d'un axe(j)
%----------------------------------------------------------------
Contrib=(1/size(X,1))*(CP.^2)*100./(ones(size(X))*diag(ACP(:,1)));

% Qualite de representation(i,j) de l'individu(i) par l'axe(j)
%-------------------------------------------------------------
dist=(sum((X').^2))';
QR=(CP.^2)./(diag(dist)*(ones(size(X))));

% Coordonnées(i,j) des variables(i) dans les plans principaux(j)
%---------------------------------------------------------------
VEPV=VEPU.*repmat(sqrt(ACP(:,1))',nb,1);



	% Sorties Graphiques
	%_______________________________________________________________________

% Histogramme de l'inertie et inertie cumulée
%--------------------------------------------
figure
bar(inertie(:,1));
hold on
plot(inertie(:,2),'-or')
titre=['Inertie des ' int2str(nb) ' premieres valeurs propres'];
title(titre);
axis([0.5 nb+0.5 0 100])

% Nuage des individus sur le plan factoriel engendré par les 2 premiers axes principaux
%--------------------------------------------------------------------------------------
figure
plot(CP(:,1),CP(:,2),'*');
for i=1:length(CP(:,1))
text(CP(i,1)-0.02,CP(i,2)+0.12,num2str(i));
end
axe=axis;
hold on
plot([axe(1) axe(2)],[0 0],'k',[0 0],[axe(3) axe(4)],'k')
title('Nuage des individus sur le plan des 2 premiers axes principaux');

% Nuage des paramètres sur le plan factoriel engendré par les 2 premiers axes principaux
%---------------------------------------------------------------------------------------
figure
% Tracé du cercle de rayon 1 et de centre 0
t=[-pi:0.01:pi];
x=cos(t);
y=sin(t);
plot(x,y,'r');
hold on
compass(VEPV(:,1),VEPV(:,2));
for i=1:length(VEPV(:,1))
text(VEPV(i,1)-0.02,VEPV(i,2)+0.08,num2str(i));
end
title('Nuage des paramètres sur leplan des 2 premiers axes principaux');
axis('equal')
axe=axis;
plot([axe(1) axe(2)],[0 0],'k',[0 0],[axe(3) axe(4)],'k')