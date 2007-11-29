%====================================================================================================
% DECIMATION S-function
% This S-function stands for the decimation controler, and addresses the blocks where the
% corresponding decimations will be filtered
%====================================================================================================

function [sys,x0,str,ts] = decimation(t,x,u,flag,param1,param2,param3,param4)

F_sample = param1;       % sampling frequency
N = param2;              % fft input length
D = param3;              % decimation factor and number of blocks
L = param4;              % subfilter length
M = N*(L+1);             % number of samples in a matrix
A=N/D;                   % number of lines in a matrix

in = 2;                  % number of inputs
        clocksample =1;         % first input: clock
        inputsignal =2;         % second output: input signal

out = 2*D+3;             % number of outputs
                                % first output: raw of the matrix where the current 
                                % decimation is written
                                % second output: address of the RAM where the current
                                % decimation is written
                                % outputs from 3 to 2+D: decimations that are the input
                                % of the D blocks countaining the subfilters
                                % outputs from 2+D+1 to 2+2*D: write enable for the RAM
                                % of each block countaining the subfilters

discr = 2*D+7;             % number of discrete states:
        decimsample = 2*[1:D]-1;% array for the decimated samples
        writeenable = 2*[1:D];  % array for the write enable signals
        counterM    = 2*D+1;    % counter from 0 to M-1
        counterN    = 2*D+2;    % counter from 0 to N-1
        counterD    = 2*D+3;    % counter from 0 to D-1
        counterL    = 2*D+4;    % counter from 0 to L-1
        counterA    = 2*D+5;    % counter from 0 to A-1
        newraw      = 2*D+6;    % trigger indicating that we start to write in a new raw
        addressRAM  = 2*D+7;    % write address of the RAM, for all the blocks
                                
                                
switch flag,            % flag indicating the state of the S-function
    
  case 0,               % Initialization
    [sys,x0,str,ts]=mdlInitializeSizes(M,F_sample,in,out,discr,clocksample,inputsignal,...
                                        decimsample,writeenable,counterM,counterN,counterD,...
                                        counterL,counterA,newraw,addressRAM);
                                    
  case 2,               % Update
    sys=mdlUpdate(t,x,u,M,N,D,L,A,clocksample,inputsignal,decimsample,writeenable,counterM,...
                  counterN,counterD,counterL,counterA,newraw,addressRAM);
              
  case 3,               % Outputs
    sys=mdlOutputs(t,x,u,M,N,D,L,A,clocksample,inputsignal,decimsample,writeenable,counterM,...
                    counterN,counterD,counterL,counterA,newraw,addressRAM);
                
  case {1,4,9}          % Unused flags:
    sys=[];                 % 1: mdlDerivatives(t,x,u);
                            % 4: mdlGetTimeOfNextVarHit(t,x,u);
                            % 9: mdlGetTimeOfNextVarHit(t,x,u);
                            
  otherwise             % Unexpected flags
    error(['Unhandled flag = ',num2str(flag)]);
    
end

% end DECIMATION

%====================================================================================================
% mdlInitializeSizes
% Return the sizes, initial conditions, and sample times for the S-function.
%====================================================================================================

function [sys,x0,str,ts]=mdlInitializeSizes(M,F_sample,in,out,discr,clocksample,inputsignal,...
                                            decimsample,writeenable,counterM,counterN,counterD,...
                                            counterL,counterA,newraw,addressRAM)

sizes = simsizes;               % call simsizes for a sizes structure, fill it in
                                % and convert it to a sizes array.

sizes.NumContStates  = 0;       % number of continuous states
sizes.NumDiscStates  = discr;   % number of discrete states
sizes.NumOutputs     = out;     % number of outputs
sizes.NumInputs      = in;      % number of inputs
sizes.DirFeedthrough = 1;       % the outputs depends on the inputs
sizes.NumSampleTimes = 1;       % at least one sample time is needed

sys = simsizes(sizes);          % take the previous parameters into account
                                % to create the characteristics of the S-function

% here we initialize all the discrete states
x0(decimsample)=0;  % array for the decimated samples
x0(writeenable)=0;  % array for the write enable for the RAM of each block of sub-filter
x0(counterM)=M-1;   % counter from 0 to M-1             
x0(counterN)=0;     % counter from 0 to N-1
x0(counterD)=0;     % counter from 0 to D-1
x0(counterL)=0;     % counter from 0 to L-1
x0(counterA)=0;     % counter from 0 to A-1
x0(newraw)=0;       % trigger indicating that we start to fill in a new raw in the matrix
x0(addressRAM)=0;   % write address of the RAM, for all the blocks

str = [];                    % str is always an empty matrix
ts  = [ F_sample 0 ];        % initialize the array of sample times (frequency, offset)

% end mdlInitializeSizes

%====================================================================================================
% mdlUpdate
% Handle discrete state updates, sample time hits, and major time step
% requirements.
%====================================================================================================

function sys=mdlUpdate(t,x,u,M,N,D,L,A,clocksample,inputsignal,decimsample,writeenable,...
                        counterM,counterN,counterD,counterL,counterA,newraw,addressRAM)

% if the clock is high
if u(clocksample)==1      
    
    % if we filled in a complete matrix, then we re-initialize the counters and signals
    if x(counterM)==M-1
        sys(counterM)=0;
        sys(counterN)=0;
        sys(counterD)=1;
        sys(counterL)=0;
        sys(counterA)=0;
        sys(newraw)=1;          % we start to fill in a new raw, so the trigger is on
        sys(addressRAM)=0;      % we start to write in the RAM of each block at @0

    % else if the matrix is not full    
    else
        sys(counterM)=x(counterM)+1;         % increment the samples counter
        
        % if we finished to fill in a raw, then we re-initialize the counters and signals
        if x(counterN)==N-1
            sys(counterN)=0;
            sys(counterD)=1;
            sys(counterA)=0;
            sys(newraw)=1;                   % we start to fill in a new raw, so the trigger is on
            sys(addressRAM)=x(addressRAM)+1; % we increment the write address of the RAM
            
            % if the raw was the last one, restart counting raws, else increment the raw counter
            if x(counterL)==L
                sys(counterL)=0;
            else
                sys(counterL)=x(counterL)+1;
            end;
            
        % else if we are still filling in the same raw    
        else
            sys(newraw)=0;                   % this is not a new raw, so the trigger is off
            sys(counterN)=x(counterN)+1;     % increment the number of samples in the raw
            sys(counterL)=x(counterL);       % the raw remains the same
            sys(addressRAM)=A*x(counterL)+x(counterA); % write in the same raw, in the next line
            
            % if the decimation counter reached its maximum value (if we addressed all the blocks)
            % then restart the counter from 0
            if x(counterD)==D-1
                sys(counterD)=0;
                
                % if we finished to fill in a raw, then restart the line counters from 0, else increment
                if x(counterA)==A-1
                    sys(counterA)=0;
                else
                    sys(counterA)=x(counterA)+1;
                end;
            
            % else if we didn't address all the blocks, then increment D counter and line counter
            else
                sys(counterD)=x(counterD)+1;
                sys(counterA)=x(counterA);
            end;
        end;
    end;
    
    % here we affect the discrete states that correspond directly to the outputs
    sys(decimsample)=x(decimsample);        % hold the same value for all the blocks, except for
    sys(2*x(counterD)+1)=u(inputsignal);    % the block waiting for the last decimation
    sys(2*x(counterD)+2)=1;                 % enable to write in this block
    
end;
% end mdlUpdate

%====================================================================================================
% mdlOutputs
% Return the block outputs.
%====================================================================================================

function sys=mdlOutputs(t,x,u,M,N,D,L,A,clocksample,inputsignal,decimsample,writeenable,...
                        counterM,counterN,counterD,counterL,counterA,newraw,addressRAM)

sys(1)=x(newraw);                   % first output: trigger indicating that we start to fill in a
                                    % new raw of the matrix
sys(2)=x(counterL);                 % the second output indicates the number of the raw where the
                                    % current decimation is written
sys(3)=x(addressRAM);               % the third is the write address of the RAM, for all the blocks
sys(3+decimsample)=x(decimsample);  % the outputs 4,6,8,...,2*D+2 are the decimated samples for
                                    % the D blocks
sys(3+writeenable)=x(writeenable);  % the outputs 5,7,9,...,2*D+3 are the write enable for the
                                    % D blocks

% end mdlOutputs