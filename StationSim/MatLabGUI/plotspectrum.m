 function plotspectrum(xq,px,py,look_dir_phi,look_dir_theta,snapshot_number)

dirpath='data';
load([dirpath '\output_options']);
if OutputSignal
    load([dirpath '\antenna_signals']);
    load([dirpath '\rfi_eigen']);
    load([dirpath '\subband_options']);
    
    % First plot spectrum without adaptive nulling        
    w=steerv(px,py,look_dir_phi,look_dir_theta);
    Y=w(:)'*xq;       % DBF
    N=1024;
%     if snapshot_number>N                % Find automatic segment length
%         NBINT=fix(snapshot_number/N);
%     else
%         NBINT=1; 
%         N=snapshot_number;
%     end
 %   y=fx(Y, N, NBINT);        % simple autocorrelation
    figure(9);
    subplot(1,2,1)
    x=(0:1:N/2-1);
    plot(20*log10(abs(Y)));
    %plot(x,20*log10(y));
    %axis([0 N/2 10 100])
    title('Spectrum before adaptive nulling');
    
    % Then plot spectrum with adaptive nulling
    w=WeightVector;
    Y=w(:)'*xq;%AntennaSignals(:,:); 
%     if snapshot_number>N                % Find automatic segment length
%         NBINT=fix(snapshot_number/N);
%     else
%         NBINT=1; 
%         N=snapshot_number;
%     end
  %  y=fx(Y, N, NBINT);        % simple autocorrelation
    subplot(1,2,2)
    x=(0:1:N/2-1);
    plot(20*log10(abs(Y)));
    %plot(x,20*log10(y));
    %axis([0 N/2 10 100])
    
    title('Spectrum after adaptive nulling')
end;