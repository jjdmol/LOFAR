subplot(HPerioMod)
periodogram(mat,hamming(length(mat)),'onesided',16384,fs);