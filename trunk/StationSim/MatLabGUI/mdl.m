function kmin = mdl(d, nantl,nsnsh)
disp('Attention load de la valeur du nombre d''antennes'); 
% mimimum description length algorithm
% M = number of antennas, N = number of snapshots

% res=find(d>10e11)
% kmin=length(res)
       kmin=0;
        for k=1:nantl
%            nom    = 1/(nantl-k)*sum(abs(diag(d(k+1:nantl,k+1:nantl))));
%            denom  = prod(abs(diag(d(k+1:nantl,k+1:nantl))))^(1/(nantl-k));
%             MDL(k) = (nantl - k) * nsnsh * log(nom/denom) + ...
%            k/2*(2*nantl-k+1)*log(nsnsh);
            quot(k) = prod((d(k:nantl))) *(((1/(nantl-(k-1)))*(sum(d(k:nantl))))^(-(nantl-(k-1))));
             if (isinf(quot(k)))
                 MDL(k)=1.797*10^308;
             else
             MDL(k) = -(nsnsh) * log(quot(k)) + (k-1)/2 * (2*nantl - (k-1)) * log(nsnsh); 
             end
             if (k == 1)             
                 mdlmin=(MDL(k));
             else
             if ( (MDL(k)) < mdlmin )
                     kmin = k-1;
                     mdlmin = (MDL(k));
                 end
             end
        end
        save('data/mdl-temp.mat','MDL');
        fprintf('\t\t\tMDL found %d sources.\n',kmin);