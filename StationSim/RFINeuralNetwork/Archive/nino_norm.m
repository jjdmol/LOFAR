nNino=2
TailleFenetre=3
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


load -ascii el_nino_PC.mat;

x=el_nino_pc;
Don = x(:,nNino);

ncol = 1;


% Cherche l'ndice du premier exemple non invalide (different de -9999)
iAllOk = find(Don ~= -9999);

if isempty(iAllOk)
	iOk = 1;
else
	iOk = iAllOk(1);
end
Don2 = x(iOk:length(Don),2);
Don = Don(iOk:length(Don));
moy=mean(Don);
ecarttype=std(Don);

whos


Don=(Don-moy)/ecarttype
class=[];

for i=1:length(Don)
   if (abs(Don(i))>1) 
      class(i)=1;
   else class(i)=0;
   end
end
class=class';

% declare structure pour les noms des variables
VarNames = cell(1, TailleFenetre);


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
labellisee = class(iDon,1);
for ii = 2:TailleFenetre
	VarNames{ii} = { [ Nom '_T-' num2str(TailleFenetre - ii + 1) ] };
   DonNino = [DonNino, Don(iDon+ii-1,1)];
   labellisee = [labellisee, class(iDon+ii-1,1)];
end
