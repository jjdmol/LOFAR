function M = Rota2D( theta )

	if nargin ~= 1
		error(sprintf(' *** bad arg.number ***\n\n\tM = Rota2D( Theta )\n\n'));
	end
	A = pi * theta / 180;
	cos_t = cos(A);
	sin_t = sin(A);
	M = [ cos_t sin_t; -sin_t cos_t ];

	return
