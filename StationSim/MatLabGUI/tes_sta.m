dirpath = 'data';
NumberOfAntennas=92;
load([dirpath '/antenna_config.mat']);
phi=0.2;
theta=0.4;
test_vector=exp(-1*sqrt(-1)*2*pi* ...
                      (px*cos(phi)*sin(theta)+py*sin(phi)*sin(theta)));
test_mat=repmat(test_vector,300,1);
fid=fopen('data\test_vectorEssai.txt','w+');
fprintf(fid,[num2str(size(test_mat,2)) 'x' num2str(size(test_mat,1))   ' \n ']);
fprintf(fid,'[');
j=0
fin=num2str(size(test_mat,1))
fin_plu=num2str(size(test_mat,2))
for j=1:fin_plu
    for k=1:fin
        fprintf(fid,'(%e, %e)\t',real(test_mat(j,k)),imag(test_mat(j,k)));
    end
    fprintf(fid,'\n');
end
fprintf(fid,']');
fclose(fid);
