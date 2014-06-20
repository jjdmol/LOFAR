function [cal, flags] = calibrate(acc, antpos, srcpos, freq)
% [cal, flags] = calibrate(acc, srcpos, antpos, freq)
%
% Function performing the station calibration routine on the array
% covariance cube (ACC) provided returning calibration factors and flags
% per antenna per subband indicating bad antennas and RFI. This is a
% monolithic variant of the station calibration implemented earlier for
% compilation to a C++ shared library used by the LCU.
%
% Arguments
% acc    : Nrcu x Nrcu x Nsb array covariance cube containing an array
%          covariance matrix for each subband
% antpos : Nelem x 3 matrix with (x, y, z)-positions of the antennas in
%          Cartesian peak UR coordinates in meters
% srcpos : Nsrc x 3 matrix with (l, m, n)-coordinates of the calibrator
%          sources to be used in Cartesian peak UR coordinates
% freq   : Nsb x 1 vector containing the center frequencies of the subbands
%          in Hertz. If a frequency is set to zero, it is assumed to be in
%          a stopband or aliased band and the corresponding subband will be
%          skipped.
%
% Return values
% cal    : Nsb x Nrcu matrix containing complex calibration factors per
%          subband per receiver path. These values can be applied to the
%          beam former coefficient to get a nominal beam.
% flags  : Nsb x Nrcu matrix containing flags per subband per receive path
%          indicating the result from RFI and bad antenna detection.
%
% Stefan Wijnholds, 19 November 2008

    % make sure Matlab does not claim all CPUs
    maxNumCompThreads(1);
    disp(['Calibration routine will use ' num2str(maxNumCompThreads) ' cores']);

    % parameters
    c = 2.99792e8;           % speed of light in m/s
    uvlimlambda = 4;         % baseline restriction in wavelength
    uvlimmeter = 20;         % baseline restriction in meters
    Nelem = size(antpos, 1); % number of elements in the array
    Nsb = length(freq);      % number of subband
    Ns = freq(2)-freq(1);    % number of samples after 1 s integration
    sbidx = 1:Nsb;           % index for subbands
    diffstop = 1e-3;         % stop criterion for calibration routine
    maxiter = 10;            % maximum number of iterations in calibration loop

    % tresholds for RFI detection and bad antenna detection
    tresholdRFI = 10 * sqrt(2 * Nelem / Ns);  % 10 sigma
    tresholdRCU = 5;                          % 5 sigma

    % ignore subbands in stopband or aliased bands
    sbsel = freq > 0;

    % RFI detection
    [cleansbx, cleansby] = RFIdetection(acc, tresholdRFI);
    sbselx = sbsel & cleansbx;
    sbsely = sbsel & cleansby;

    % check for bad antennas
    [selx, sely] = detectBadElem(acc, sbselx, sbsely, tresholdRCU);

    % determine baseline lengths
    u = meshgrid(antpos(:, 1)) - meshgrid(antpos(:, 1)).';
    v = meshgrid(antpos(:, 2)) - meshgrid(antpos(:, 2)).';
    w = meshgrid(antpos(:, 3)) - meshgrid(antpos(:, 3)).';
    uvwdist = sqrt(u.^2 + v.^2 + w.^2);

    % calibrate array of x-elements
    tic
    calx = zeros(Nsb, sum(selx));
    sbidxx = sbidx(sbselx);
    for idx = sbidxx;
        disp(['Working on subband ' num2str(find(sbidxx == idx)) ' of ' num2str(sum(sbselx))]);
        % initialization
        A = exp(-(2 * pi * i * freq(idx) / c) * antpos(selx, :) * srcpos.');
        Rhat = squeeze(acc(1:2:end, 1:2:end, sbidx(idx)));
        Rhat = Rhat(selx, selx);
        mask = uvwdist < min([uvlimlambda * (c / freq(idx)), uvlimmeter]);
        mask = mask(selx, selx);
        flux = real(inv(abs(A' * A).^2) * khatrirao(conj(A), A)' * (Rhat(:) .* (1 - mask(:))));
        flux = flux / flux(1);
        flux(flux < 0) = 0;
        % calibrate using WALS method
        calx(idx, :) = cal_ext(Rhat, A, flux, mask, diffstop, maxiter);
    end
    calx0 = checkCal(calx, selx, Nsb, Nelem);
    toc
    
    % calibrate array of y-elements
    tic
   caly = zeros(Nsb, sum(sely));
    sbidxy = sbidx(sbsely);
    for idx = sbidxy;
        disp(['Working on subband ' num2str(find(sbidxy == idx)) ' of ' num2str(sum(sbsely))]);
        % initialization
        A = exp(-(2 * pi * i * freq(idx) / c) * antpos(sely, :) * srcpos.');
        Rhat = squeeze(acc(2:2:end, 2:2:end, sbidx(idx)));
        Rhat = Rhat(sely, sely);
        mask = uvwdist < min([uvlimlambda * (c / freq(idx)), uvlimmeter]);
        mask = mask(sely, sely);
        flux = real(inv(abs(A' * A).^2) * khatrirao(conj(A), A)' * (Rhat(:) .* (1 - mask(:))));
        flux = flux / flux(1);
        flux(flux < 0) = 0;
        % calibrate using WALS method
        caly(idx, :) = cal_ext(Rhat, A, flux, mask, diffstop, maxiter);
    end
    caly0 = checkCal(caly, sely, Nsb, Nelem);
    toc

    cal = zeros(Nsb, 2 * Nelem);
    cal(:, 1:2:end) = calx0;
    cal(:, 1:2:end) = caly0;
    flags = (cal == 0);
    % interpolation (currently not implemented)
    cal(cal == 0) = 1;

end % end function calibrate

% Helper functions:
%   RFIdetection: detects RFI occupied subbands unsuitable for calibration
%   detectBadElem: detect failing elements
%   checkCal: post-calibration check on calibration results
%   cal_ext: implements WALS calibration method
%   computeAlphaW: helper function for cal_ext
%   khatrirao: computes Khatri-Rao product of two matrices

function [cleansbx, cleansby] = RFIdetection(acc, treshold)
    Nsb = size(acc, 3);
    teststatx = zeros(Nsb, 1);
    teststaty = zeros(Nsb, 1);
    for sb = 1:Nsb
        Rhat = squeeze(acc(:, :, sb));
        Rwhite = Rhat ./ sqrt(diag(Rhat) * diag(Rhat).');
        teststatx(sb) = norm(Rwhite(1:2:end, 1:2:end), 'fro').^2;
        teststaty(sb) = norm(Rwhite(2:2:end, 2:2:end), 'fro').^2;
    end
    cleansbx = max(abs(teststatx - [teststatx(2:end); 0]), abs(teststatx - [0; teststatx(1:end-1)])) < treshold;
    cleansby = max(abs(teststaty - [teststaty(2:end); 0]), abs(teststaty - [0; teststaty(1:end-1)])) < treshold;
end % end function RFIdetection

function [selx, sely] = detectBadElem(acc, sbselx, sbsely, tresholdRCU)
    Nelem = size(acc, 1) / 2;
    ac = zeros(2 * Nelem, size(acc, 3));
    for idx = 1:2 * Nelem
        ac(idx, :) = squeeze(acc(idx, idx, :));
    end
    acx = ac(1:2:end, sbselx);
    acy = ac(2:2:end, sbsely);
    teststatx = abs(acx - repmat(median(acx), Nelem, 1));
    treshold = tresholdRCU * repmat(std(acx), Nelem, 1);
    selx = sum((teststatx < treshold).') == sum(sbselx);
    teststaty = abs(acy - repmat(median(acy), Nelem, 1));
    treshold = tresholdRCU * repmat(std(acy), Nelem, 1);
    sely = sum((teststaty < treshold).') == sum(sbsely);
end % end function detectBadElem

function cal = checkCal(cal, sel, Nch, nelem)
    cal = cal ./ repmat(median(abs(cal), 2), [1 size(cal, 2)]);
    cal(isnan(cal)) = 0;
    delta = max(abs(cal - [cal(2:end, :); zeros(1, size(cal, 2))]), abs(cal - [zeros(1, size(cal, 2)); cal(1:end-1, :)]));
    selcalf = sum(delta < 0.3, 2) > 0.5 * nelem;
    selcalant = sum(abs(cal(selcalf, :)) < 3) == max(sum(abs(cal(selcalf, :)) < 3));
    cal(~selcalf, :) = 0;
    cal(:, ~selcalant) = 0;
    caltemp = zeros(Nch, nelem);
    caltemp(:, sel) = cal;
    cal = caltemp;
end % end function checkCal

function cal = cal_ext(Rhat, A, sigmas, mask, diffstop, maxiter)
    % parameters
    [Nelem, Nsrc] = size(A);

    % selection matrix for non-diagonal noise matrix
    % selection matrix main diagonal
    Isdiag = khatrirao(eye(Nelem), eye(Nelem));
    % selection matrix real parts off-diagonal elements
    seltriu = triu(mask, 1);
    seltriu = diag(seltriu(:));
    seltriu = seltriu(:, diag(seltriu) == 1);
    seltril = tril(mask, -1);
    seltril = diag(seltril(:));
    seltril = seltril(:, diag(seltril) == 1);
    Isre = seltriu + seltril;
    % selection matrix imaginary parts off-diagonal elements
    Isim = seltriu - seltril;
    % complete selection matrix
    Isel = [Isdiag, Isre, Isim];

    % initialization
    ghat = zeros(Nelem, maxiter+1);
    sigmahat = zeros(Nsrc, maxiter+1);
    sigmahat(:, 1) = sigmas;
    sigmanhat = zeros(sum(mask(:)), maxiter+1);
    sigmanhat(1:Nelem, 1) = 1;
    Sigma_n = zeros(Nelem);

    % implementation using WALS
    for iter = 2:maxiter+1
        % estimate g using baseline restriction
        alpha = computeAlphaW(Rhat .* (1 - mask), A * diag(sigmahat(:, iter-1)) * A' .* (1 - mask));
        [v, d] = eig(alpha);
        [dummy, idx] = max(diag(d));
        ghat(:, iter) = conj(v(:, idx));
        GA = diag(ghat(:, iter)) * A;
        Rest = GA * diag(sigmahat(:, iter-1)) * GA';
        compidx = (1 - eye(Nelem)) ~= 0;
        normg = abs(sqrt(pinv(Rest(compidx)) * Rhat(compidx)));
        ghat(:, iter) = normg * ghat(:, iter) / (ghat(1, iter) / abs(ghat(1, iter)));

        % estimate sigmanhat using sigmahat from previous iteration and ghat
        R0 = normg^2 * Rest;
        Sigma_n(:) = Isel * sigmanhat(:, iter-1);
        R = R0 + Sigma_n;
        pinvIsel = [Isdiag, 0.5 * Isre, 0.5 * Isim].';
        sigmanhat(:, iter) = pinvIsel * (Rhat(:) - R0(:));
    
        % estimate sigmahat using sigmanhat and ghat
        Sigma_n(:) = Isel * sigmanhat(:, iter);
        invR = inv(R);
        sigmahat(:, iter) = real(inv(abs(conj(GA' * invR * GA)).^2) * diag(GA' * invR * (Rhat - Sigma_n) * invR * GA));
        if sum(isfinite(sigmahat(:, iter))) ~= 0
            sigmahat(:, iter) = sigmahat(:, iter-1);
        end
        sigmahat(:, iter) = max(sigmahat(:, iter) / sigmahat(1, iter), 0); % no negative values!
        
        % test for convergence
        theta_prev = [ghat(:, iter-1); sigmahat(:, iter-1); sigmanhat(:, iter-1)];
        theta = [ghat(:, iter); sigmahat(:, iter); sigmanhat(:, iter)];
        if (abs(pinv(theta_prev) * theta - 1) < diffstop)
            break
        end
    end
    cal = conj(1 ./ ghat(:, iter));
end % end function cal_ext

function alphamat = computeAlphaW(Rhat, R0)
% alphamat = computeAlphaW(Rhat, R0)
%
% Support function for cal_ext which computes a matrix
% alphamat = g * (1./g).' which corresponds to the matrix M in [1], where g
% are the complex gains of the elements producing the input signals for
% which Rhat is the measured array covariance matrix (ACM) and R0 is the
% model ACM. If specific entries in these matrices are set to zero, these
% entries are not used for estimating alpha. This feature can be exploited
% when a baseline restriction is applied.
%
% Parameters
% Rhat     : p.p measured ACM
% R0       : p.p expected (model) ACM
%
% Return value
% alphamat : p.p matrix representing the best estimate for g * (1./g).'
%
% References
% [1] Stefan J. Wijnholds and Albert-Jan Boonstra, "A Multisource
% Calibration Method for Phased Array Radio Telescopes", IEEE Workshop on
% Sensor Array and Multichannel Processing (SAM), Waltham (MA), USA,
% July2006
%
% Stefan J. Wijnholds, 2006

    [nelem, nelem] = size(Rhat);
    Rhat_red = Rhat - diag(diag(Rhat));
    R0_red = R0 - diag(diag(R0));
    alphamat = zeros(nelem);
    
    for i = 1:nelem
        for j = 1:nelem
            nonzero = (Rhat_red(i, :) ~= 0) & (Rhat_red(j, :) ~= 0 & (R0_red(i, :) ~= 0) & (R0_red(j, :) ~= 0));
            rhati = Rhat(i, nonzero)';
            rhatj = Rhat(j, nonzero)';
            r0i = R0(i, nonzero)';
            r0j = R0(j, nonzero)';
            w = abs(r0i .* rhatj).^2;
            alphamat(i, j) =  sum(w .* (rhati .* r0j) ./ (rhatj .* r0i)) / sum(w);
        end
    end
end % end function computeAlphaW

function C = khatrirao(A, B)
% C = khatrirao(A, B)
%
% Computes the Khatri-Rao (column wise Kronecker) product of A and B.
%
% Parameters
% A, B : 2-D matrices with equal second dimension. This is not checked by
%        the function, so entering matrices with insuitable dimensions may
%        produce unexpected results.
%
% Return value
% C    : column wise Kronecker product of A and B
%
% Stefan J. Wijnholds, 2006

    C = zeros(size(B, 1) * size(A, 1), size(A, 2));
    for n = 1:size(A, 2)
        C(:, n) = kron(A(:, n), B(:, n));
    end
end % end function khatrirao
