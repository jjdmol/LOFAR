 function [Y1,Y2]=plotspectrum3(Signal,px,py,look_dir_phi,look_dir_theta,WeightVector)

    snapshot_number=100;
    % First plot spectrum without adaptive nulling        
    w=steerv(px,py,look_dir_phi,look_dir_theta);
    Y1=w(:)'*Signal;       % DBF
    N=1024;
    if snapshot_number>N                % Find automatic segment length
        NBINT=fix(snapshot_number/N);
    else
        NBINT=1; 
        N=snapshot_number;
    end
    %y=fx(Y, N, NBINT);        % simple autocorrelation
    figure;
    subplot(1,2,1)
    x=(0:1:N/2-1);
    plot(20*log10(abs(Y1)));
    %plot(x,20*log10(y));
    %axis([0 length(Y1) 40 45])
    title('Time series in the subband before adaptive nulling');
    
    % Then plot spectrum with adaptive nulling
    w=WeightVector;
    Y2=w(:)'*Signal;%AntennaSignals(:,:); 
    if snapshot_number>N                % Find automatic segment length
   %     NBINT=fix(snapshot_number/N);
    else
        NBINT=1; 
        N=snapshot_number;
    end
    y=fx(Y1, N, NBINT);        % simple autocorrelation
    subplot(1,2,2)
    x=(0:1:N/2-1);
    plot(20*log10(abs(Y2)));
    %plot(x,20*log10(y));
    axis([0 length(Y2) -50 0])    
    title('Time series in the subband after adaptive nulling')
