tic
DSP_Gmoy=[];
DSP_Dmoy=[];
calcul=0;
fin=length(x(:,1))/(128*16384); 
step0=1;
step1=16384*128;
for j=1:fin-1
        A=x(step0:step1);
        FFT_D_moy255=[];
        FFT_G_moy255=[];
        D=specgram(A,16384,20*10^6,hann(16384),0);   
        FFT_D_moy255=module_carre(D);
        DSP_Dinst200=[DSP_Dinst200;FFT_D_moy255'];
        step0=step1;
        step1=j+1*step1;
        calcul=j
end
figure(1)
imagesc(10*log(DSP_Ginst200'))
colormap(1-gray)
title('PSD') 
toc