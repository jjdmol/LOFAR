function D = creadata_uni (N)

if nargin < 1
	N = 1000;
end

fprintf(1,'Creation d-une base de donnees avec une distribution uniforme\navec %d donnees ...\n', N);

D = rand(N,2);

return
