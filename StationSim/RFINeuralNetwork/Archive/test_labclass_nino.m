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
NbClasses = max(nino_cls);

% Define et remplie une matrice de cardinalite par classe et par cellule
MatClass = zeros(prod(sMap.msize),NbClasses);
for iDon = 1: size(nino_cls,1)
   iCell = Bmus(iDon);
   MatClass(iCell,nino_cls(iDon)) = MatClass(iCell,nino_cls(iDon)) + 1;
end

% Effacer tout label
sMap = som_label(sMap, 'clear');

% Boucle par cellule de la carte
for iCell = 1: prod(sMap.msize)
	% Boucle par classe et recherche de donnees de la classe
	% dans la cellule 'iCell' de la boucle
	cellLabels = cell(1);
	for iClass = 1: NbClasses
		if MatClass(iCell,iClass) ~= 0
			cellLabels{1} = [ num2str(iClass) ' (' num2str(MatClass(iCell,iClass)) ')' ];
			sMap = som_label(sMap,iCell, cellLabels{1});
		end
	end
end


figure
h=som_show(sMap,1);
hlbl=som_addlabels(sMap);
