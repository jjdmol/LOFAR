mat1=Signal_modulation;
freqsample=fs;
frequency_res=str2num(get(Freq_Res,'string'));
set(Total_time,'String',1/freqsample*length(mat));
Puissance=round(log2(freqsample/frequency_res));
real_res=freqsample/2^Puissance;
set(real_fr,'String',real_res);
Integration=str2num(get(Int_Time,'String'));
tic
DSP_1inst=[];
calcul=0;
step=Integration*2^Puissance;
fin=round(length(mat1(:))/(step))-1 
if fin<1
     message=['Integration Time must be equal or less than ' num2str(round(length(mat(:))/(2^Puissance))-2) ''];
    title='Attention';
    msgbox(message,title,'warn')
else
step0=1;
step1=step;
for j=1:round(fin)
        A=mat1(step0:step1);
        FFT_D_moy=[];
        D=specgram(A,2^Puissance,freqsample,hann(2^Puissance),0);   
        FFT_D_moy=module_carre(D);
        if Integration~=1
        FFT_D_moy=mean(FFT_D_moy');
        DSP_1inst=[DSP_1inst;FFT_D_moy];
        else
        DSP_1inst=[DSP_1inst;FFT_D_moy'];
        end
        step0=step1;
        step1=step1+step;
        calcul=j;
end
set(0,'CurrentFigure',HMainFig_gene)
set(real_time,'string',1/real_res*Integration);
subplot(PolyphasePlot)
imagesc(20*log(DSP_1inst'))
colormap(1-gray)
indice=1;
%title('PSD') 

indice_thres=2;
clear A;
clear FFT_D_moy;
clear D;
clear step0;
clear step1;
clear x;
clear fin;
clear xtmp;
Thresholding
toc
end