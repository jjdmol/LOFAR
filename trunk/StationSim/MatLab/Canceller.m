function [SelectedSubbandSignals] = CSCS(SelectedSubbandSignals, BlankingVectors)

% The spectra of the two pairs are calculated using the FFT
SelectedSubbandSignals = SelectedSubbandSignals .* BlankingVectors;
