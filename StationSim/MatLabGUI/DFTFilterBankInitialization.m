function [SubFilter] = DFTFilterBankInitialization (SubbandFilterLength, NumberOfSubbands)

% Init
alpha = 1; %NumberOfSubbands / D;
quant_h = 32;

%The filter  
h = fircls1(NumberOfSubbands - 1, SubbandFilterLength / NumberOfSubbands * alpha, .01, .001);
h = resample(h, NumberOfSubbands * SubbandFilterLength, NumberOfSubbands);
h = h / sqrt(h * h'); % normalisation

% The filter 
h = h / max(h);
h = double(uencode(h, quant_h, 1, 'signed'));
h = h / max(h);                                 % Quantization and normalization of the filter in order to avoid a bias

% Creation of subfilters of length SubbandFilterLength
for n = 1 : NumberOfSubbands 
    for i = 1 : SubbandFilterLength
         SubFilter(n, i) = h((i - 1) * NumberOfSubbands + n);
    end
end
