function X = centree_reduit(Z)
% Fonction qui calcule la matrice centree à partir de la matrice de données

% Calcul des moyennes et des écarts types empiriques de chaque variable
% et création d'une matrice avec les vecteurs moyennes et écarts types
% en lignes
UN = ones(size(Z));
Moy = mean(Z);
Moy = diag(Moy);
Moy = UN * Moy;


% Calcul de la matrice X des données centrées
X = (Z - Moy);

