function f=spectralfft(sig,len)

%LOFAR spectral FFT
% sig(len)	: array of len time samples  
% len           : number of samples in sig

f=fft(sig,len);
%disp(f);
return
