% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Generation des donnees:
% -distribution uniforme
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

n_data = 400 ;
D = creadata_uni(n_data);

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Definition de la Structure des Donnees
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

sData = som_data_struct(D,'uniform');

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Initialisation de la carte topologique ou 'map'
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

msize = [4 4];
sMap  = som_init(sData,msize);
disp('initialisation');
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Entrainement de la carte : procedure 'automatique'
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
disp('c''est maintenant');
sMap = som_train(sMap,sData,[],[],[],'',3);

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Affichage 2D des données de la carte
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
figure
plot(D(:,1),D(:,2),'b+'); 
title('structure de la grille and data vectors (+)');
hold on
som_showgrid(sMap,sMap.codebook), hold off, axis on

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Visualisation des poids
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

figure
h = som_show(sMap);



% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Entrainement de la carte : procedure 'manuelle' a deux
% etapes d¹apprentissage	% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%sMap  = som_init(sData,msize);
disp('c''est maintenant');
figure
epochs     = 40;
radius     = [5 1];
sMap       = som_train(sMap,sData,epochs, radius, [],'',3);

figure
plot(D(:,1),D(:,2),'b+')
hold on
som_showgrid(sMap,sMap.codebook), hold off, axis on

figure
h = som_show(sMap);
disp('c''est maintenant voila');
figure
epochs     = 20;
radius     = [1 0.25];
sMap       = som_train(sMap,sData,epochs, radius, [],'',3);

figure
plot(D(:,1),D(:,2),'b+')
hold on
som_showgrid(sMap,sMap.codebook), hold off, axis on

figure
h = som_show(sMap);

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Modification de la fonction de voisinage.
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%sMap = som_trainops(sMap, 'gaussian','batch');
%sMap = som_trainops(sMap, 'gaussian','seq');
%sMap = som_trainops(sMap, 'bubble');

	
