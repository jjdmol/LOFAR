fid = fopen('result_ffttrans.txt','w');
fprintf(fid,'\r\nfft = 128 \r\n',i);
for i=1:size(data_inter,2)
   %fprintf(fid,'\r\nfft = 128 %i \r\n',i);
    for j=1:size(data_inter,1)
       fprintf(fid, '( %e + %ei ) ',real(data_inter(j,i)), imag(data_inter(j,i)));
       %fprintf(fid, '( %e ) ',freq_shift(i,j));
   end
end
fclose(fid)