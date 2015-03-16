f_d, dphi = var('f_d, dphi')

eq1 = f_d  == 1 - (sin(dphi/2)/(dphi/2))
eq2 = 1e-3 == 1 - (sin(dphi/2)/(dphi/2))

find_root(eq2, 1e-20, 1.0)
