%Function using Filterbanks, it contains also the different steps of the quantization in the following way:
%   Input signals are quantized on quant_signal bits.
%   The filter (and so the subfilters) quantized on quant_h bits.
%   The output signal of each subfilter is quantized on quant_inputfft bits.
%   The output signal of each channel after the FFT algorithm is quantized on quant_outputfft bits.
   
%Returns:
%     output signal: vector of number of bins
   
function [OutputSignal] = DFTFilterBankQuantised(InputSignal, quant_signal, quant_inputfft, quant_outputfft,...
                                                     SubFilter, NumberOfSubbands, SubbandFilterLength, NumberOfBlocks, Stage)

if Stage > 1  
    overlap=0;
else
    overlap=0;
    %overlap=NumberOfSubbands/8;
   % overlap=NumberOfSubbands/2;
end                                                 
                                                 
% Initialization    
OutputSignal = zeros(1, NumberOfSubbands);
MaxBlocks = floor(length(InputSignal) / NumberOfSubbands);
if MaxBlocks < NumberOfBlocks
    NumberOfBlocks = MaxBlocks;
end
                                               
% Normalization of the input signals in order to replace the signal in the range [-1,+1] 

%InputSignal = InputSignal / max(InputSignal);
%InputSignal = double(uencode(InputSignal, quant_signal, 1, 'signed'));
%InputSignal = InputSignal / max(InputSignal);


% The first signal: rearanging the input signal in order to simulate the switcher
if overlap>0
for ob = 1: NumberOfBlocks-(SubbandFilterLength+1) 
  for i = 1 : SubbandFilterLength
    for s = 1 : NumberOfSubbands
        
        DownsampledSignal(s,i, ob) = InputSignal(NumberOfSubbands * (i - 1) + s +(ob-1)*( NumberOfSubbands-overlap)); 
    end    % i refers to the channel, s to the sample in the sequence of NumberOfBlocks points
    % DownsampledSignal is a matrix of NumberOfSubbands lines, each line is the input of a subfilter 
  end
end
else
for i = 1 : NumberOfBlocks
    for s = 1 : NumberOfSubbands
        DownsampledSignal(s, i) = InputSignal(NumberOfSubbands * (i - 1) + s); 
    end    % i refers to the channel, s to the sample in the sequence of NumberOfBlocks points
    % DownsampledSignal is a matrix of NumberOfSubbands lines, each line is the input of a subfilter 
end
end

% Convolution
for ob = 1: NumberOfBlocks-(SubbandFilterLength+1)
    if overlap>0
    if SubbandFilterLength>1
      for s = 1 : NumberOfSubbands % Normalization of the result of the convolution by 1/SubbandFilterLength to stay in [-1,1]
        
         ConvolutionOutput(s) = 1 / SubbandFilterLength * DownsampledSignal(s,1:1:SubbandFilterLength,ob) * SubFilter(s,1:1:SubbandFilterLength)'; 
        % Do only an addition
     %   ConvolutionOutput(s) = DownsampledSignal(s, i : i + SubbandFilterLength - 1) * ones(SubbandFilterLength, 1);         
      end
    else
      for s = 1 : NumberOfSubbands % Normalization of the result of the convolution by 1/SubbandFilterLength to stay in [-1,1]
        
         ConvolutionOutput(s) = DownsampledSignal(s,1:1:SubbandFilterLength,ob); 
        % Do only an addition
     %   ConvolutionOutput(s) = DownsampledSignal(s, i : i + SubbandFilterLength - 1) * ones(SubbandFilterLength, 1);         
      end
    end
    else
    for s = 1 : NumberOfSubbands % Normalization of the result of the convolution by 1/SubbandFilterLength to stay in [-1,1]
         ConvolutionOutput(s) = 1 / SubbandFilterLength * DownsampledSignal(s, ob : ob + SubbandFilterLength - 1) * SubFilter(s, :)'; 
        % Do only an addition
%        ConvolutionOutput(s) = DownsampledSignal(s, i : i + SubbandFilterLength - 1) * ones(SubbandFilterLength, 1);         
    end
    end
    % Quantise the input of the FFT
   %ConvolutionOutput = double(uencode(ConvolutionOutput, quant_inputfft, 1, 'signed')) / 2^(quant_inputfft - 1);  
    
%  This section is commented out for Station sim verification, don't forget to decomment for normal operation (SDP.m)!!!
%
    %Performing the FFT
    FFTOutput = 1 / NumberOfSubbands * fft(ConvolutionOutput); % Normalization of the result of the FFT by 1/NumberOfSubbands
    %FFTOutput = double(uencode(FFTOutput, quant_outputfft, 1, 'signed')) / 2^(quant_outputfft - 1);
    %test=OutputSignal;
    % Concatenate the output of the FFT to the output signal
    
  if Stage > 1  
    if ob == 1
         OutputSignal=[FFTOutput(NumberOfSubbands/2:-1:1) FFTOutput(NumberOfSubbands:-1:NumberOfSubbands/2+1)];

       
    %    OutputSignal = fftshift(FFTOutput);
        
    else % This matrix contains NumberOfSubbands collons (NumberOfSubbands channels) and the lines are the realizations
      OutputSignal = [OutputSignal; [FFTOutput(NumberOfSubbands/2:-1:1) FFTOutput(NumberOfSubbands:-1:NumberOfSubbands/2+1) ]];
      
    %  OutputSignal = [OutputSignal; fftshift(FFTOutput)];
    end
  else
     if ob == 1
        OutputSignal = FFTOutput;
    else % This matrix contains NumberOfSubbands collons (NumberOfSubbands channels) and the lines are the realizations
        OutputSignal = [OutputSignal; FFTOutput];
    end
  end  
      
%     % Concatenate the output of the Convolution to the output signal
 %    if ob == 1
%       OutputSignal = ConvolutionOutput;
%else % This matrix contains NumberOfSubbands collons (NumberOfSubbands channels) and the lines are the realizations
 %       OutputSignal = [OutputSignal; ConvolutionOutput];
end

