file_name='data/evd_weight.txt'
fid=fopen(file_name,'r');
out=fscanf(fid,'%f');
out2=reshape(out,2,floor(length(out))/2).';
out=complex(out2(:,1),out2(:,2));
fclose(fid);