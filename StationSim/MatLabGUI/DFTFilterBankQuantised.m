%Function using Filterbanks, it contains also the different steps of the quantization in the following way:
%   Input signals are quantized on quant_signal bits.
%   The filter (and so the subfilters) quantized on quant_h bits.
%   The output signal of each subfilter is quantized on quant_inputfft bits.
%   The output signal of each channel after the FFT algorithm is quantized on quant_outputfft bits.
   
%Returns:
%     output signal: vector of number of bins
   
function [OutputSignal] = DFTFilterBankQuantised(InputSignal, quant_signal, quant_inputfft, quant_outputfft,...
                                                     SubFilter, NumberOfSubbands, SubbandFilterLength, NumberOfBlocks)

% Initialization                                               
OutputSignal = zeros(1, NumberOfSubbands);
MaxBlocks = floor(length(InputSignal) / NumberOfSubbands);
if MaxBlocks < NumberOfBlocks
    NumberOfBlocks = MaxBlocks;
end
                                               
% Normalization of the input signals in order to replace the signal in the range [-1,+1]                                                   
if (max(InputSignal) ~= 0)
  % guard against null padded antenna signals
  InputSignal = InputSignal / max(InputSignal);                                                  
  InputSignal = double(uencode(InputSignal, quant_signal, 1, 'signed'));
  InputSignal = InputSignal / max(InputSignal);
end
% The first signal: rearanging the input signal in order to simulate the switcher
for i = 1 : NumberOfBlocks
    for s = 1 : NumberOfSubbands
        DownsampledSignal(s, i) = InputSignal(NumberOfSubbands * (i - 1) + s); 
    end    % i refers to the channel, s to the sample in the sequence of NumberOfBlocks points
    % DownsampledSignal is a matrix of NumberOfSubbands lines, each line is the input of a subfilter 
end

% Convolution
for i = 1 : NumberOfBlocks - SubbandFilterLength + 1

    for s = 1 : NumberOfSubbands % Normalization of the result of the convolution by 1/SubbandFilterLength to stay in [-1,1]
        ConvolutionOutput(s) = 1 / SubbandFilterLength * DownsampledSignal(s, i : i + SubbandFilterLength - 1) * SubFilter(s, :)'; 
    end
    
    % Quantise the input of the FFT
    ConvolutionOutput = double(uencode(ConvolutionOutput, quant_inputfft, 1, 'signed')) / 2^(quant_inputfft - 1);  
    
    %Performing the FFT
    FFTOutput = 1 / NumberOfSubbands * fft(ConvolutionOutput); % Normalization of the result of the FFT by 1/NumberOfSubbands
    FFTOutput = double(uencode(FFTOutput, quant_outputfft, 1, 'signed')) / 2^(quant_outputfft - 1);
    
    % Concatenate the output of the FFT to the output signal
    if i == 1
        OutputSignal = FFTOutput;
    else % This matrix contains NumberOfSubbands collons (NumberOfSubbands channels) and the lines are the realizations
        OutputSignal = [OutputSignal; FFTOutput];
    end

%     % Concatenate the output of the FFT to the output signal
%     if i == 1
%         OutputSignal = ConvolutionOutput;
%     else % This matrix contains NumberOfSubbands collons (NumberOfSubbands channels) and the lines are the realizations
%         OutputSignal = [OutputSignal; ConvolutionOutput];
%     end

end
