%
% Subspace projection
%
function w=awe(vector, value, d, nrfi, nant)
%
% Author       : Sylvain Alliot
% Organisation : ASTRON (Netherlands Foundation for research in Astronomy)
% Date         : 25 November 2001
%
    B = vector(:, nant-nrfi+1:1:nant);
    temp=((B.'*B)^-1*B.');
    w=d-B*temp*d;
