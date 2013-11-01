%====================================================================================================
% READSYNCHRO S-function
% This S-function stands for the controler of the subfilters and of the MAC:
% it synchronises the samples after decimation and the subfilter coefficients,
%====================================================================================================

function [sys,x0,str,ts] = synchro(t,x,u,flag,param1,param2,param3,param4)

F_MAC = param1;          % MAC frequency
                         % NOTE: this parameter must be the same as the one in the clock source
                         % linked to the first input of the S-function
N = param2;              % fft input length
D = param3;              % decimation factor
L = param4;              % subfilter length
A=N/D;                   % number of lines in a matrix
B=A*L;                   % number of samples to read out of the RAM while a raw is written
C=B/2;                   % number of coefficients to read out of the ROM
                         % the symmetry of the filter is taken into account
E=L/2;                   % this variable is used to generate the read address of the ROM

in=3;                    % number of inputs
        clockMAC = 1;           % the first input is the MAC clock
                                % NOTE: this clock MUST be smaller than the clock of the decimation
        newraw   = 2;           % the second input is the trigger signal
        raw      = 3;           % the third input is the number of the raw where the current 
                                % decimation is written
        
out=8;                   % number of outputs
                                % output 1: error signal indicating that the matrix was not entirely
                                % read before the occurence of the trigger signal
                                % output 2: read address of the RAM for all the blocks
                                % output 3: read address of the ROM for all the blocks
                                % output 4: enable of the multiplier for all the blocks
                                % output 5: enable of the first adder for all the blocks
                                % output 6: clear of the first adder for all the blocks
                                % output 7: enable of the second adder for all the blocks
                                % output 8: clear of the second adder for all the blocks

discr=L+21;              % number of discrete states
        countersreadRAM = [1:L+1];      % array of indices
                                            % each index corresponds to a counter
                                            % each counter coounts the addresses of the raw 
                                            % corresponding to the index.
        restart         = L+2;          % restart synchronisation after the detection of a new raw
        countersample   = L+3;          % counter of samples to read (from 0 to B-1)
        counterraw      = L+4;          % counter of raw ( from 0 to L)
        errorsignal     = L+5;          % signal indicating if it is possible to read the matrix 
        rawtoread       = L+6;          % counter corresponding of the index of the raw to read in
                                        % countersreadRAM.
                                            % for instance, if there are 4 raws in the matrix, and if
                                            % we are writting in the second raw, rawtoread will be:
                                            % 3,4,1,3,4,1,3,4,1.....
                                            % if we are writting in the first raw: 2,3,4,2,3,4,2,...
        readRAM         = L+7;          % read address of the RAM
        counterROM1     = L+8;          % counter used to generate the read address of the ROM
        counterROM2     = L+9;          % counter used to generate the read address of the ROM
        counterROM3     = L+10;         % combination of counterROM1 and counterROM2 used to generate
                                        % the read address of the ROM
        readROM         = L+11;         % read address of the ROM
        multenable      = L+12;         % multiplier enable signal
        synchromult     = L+13;         % delay of one sample for the multenable signal
                                            % this delay corresponds to the read time of the RAM and
                                            % the ROM
        adder1enable    = L+14;         % adder1 enable signal
        adder1clear     = L+15;         % adder1 clear signal
        adder2enable    = L+16;         % adder2 enable signal
        adder2clear     = L+17;         % adder2 clear signal
        synchroena1     = L+18;         % delay of one sample for the enable signal of adder1
        synchrocle1     = L+19;         % delay of one sample for the clear signal of adder1
        synchroena2     = L+20;         % delay of one sample for the enable signal of adder2
        synchrocle2     = L+21;         % delay of one sample for the clear signal of adder2
                                            % this delay of one sample also corresponds to the read 
                                            % time of the RAM and ROM

switch flag,            % flag indicating the state of the S-function
    
  case 0,               % Initialization 
    [sys,x0,str,ts]=mdlInitializeSizes(F_MAC,in,out,discr,N,L,A,B,clockMAC,newraw,raw,countersreadRAM,...
                                        restart,countersample,counterraw,errorsignal,rawtoread,readRAM,...
                                        counterROM1,counterROM2,counterROM3,readROM,multenable,...
                                        synchromult,adder1enable,adder1clear,adder2enable,adder2clear,...
                                        synchroena1,synchrocle1,synchroena2,synchrocle2);
    
  case 2,               % Update 
    sys=mdlUpdate(t,x,u,F_MAC,N,D,L,A,B,C,E,clockMAC,newraw,raw,countersreadRAM,restart,countersample,...
                  counterraw,errorsignal,rawtoread,readRAM,counterROM1,counterROM2,counterROM3,readROM,...
                  multenable,synchromult,adder1enable,adder1clear,adder2enable,adder2clear,synchroena1,...
                  synchrocle1,synchroena2,synchrocle2);
    
  case 3,               % Outputs 
    sys=mdlOutputs(t,x,u,L,clockMAC,newraw,raw,countersreadRAM,restart,countersample,counterraw,...
                    errorsignal,rawtoread,readRAM,counterROM1,counterROM2,counterROM3,readROM,...
                    multenable,synchromult,adder1enable,adder1clear,adder2enable,adder2clear,...
                    synchroena1,synchrocle1,synchroena2,synchrocle2);
    
  case {1,4,9}          % Unused flags:
    sys=[];                 % 1: mdlDerivatives(t,x,u);
                            % 4: mdlGetTimeOfNextVarHit(t,x,u);
                            % 9: mdlGetTimeOfNextVarHit(t,x,u);
                            
  otherwise             % Unexpected flags
    error(['Unhandled flag = ',num2str(flag)]);
end

% end READSYNCHRO

%====================================================================================================
% mdlInitializeSizes
% Return the sizes, initial conditions, and sample times for the S-function.
%====================================================================================================

function [sys,x0,str,ts]=mdlInitializeSizes(F_MAC,in,out,discr,N,L,A,B,clockMAC,newraw,raw,...
                                            countersreadRAM,restart,countersample,counterraw,...
                                            errorsignal,rawtoread,readRAM,counterROM1,counterROM2,...
                                            counterROM3,readROM,multenable,synchromult,adder1enable,...
                                            adder1clear,adder2enable,adder2clear,synchroena1,...
                                            synchrocle1,synchroena2,synchrocle2)

sizes = simsizes;           % call simsizes for a sizes structure, fill it in and convert it to a
                            % sizes array.

sizes.NumContStates  = 0;               % number of continuous states
sizes.NumDiscStates  = discr;           % number of discrete states
sizes.NumOutputs     = out;             % number of outputs
sizes.NumInputs      = in;              % number of inputs
sizes.DirFeedthrough = 1;               % the outputs depends on the inputs
sizes.NumSampleTimes = 1;               % at least one sample time is needed

sys = simsizes(sizes);                  % take the previous parameters into account
                                        % to create the characteristics of the S-function

% here we initialize all the discrete states
x0(countersreadRAM)=0;      % array of indices
x0(restart)=0;              % restart synchronisation
x0(countersample)=N-1;      % counter of samples to read (from 0 to B-1)
x0(counterraw)=0;           % counter of raw ( from 0 to L)
x0(errorsignal)=0;          % signal indicating if it is possible to read the matrix 
x0(rawtoread)=2;            % counter corresponding of the index of the raw to read in 
                            % countersreadRAM.  
x0(readRAM)=0;              % read address of the RAM
x0(counterROM1)=0;          % counter used to generate the read address of the ROM
x0(counterROM2)=0;          % counter used to generate the read address of the ROM
x0(counterROM3)=0;          % combination of counterROM1 and counterROM2 used to generate
                            % the read address of the ROM
x0(readROM)=0;              % read address of the ROM
x0(multenable)=0;           % multiplier enable signal
x0(synchromult)=0;          % delay of one sample for the multenable signal
x0(adder1enable)=0;         % adder1 enable signal
x0(adder1clear)=0;          % adder1 clear signal
x0(adder2enable)=0;         % adder2 enable signal
x0(adder2clear)=0;          % adder2 clear signal
x0(synchroena1)=0;          % delay of one sample for the adder1enable signal
x0(synchrocle1)=0;          % delay of one sample for the adder1clear signal
x0(synchroena2)=0;          % delay of one sample for the adder2enable signal
x0(synchrocle2)=0;          % delay of one sample for the adder2clear signal

str = [];                   % str is always an empty matrix
ts  = [ F_MAC 0 ];          % initialize the array of sample times

% end mdlInitializeSizes

%====================================================================================================
% mdlUpdate
% Handle discrete state updates, sample time hits, and major time step
% requirements.
%====================================================================================================

function sys=mdlUpdate(t,x,u,F_MAC,N,D,L,A,B,C,E,clockMAC,newraw,raw,countersreadRAM,restart,...
                        countersample,counterraw,errorsignal,rawtoread,readRAM,counterROM1,...
                        counterROM2,counterROM3,readROM,multenable,synchromult,adder1enable,...
                        adder1clear,adder2enable,adder2clear,synchroena1,synchrocle1,synchroena2,...
                        synchrocle2)
                    
% if the clock is high                    
if u(clockMAC)==1              
         
    %================================================================================================
    % this section processes the occurence of a 'new raw': we start to fill in a new raw
    
    % if the trigger indicates that we started to fill in a new raw of the matrix
    if u(newraw)==1 
        % if the samples counter is different from 0
        if x(countersample)~0
            % if the synchronisation has not been done, do it and initilizes all signals and counters
            if x(restart)==0
                sys(countersample)=0;
                sys(restart)=1;                 % synchronisation
                sys(counterraw)=0;
                sys(countersreadRAM)=A*(0:L);
                sys(counterROM1)=0;
                sys(counterROM2)=0;
                sys(counterROM3)=0;
                sys(multenable)=1;
                sys(adder1enable)=1;
                sys(adder1clear)=0;
                sys(adder2enable)=0;
                sys(adder2clear)=0;
                % if the raw where the decimation is written is the last one, the index rawtoread 
                % is 1 (we start reading the first raw), else we start reading the raw that is just 
                % after the one where we are wrtting.
                if u(raw)==L
                    sys(rawtoread)=1;
                else
                    sys(rawtoread)=u(raw)+2;
                end;
                % here we check that we read all the samples out of the RAM and all the 
                % coefficients out of the ROM, and then we affect the error signal
                if x(countersample)<B-1
                    sys(errorsignal)=1;
                else
                    sys(errorsignal)=x(errorsignal);
                end;
            else
                % else if the synchronisation has already been done    
                sys(countersample)=x(countersample)+1;      % increment the samples counters
                sys(restart)=x(restart);                    % restart remains the same
                sys(errorsignal)=x(errorsignal);            % the error signal remains the same
                % if the samples counter still corresponds to a sample which is in the matrix
                % then we can enable the multiplier
                if x(countersample)<B-1
                    sys(multenable)=1;
                    
                    %==============================================================================
                    % here we affect the rawtoread discrete state
                    
                    % if the raw where the current decimation is written is the first one
                    % then the index rawtoread counts from the second raw to the last raw:
                    % we avoid the first raw
                    if u(raw)==0;
                        if x(rawtoread)==L+1
                            sys(rawtoread)=2;
                        else
                            sys(rawtoread)=x(rawtoread)+1;
                        end;
                    else
                        % else if the raw where the decimation is written is the last one
                        % then the index rawtoread counts from the first raw to the one before
                        % the last raw: we avoid the last raw
                        if u(raw)==L
                            if x(rawtoread)==L
                                sys(rawtoread)=1;
                            else
                                sys(rawtoread)=x(rawtoread)+1;
                            end;
                        else
                            % else if the raw where the current decimation is written is neither
                            % the first nor the last, then:
                            
                            % if the last value of rawtoread corresponds to this raw,
                            % then the value of the index is incremented of 2 in order to avoid
                            % the raw where we are writting the decimation
                            if x(rawtoread)==u(raw)
                                if x(rawtoread)==L
                                    sys(rawtoread)=1;
                                else
                                    sys(rawtoread)=x(rawtoread)+2;
                                end;
                            else
                                % if the last value of rawtoread is different from this raw
                                % then the index is incremented of one: we can read next raw
                                if x(rawtoread)==L+1
                                    sys(rawtoread)=1;
                                else
                                    sys(rawtoread)=x(rawtoread)+1;
                                end;
                            end;
                        end;
                    end;
                    %==============================================================================
                    
                    %==============================================================================
                    % here we affect the counters used to generate the read address of the ROM
                    % and we affect the control signals of the multiplier and adders
                    
                    % if we read a full line of the matrix (all the raws)
                    % re-initialize all the counters
                    if x(counterraw)==L-1
                        sys(counterraw)=0;
                        sys(countersreadRAM)=x(countersreadRAM)+1;
                        sys(counterROM1)=x(counterROM1)+1;
                        sys(counterROM2)=0;
                        sys(counterROM3)=x(counterROM1)+1;
                    else
                        % else if we didn' t finish to read the line, we keep reading this line 
                        sys(counterraw)=x(counterraw)+1;
                        sys(countersreadRAM)=x(countersreadRAM);
                        % if we are in the first half a a line (first half of a filter)
                        % then we increment the counters used for the read address of the ROM
                        % and we don' t change the control signals of the MAC
                        if x(counterraw)<E-1
                            sys(counterROM1)=x(counterROM1);
                            sys(counterROM2)=x(counterROM2)+A;
                            sys(counterROM3)=x(counterROM1)+x(counterROM2)+A;
                            sys(adder1enable)=x(adder1enable);
                            sys(adder1clear)=x(adder1clear);
                            sys(adder2enable)=x(adder2enable);
                            sys(adder2clear)=x(adder2clear);
                        else
                            % else if we are in the second part of a line (or filter)
                            % the control signals of the MAC remain the same
                            sys(adder1enable)=x(adder1enable);
                            sys(adder1clear)=x(adder1clear);
                            sys(adder2enable)=x(adder2enable);
                            sys(adder2clear)=x(adder2clear);
                            % if we are exactly in the middle then we re-initialize the
                            % counters used to generate the read address of the ROM, so
                            % that we can take the symmetry of the filter into account
                            if x(counterraw)==E-1
                                sys(counterROM1)=x(counterROM1);
                                sys(counterROM2)=C-1;
                                sys(counterROM3)=C-1-x(counterROM1);
                            else
                                % if we already passed the middle of the line (or filter)
                                % we increment the counters for the read address of the ROM
                                sys(counterROM1)=x(counterROM1);
                                sys(counterROM2)=x(counterROM2)-A;
                                sys(counterROM3)=x(counterROM2)-A-x(counterROM1);
                            end;
                        end;
                    end;
                    %==============================================================================
                else
                    % else if the samples counter refers to a sample which is not in the matrix
                    % then we disable the multiplier and we use default values
                    sys(counterraw)=0;
                    sys(countersreadRAM)=x(countersreadRAM);
                    sys(rawtoread)=x(rawtoread);
                    sys(counterROM1)=0;
                    sys(counterROM2)=0;
                    sys(counterROM3)=0;
                    sys(multenable)=0;
                end;
            end;
        else
            % else if the samples counter already restarted counting from 0, then increment this 
            % counter, don't restart (even if the trigger is high), and keep the last value of
            % the error signal
            sys(countersample)=x(countersample)+1;
            sys(restart)=x(restart);
            sys(errorsignal)=x(errorsignal);
            % if the samples counter still corresponds to a sample which is in the matrix
            % then we can enable the multiplier
            if x(countersample)<B-1
                sys(multenable)=1;
                
                %==================================================================================
                % here we affect the rawtoread discrete state
                    
                % if the raw where the current decimation is written is the first one
                % then the index rawtoread counts from the second raw to the last raw:
                % we avoid the first raw
                if u(raw)==0;
                    if x(rawtoread)==L+1
                        sys(rawtoread)=2;
                    else
                        sys(rawtoread)=x(rawtoread)+1;
                    end;
                else
                    % else if the raw where the decimation is written is the last one
                    % then the index rawtoread counts from the first raw to the one before
                    % the last raw: we avoid the last raw
                    if u(raw)==L
                        if x(rawtoread)==L
                            sys(rawtoread)=1;
                        else
                            sys(rawtoread)=x(rawtoread)+1;
                        end;
                    else
                        % else if the raw where the current decimation is written is neither
                        % the first nor the last, then:
                        
                        % if the last value of rawtoread corresponds to this raw,
                        % then the value of the index is incremented of 2 in order to avoid
                        % the raw where we are writting the decimation
                        if x(rawtoread)==u(raw)
                            if x(rawtoread)==L
                                sys(rawtoread)=1;
                            else
                                sys(rawtoread)=x(rawtoread)+2;
                            end;
                        else
                            % if the last value of rawtoread is different from this raw
                            % then the index is incremented of one: we can read next raw
                            if x(rawtoread)==L+1
                                sys(rawtoread)=1;
                            else
                                sys(rawtoread)=x(rawtoread)+1;
                            end;
                        end;
                    end;
                end;
                %==================================================================================
                
                %==================================================================================
                % here we affect the counters used to generate the read address of the ROM
                % and we affect the control signals of the multiplier and adders
                
                % if we read a full line of the matrix (all the raws)
                % re-initialize all the counters
                if x(counterraw)==L-1
                    sys(counterraw)=0;
                    sys(countersreadRAM)=x(countersreadRAM)+1;
                    sys(counterROM1)=x(counterROM1)+1;
                    sys(counterROM2)=0;
                    sys(counterROM3)=x(counterROM1)+1;
                else
                    % else if we didn' t finish to read the line, we keep reading this line 
                    sys(counterraw)=x(counterraw)+1;
                    sys(countersreadRAM)=x(countersreadRAM);
                    % if we are in the first half a a line (first half of a filter)
                    % then we increment the counters used for the read address of the ROM
                    % and we don' t change the control signals of the MAC
                    if x(counterraw)<E-1
                        sys(counterROM1)=x(counterROM1);
                        sys(counterROM2)=x(counterROM2)+A;
                        sys(counterROM3)=x(counterROM1)+x(counterROM2)+A;
                        sys(adder1enable)=x(adder1enable);
                        sys(adder1clear)=x(adder1clear);
                        sys(adder2enable)=x(adder2enable);
                        sys(adder2clear)=x(adder2clear);
                    else
                        % if we are exactly in the middle then we re-initialize the
                        % counters used to generate the read address of the ROM, so
                        % that we can take the symmetry of the filter into account
                        if x(counterraw)==E-1
                            sys(counterROM1)=x(counterROM1);
                            sys(counterROM2)=C-1;
                            sys(counterROM3)=C-1-x(counterROM1);
                        else
                            % if we already passed the middle of the line (or filter)
                            % we decrement the counters used to generate the read address
                            % of the ROM to get the symmetry of the filter
                            sys(counterROM1)=x(counterROM1);
                            sys(counterROM2)=x(counterROM2)-A;
                            sys(counterROM3)=x(counterROM2)-A-x(counterROM1);
                        end;
                    end;
                end;
                %==================================================================================
            else
                % else if the samples counter refers to a sample which is not in the matrix
                % then we disable the multiplier and we use default values
                sys(counterraw)=0;
                sys(countersreadRAM)=x(countersreadRAM);
                sys(rawtoread)=x(rawtoread);
                sys(counterROM1)=0;
                sys(counterROM2)=0;
                sys(counterROM3)=0;
                sys(multenable)=0;
            end;
        end;
    %================================================================================================
    
    %================================================================================================
    else
        % else if the trigger is low, then there is no need to restart, so we can increment the
        % samples counter, and hold the same value for the error signal
        sys(countersample)=x(countersample)+1;
        sys(restart)=0;
        sys(errorsignal)=x(errorsignal);
        % if the samples counter still corresponds to a sample which is in the matrix
        % then we can enable the multiplier
        if x(countersample)<B-1
            sys(multenable)=1;
                
            %========================================================================================
            % here we affect the rawtoread discrete state
            
            % if the raw where the current decimation is written is the first one
            % then the index rawtoread counts from the second raw to the last raw:
            % we avoid the first raw
            if u(raw)==0;
                if x(rawtoread)==L+1
                    sys(rawtoread)=2;
                else
                    sys(rawtoread)=x(rawtoread)+1;
                end;
            else
                % else if the raw where the decimation is written is the last one
                % then the index rawtoread counts from the first raw to the one before
                % the last raw: we avoid the last raw
                if u(raw)==L
                    if x(rawtoread)==L
                        sys(rawtoread)=1;
                    else
                        sys(rawtoread)=x(rawtoread)+1;
                    end;
                else
                    % else if the raw where the current decimation is written is neither
                    % the first nor the last, then:
                    
                    % if the last value of rawtoread corresponds to this raw,
                    % then the value of the index is incremented of 2 in order to avoid
                    % the raw where we are writting the decimation
                    if x(rawtoread)==u(raw)
                        if x(rawtoread)==L
                            sys(rawtoread)=1;
                        else
                            sys(rawtoread)=x(rawtoread)+2;
                        end;
                    else
                        % if the last value of rawtoread is different from this raw
                        % then the index is incremented of one: we can read next raw
                        if x(rawtoread)==L+1
                            sys(rawtoread)=1;
                        else
                            sys(rawtoread)=x(rawtoread)+1;
                        end;
                    end;
                end;
            end;
            %========================================================================================
            
            %========================================================================================
            % here we affect the counters used to generate the read address of the ROM
            % and we affect the control signals of the multiplier and adders
            
            % if we read a full line of the matrix (all the raws)
            % re-initialize all the counters
            if x(counterraw)==L-1
               sys(counterraw)=0;
               sys(countersreadRAM)=x(countersreadRAM)+1;
               sys(counterROM1)=x(counterROM1)+1;
               sys(counterROM2)=0;
               sys(counterROM3)=x(counterROM1)+1;
               % if the first adder was disabled, then enable it and disable the second adder
               if x(adder1enable)==0 
                    sys(adder1enable)=1;
                    sys(adder2enable)=0;
                    % if the first adder was cleared, unclear it, else keep the same state
                    if x(adder1clear)==1
                        sys(adder1clear)=0;
                    else
                        sys(adder1clear)=x(adder1clear);
                    end;
                    % same thing for the clear of the second adder
                    if x(adder2clear)==1
                        sys(adder2clear)=0;
                    else
                        sys(adder2clear)=x(adder2clear);
                    end;
                else
                    % else if the first adder was enabled, disable it, enable the second adder
                    % and force all clear signals to zero
                    sys(adder1enable)=0;
                    sys(adder1clear)=0;
                    sys(adder2enable)=1;
                    sys(adder2clear)=0;
                end;
            else
                % else if we didn' t finish to read the line, we keep reading this line 
                sys(counterraw)=x(counterraw)+1;
                sys(countersreadRAM)=x(countersreadRAM);
                % if we are in the first half a a line (first half of a filter)
                % then we increment the counters used for the read address of the ROM
                % and we don' t change the control signals of the MAC
                if x(counterraw)<E-1
                    sys(counterROM1)=x(counterROM1);
                    sys(counterROM2)=x(counterROM2)+A;
                    sys(counterROM3)=x(counterROM1)+x(counterROM2)+A;
                    sys(adder1enable)=x(adder1enable);
                    sys(adder1clear)=x(adder1clear);
                    sys(adder2enable)=x(adder2enable);
                    sys(adder2clear)=x(adder2clear);
                else
                    % if we are in the second half of the line (or filter), then the enable signals
                    % remain the same, and we will only afect the clear signals
                    sys(adder1enable)=x(adder1enable);
                    sys(adder2enable)=x(adder2enable);
                    % if the enable signal of the first adder was high, we don't clear this adder,
                    % else we clear it during the very last sample vefore we enable it again, so
                    % that we hold the output of the adder while the second adder is computing
                    if x(adder1enable)==1
                        sys(adder1clear)=0;
                    else
                        if x(counterraw)==L-2
                            sys(adder1clear)=1;
                        else
                            sys(adder1clear)=0;
                        end;
                    end;
                    % same thing with the second adder
                    if x(adder2enable)==1
                        sys(adder2clear)=0;
                    else
                        if x(counterraw)==L-2
                            sys(adder2clear)=1;
                        else
                            sys(adder2clear)=0;
                        end;
                    end;
                    % if we are exactly in the middle then we re-initialize the
                    % counters used to generate the read address of the ROM, so
                    % that we can take the symmetry of the filter into account
                    if x(counterraw)==E-1
                        sys(counterROM1)=x(counterROM1);
                        sys(counterROM2)=C-1;
                        sys(counterROM3)=C-1-x(counterROM1);
                    else
                        % if we already passed the middle of the line (or filter)
                        % we decrement the counters used to generate the read address
                        % of the ROM to get the symmetry of the filter
                        sys(counterROM1)=x(counterROM1);
                        sys(counterROM2)=x(counterROM2)-A;
                        sys(counterROM3)=x(counterROM2)-A-x(counterROM1);
                    end;
                end;
            end;
            %========================================================================================
        else
            % else if the samples counter refers to a sample which is not in the matrix
            % then we disable the multiplier and we use default values
            sys(counterraw)=0;
            sys(countersreadRAM)=x(countersreadRAM);
            sys(rawtoread)=x(rawtoread);
            sys(counterROM1)=0;
            sys(counterROM2)=0;
            sys(counterROM3)=0;
            sys(multenable)=0;
            sys(adder1enable)=0;
            sys(adder1clear)=0;
            sys(adder2enable)=0;
            sys(adder2clear)=0;
        end;
    end;
    %================================================================================================
    
    % here we prepare the outputs:
    
    % the read address of the RAM is the value of counter that correponds to the rawtoread index:
    sys(readRAM)=x(x(rawtoread));
    % the read address of the ROM is the counterROM3 signal delayed of one sample.
    sys(readROM)=x(counterROM3);
    % the multenable, adder1enable,adder2enable, adder1clear and adder2clear signals are delayed of 
    % one sample because of the read time of the memories, which is one sample
    sys(synchromult)=x(multenable);
    sys(synchroena1)=x(adder1enable);
    sys(synchrocle1)=x(adder1clear);
    sys(synchroena2)=x(adder2enable);
    sys(synchrocle2)=x(adder2clear);
    
end; % end of the test concerning the clock
    
% end mdlUpdate

%====================================================================================================
% mdlOutputs
% Return the block outputs.
%====================================================================================================

function sys=mdlOutputs(t,x,u,L,clockMAC,newraw,raw,countersreadRAM,restart,countersample,...
                        counterraw,errorsignal,rawtoread,readRAM,counterROM1,counterROM2,...
                        counterROM3,readROM,multenable,synchromult,adder1enable,adder1clear,...
                        adder2enable,adder2clear,synchroena1,synchrocle1,synchroena2,synchrocle2)

sys(1)=x(errorsignal);  % output 1: error signal indicating that the matrix was not entirely
                        % read before the occurence of the trigger signal
sys(2)=x(readRAM);      % output 2: read address of the RAM for all the blocks
sys(3)=x(readROM);      % output 3: read address of the ROM for all the blocks
sys(4)=x(synchromult);  % output 4: enable of the multiplier for all the blocks
sys(5)=x(synchroena1);  % output 5: enable of the first adder for all the blocks
sys(6)=x(synchrocle1);  % output 6: clear of the first adder for all the blocks
sys(7)=x(synchroena2);  % output 7: enable of the second adder for all the blocks
sys(8)=x(synchrocle2);  % output 8: clear of the second adder for all the blocks
                                
% end mdlOutputs