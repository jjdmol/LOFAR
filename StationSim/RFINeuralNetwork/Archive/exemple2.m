% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Exemples de classification
%
%  Entrainement avec parametres par defaut
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

n_data     = 199;
m_size     = [6 4];
neigh1     = 'gaussian';
train_type = 'batch';
alpha1     = 0.05;
Radius		=[5 0.25];

[D, labs, cnames] = creadataH(n_data);

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Plot donnees de départ
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
figure
plot_data_tp2(D,labs);

sData = som_data_struct(D,'aleaH', labs, cnames);

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Initialisation de la carte
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
sMap = som_init(sData, m_size);

figure
plot_data_tp2(D,labs), hold on, som_showgrid(sMap,sMap.codebook), hold off, axis on
title('Etat initial');

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Apprentissage (automatique)
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

figure
sMap       = som_train(sMap,sData,[], Radius, [], '',3);

figure
plot_data_tp2(D,labs), hold on, som_showgrid(sMap,sMap.codebook), hold off, axis on
title('Etat Learn1');
axis image



% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Apprentissage manuel en deux temps
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
sMap = som_init(sData, m_size);

figure
plot_data_tp2(D,labs), hold on, som_showgrid(sMap,sMap.codebook), hold off, axis on
title('Etat initial');

sMap  = som_trainops(sMap, neigh1);
figure
Epochs = 60;
Radius = [5 2];
sMap       = som_train(sMap, sData, Epochs, Radius, [], '',3);


Epochs = 60;
Radius = [2 0.3];
sMap       = som_train(sMap, sData, Epochs, Radius, [], '',3);

figure
plot_data_tp2(D,labs), hold on, som_showgrid(sMap,sMap.codebook), hold off, axis on
title('Etat Learn2');


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%% Labeliser avec les Hits (nombre de données par cell)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Effacer tout label
sMap = som_label(sMap, 'clear');

% Calculer les ŒHits¹
% Hits ... pour savoir combien de données sont attachés a
% chaque cellule du map.
Hits = som_hits(sMap, sData);

% affiche les poids par plans. La taille des cellules est
% proportionnelle a sa cardinalité (Hits)
figure
 h=som_show(sMap,[], '', 'vert', Hits);
 
%Afficher seulement le plan 1 avec les Hits;
% h=som_show(sMap,1, '', 'vert', Hits);

%Afficher en taille fixe tous les plans
%h=som_show(sMap);


% ajoute la cardinalité comme Label et affiches les Labels.
H1 = Hits(:);
I1 = 1:length(H1);
sMap = som_label(sMap,I1', num2cell(H1));
title('Labeliser avec le nombre de données par cellules')
hlbl=som_addlabels(sMap);



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%% Labeliser avec l'indice des cellules
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Effacer tout label
sMap = som_label(sMap, 'clear');

% Calculer les ŒHits¹
%Hits = som_hits(sMap, sData)

% Calculer la Taille du Map
Taille1 = sMap.msize(1);
Taille2 = sMap.msize(2);
Indices = 1:(Taille1 * Taille2);

% affiche les poids du plan 1 uniquement
figure

h=som_show(sMap,1);

% ajoute l'indice comme Label et affiches les Labels.
I1 = 1:length(Indices);
sMap = som_label(sMap,I1', num2cell(Indices'));
hlbl=som_addlabels(sMap);
title('Labeliser avec indice des cellules')



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%% Ajout des labels des donnees (on peut puisqu'il s'agit
%%% de donnees labelisés)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Effacer tout label
sMap = som_label(sMap, 'clear');

figure
title('Labeliser avec les labels des données');
h=som_show(sMap);
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
title('Labels manuels');
h=som_show(sMap);

sMap = som_label(sMap,[1; 4; 8],   ['UN';'DE';'TR']);
sMap = som_label(sMap,[1 1; 4 1; 2 2],   ['UN';'DE';'TR']);
hlbl=som_addlabels(sMap);



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
[Bmus, Qerrors] = som_bmus(sMap, sData);

Hits = som_hits(sMap, sData)
nMax = max(Hits(:))
iCell = find(Hits == nMax)
[d n]=size(iCell);
iDon=[];
for i=1:d
iDon = [iDon find(Bmus == iCell(i))];
end
iDon

%effacer tout label
sMap = som_label(sMap,'clear');

LabelChoix=sData.labels(iDon);
for ii=1:length(LabelChoix)
	sMap=som_label(sMap,iCell,LabelChoix{ii});
end
figure
h=som_show(sMap,0);
hlbl=som_addlabels(sMap);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%% Labelliser chaque cellule avec la classe et la cardinalité
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

[Bmus, Qerrors] = som_bmus(sMap, sData);
Hits = som_hits(sMap, sData);
NbClasses=3;

%Définie et remplie une matrice de cardinalité par classe et par cellule
MatClass=zeros(prod(sMap.msize),NbClasses);
for iDon=1:size(D,1)
	iCell=Bmus(iDon);
	Label=labs(iDon);
	if strcmp(Label,'un')
		iClass=1;
	elseif strcmp(Label,'deux')
		iClass =2;
	else
		iClass=3;
	end
	MatClass(iCell,iClass)=MatClass(iCell,iClass)+1;
end


%effacer tout label
sMap = som_label(sMap,'clear');

%Boucle par cellule de la carte
for iCell=1:prod(sMap.msize)
	%Boucle par classe et recherche de données 
	%de la classe dans la cellule iCell de la boucle
	cellLabels=cell(1);
	for iClass=1:NbClasses
		if MatClass(iCell,iClass)~=0
			cellLabels{1}=[num2str(iClass) '('num2str(MatClass(iCell,iClass)) ')'];
			sMap=som_label(sMap,iCell,cellLabels{1});
		end
	end
end

figure
h=som_show(sMap,1);
hlbl=som_addlabels(sMap);

	
