function [DonNino, NinoSOMLabels, NinoLabels, VarNames] = load_nino_class(nNino,nNino_Label,TailleFenetre, alpha, seuil)
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% fonction load_nino_class
%
%    [DonNino, NinoSOMLabels, NinoLabels, VarNames] = load_nino_class(nNino,nNino_Label,TailleFenetre, alpha, seuil)
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
% Retourne les Donnees, la liste de labels en format SOM
% et en format numerique et les noms de variables.
%
% Dans les Donnees, chaque ligne represente un pattern
% ou entree au reseau.
%
% Les labels sont choisis selon :
%  - Si variable nNino_lab en temps T, normalisee, est
%    superieur au Seuil, alors Label == 1
%  - Sinon, Label = 0
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if (nargin < 5) | (nargout < 4)
	error(sprintf('\n *** load_nino appel invalide:\nappel:\n\tNino = load_nino(nNino,TailleFenetre);\n'));
end
load -ascii el_nino_PC.mat;
x=el_nino_PC;
ncol = 1;

Don = x(:,nNino);
DonLabel = x(:,nNino_Label);

% Cherche l'indice du premier exemple non invalide (different de -9999)
iAllOk = find(Don ~= -9999);

if isempty(iAllOk)
	iAllOk = find(DonLabel ~= -9999);
	if isempty(iAllOk)
		iOk = 1;
	else
		iOk = iAllOk(1);
	end
else
	iOk = iAllOk(1);
end

n_don = size(Don,1);

Don = Don(iOk:n_don);
DonLabel = DonLabel(iOk:n_don);

DonLabelN = (DonLabel - mean(DonLabel(:))) / (alpha * std(DonLabel(:)));
iDonLabel = find(DonLabelN >= seuil);

% re-initialisation du nombre de donnees
n_don = size(Don,1);

NinoLabels = zeros(n_don,1);
if ~isempty(iDonLabel)
	NinoLabels(iDonLabel) = 1;
end

% declare structure pour les noms des variables
VarNames = cell(1, TailleFenetre);

if nNino == 1
	Nom = 'DATE';
elseif nNino < 6
	Nom = 'TEMP';
elseif nNino < 9
	Nom = 'TX';
else
	Nom = 'TY';
end

VarNames{1} = { [ Nom ' T-' num2str(TailleFenetre) ] };

iDon = 1:(n_don - TailleFenetre);
DonNino = Don(iDon,1);
for ii = 2:TailleFenetre
	VarNames{ii} = { [ Nom ' T-' num2str(TailleFenetre - ii + 1) ] };
	DonNino = [DonNino, Don(iDon+ii-1,1)];
end

NinoLabels = NinoLabels(iDon+TailleFenetre);
whos
% re-initialisation du nombre de donnees
n_don = size(DonNino,1);

% declare la structure pour les labels au format SOM (cells)
NinoSOMLabels = cell(n_don,1);
for ii = 1:n_don
	if NinoLabels(ii) == 1
		NinoSOMLabels{ii} = { 'OUI' };
	else
		NinoSOMLabels{ii} = { 'non' };
	end
end




figure
plot(DonNino), hold on
plot([1 n_don], [seuil seuil],'-r','Linewidth',2)
stem(NinoLabels,'k')
title([ 'profil des donnees. Fenetre=' num2str(TailleFenetre) ...
   ', Variable=' Nom num2str(nNino-1) ', Var. x Label=' Nom num2str(nNino_Label)]);
