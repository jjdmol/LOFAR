% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Chargement des donnees
% iris_don ... donnees a 4 variables representant
%              4 mesures prises sur des fleus iris.
% iris_cls ... classe d'iris (3 classes).
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

load -ascii iris_don.mat
load -ascii iris_cls.mat



%utilisation des 100 premières données pour faire la
%carte et la labelliser

iris_don=iris_don(1:100,:);
iris_cls=iris_cls(1:100,:);
n_data = size(iris_cls,1);

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Definition de Labels
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

iris_labs = cell(n_data,1);

for i_cell = 1:n_data
	iris_labs{i_cell} = { sprintf('%d', iris_cls(i_cell)) };
end

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Definition des noms des variables d'entree
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

iris_varnames = { {'var1'}, {'var2'}, {'var3'}, {'var4'} };

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Definition de la Structure des Donnees
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

sData = som_data_struct(iris_don,'iris', iris_labs, iris_varnames);

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Normalisation des Donnees (pas obligatoire)
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%sData = som_normalize_data(sData,'som_var_norm');

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Initialisation de la carte topologique ou 'map'
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

msize     = [7 7];
init_type = 'random';   %% 'random' ou 'linear'
lattice   = 'hexa';     %% 'hexa' ou 'rect'
shape     = 'rect';     %% 'rect', 'cyl', ou 'toroid'
sMap  = som_init(sData, msize, init_type, lattice, shape);

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Options de la carte
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

Neigh     = 'gaussian'; %% 'gaussian', 'cutgauss', 'bubble' ou 'ep'
TrainType = 'batch';    %% 'seq' ou 'batch'
Mask      = ones(size(iris_don,2), 1);  %% active tous les donnees
sMap  = som_trainops(sMap, Neigh, TrainType, Mask);

figure

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Entrainement de la carte
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

epochs     = 50;
radius     = [5 1];
alpha      = 0.1;
alpha_type = 'linear';
tracking   = 3;
sMap       = som_train(sMap,sData,epochs, radius, alpha, alpha_type, tracking);

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Entrainement fin de la carte
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

epochs     = 100;
radius     = [1 0.3];
alpha      = 0.05;
alpha_type = 'linear';
tracking   = 3;
sMap       = som_train(sMap,sData,epochs, radius, alpha, alpha_type, tracking);


% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Projection de la carte et des donnees
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

pMap = som_projection(sMap,2,'pca');
figure
som_showgrid(sMap,pMap), hold off, axis on
title('Projection par ACP');

pMap = som_sammon(sMap,2);
figure
som_showgrid(sMap,pMap), hold off, axis on
title('Projection par Sammon');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%% Labelliser chaque cellule avec la classe et la cardinalité
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

[Bmus, Qerrors] = som_bmus(sMap, sData);
Hits = som_hits(sMap, sData);
NbClasses=max(iris_cls);

%Définie et remplie une matrice de cardinalité par classe et par cellule
MatClass=zeros(prod(sMap.msize),NbClasses);
for iDon=1:size(iris_cls,1)
	iCell=Bmus(iDon);
	MatClass(iCell,iris_cls(iDon))=MatClass(iCell,iris_cls(iDon))+1;
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
