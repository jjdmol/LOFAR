--**************************************************************************************************--
--*****************          	CONTROL OF THE FFT CORES						********************--
--*****************  Date: 11/12/02 Author: J.L. 								********************--
--*****************  This file synchronizes the prefilters output and			********************--
--*****************  controls the Altera FFT Cores	in a circular permutation	********************--
--*****************  The output is ready for separation of the complex FFT		********************--
--**************************************************************************************************--

--************************** libraries declaration *************************************************--
library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_arith.all;
use IEEE.std_logic_unsigned.all;
library work;
use work.pack.all;
--*************************************************************************************************--

--*********************** entity ******************************************************************--
entity myfftcontrol is
	port(
			clock1: 		in std_logic;									-- write clock
			clock2:			in std_logic;									-- read clock
			clear:			in std_logic;									-- clear signal			
			synchro:		in std_logic;									-- synchronisation with the prefilters
			inblocfirst:	in inputfft;									-- filtering of the first signal
			inblocsecond:	in inputfft;									-- filtering of the second signal
			countersync:	out std_logic_vector(CODE_N_FFT-1 downto 0);	-- counter for the synchronisation with the fft cores
			blocindex:		out std_logic_vector(CODE_CORES-1 downto 0);	-- index of the fft block that can be read
			resetfft:		out std_logic_vector(FFT_CORES-1 downto 0);		-- bus for the reset signals for each fft block
			gofft:			out std_logic_vector(FFT_CORES-1 downto 0);		-- bus for the go signals for each fft block
			readfft:		out std_logic_vector(FFT_CORES-1 downto 0);		-- bus for the read signals for each fft block
			writefft:		out std_logic_vector(FFT_CORES-1 downto 0);		-- bus for the reset signals for each fft block			
			writeaddress:	out std_logic_vector(CODE_N_FFT-1 downto 0);	-- write address for all the fft blocks
			readaddress:	out std_logic_vector(CODE_N_FFT-1 downto 0);	-- read address for all the fft blocks			
			realpart:		out std_logic_vector(FFT_DATA_WIDTH-1 downto 0);				-- real part of the signal for the fft blocks
			imagpart:		out std_logic_vector(FFT_DATA_WIDTH-1 downto 0)				-- imaginary part of the signal for the fft blocks											
		);		
begin
end myfftcontrol;
--*************************************************************************************************--

--*********************** architecture ************************************************************--
architecture archifftcontrol of myfftcontrol is	
	--************* signals ************************************************************--
	signal counterN : 				std_logic_vector(15 downto 0);						-- counters from 0 to N-1, for each fft block (pipeline)
	signal delay1_realpart:			std_logic_vector(FFT_DATA_WIDTH-1 downto 0);						-- one clock cycle delay for the real part signal
	signal delay1_imagpart:			std_logic_vector(FFT_DATA_WIDTH-1 downto 0);						-- one clock cycle delay for the imag. part signal
	signal delay1_countsync:		std_logic_vector(CODE_N_FFT-1 downto 0);			-- first clock cycle delay for the synchronization counter
	signal delay2_countsync:		std_logic_vector(CODE_N_FFT-1 downto 0);			-- second clock cycle delay for the synchronization counter
	signal delay3_countsync:		std_logic_vector(CODE_N_FFT-1 downto 0);			-- third clock cycle delay for the synchronization counter
	signal way_read:				std_logic;											-- indicates if we have to increment or decrement the read address counter
	--**********************************************************************************--	
begin
	process(clear,clock1)
		--********* variables **********************************************************--
		variable init:					std_logic;										-- one sample is required to initialize the controler
		variable blocfftread: 			std_logic_vector(CODE_CORES-1 downto 0);		-- fft blocks counter
		variable last_blocfftread:		std_logic_vector(CODE_CORES-1 downto 0);		-- index of the last fft block that has been read
		variable last_blocffwrite:		std_logic_vector(CODE_CORES-1 downto 0);		-- index of the last fft block that has been written
		variable temp_read: 			std_logic_vector(FFT_CORES-1 downto 0);			-- bus for the read enable of all the fft blocks 
		variable temp_write: 			std_logic_vector(FFT_CORES-1 downto 0);			-- bus for the write enable of all the fft blocks
		variable temp_go:				std_logic_vector(FFT_CORES-1 downto 0);			-- bus for the go enable of all the fft blocks
		variable temp_read_address:		std_logic_vector(CODE_N_FFT-1 downto 0);		-- variable for the read address for all the fft blocks
		variable temp_write_address:	std_logic_vector(CODE_N_FFT-1 downto 0);		-- variable for the write address for all the fft blocks
		variable counterL:				std_logic_vector(BIT_RAW-1 downto 0);			-- counter used to loop on the MAC blocks outputs
		variable temp_synchro:			std_logic;										-- variable used to count the channel output index, for synchronization
		variable read_updown:			std_logic_vector(CODE_N_FFT-1 downto 0);		-- indicates if we have to increment or decrement the read address counter
		variable count_half:			std_logic_vector(CODE_N_FFT-1 downto 0);		-- counting one clock cycle over two
		--******************************************************************************--
	begin
		--**************** clear: re init signals and variables ************************--
		if clear='1' then																
			init:='0';																	
			temp_read_address:=(others=>'0');									
			readfft<=(others=>'0');
			temp_write_address:=(others=>'0');
			writefft<=(others=>'0');
			temp_go:=(others=>'0');
			gofft<=(others=>'0');
			resetfft<=(others=>'0');
			countersync<=(others=>'0');
			blocindex<=(others=>'0');
		--******************************************************************************--
		elsif (clock1'event and clock1='1')  then										-- if rising edge of the clock
			--***** initialization *****--
			if init='0' then															-- if the initialization has not been done
				init:='1';																-- then we do it:
				blocfftread:=conv_std_logic_vector(conv_unsigned(FFT_CORES-1,CODE_CORES),CODE_CORES);-- force the fft blocks counter to its maximum value, so it will restart counting from 0
			--**************************--
			else																		-- if the initialization already took place						
			--************** control of the FFT cores **********************************--	
				temp_synchro:=synchro;
				--****************** synchronization ***********************************--
				if temp_synchro = '1' then												-- re initialize the signals and variables									
					counterN<=(others=>'0');											
					last_blocffwrite:=last_blocfftread;
					last_blocfftread:=blocfftread;										-- save the index of the last fft block that started
					counterL:=(others=>'0');											
					read_updown:=(others=>'0');
					way_read<='0';
					count_half:=(others=>'0');
					temp_read_address:=(others=>'0');									
					temp_write_address:=(others=>'0');									
					--*** bloc to read ***-
					if conv_integer(blocfftread)=FFT_CORES-1 then						-- if the last fft block that started is the  last fft block
						blocfftread:=(others=>'0');										-- use the first fft block
					else																-- if the counter didn't reach its maximum value
						blocfftread:=blocfftread+'1';									-- use next fft block
					end if;
					--*** update go/read/write/reset ***--
					for i in 0 to FFT_CORES-1 loop										-- for each fft block
						temp_go(i):=temp_go(i);											-- force go to 0 for this block
						if i= conv_integer(blocfftread) then							-- if the index refers to the fft block to re initialize
							temp_read(i):='1';											-- enable reading the output of this block
							temp_write(i):='0';											-- the write signal for this block remains the same
						else															-- but for all the fft block that don't need to restart 
							temp_read(i):='0';											-- don't enable to read their output
							if i= conv_integer(last_blocfftread) then					-- if the index refers to the block we read
								temp_write(i):='1';										-- we can now write in this block
							end if;
							if i= conv_integer(last_blocffwrite) then					-- if the index refers to the block where we wrote
								temp_go(i):='0';										-- we need to restart it, so we have to stop it before 
								temp_write(i):='0';										-- we don't want to re write in this block
							end if;
						end if;								
					end loop;
				--**********************************************************************--	
				else																	-- if the synchronisation has already been done
				--******************* synchronisation already done *********************--	
					counterN<=counterN+'1';
					--*** counters ***--
					if conv_integer(counterL)=PREFILTER_L-1 then						-- if the last MAC output was comimg from the last subfilter
						counterL:=(others=>'0');										-- restart fromthe first subfilter
					else																-- if the counter didn't reach its maximum value
						counterL:=counterL+'1';											-- increment counter
					end if;
					--**** counting one sample over two, to prepare the separation of the FFT output ******--
					if way_read='0' then												 
						way_read<='1';
						read_updown:=conv_std_logic_vector(N_FFT-1-conv_integer(count_half),CODE_N_FFT);
					else
						way_read<='0';
						count_half:=count_half+'1';
						read_updown:=count_half;
					end if;																								
					temp_read_address:=temp_read_address+'1';							-- increment read address counter
					temp_write_address:=temp_write_address+'1';							-- keep writting new fft input
					--*** update go/read/write/reset ***--
					for i in 0 to FFT_CORES-1 loop											-- for each fft block
						temp_read(i):='0';												-- disable reading
						if i= conv_integer(last_blocfftread) then						-- if the index corresponds to the re-initialized fft block
							temp_write(i):='1';											-- enable writting in this block
						else															-- for the other blocks
							temp_write(i):='0';											-- disable writting
						end if;						
						if i= conv_integer(blocfftread) then							-- if the index refers to the block we are reading											-- for the other blocks
							temp_read(i):='1';											-- keep reading it
							temp_write(i):='0';											-- don't write in this block
						end if;
					end loop;
					if conv_integer(counterN)=3 then									
						resetfft(conv_integer(last_blocffwrite))<='1';					-- reset the block where we just finished to write
					elsif conv_integer(counterN)=4 then
						resetfft(conv_integer(last_blocffwrite))<='0';					-- stop to reset this block
					elsif conv_integer(counterN)=7 then
						temp_go(conv_integer(last_blocffwrite)):='1';					-- restart this block
					end if;														
				end if;
				--**********************************************************************--
			end if;
			--*************** delays and synchronisation of the output signals *********--
			readfft<=temp_read;															-- read enable bus
			writefft<=temp_write;														-- write enable bus
			gofft<=temp_go;																-- go bus
			readaddress<=read_updown;													-- read address of the FFT core
			writeaddress<=temp_write_address;											-- write address in the the fft blocks
			delay1_realpart<=inblocfirst(conv_integer(counterL+1));						-- one clock cycle delay for the real part signal
			delay1_imagpart<=inblocsecond(conv_integer(counterL+1));					-- one clock cycle delay for the imag. part signal
			realpart<=delay1_realpart;													-- real part of the fft output
			imagpart<=delay1_imagpart;													-- imaginary part of the fft input			
			delay1_countsync<=temp_read_address;										-- first clock cycle delay for synchronisation
			delay2_countsync<=delay1_countsync;											-- second clock cycle delay for synchronisation
			delay3_countsync<=delay2_countsync;											-- third clock cycle delay for synchronisation
			countersync<=delay3_countsync;												-- fourth clock cycle delay for synchronisation
			blocindex<=blocfftread;														-- index of the FFT core that is currently read
			--**************************************************************************--
		end if;			
	end process;				
end archifftcontrol;