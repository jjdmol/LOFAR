function [DonNino, VarNames] = load_nino(nNino,TailleFenetre)
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% fonction load_nino
%
%    [DonNino, VarNames] = load_nino(nNino,TailleFenetre)
%
% Lecture et creation d'une table de donnees selon
% une variable NINOx et selon une taille de fenetre
% temporelle.
% 
% Chaque mesure NINO correspond a un mois la fenetre
% correspond donc au nombre de mois a prendre en
% consideration.
%
% La matrice resultante prends en compte les donnees
% depuis la premiere valeur differente de -9999 a
% l'avant derniere valeur.
%
% Cette derniere n'est pas prise car elle correspond
% dans le temps a l'evenement en temps T a predire par
% les donnees passes T-1, T-2, ..., T-P, ou P est la
% taille de la fenetre.
%
% Retourne les Donnees et les noms de variables.
% Dans les Donnees, chaque ligne represente un pattern
% ou entree au reseau.
%
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

if (nargin < 2) | (nargout < 2)
	error(sprintf('\n *** load_nino appel invalide:\nappel:\n\tNino = load_nino(nNino,TailleFenetre);\n'));
end

load -ascii el_nino_PC.mat;

x=el_nino_PC;

ncol = 1;

Don = x(:,nNino);
moy=mean(Don)
ecarttype=std(Don)
Don=(Don-moy)/ecarttype
% Cherche l'ndice du premier exemple non invalide (different de -9999)
iAllOk = find(Don ~= -9999);

if isempty(iAllOk)
	iOk = 1;
else
	iOk = iAllOk(1);
end

Don = Don(iOk:length(Don));

% declare structure pour les noms des variables
VarNames = cell(1, TailleFenetre);
whos
if nNino == 1
	Nom = 'DATE';
elseif nNino < 5
	Nom = 'TEMP';
elseif nNino < 9
	Nom = 'TX';
else
	Nom = 'TY';
end

VarNames{1} = { [ Nom '_T-' num2str(TailleFenetre) ] };

iDon = 1:(size(Don,1) - TailleFenetre);
DonNino = Don(iDon,1);
whos
for ii = 2:TailleFenetre
	VarNames{ii} = { [ Nom '_T-' num2str(TailleFenetre - ii + 1) ] };
	DonNino = [DonNino, Don(iDon+ii-1,1)];
end
