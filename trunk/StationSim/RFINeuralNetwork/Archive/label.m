%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%% Labeliser avec les Hits (nombre de données par cell)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Effacer tout label
sMap = som_label(sMap, 'clear')

% Calculer les ŒHits¹
% Hits ... pour savoir combien de données sont attachés a
% chaque cellule du map.
Hits = som_hits(sMap, sData)

% affiche les poids par plans
figure
h=som_show(sMap)
% h=som_show(sMap,[], '', 'vert', Hits)

% ajoute la cardinalité comme Label et affiches les Labels.
H1 = Hits(:);
I1 = 1:length(H1)
sMap = som_label(sMap,I1', num2cell(H1));
hlbl=som_addlabels(sMap);


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%% Labeliser avec l'indice des cellules
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Effacer tout label
sMap = som_label(sMap, 'clear');

% Calculer les ŒHits¹
Hits = som_hits(sMap, sData);

% Calculer la Taille du Map
Taille1 = sMap.msize(1);
Taille2 = sMap.msize(2);
Indices = 1:(Taille1 * Taille2);

% affiche les poids du plan 1 uniquement
figure
h=som_show(sMap,1)

% ajoute l'indice comme Label et affiches les Labels.
I1 = 1:length(Indices);
sMap = som_label(sMap,I1', num2cell(Indices'));
hlbl=som_addlabels(sMap);


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%% Ajout des labels des donnees (on peut puisqu'il s'agit
%%% de donnees labelisés)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Effacer tout label
sMap = som_label(sMap, 'clear')

figure
h=som_show(sMap)
% h=som_show(sMap,[], '', 'vert', som_hits(sMap,sData))

% Copie les Labels de sData dans sMap
sMap = som_autolabel(sMap, sData);

% Affiche les Labels
hlbl=som_addlabels(sMap);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%% Test de labelisation manuel
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
sMap = som_label(sMap, 'clear');
figure
h=som_show(sMap)

sMap = som_label(sMap,[1; 4; 8],   ['UN';'DE';'TR'])
sMap = som_label(sMap,[1 1; 4 1; 2 2],   ['UN';'DE';'TR'])
hlbl=som_addlabels(sMap)


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%% Identifier les données appartennant a une cellule
%%% particuliere, en l'occurrence la ou les cellules qui 
%%% contiennent le plus de donnees
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%%% Best Matching Units ... pour savoir a quelle cellule est attaché
%%% chaque donnée.
%%%
%%%     [Bmus, Qerrors] = som_bmus(sMap, sData, [which], [mask])
%%%
[Bmus, Qerrors] = som_bmus(sMap, sData)

Hits = som_hits(sMap, sData)
nMax = max(Hits(:))
iCell = find(Hits == nMax)

iDon = find(Bmus == iCell)
