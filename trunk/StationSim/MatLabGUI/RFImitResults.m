function RFImitResults(SOI, SubbandSelector,DFTSystemResponse,NumberOfSubBands,SubbandFilterLength, ...
                SelectedSubbandSignals, NumberOfAntennas, NewDataLength,FlaggingCube, quant_signal,quant_inputfft,...
                quant_outputfft);    
            
            
% Separate the signal of interest into subbands and select a number of them, in order to be able to do SNR calculation
SOISubbandSignals = SubbandSeparator(SOI, SubbandSelector, DFTSystemResponse, NumberOfSubBands, ...
                                     SubbandFilterLength, 1, quant_signal, quant_inputfft, quant_outputfft);

% calculate the power spectrum of antennas
for a = 1 : NumberOfAntennas
    for s = 1 : length(SubbandSelector)
        PowerSpectrumSelectedSubbands(a, :, s) = SelectedSubbandSignals(a, :, s) .* conj(SelectedSubbandSignals(a, :, s));
        PowerSpectrumSOISubbands(1, :, s) = SOISubbandSignals(1, :, s) .* conj(SOISubbandSignals(1, :, s));        
    end
end

for t = 1 : NewDataLength(2)
    for s = 1 : NumberOfSubBands
        PowSpecAntenna1(t, s) = PowerSpectrumSelectedSubbands(1, t, s);
        PowSpecInAntenna1(t,s)=PowerSpectrumSOISubbands(1,t,s);
    end
end
% 
% figure
% subplot(2,1,1),plot(PowSpecInAntenna1);
% subplot(2,1,2),plot(PowSpecAntenna1);

figure
subplot(2,1,1),surf(PowSpecInAntenna1);
 colormap('default');
 shading interp;
 xlabel('frequency')
 ylabel('time')
 axis([1 NumberOfSubBands/2  1 NewDataLength(2) 0 0.0001])
 zlabel('Dirty power spectrum at antenna 1')
 view(20,45)
subplot(2,1,2),surf(PowSpecAntenna1);
 colormap('default');
 shading interp;
 xlabel('frequency')
 ylabel('time')
 axis([1 NumberOfSubBands/2  1 NewDataLength(2) 0 0.0001])
 zlabel('Clean power spectrum at antenna 1')
 view(20,45)
