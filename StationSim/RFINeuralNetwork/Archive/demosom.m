function [sD, sMap] = demosom (N, rad, alf)
% function [sD, sMap] = demosom (N, rad, alf)
%
% N ..... Nombre d'iterations [Phase1, Phase2]
% rad ... Radius [Phase1_ini, Phase1_fin ; Phase2_ini, Phase2_fin]
% alf ... Radius [Phase1, Phase2]
switch nargin
  case 0,
    N = [20 20];
    rad = [10 1; 3 1];
    alf = [0.05 0.05];
  case 1,
    rad = [10 1; 3 1];
    alf = [0.05 0.05];
  case 2,
    alf = [0.05 0.05];
end

if 1 == length(N)
  N = [N N];
end

if 1 == length(alf)
  alf = [alf alf];
end

switch length(rad(:))
  case 1,
    rad = [rad rad; rad rad];
  case 2,
    rad = [rad(1) rad(1); rad(2) rad(2)];
  case 3,
    rad = [rad(1) rad(1); rad(2) rad(3)];
end

[D, labs, cnames] = creadataH(200);

sD = som_data_struct(D,'aleaH', labs, cnames);
%% sD = som_label(sD, [i1'; i2'; i3'], Labels);

localax    = [ floor(min(D(:,1))), ceil(max(D(:,1))), floor(min(D(:,2))), ...
		       ceil(max(D(:,2))) ];
msize      = [10 20];
init_type  = 'random';     %% 'random' ou 'linear'
lattice    = 'hexa';       %% 'hexa' ou 'rect'
shape      = 'rect';       %% 'rect', 'cyl', ou 'toroid'
sMap       = som_init(sD, msize, init_type, lattice, shape);

fig1 = figure;
subplot(1,3,1)
plot(D(:,1),D(:,2),'+r'), hold on, som_showgrid(sMap,sMap.codebook), hold off, axis on
title('Etat initial');
axis(localax), axis square
neigh      = 'gaussian';
train_type = 'seq';
mask       = ones(size(D,2),1);
sMap       = som_trainops(sMap, neigh, train_type, mask);

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Premier apprentissage
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
epochs     = N(1);
radius     = rad([1, 2]);
alpha      = alf(1);
alpha_type = 'linear';
tracking   = 3;
fig2=figure
sMap       = som_train(sMap,sD,epochs, radius, alpha, alpha_type, tracking);

figure(fig1)
subplot(1,3,2)
plot(D(:,1),D(:,2),'+r'), hold on, som_showgrid(sMap,sMap.codebook), hold off, axis on
title(sprintf('Apres %d iterations', epochs));
axis(localax), axis square

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Apprentissage fin
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
epochs     = N(2);
radius     = rad([3, 4]);
alpha      = alf(2);
alpha_type = 'linear';
tracking   = 3;
figure(fig2)
sMap       = som_train(sMap,sD,epochs, radius, alpha, alpha_type, tracking);

figure(fig1)
subplot(1,3,3)
plot(D(:,1),D(:,2),'+r'), hold on, som_showgrid(sMap,sMap.codebook), hold off, axis on
title(sprintf('Apres %d+%d iterations', N));
axis(localax), axis square

print('-dpsc',sprintf('demosom_%d_%d', N(1), N(2)));
