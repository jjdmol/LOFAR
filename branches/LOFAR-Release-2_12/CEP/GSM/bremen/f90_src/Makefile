all:
	f2py --opt=-O3 -c -m gsm_matcher gsm_matcher.f90 

openmp:
	f2py --f90flags="-fopenmp -lgomp" -lgomp --opt=-O3 -c -m gsm_matcher gsm_matcher.f90 
