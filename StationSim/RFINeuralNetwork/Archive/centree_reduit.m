function X = centree_reduit(Z)
% Fonction qui calcule la matrice centree reduite à partir de la matrice de données

% Calcul des moyennes et des écarts types empiriques de chaque variable
% et création d'une matrice avec les vecteurs moyennes et écarts types
% en lignes
UN = ones(size(Z));
Moy = mean(Z);
Moy = diag(Moy);
Moy = UN * Moy;

taille=size(Z,1);
Ecart_type = diag(sqrt((taille-1)/taille)*std(Z));
Ecart_type = UN * Ecart_type;

% Calcul de la matrice X des données centrées réduites
X = (Z - Moy)./Ecart_type ;

