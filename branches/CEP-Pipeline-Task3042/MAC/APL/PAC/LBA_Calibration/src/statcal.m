function [cal, sigmas, Sigman] = statcal(acc, t_obs, freq, xpos, ypos, lon, lat, restriction, maxrestriction)

% [cal, sigmas, Sigman] =
%     statcal(acc, t_obs, freq, xpos, ypos, lon, lat, restriction,
%             maxresitriction)
%
% Wrapper function to provide the appropriate data to cal_ext
%
% arguments
% acc            : Nelem x Nelem x Nch array covariance matrix
% t_obs          : Nch x 1 vector of observing times
% freq           : Nch x 1 vector of observing frequencies
% xpos           : Nelem x 1 vector of x-positions of the antennas
% ypos           : Nelem x 1 vector of y-positions of the antennas
% lon            : geographic longitude ot the array
% lat            : geographic latitude of the array
% restriction    : relative baseline restriction in wavelength
% maxrestriction : maximum absolute baseline restriction in meters
%
% return values
% cal    : Nelem x 1 vector with complex valued calibration corrections
% sigmas : Nsrc x 1 vector with estimated apparent source powers
% Sigman : Nelem x Nelem estimated array covariance matrix
%
% SJW, 2009

% parameter section
diffstop = 1e-3;
maxiter = 5;
c = 2.99792e8;
Nelem = length(xpos);
Nsb = length(freq);
load srclist3C
srcsel = [324, 283, 88, 179]; % A-team
nsrc = length(srcsel) + 1; % sun will be included later

% initialization
cal = zeros(Nsb, Nelem);
sigmas = zeros(Nsb, nsrc);
Sigman = zeros(Nsb, Nelem, Nelem);
u = meshgrid(xpos) - meshgrid(xpos).';
v = meshgrid(ypos) - meshgrid(ypos).';
uvdist = sqrt(u.^2 + v.^2);

for idx = 1:length(freq)
    disp(['working on subband ' num2str(idx) ' of ' num2str(length(freq))]);
    pause(0.01);
    tic
    [raSun, decSun] = SunRaDec(JulianDay(t_obs(idx)));
    rasrc = [srclist3C.alpha(srcsel); raSun * pi / 180];
    decsrc = [srclist3C.delta(srcsel); decSun * pi / 180];
    [lsrc, msrc] = radectolm(rasrc, decsrc, JulianDay(t_obs(idx)), lon, lat);
    up = ~isnan(lsrc(:, 1));
    A = exp(-(2 * pi * i * freq(idx) / c) * (xpos * lsrc(up, 1).' + ypos * msrc(up, 1).'));
    Rhat = squeeze(acc(:, :, idx));

    mask = uvdist < min([restriction * (c / freq(idx)), maxrestriction]);

    flux = real(inv(abs(A' * A).^2) * khatrirao(conj(A), A)' * (Rhat(:) .* (1 - mask(:))));
    flux = flux / flux(1);
    flux(flux < 0) = 0;

    % implementation of WALS method
    % estimate direction independent gains, apparent source powers and
    % receiver noise powers assuming that the source locations are correct
    [ghat, sigmahat, Sigmanhat] = cal_ext(Rhat, A, flux, mask, diffstop, maxiter);
    
    cal(idx, :) = conj(1./ghat);
    sigmas(idx, up) = sigmahat;
    Sigman(idx, :, :) = Sigmanhat;    
    toc
end
