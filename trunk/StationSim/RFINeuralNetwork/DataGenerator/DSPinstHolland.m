tic
DSP_1inst=[];
calcul=0;
step=64*16384;
fin=length(x(:))/(step); 
step0=1;
step1=step;
for j=1:fin
        A=x(step0:step1);
        FFT_D_moy=[];
        D=specgram(A,16384,20*10^6,hann(16384),0);   
        FFT_D_moy=module_carre(D);
        DSP_1inst=[DSP_1inst;FFT_D_moy'];
        step0=step1;
        step1=step1+step;
        calcul=j
end
figure(1)
imagesc(10*log(DSP_1inst'))
colormap(1-gray)
title('PSD') 
figure(2)
plot(10*log10(mean(DSP_1inst)))
title('Mean') 

clear A;
clear FFT_D_moy;
clear D;
clear step0;
clear step1;
%clear x;
clear fin;
clear xtmp;
toc