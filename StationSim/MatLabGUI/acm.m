%
% Measure the ACM from a snapshot sequence of data and solves the eigen system
% Find RFI steering vectors
% Plot the linear dependence between eigenvectors and steeringvectors of rfi

function [v, d,ACM] = acm(sh, nantl, nsnsh,phiRFI, thetaRFI, px, py, nrfi, testRFI,alpha)
%
% Author       : Sylvain Alliot
% Organisation : ASTRON (Netherlands Foundation for research in Astronomy)
% Date         : 25 November 2001
%
   
    dirpath='data';
    ACM = zeros( nantl, nantl,size(sh,3));
    v = zeros( nantl, nantl,size(sh,3));
    d = zeros( nantl, nantl,size(sh,3));
    for j=1:size(sh,3)
    for k=1:nsnsh
       ACM(:,:,j) = (1-alpha)*ACM(:,:,j) + (sh(:,k,j) * sh(:,k,j)'); 
        %ACM = ACM + sh(:,k) * sh(:,k)';
     end
     ACM(:,:,j)= ACM(:,:,j) - eye(nantl, nantl);
     %ACM(:,:,j) =ACM(:,:,j)./nsnsh;  
     [vec, dval] = eig(ACM(:,:,j));
     v(:,:,j)=vec;
     d(:,:,j)=dval;
     end
     %[v, d] = eig(ACM);
    
%     load([dirpath '/output_options.mat']);
%     load([dirpath '/bf_options.mat']);
%     load([dirpath '/signal_options.mat']);
         
%     if signal_eigen_acm
%         figure(9)
%         if (usePASTd)
%             subplot(1,3,3)
%         end
%         plot((-sort(-diag(abs(d.')))),'+')
%         title('Eigen values of ACM')
%         %axis([0 100 -5 4]);
%     end;
%     % set(gca,'Tag','plotei');  % restore the value
% 
%     
%    
%     %ccc = measure of width spread of eigenvalues
%     ccc=max(max((abs(d))))-min(abs(diag(d)));
%     
% %     
% %     ObjHndl = findobj('Tag','MDL');
% %     set(ObjHndl,'String',['number of RFI found with MDL:   ', num2str(kmin)]);  
% % 
% 
%     if (signal_eigen_sv)
% 
%         %plot the linear dependence between eigenvectors and steeringvectors of rfi    
%         figure;
%         phi_t          = phiRFI;
%         theta_t        = thetaRFI;
%         inp2=ones(nrfi,nrfi);
%         for k = 1:nrfi
%             for l = 1:nrfi
%                 inp2(k,l) = abs(v(:,k)'*reshape(steerv(px,py,phi_t(l),theta_t(l)), nantl, 1 ))/nantl;
%             end
%         end
%         cs=contour(inp2);  
%         clabel(cs);   %surf(inp2)
%         shading interp
%         if nrfi==1
%             nrfi=2;
%         end;
%         xlim([1 nrfi ])        
%         ylim([1 nrfi ])
%         view(2)
%         xlabel('eigenvector_i')
%         ylabel('steervector_j')
%         title('linear dependence of steeringvectors and eigenvectors')
%     % set(gca,'Tag','plotdep2');  % restore the value   
%     end 
