function record_blietz(Values,dirpath_subband,filename)

fid=fopen([dirpath_subband filename],'w+');
fprintf(fid,'%g x %g\n',size(Values,2),size(Values,1));
fprintf(fid,'[');
for i=1:size(Values,2)
    for j=1:size(Values,1)
    fprintf(fid,'(%g,%g)\t', real(Values(j,i)),imag(Values(j,i)));
    end
 fprintf(fid,'\n');
end
fprintf(fid,']');
fclose(fid);
end
