function [D, labs, cnames] = creadataH (N,Center,Theta)

if nargin < 1
	N = 1000;
	Center = [0 0];
	Theta = 30;
elseif nargin < 2
	Center = [0 0];
	Theta = 30;
elseif nargin < 3
	Theta = 30;
end

if length(Center) == 1
	Center = [Center Center];
end

fprintf(1,'Creation d-une base de donnees en forme de H avec %d donnees ...\n', N);

MaxX = 1;
MaxY = 1;
Vect1 = [MaxX 0; 0 MaxY];
D1 = rand(floor(N/3),2) * Vect1; D1 = [ D1(:,1) / 3, D1(:,2) ];
D2 = rand(ceil(N/3),2) * Vect1; D2 = [ (MaxX / 3) + (D2(:,1) / 3), (MaxY / 3) + (D2(:,2) / 3) ];
D3 = rand(floor(N/3),2) * Vect1; D3 = [ (2 * MaxX / 3) + (D3(:,1) / 3), D3(:,2) ];

D = [D1; D2; D3];

%%%
%%% centrer les donnees
%%%
D(:,1) = D(:,1) - mean(D(:,1));
D(:,2) = D(:,2) - mean(D(:,2));

%%%
%%% rotation des donnees qutour de leur centre
%%%
D = D * Rota2D(Theta);

%%%
%%% deplacement du centre de gravite
%%%
D(:,1) = D(:,1) + Center(1);
D(:,2) = D(:,2) + Center(2);


i1 = 1:length(D1);
i2 = length(D1) + (1:length(D2));
i3 = length(D1) + length(D2) + (1:length(D3));
labs = cell(size(D,1),1);
cnames = {'X', 'Y'};


for i = 1:length(i1)
  labs{i1(i)} = 'un';
end

for i = 1:length(i2)
  labs{i2(i)} = 'deux';
end

for i = 1:length(i3)
  labs{i3(i)} = 'trois';
end
