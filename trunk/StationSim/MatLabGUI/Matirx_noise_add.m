
factor_no=str2num(get(NoisePower,'String'))
factor_noise1=factor_no-Noise_reference;
factor_noise=10^(factor_noise1/20);
matrix2=10^(calibration/20)*matrix;
final_matrix=factor_noise*matrix_noise+matrix2;
set(0,'CurrentFigure',HMainFig_gene)
subplot(PolyphasePlot)
imagesc(20*log10(abs(final_matrix)))