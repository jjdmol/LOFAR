
%
% station direction coordinates for 7 timesteps
% 7 x 3 matrix
%
lmn_t=[                               % pointing towards
       0 0 1;                         % zenith
       0 1 0;                         % northern horizon
       1 0 0;                         % eastern horizon
       sin(pi/4) sin(pi/4) 0;         % north-eastern horizon
       sin(pi/4) 0 sin(pi/4);         % east-45 degrees elevation
       0 sin(pi/4) sin(pi/4);         % north-45 degrees elevation
       sqrt(1/3) sqrt(1/3) sqrt(1/3); % north-east-45 degrees elevation
]
size(lmn_t)

%
% antenna locations 7 x 3 matrix for 7 elements
%
pos=[ 0 0 0; 0 0 1; 0 1 0; 1 0 0; 1 1 0; 0 1 1; 1 0 1 ]
%pos=[0 0 0]
size(pos)

%
% create spectral window specification (2 subbands)
% starting at 10MHz
% 2 vector
%
freq=[0:1];
freq *= 256e3;
freq += 10e6

%
% speed of light
%
c=299792458.0;

%
% calculate weights dimensions 7 timesteps, 7 elements, 2 subbands
%
M0 = zeros(7,7);
M0 = lmn_t(:,2) * pos(:,1)' - lmn_t(:,1) * pos(:,2)' - lmn_t(:,3) * pos(:,3)';
% M0(1,:)

%
% Calculate weights for the subbands and write to file
%
wfile = fopen ("weights.dat", "w")
for subband=1:2
  weights = exp(2*pi*j*freq(subband)*M0/c);
  weights = reshape(weights.', 49,1)
  w = zeros(49,2);
  w(:,1) = real(weights)
  w(:,2) = imag(weights)
  size(w)
  fwrite(wfile, w.', "double")
endfor
fclose(wfile)
