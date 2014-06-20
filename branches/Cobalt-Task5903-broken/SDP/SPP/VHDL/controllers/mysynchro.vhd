--**************************************************************************************************--
--*****************          SYNCHRONISATION OF SAMPLES AND COEFFICIENTS,		********************--
--*****************			 		CONTROL OF THE PREFILTERS					********************--
--*****************  Date: 11/12/02 Author: J.L. 								********************--
--*****************  This file enables to read the decimated samples out of		********************--
--*****************  RAMs, to read the coefficients out of ROMs, to synchronize	********************--
--*****************  them and to control the Multipliers and Accumulators of	********************--
--*****************  the prefilters												********************--
--**************************************************************************************************--

--************************** libraries declaration *************************************************--
library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_arith.all;
use IEEE.std_logic_unsigned.all;
library work;
use work.pack.all;
--**************************************************************************************************--

--************************** entity ****************************************************************--
entity mysynchro is
	port(
			clockdecim: 	in std_logic;								-- decimation clock
			clocksync:		in std_logic;								-- subfilters clock
			clear:			in std_logic;								-- clear signal			
			newraw:			in std_logic;								-- indicates that we start to fill in a new raw
			rawindex:		in std_logic_vector(BIT_RAW-1 downto 0);	-- indicates the index of this new raw						
			errorsig:		out std_logic;								-- indicates if we start to fill in a new raw whearas is didn't read the full matrix
			readRAM:		out std_logic_vector(BIT_RAM_DECIM-1 downto 0);	-- read address of the RAMs for all the subfilters
			readROM:		out std_logic_vector(BIT_ROM-1 downto 0);	-- read address of the ROMs for all the subfilters
			multenable:		out std_logic;								-- multiplier enable for all the subfilters
			adder1enable:	out std_logic;								-- first adder enable, for all the subfilters
			adder1clear:	out std_logic;								-- first adder clear, for all the subfilters
			adder1latch:	out std_logic;								-- first adder latch, for all the subfilters
			adder2enable:	out std_logic;								-- second adder enable, for all the subfilters
			adder2clear:	out std_logic;								-- second adder clear, for all the subfilters
			adder2latch:	out std_logic;								-- second adder latch, for all the subfilters 
			synchrofft:		out std_logic								-- synchronisation signal for the fft controler					
		);		
begin
end mysynchro;
--*************************************************************************************************--

--************************* architecture **********************************************************--
architecture archisynchro of mysynchro is	
	--********************* signals ************************************--	
	signal restart:				std_logic;								-- synchronisation between the 'mydecim' and 'mysynchro' blocs
	signal countersample: 		std_logic_vector(15 downto 0);			-- counter of the number of samples to read out of the matrix
	signal counterraw:			std_logic_vector(BIT_RAW-1 downto 0);	-- raw counter
	signal rawtoread:     		std_logic_vector(BIT_RAW-1 downto 0);	-- indicates the raw that has to be read out
	signal countersreadRAM: 	internalcountersreadRAM;				-- internal counters required to generate the read address of the RAM
	signal counter1readROM:		std_logic_vector(15 downto 0);			-- first counter used to generate the read address of the ROM
	signal counter2readROM:		std_logic_vector(15 downto 0);  		-- second counter used to generate the read address of the ROM
	signal delay1_multenable:   std_logic;								-- first clock cycle delay for the enable of the multiplier
	signal delay2_multenable:   std_logic;								-- second clock cycle delay for the enable of the multiplier (2samples= read time of the memories)
	signal delay1_add1enable:   std_logic;								-- first clock cycle delay for the enable of the first adder
	signal delay2_add1enable:	std_logic;								-- second clock cycle delay for the enable of the first adder
	signal delay3_add1enable:	std_logic;								-- third clock cycle delay for the enable of the first adder (3 samples= 1 more sample than the multiplier because of the pipeline)
	signal delay1_add1clear:	std_logic;								-- first clock cycle delay for the clear of the first adder
	signal delay2_add1clear:	std_logic;								-- second clock cycle delay for the clear of the first adder
	signal delay3_add1clear:	std_logic;								-- third clock cycle delay for the clear of the first adder (3 samples= 1 more sample than the multiplier because of the pipeline)
	signal delay1_add2enable:   std_logic;								-- first clock cycle delay for the enable of the second adder
	signal delay2_add2enable:	std_logic;								-- second clock cycle delay for the enable of the second adder
	signal delay3_add2enable:	std_logic;								-- third clock cycle delay for the enable of the second adder (3 samples= 1 more sample than the multiplier because of the pipeline)
	signal delay1_add2clear:	std_logic;					 			-- first clock cycle delay for the clear of the second adder
	signal delay2_add2clear:	std_logic;								-- second clock cycle delay for the clear of the second adder
	signal delay3_add2clear:	std_logic;								-- third clock cycle delay for the clear of the second adder (3 samples= 1 more sample than the multiplier because of the pipeline)
	signal delay_add1latch:		std_logic;								-- one clock cycle delay for the latch enable after the first adder
	signal delay_add2latch:		std_logic;								-- one clock cycle delay for the latch enable after the second adder		
	signal delay1_syncfft:		std_logic;								-- first clock cycle delay for the synchronisation of the fft controler
	signal delay2_syncfft:		std_logic;								-- second clock cycle delay for the synchronisation of the fft controler
	signal delay3_syncfft:		std_logic; 								-- third clock cycle delay for the synchronisation of the fft controler
	--******************************************************************--			
begin
	process(clear,clocksync)
		--****************** variables *********************************--
		variable init:					std_logic;						-- first sample is used to initialise some signals and variables
		variable temp_error:			std_logic;						-- intermediate variable for the error signal
		variable temp_rawtoread:		std_logic_vector(BIT_RAW-1 downto 0);-- intermediate variable for the rawtoread signal
		variable temp_add1enable:		std_logic;						-- intermediate variable for the enable of the first adder
		variable temp_add1clear:		std_logic;						-- intermediate variable for the clear of the first adder
		variable temp_add2enable:		std_logic;						-- intermediate variable for the enable of the second adder
		variable temp_add2clear:		std_logic;						-- intermediate variable for the clear of the second adder
		variable counter3readROM:		std_logic_vector(15 downto 0);	-- intermediate variable for the readROM signal
		variable temp_syncfft:			std_logic;						-- intermediate variable to synchronize the FFT cores
		--**************************************************************--
		begin
			--*********************** clear: re init signals and variables *****************************--
			if clear='1' then											
				init:='0';								
				restart<='0';							 
				temp_syncfft:='0';
				synchrofft<='0';			
				rawtoread<=(others=>'0');
				temp_error:='0';			
				temp_rawtoread:=(others=>'0');		
				countersample<=(others=>'0');
				counterraw<=(others=>'0');
				temp_add1enable:='0';
				temp_add1clear:='0';
				temp_add2enable:='0';
				temp_add2clear:='0';
				counter1readROM<=(others=>'0');
				counter2readROM<=(others=>'0');
				counter3readROM:=(others=>'0');		
				errorsig<='0';
				readRAM<=(others=>'0');
				readROM<=(others=>'0');
				delay1_multenable<='0';
				delay2_multenable<='0';
				multenable<='0';
				adder1enable<=temp_add1enable;
				adder1clear<=temp_add1clear;
				adder2enable<=temp_add2enable;
				adder2clear<=temp_add2clear;		
			--******************************************************************************************--			
			
			--*********************** begin of synchronisation *****************************************--
			elsif (clocksync'event and clocksync='1')  then							-- if there is a rising edge of the clock
				--******** initialization ******************************************--
				if init='0' then													-- if the initialisation didn't take place
					init:='1';														-- then do it:
					countersample<=conv_std_logic_vector(conv_unsigned(SAMPLES_READ_BLOCK,16),16);	-- force countersample to a high value to avoid the error signal to be high before starting synchronization
					rawtoread<=conv_std_logic_vector(conv_unsigned(PREFILTER_L,BIT_RAW),BIT_RAW);	-- force rawtoread at its maximum value so that it will automatically restart counting from 0		
				--******************************************************************--
				else																-- else if the initialisation already took place
					--******************** new raw *********************************--
					if newraw='1' then 												-- if we are starting to fill in a new raw of the matrix	
						restart<='1';												-- indicate that we synchronised the two controlers (mydecim and mysynchro)
						--**************** restart synchro *************************--
						if restart='0' then											-- if the synchronisation has not been done, we do it
							countersample<=(others=>'0');							-- restart counting the number of samples to read out of the matrix
							delay1_multenable<='1';									-- enable multiplication
							counterraw<=(others=>'0');								-- restart counting the raws
							temp_add1enable:='1';									-- enable first adder
							temp_add1clear:='0';									-- don't clear first adder
							temp_add2enable:='0';									-- disable second adder
							temp_add2clear:='0';									-- clear second adder
							counter1readROM<=(others=>'0');							-- restart counter 
							counter2readROM<=(others=>'0');							-- restart counter
							counter3readROM:=(others=>'0');							-- restart counter
							temp_syncfft:='1';										-- synchronisation with the fft blocs controler
							--***** raw to read *****--	
							if conv_integer(rawindex)=PREFILTER_L-1 then												-- if the last raw where we wrote is the last but two raw
								rawtoread<=conv_std_logic_vector(conv_unsigned(PREFILTER_L+1,BIT_RAW),BIT_RAW);					-- then we will start to read from the last raw
							else																			-- else 
								if conv_integer(rawindex)=PREFILTER_L then											-- if the last raw where we wrote is the last but one raw
									rawtoread<=conv_std_logic_vector(conv_unsigned(1,BIT_RAW),BIT_RAW);					-- then we will start to read from the first raw
								else																		-- else
									temp_rawtoread:=rawindex+2;												-- we will start to read from the second raw after the raw where we were writting
									rawtoread<=temp_rawtoread;
								end if;
							end if;
							--**** error signal ****--
							if conv_integer(countersample)<SAMPLES_READ_BLOCK-1 then											-- if we didn't read all the samples of the matrix
								temp_error:='1';															-- this is an error
							else																			-- else
								temp_error:=temp_error;														-- the error signal wil remain the same
							end if;													
							--** counter read RAM **--						
							for i in 1 to PREFILTER_L+1 loop
								countersreadRAM(i)<=conv_std_logic_vector(conv_unsigned(SAMPLES_RAW*(i-1),16),16);	-- counters re-initialisation
							end loop;
						--************************************************************--	
						else
						--*************	synchronisation already done *****************--																	
							countersample<=countersample+1;													-- increment the samples counter
							temp_error:=temp_error;															-- hold the error signal
							temp_syncfft:='0';																-- the synchronisation signal for the fft blocks controler is finished
							--****** control of the prefilters: all the samples have not been filtered *****--
							if conv_integer(countersample)<SAMPLES_READ_BLOCK-1 then											-- if the sample counter still refers to a sample which we have to read
								delay1_multenable<='1';														-- then we need to use the multiplier
								--******* raw to read *********--
								if conv_integer(rawindex)=0 then											-- if we are currently writting in the first raw of the matrix
									if conv_integer(rawtoread)=PREFILTER_L+1 then										-- if we were reading the last raw
										rawtoread<=conv_std_logic_vector(conv_unsigned(2,BIT_RAW),BIT_RAW);				-- we can skip the first raw and restart to read from the second raw
									else																	-- if we were not reading the last raw
										rawtoread<=rawtoread+1;												-- we can read next raw
									end if;			
								else																		-- if we are not writting in the first raw
									if conv_integer(rawindex)=PREFILTER_L then										-- if we are currently writting in the last raw
										if conv_integer(rawtoread)=PREFILTER_L then									-- if we were reading the last but one raw of the matrix
											rawtoread<=conv_std_logic_vector(conv_unsigned(1,BIT_RAW),BIT_RAW);			-- skip last raw and restart to read from the first raw
										else																-- if we didn't read the last but one raw
											rawtoread<=rawtoread+1;											-- read next raw
										end if;		
									else																	-- if we are writting neither in the first nor in the last raw
										if rawtoread=rawindex then											-- if the last raw we read was the one before the raw where we are writting
											if conv_integer(rawtoread)=PREFILTER_L then								-- if this raw is the last but one
												rawtoread<=conv_std_logic_vector(conv_unsigned(1,BIT_RAW),BIT_RAW);		-- restart to read from the first raw
											else															-- if the raw we were reading is not the last but one raw
												rawtoread<=rawtoread+2;										-- skip reading the raw where we are writting
											end if;
										else																-- if we were not reading the last but one raw
											if conv_integer(rawtoread)=PREFILTER_L+1 then								-- if we were reading the last raw
												rawtoread<=conv_std_logic_vector(conv_unsigned(1,BIT_RAW),BIT_RAW);		-- restart reading the first raw
											else															-- if we were not reading the last raw
												rawtoread<=rawtoread+1;										-- read next raw
											end if;
										end if;		
									end if;			
								end if;				
								--*********** last raw **********--	
								if conv_integer(counterraw)=PREFILTER_L-1 then										-- if we read all the raws of the matrix
									counterraw<=(others=>'0');												-- restart counting the raws
									for i in 1 to PREFILTER_L+1 loop
										countersreadRAM(i)<=countersreadRAM(i)+1;							-- re-initialize the internal counters
									end loop;
									counter1readROM<=counter1readROM+1;										-- keep incrementing the first counter for the read address of the ROM
									counter2readROM<=(others=>'0');											-- restart the second counter for the read address of the ROM
									counter3readROM:=counter1readROM+1;										-- the third counter is the summ of the two others
									if temp_add1enable='0' then												-- if the first adder was disabled
										temp_add1enable:='1';												-- enable it
										temp_add2enable:='0';												-- disable the second adder
										if temp_add1clear='1' then											-- if the first adder was cleared
											temp_add1clear:='0';											-- use it
										end if;
										if temp_add2clear='1' then											-- same for the second adder
											temp_add2clear:='0';
										end if;
									else																	-- if the first adder was enabled
										temp_add1enable:='0';												-- disable it
										temp_add2enable:='1';												-- enable the first adder
										temp_add1clear:='0';												-- don't clear the first adder
										temp_add2clear:='0';												-- don't clear the second adder
									end if;
								else
								--******** not the last raw ******--																		-- if we didn'read all the raws of the matrix
									counterraw<=counterraw+1;												-- read next raw
									for i in 1 to PREFILTER_L+1 loop
										countersreadRAM(i)<=countersreadRAM(i);								-- the internal counters remain the same
									end loop;
									if conv_integer(counterraw)<HALF_PREFILTER-1 then									-- if we read less than half of the raws
									counter2readROM<=counter2readROM+SAMPLES_RAW;										-- increment the counters for the read address of the ROM
									counter3readROM:=counter1readROM+counter2readROM+SAMPLES_RAW;
									else																	-- if we already read half of the raws of the matrix
										if temp_add1enable='1' then											-- if the first adder was enabled
											temp_add1clear:='0';											-- use the first adder
										else																-- if the first adder was enabled
											if conv_integer(counterraw)=PREFILTER_L-2 then							-- if the raws counter was indicationg the last but one raw to read
												temp_add1clear:='1';										-- clear the first adder
											else															-- if we didn't read all the raws
												temp_add1clear:='0';										-- use the first adder
											end if;
										end if;
										if temp_add2enable='1' then											-- same for the second adder, which works in parallel with the first one 
											temp_add2clear:='0';
										else
											if conv_integer(counterraw)=PREFILTER_L-2 then
												temp_add2clear:='1';
											else
												temp_add2clear:='0';
											end if;
										end if;
										if conv_integer(counterraw)=HALF_PREFILTER-1 then								-- if we were reading the raw corresponding to the half of the number of raws we have to read
											counter2readROM<=conv_std_logic_vector(conv_unsigned(HALF_READ_BLOCK-1,16),16);-- reinitialize the counters for the read address of the ROM
											counter3readROM:=conv_std_logic_vector(conv_unsigned(HALF_READ_BLOCK-1,16),16)-counter1readROM;
										else																-- if we already read more than half of the raws to read
											counter2readROM<=counter2readROM-SAMPLES_RAW;								-- decrement the counters for the read addres of the ROM
											counter3readROM:=counter2readROM-SAMPLES_RAW-counter1readROM;
										end if;	
									end if;
								end if;
							else
							--****** all the samples have been processed******--																			-- if we read all the samples we had to read out of the matrix
								delay1_multenable<='0';														-- disable the multiplier
								counterraw<=(others=>'0');													-- restart counting raws
								rawtoread<=rawtoread;														-- hold the value of the raw we were reading
								for i in 1 to PREFILTER_L+1 loop
									countersreadRAM(i)<=countersreadRAM(i);									-- hold the value of the internal counters
								end loop;
								temp_add1enable:='0';														-- disable the adders
								temp_add2enable:='0';
								temp_add1clear:='0';														-- clear the adders
								temp_add2clear:='0';
							end if;					
						end if;									
						--**********************************************************************************--
					else																					-- if we are not reading a new raw
					--**************** still the same raw **************************************************--
						restart<='0';																		-- we don't have to restart
						countersample<=countersample+1;														-- increment the samples counter
						temp_error:=temp_error;																-- hold the error signal
						temp_syncfft:='0';																	-- there is no need to synchronise the fft controler
						--** there are still samples to be processed *****************************--
						if conv_integer(countersample)<SAMPLES_READ_BLOCK-1 then												-- if the sample counter still refers to a sample which we have to read
							delay1_multenable<='1';															-- then we need to use the multiplier
							--*** raw to read ***--
							if conv_integer(rawindex)=0 then												-- if we are currently writting in the first raw of the matrix
								if conv_integer(rawtoread)=PREFILTER_L+1 then											-- if we were reading the last raw
									rawtoread<=conv_std_logic_vector(conv_unsigned(2,BIT_RAW),BIT_RAW);					-- we can skip the first raw and restart to read from the second raw
								else																		-- if we were not reading the last raw
									rawtoread<=rawtoread+1;													-- we can read next raw
								end if;			
							else																			-- if we are not writting in the first raw
								if conv_integer(rawindex)=PREFILTER_L then											-- if we are currently writting in the last raw
									if conv_integer(rawtoread)=PREFILTER_L then										-- if we were reading the last but one raw of the matrix
										rawtoread<=conv_std_logic_vector(conv_unsigned(1,BIT_RAW),BIT_RAW);				-- skip last raw and restart to read from the first raw
									else																	-- if we didn't read the last but one raw
										rawtoread<=rawtoread+1;												-- read next raw
									end if;		
								else																		-- if we are writting neither in the first nor in the last raw
									if rawtoread=rawindex then												-- if the last raw we read was the one before the raw where we are writting
										if conv_integer(rawtoread)=PREFILTER_L then									-- if this raw is the last but one
											rawtoread<=conv_std_logic_vector(conv_unsigned(1,BIT_RAW),BIT_RAW);			-- restart to read from the first raw
										else																-- if the raw we were reading is not the last but one raw
											rawtoread<=rawtoread+2;											-- skip reading the raw where we are writting
										end if;
									else																	-- if we were not reading the last but one raw
										if conv_integer(rawtoread)=PREFILTER_L+1 then									-- if we were reading the last raw
											rawtoread<=conv_std_logic_vector(conv_unsigned(1,BIT_RAW),BIT_RAW);			-- restart reading the first raw
										else																-- if we were not reading the last raw
											rawtoread<=rawtoread+1;											-- read next raw
										end if;
									end if;		
								end if;			
							end if;				
							--*** last raw ***--
							if conv_integer(counterraw)=PREFILTER_L-1 then											-- if we read all the raws of the matrix
								counterraw<=(others=>'0');													-- restart counting the raws
								for i in 1 to PREFILTER_L+1 loop				
									countersreadRAM(i)<=countersreadRAM(i)+1;								-- re-initialize the internal counters
								end loop;
								counter1readROM<=counter1readROM+1;											-- keep incrementing the first counter for the read address of the ROM
								counter2readROM<=(others=>'0');												-- restart the second counter for the read address of the ROM
								counter3readROM:=counter1readROM+1;											-- the third counter is the summ of the two others
								if temp_add1enable='0' then													-- if the first adder was disabled
									temp_add1enable:='1';													-- enable it
									temp_add2enable:='0';													-- disable the second adder
									
									if temp_add1clear='1' then												-- if the first adder was cleared
										temp_add1clear:='0';												-- use it
									end if;
									if temp_add2clear='1' then												-- same for the second adder
										temp_add2clear:='0';
									end if;									
								else																		-- if the first adder was enabled
									temp_add1enable:='0';													-- disable it
									temp_add2enable:='1';													-- enable the first adder
									temp_add1clear:='0';													-- don't clear the first adder
									temp_add2clear:='0';													-- don't clear the second adder
								end if;
							else
							--**** not the last raw *****--																			-- if we didn'read all the raws of the matrix
								counterraw<=counterraw+1;													-- read next raw
								for i in 1 to PREFILTER_L+1 loop
									countersreadRAM(i)<=countersreadRAM(i);									-- the internal counters remain the same
								end loop;
								if conv_integer(counterraw)<HALF_PREFILTER-1 then										-- if we read less than half of the raws																												
									counter2readROM<=counter2readROM+SAMPLES_RAW;										-- increment the counters for the read address of the ROM
									counter3readROM:=counter1readROM+counter2readROM+SAMPLES_RAW;
								else																		-- if we already read half of the raws of the matrix
									if temp_add1enable='1' then												-- if the first adder was enabled
										temp_add1clear:='0';												-- use the first adder
									else																	-- if the first adder was enabled
										if conv_integer(counterraw)=PREFILTER_L-2 then								-- if the raws counter was indicationg the last but one raw to read
											temp_add1clear:='1';											-- clear the first adder
										else																-- if we didn't read all the raws
											temp_add1clear:='0';											-- use the first adder
										end if;
									end if;
									if temp_add2enable='1' then												-- same for the second adder, which works in parallel with the first one 
										temp_add2clear:='0';
									else
										if conv_integer(counterraw)=PREFILTER_L-2 then
											temp_add2clear:='1';
										else
											temp_add2clear:='0';
										end if;
									end if;
									if conv_integer(counterraw)=HALF_PREFILTER-1 then									-- if we were reading the raw corresponding to the half of the number of raws we have to read
										counter2readROM<=conv_std_logic_vector(conv_unsigned(HALF_READ_BLOCK-1,16),16);					-- reinitialize the counters for the read address of the ROM
										counter3readROM:=conv_std_logic_vector(conv_unsigned(HALF_READ_BLOCK-1,16),16)-counter1readROM;
									else																	-- if we already read more than half of the raws to read
										counter2readROM<=counter2readROM-SAMPLES_RAW;									-- decrement the counters for the read addres of the ROM
										counter3readROM:=counter2readROM-SAMPLES_RAW-counter1readROM;
									end if;
								end if;
							end if;
						else
						--******** all the samples have been processed ************--																				-- if we read all the samples we had to read out of the matrix
							delay1_multenable<='0';															-- disable the multiplier
							counterraw<=(others=>'0');														-- restart counting raws
							rawtoread<=rawtoread;															-- hold the value of the raw we were reading
							for i in 1 to PREFILTER_L+1 loop
								countersreadRAM(i)<=countersreadRAM(i);										-- hold the value of the internal counters
							end loop;
							temp_add1enable:='0';															-- disable the adders
							temp_add2enable:='0';															
							temp_add1clear:='0';															-- clear the adders
							temp_add2clear:='0';															
						end if;					
					end if;
					--**************************************************************************--	
				end if;
				--************* delays for the control signals *********************--
				errorsig<=temp_error;												-- error signal
				readRAM<=countersreadRAM(conv_integer(rawtoread))(BIT_RAM_DECIM-1 downto 0);-- read address of the RAM
				readROM<=counter3readROM(BIT_ROM-1 downto 0);								-- read address of the ROM
				delay2_multenable<=delay1_multenable;								-- one sample delay of the muliplier enable signal
				multenable<=delay2_multenable;										-- multiplier enable signal
				delay1_add1enable<=temp_add1enable;									-- one sample delay for the first adder enable signal
				delay2_add1enable<=delay1_add1enable;								-- second delay for the first adder enable signal
				delay3_add1enable<=delay2_add1enable;								-- third delay for the first adder enable signal
				adder1enable<=delay3_add1enable;									-- first adder enable signal
				delay1_add1clear<=temp_add1clear;									-- one sample delay for the first adder clear signal
				delay2_add1clear<=delay1_add1clear;									-- second delay for the first adder clear signal
				delay3_add1clear<=delay2_add1clear;									-- third delay for the first adder clear signal
				adder1clear<=delay3_add1clear;										-- first adder clear signal
				delay_add1latch<=delay3_add2clear;									-- one sample delay for the first adder latch signal
				adder1latch<=delay_add1latch;										-- first adder latch signal
				delay1_add2enable<=temp_add2enable;									-- one sample delay for the second adder enable signal
				delay2_add2enable<=delay1_add2enable;								-- second delay for the second adder enable signal
				delay3_add2enable<=delay2_add2enable;								-- third delay for the second adder enable signal
				adder2enable<=delay3_add2enable;									-- second adder enable signal
				delay1_add2clear<=temp_add2clear;									-- one sample delay for the second adder clear signal
				delay2_add2clear<=delay1_add2clear;									-- second delay for the second adder clear signal
				delay3_add2clear<=delay2_add2clear;									-- third delay for the second adder clear signal
				adder2clear<=delay3_add2clear;										-- second adder clear signal
				delay_add2latch<=delay3_add1clear;									-- one sample delay for the second adder latch signal
				adder2latch<=delay_add2latch;										-- second adder latch signal
				delay1_syncfft<=temp_syncfft;										-- one sample delay for the fft controler synchronisation
				delay2_syncfft<=delay1_syncfft;										-- second sample delay for the synchronisation of the fft controler
				delay3_syncfft<=delay2_syncfft;										-- third sample delay for the synchronisation of the fft controler
				synchrofft<=delay3_syncfft;											-- synchronisation signal for the fft controler				
				--******************************************************************--
			end if;				
		end process;				
end archisynchro;