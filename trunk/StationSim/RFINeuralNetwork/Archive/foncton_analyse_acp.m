function [ACP]=foncton_analyse_acp(x)% Script effectuant l'analyse en composantes principales d'une matrice 
% contenant sur chaque ligne les differents individus et sur chaque colonne
% les differents parametres.
%
% Les variables de sortie sont :
% ACP     : matrice dont chaque ligne contient une valeur propre (class�es
%           dans l'ordre d�croissant) et le vecteur propre correspondant
% CP      : coordonn�es des individus dans les plans principaux
% Contrib : contribution des individus � chaque axe
% QR	  : qualit� de repr�sentation des individus par chaque axe
% VEPV	  : coordonn�es des param�tres dans les plans principaux
%
% Les sorties graphiques sont :
% - inertie et inertie cumul�e de chacune des composantes principales
% - coordonn�es des individus dans le plan principal (1,2)
% - coordonn�es des param�tres dans le plan principal (1,2)

%_______________________________________________________________________________



	% Lecture des donn�es et calcul de l'ACP
	%_______________________________________________________________________


% Nettoyage de variables
%-----------------------
%clear X ACP Contrib QR VEPU VEPV inertie CP dist 

%Entree des donnees
%------------------

%donnees=input('matrice des donn�es : ','s');
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


% Inertie et inertie cumul�e des valeurs propres
%-----------------------------------------------
inertie(:,1) = (ACP(:,1)/sum(ACP(:,1)))*100;
inertie(:,2) = cumsum(inertie(:,1));


% Coordonn�es(i,j) des individus(i) dans les plans principaux(j) 
% --------------------------------------------------------------
VEPU=ACP(:,2:nb+1);
VEPU=VEPU';
CP=X*VEPU;

% Contribution(i,j) d'un individu(i) � la fabrication d'un axe(j)
%----------------------------------------------------------------
Contrib=(1/size(X,1))*(CP.^2)*100./(ones(size(X))*diag(ACP(:,1)));

% Qualite de representation(i,j) de l'individu(i) par l'axe(j)
%-------------------------------------------------------------
dist=(sum((X').^2))';
QR=(CP.^2)./(diag(dist)*(ones(size(X))));

% Coordonn�es(i,j) des variables(i) dans les plans principaux(j)
%---------------------------------------------------------------
VEPV=VEPU.*repmat(sqrt(ACP(:,1))',nb,1);



	% Sorties Graphiques
	%_______________________________________________________________________

% Histogramme de l'inertie et inertie cumul�e
%--------------------------------------------
figure
bar(inertie(:,1));
hold on
plot(inertie(:,2),'-or')
titre=['Inertie des ' int2str(nb) ' premieres valeurs propres'];
title(titre);
axis([0.5 nb+0.5 0 100])

% Nuage des individus sur le plan factoriel engendr� par les 2 premiers axes principaux
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

% Nuage des param�tres sur le plan factoriel engendr� par les 2 premiers axes principaux
%---------------------------------------------------------------------------------------
figure
% Trac� du cercle de rayon 1 et de centre 0
t=[-pi:0.01:pi];
x=cos(t);
y=sin(t);
plot(x,y,'r');
hold on
compass(VEPV(:,1),VEPV(:,2));
for i=1:length(VEPV(:,1))
text(VEPV(i,1)-0.02,VEPV(i,2)+0.08,num2str(i));
end
title('Nuage des param�tres sur leplan des 2 premiers axes principaux');
axis('equal')
axe=axis;
plot([axe(1) axe(2)],[0 0],'k',[0 0],[axe(3) axe(4)],'k')