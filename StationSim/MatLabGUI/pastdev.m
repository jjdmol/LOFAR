%
% Implementation of PASTd to determine Eigen vectors (W) and values (d) 
% Grant Hampson
% Ver 1.0 - 12/8/2000
%
% Adapted for LOFAR stationGUI 
% Chris Broekema
% september 2002

function [W, d, snapcnt, vecconv] = pastdev(xq,EvalueSVD,EvectorSVD, numsnaps, interval, BetaPASTd, nantl)


% M = Number of antennas
% N = Number of snapshots (Hampson used reverse notation)
[N,M] = size(xq)  % obtain input size
snapcnt = 0;
EPS=1/nantl;


if numsnaps > N
   fprintf('        PASTd: Warning, numsnaps exceeds N! Reducing to N\n');
   numsnaps = N;
end

%
% Initilisation of the PASTd algorithm
%
epochs = length(1:interval:numsnaps)+1;
W = EvectorSVD;
d = ones(M,epochs)*2; 

%
% Run the PASTd algorithm
%
for snapidx = 1:interval:numsnaps
   snapcnt = snapcnt + 1;
   Wold = W;

   [W, d(:,snapcnt+1)] = pastd(xq(snapidx,:).', W, d(:,snapcnt), M, BetaPASTd);

   % Measure Eigen Vector convergence
   for iii = 1:M
      vecconv(snapcnt,iii) = abs(W(:,iii)'*Wold(:,iii));
   end
end   
%W = conj(W); % PASTd produces the conjugate of the ED technique.

if snapcnt < numsnaps
  fprintf('        PASTd: Warning, used only %d snaps.\n',int2str(snapcnt));
end

return;

function [W, d] = pastd(x, W, d, nmax, BetaPASTd)
%
% The PASTd function from Yang.
%
for n = 1:nmax
   y(n) = W(:,n)'*x;
   d(n) = BetaPASTd*d(n) + abs(y(n))^2;
   e = x - W(:,n)*y(n);
   W(:,n) = W(:,n) + e*(conj(y(n)/d(n)));
   x = x - W(:,n)*y(n);
end