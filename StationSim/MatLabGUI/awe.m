
% Subspace projection
%
function w=awe(vector, value, nrfi, nant,d,useWeight)
%
% Author       : Sylvain Alliot
% Organisation : ASTRON (Netherlands Foundation for research in Astronomy)
% Date         : 25 November 2001
% awe(Evector, Evalue, LookingDirection, rfi_number, NumberOfAntennas,LookingDirection,useWeight);
 % load('data/bf_options.mat');

    if (size(vector,1) > nant)
        fprintf('SOMETHING IS SERIOUSLY WRONG!\n');
    end

    if useWeight
        % SVD and PASTd place the RFI vectors in front
        B=vector(:,1:nrfi);
    else
        % EVD places it at the rear
        B = vector(:, nant-nrfi+1:1:nant);
    end
    temp=((B'*B)^-1*B');
    if temp
        w=d-B*temp*d; % (I-V)*d = P * d  w should be perpendicular to the looking direction
    else 
        % temp is empty, no rfi's detected, return looking direction
        w=d;
    end
