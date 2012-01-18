
%
% station direction coordinates for 7 timesteps
% 7 x 3 matrix
%
lmn_t=[                               % pointing towards
       0  0 1;                        % zenith
       0  1 0;                        % northern horizon
       0 -1 0;                        % southern horizon

%       1 0 0;                         % eastern horizon
%       sin(pi/4) sin(pi/4) 0;         % north-eastern horizon
%       sin(pi/4) 0 sin(pi/4);         % east-45 degrees elevation
%       0 sin(pi/4) sin(pi/4);         % north-45 degrees elevation
%       sqrt(1/3) sqrt(1/3) sqrt(1/3); % north-east-45 degrees elevation
]

%
% antenna positions
% first antenna in origin, second antenna 100 meters away
%
pos=[ 0 0 0; 100 0 0; ]

%
% create spectral window specification (2 subbands)
% starting at 10MHz
% 2 vector
%
freq=[0:0];
%freq *= 256e3;
%freq += 1.5625e6
freq += 1.5e6

%
% speed of light
%
c=299792458.0;

%
% calculate weights dimensions 7 timesteps, 7 elements, 2 subbands
%
M0 = zeros(3,2);
M0 = lmn_t(:,2) * pos(:,1)' - lmn_t(:,1) * pos(:,2)' + lmn_t(:,3) * pos(:,3)';
% M0(1,:)
weights = exp(2*pi*j*freq(1)*M0/c)

%
% Calculate weights for the subbands and write to file
%
%wfile = fopen ("weights2.dat", "w")
%weights = exp(2*pi*j*freq(1)*M0/c);
%weights = reshape(weights.', 6,1)
%w = zeros(6,2);
%w(:,1) = real(weights)
%w(:,2) = imag(weights)
%size(w)
%fwrite(wfile, w.', "double")
%fclose(wfile)
