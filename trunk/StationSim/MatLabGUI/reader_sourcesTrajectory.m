function [Phi_theta]=reader_sourcesTrajectory(record_traj, dirpath);
for i=1:length(record_traj)
    traject=reader_Trajectory([dirpath 'Matlab_Dat_Files\' record_traj{i}]);
if (i>1) 
    ecart=size(Phi_theta,2)-size(squeeze(traject(1:2,:)),2);
    if ecart>0
    Phi_theta=Phi_theta(:,1:size(Phi_theta,2)-ecart,:);
    else
    traject=traject(:,1:size(traject,2)-ecart);
end
end 
    Phi_theta(:,:,i)=squeeze(traject(1:2,:));
end
%Phi_theta=Phi_theta.';