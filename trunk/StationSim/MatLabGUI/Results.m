function Results(SOI, WithOut, SubbandSelector, DFTSystemResponse, NumberOfSubbands, SubbandFilterLength, ...
                 SelectedSubbandSignals, CleanSelectedSubbandSignals, NumberOfPlot, NumberOfAntennas, ...
                 NewDataLength, FlaggingCube)

dirpath='data';
load([dirpath '\channel_options.mat']);

% Separate the signal of interest into subbands and select a number of them, in order to be able to do SNR calculation
SOISubbandSignals = SubbandSeparator(SOI, SubbandSelector, DFTSystemResponse, NumberOfSubbands, ...
                                     SubbandFilterLength, 1, ch_quant_signal, ch_quant_inputfft, ch_quant_outputfft);

% Separate the signal of interest into subbands and select a number of them, in order to be able to do SNR calculation
WithOutSubbandSignals = SubbandSeparator(WithOut, SubbandSelector, DFTSystemResponse, NumberOfSubbands, ...
                                     SubbandFilterLength, 1, ch_quant_signal, ch_quant_inputfft, ch_quant_outputfft);


% calculate the power spectrum of antennas
for a = 1 : NumberOfAntennas
    for s = 1 : length(SubbandSelector)
        PowerSpectrumSelectedSubbands(a, :, s) = SelectedSubbandSignals(a, :, s) .* conj(SelectedSubbandSignals(a, :, s));
   	    CleanPowerSpectrumSelectedSubbands(a, :, s) = CleanSelectedSubbandSignals(a, :, s) .* conj(CleanSelectedSubbandSignals(a, :, s));
        PowerSpectrumSOISubbands(1, :, s) = SOISubbandSignals(1, :, s) .* conj(SOISubbandSignals(1, :, s));        
        PowerSpectrumWithOutSubbands(1, :, s) = WithOutSubbandSignals(1, :, s) .* conj(WithOutSubbandSignals(1, :, s));
    end
end

%%%%%%%%%%%%% Calculate the performance indicators %%%%%%%%%%%%
% top = max(PowerSpectrumSelectedSubbands);
% top = max(top);
% top = max(top);
% 
% PowerSpectrumSelectedSubbands = PowerSpectrumSelectedSubbands / top;
% PowerSpectrumSOISubbands = PowerSpectrumSOISubbands / top;
% CleanPowerSpectrumSelectedSubbands = CleanPowerSpectrumSelectedSubbands / top;

SNIRbefore = 0;
SNIRafter = 0;
for t = 1 : NewDataLength(2)
    for s = 1 : NumberOfSubbands
        SNIRbefore(t, s) = 10 * log10(PowerSpectrumSOISubbands(1, t, s) ./ PowerSpectrumSelectedSubbands(1, t, s));
        SNIRafter(t, s) = 10 * log10(PowerSpectrumSOISubbands(1, t, s) ./ CleanPowerSpectrumSelectedSubbands(1, t, s));
        SNIRwithout(t, s) = 10 * log10(PowerSpectrumSOISubbands(1, t, s) ./ PowerSpectrumWithOutSubbands(1, t, s));
        Gproc(t, s) =  SNIRafter(t, s) ./ SNIRbefore(t, s);
        Rproc(t, s) = SNIRafter(t, s) ./ SNIRwithout(t, s);
    end
end

Gproc = abs(Gproc);

%%%%%%%%%%%%% Plot the results %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

NumberOfPlot = NumberOfPlot + 1; % 1
figure(NumberOfPlot)

for j = 1 : NumberOfSubbands
    a1(j) = SelectedSubbandSignals(1, 2, j);
    a2(j) = SelectedSubbandSignals(1, 3, j);
end

p1 = a1 .* conj(a1);
p2 = a2 .* conj(a2);

subplot(4, 1, 1), plot(p1);
axis([1 NumberOfSubbands/2 0 0.0001])
subplot(4, 1, 2), plot(p2);
axis([1 NumberOfSubbands/2 0 0.0001])

for j = 1 : NumberOfSubbands
    c1(j) = CleanSelectedSubbandSignals(1, 2, j);
    c2(j) = CleanSelectedSubbandSignals(1, 3, j);
end

pc1 = c1 .* conj(c1);
pc2 = c2 .* conj(c2);

subplot(4, 1, 3), plot(pc1);
axis([1 NumberOfSubbands/2 0 0.0001])
subplot(4, 1, 4), plot(pc2);
axis([1 NumberOfSubbands/2 0 0.0001])


NumberOfPlot = NumberOfPlot + 1; % 2
figure(NumberOfPlot)
for t = 1 : NewDataLength(2)
    for s = 1 : NumberOfSubbands
        PowSpecAntenna1(t, s) = PowerSpectrumSelectedSubbands(1, t, s);
        CleanPowSpecAntenna1(t, s) = CleanPowerSpectrumSelectedSubbands(1, t, s);
        FlaggingCube1(t, s) = FlaggingCube(1, t, s);
    end
end
subplot(3,1,1),surf(PowSpecAntenna1)
colormap('default');
shading interp;
xlabel('frequency')
ylabel('time')
axis([1 NumberOfSubbands/2  1 NewDataLength(2) 0 0.0001])
zlabel('Dirty power spectrum at antenna 1')
view(20,45)

subplot(3,1,2),surf(CleanPowSpecAntenna1)
shading interp;
xlabel('frequency')
ylabel('time')
axis([1 NumberOfSubbands/2  1 NewDataLength(2) 0 0.0001])
zlabel('Cleaned power spectrum at antenna 1')
view(20,45)

subplot(3,1,3),surf(FlaggingCube1)
shading interp;
xlabel('frequency')
ylabel('time')
axis([1 NumberOfSubbands/2  1 NewDataLength(2)])
zlabel('FlaggingCube')
view(0,90)

% NumberOfPlot = NumberOfPlot + 1; % 3
% figure(NumberOfPlot)
% subplot(3, 1, 1), plot(p1);
% axis([1 NumberOfSubbands/2 0 0.0001])
% subplot(3, 1, 2), plot(pc1);
% axis([1 NumberOfSubbands/2 0 0.0001])
% subplot(3, 1, 3), plot(Gproc(2,:));
% axis([1 NumberOfSubbands/2 0 10])


NumberOfPlot = NumberOfPlot + 1; % 4
figure(NumberOfPlot)
subplot(2, 1, 1), surf(Gproc);
xlabel('frequency')
ylabel('time')
zlabel('Processing gain obtained by RFI blanking')
axis([1 NumberOfSubbands/2 1 NewDataLength(2)])
colormap('default');
shading interp;
subplot(2, 1, 2), surf(Rproc);
xlabel('frequency')
ylabel('time')
zlabel('Signal of interest lost by RFI blanking')
axis([1 NumberOfSubbands/2 1 NewDataLength(2)])
% colormap('default');
shading interp;
% subplot(3, 1, 3), plot(Gproc(2,:));
% axis([1 NumberOfSubbands/2 0 10])
