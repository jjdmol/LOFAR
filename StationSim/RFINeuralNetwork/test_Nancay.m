% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Fichier script test_labclass_nino
%
%  Afficle les poids avec la cardinalite (Hits) par classe
%
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%% Labeliser chaque cellule avec la classe et sa cardinalite
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

[Bmus, Qerrors] = som_bmus(sMap, sData);
Hits = som_hits(sMap, sData);
NbClasses = max(MyData3);

% Define et remplie une matrice de cardinalite par classe et par cellule
MatClass = zeros(prod(sMap.msize),NbClasses);
for iDon = 1: size(MyData3,1)
   iCell = Bmus(iDon);
   MatClass(iCell,MyData3(iDon)) = MatClass(iCell,MyData3(iDon)) + 1;
end





% Effacer tout label
sMap = som_label(sMap, 'clear');

% Boucle par cellule de la carte
for iCell = 1: prod(sMap.msize)
	% Boucle par classe et recherche de donnees de la classe
	% dans la cellule 'iCell' de la boucle
   cellLabels = cell(1);
 %     [ligne,colonne]=find(MatClass(iCell,:)== max(MatClass(iCell,:)));
			cellLabels{1} = [ num2str(Hits(iCell)) ' (' num2str(MyData3(iCell)) ')' ];
			sMap = som_label(sMap,iCell, cellLabels{1})
        end






figure
h=som_show(sMap,1);
hlbl=som_addlabels(sMap);
