function X = centree_reduit(Z)
% Fonction qui calcule la matrice centree � partir de la matrice de donn�es

% Calcul des moyennes et des �carts types empiriques de chaque variable
% et cr�ation d'une matrice avec les vecteurs moyennes et �carts types
% en lignes
UN = ones(size(Z));
Moy = mean(Z);
Moy = diag(Moy);
Moy = UN * Moy;


% Calcul de la matrice X des donn�es centr�es
X = (Z - Moy);

