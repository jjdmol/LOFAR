%
% Measure the ACM from a snapshot sequence of data and solves the eigen system
% Find RFI steering vectors
% Plot the linear dependence between eigenvectors and steeringvectors of rfi

function [v, d, kmin] = acm(sh, nantl, nsnsh,  phiRFI, thetaRFI, px, py, nrfi, testRFI)
%
% Author       : Sylvain Alliot
% Organisation : ASTRON (Netherlands Foundation for research in Astronomy)
% Date         : 25 November 2001
%

    dirpath='data';

    ACM = zeros( nantl, nantl);

    for k=1:nsnsh
        ACM = ACM + sh(:,k) * sh(:,k)';
    end
    ACM =ACM./nsnsh;
    ACM = ACM - eye(nantl, nantl); 
    [v, d] = eig(ACM);

    
    load([dirpath '\output_options.mat']);
    load([dirpath '\bf_options.mat']);

    if signal_eigen_acm
        figure(7);
        plot( (log10(-sort(-abs(diag(d))))), '+')
        xlabel('index eigenvalue')
        ylabel('^{10}log(eigenvalue)')
        title('eigenvalues of ACM')
    end;
    % set(gca,'Tag','plotei');  % restore the value

    % find RFI steering vectors

    if (rfi_strat == 1)
        % thresholding
        kmin=0;
        findrfi=sort(-abs(diag(d)));
        for k=1:nantl-1
            if (abs(findrfi(k))>abs(rfi_value*findrfi(nantl)))
                kmin   = kmin+1;
            end
        end
    elseif (rfi_strat == 2)
        % mimimum description length algorithm
        kmin=0;
        for k=1:nantl-1
            nom    = 1/(nantl-k)*sum(abs(diag(d(k+1:nantl,k+1:nantl))));
            denom  = prod(abs(diag(d(k+1:nantl,k+1:nantl))))^(1/(nantl-k));
            MDL(k) = (nantl - k) * nsnsh * log(nom/denom) + ...
                k/2*(2*nantl-k+1)*log(nsnsh);
            if (k == 1)
                kmin=1;
                mdlmin=(MDL(k));
            else
                if ( (MDL(k)) < mdlmin )
                    kmin   = k;
                    mdlmin = (MDL(k));
                end
            end
        end
        fprintf('MDL found %d sources.\n',kmin);
    else
        kmin = 0; % default value
    end
    
   
    %ccc = measure of width spread of eigenvalues
    ccc=max(max((abs(d))))-min(abs(diag(d)));
    
%     
%     ObjHndl = findobj('Tag','MDL');
%     set(ObjHndl,'String',['number of RFI found with MDL:   ', num2str(kmin)]);  
% 

    if (signal_eigen_sv)

        %plot the linear dependence between eigenvectors and steeringvectors of rfi    
        figure(8);
        phi_t          = phiRFI;
        theta_t        = thetaRFI;
        inp2=ones(nrfi,nrfi);
        for k = 1:nrfi
            for l = 1:nrfi
                inp2(k,l) = abs(v(:,k)'*reshape(steerv(px,py,phi_t(l),theta_t(l)), nantl, 1 ))/nantl;
            end
        end
        cs=contour(inp2);  
        clabel(cs);   %surf(inp2)
        shading interp
        if nrfi==1
            nrfi=2;
        end;
        xlim([1 nrfi ])
        ylim([1 nrfi ])
        view(2)
        xlabel('eigenvector_i')
        ylabel('steervector_j')
        title('linear dependence of steeringvectors and eigenvectors')
    % set(gca,'Tag','plotdep2');  % restore the value   
    end 
