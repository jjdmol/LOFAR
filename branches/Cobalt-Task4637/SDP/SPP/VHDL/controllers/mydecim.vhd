--**************************************************************************************************--
--*****************              	DECIMATION									********************--
--*****************  Date: 10/12/02 Author: J.L. 								********************--
--*****************  This file enables to decimate two input signals			********************--
--*****************  and to generate the control signals to store them in RAMs	********************--
--**************************************************************************************************--

--************************** libraries declaration *************************************************--
library IEEE;																-- use ieee libraries
use IEEE.std_logic_1164.all;
use IEEE.std_logic_arith.all;
use IEEE.std_logic_unsigned.all;
library work;
use work.pack.all;															-- use package
--**************************************************************************************************--

--************************** entity ****************************************************************--
entity mydecim is
	port(
			clockdecim : 			in std_logic;									-- decimation clock
			clear :					in std_logic;									-- clear signal			
			first_inputsignal : 	in std_logic_vector(BIT_IN-1 downto 0);			-- first input signal
			second_inputsignal : 	in std_logic_vector(BIT_IN-1 downto 0);			-- first input signal						
			newraw : 				out std_logic;									-- begining of a new raw
			rawindex :	 			out std_logic_vector(BIT_RAW-1 downto 0);		-- index of this new raw
			addressRAM :			out std_logic_vector(BIT_RAM_DECIM-1 downto 0);	-- read addresse of the RAMs in the subfilters											
			enablocs :				out std_logic_vector(DECIM_BLOCK-1 downto 0);				-- enable for the RAMs	
			allblocs_first :   		out decimation;									-- decimation of the first input signal
			allblocs_second :  		out decimation									-- decimation of the second input signal	
		);		
begin
end mydecim;
--**************************************************************************************************--

--************************* architecture ***********************************************************--
architecture archidecim of mydecim is			
		--************* signals ****************************************--
		signal counterM : 					std_logic_vector(15 downto 0);			-- counter from 0 to M-1
		signal counterN : 					std_logic_vector(15 downto 0);			-- counter from 0 to N-1
		signal counterD : 					std_logic_vector(7 downto 0);			-- counter from 0 to D-1
		signal delay_addressRAM : 			std_logic_vector(BIT_RAM_DECIM-1 downto 0);	-- 1 clock cycle delay of the temp_addressRAM variable before the addressRAM signal 			
		--**************************************************************--
begin
	process(clear,clockdecim)		
		--************ variables ***************************************--
		variable init :						std_logic;								-- first clock period delay is used to initialise the signals and variables
		variable temp_rawindex :			std_logic_vector(BIT_RAW-1 downto 0);	-- this variable is used to compute the new raw index
		variable temp_addressRAM : 			std_logic_vector(BIT_RAM_DECIM-1 downto 0);	-- variable to compute the read address of the RAMs for all the subfilters
		variable temp_first_inputsignal :	std_logic_vector(BIT_IN-1 downto 0);	-- variable to store the first input
		variable temp_second_inputsignal :	std_logic_vector(BIT_IN-1 downto 0);	-- variable to store the second input
		--**************************************************************--
	begin
		--*********************** clear: re init signals and variables *****************************--
		if clear='1' then														
			init:='0';
			counterM <= (others=>'0');
			counterN <= (others=>'0');
			counterD <= (others=>'0');
			newraw   <= '0';
			temp_rawindex   := (others=>'0');
			temp_addressRAM := (others=>'0');
			rawindex <= temp_rawindex;
			delay_addressRAM <= temp_addressRAM;
			addressRAM <= delay_addressRAM;			
			for i in 0 to DECIM_BLOCK-1 loop
				allblocs_first(i)  <= (others=>'0');
				allblocs_second(i) <= (others=>'0');
				enablocs(i)        <= '0';
			end loop;							
		--******************************************************************************************--
		
		--************************ decimation control **********************************************--
		elsif (clockdecim'event and clockdecim='1')  then					-- if there is a rising edge of the clock signal
			--** initialization **--
			if init='0' then												-- if the initialisation has not been done
				init := '1';												-- then we do it:
				counterM <= conv_std_logic_vector(conv_unsigned(MAX_MATRIX-1,16),16);-- re init the main counter: next value will be 0
			--********************--														
			else															-- if the initialisation already took place					
				temp_first_inputsignal  := first_inputsignal;				-- get the value of the first input signal
				temp_second_inputsignal := second_inputsignal;				-- get the value of the second input signal	
				if conv_integer(counterM)=MAX_MATRIX-1 then							-- if the main counter reached its maximum value
					counterM <= (others=>'0');								-- restart all the counters
					counterN <= (others=>'0');
					counterD <= (others=>'0');
					newraw   <= '1';										-- indicate that we start to fill in a new raw of the matrix
					temp_rawindex   := (others=>'0');						-- the index of this new raw is 0
					temp_addressRAM := (others=>'0');						-- the restart to write from address 0 in the RAMs
				else														-- if the main counter didn't reach its maximum value
					counterM<=counterM+1;									-- increment it
					if conv_integer(counterN)=N_FFT-1 then						-- if the counter from 0 to N-1 reached its maximum value											
						counterN <= (others=>'0');							-- restart it
						counterD <= (others=>'0');							-- restart the counter from 0 t D-1
						newraw   <= '1';									-- indicate that we start to fill in a new raw
						temp_rawindex   := temp_rawindex+'1';				-- increment the raw index
						temp_addressRAM := temp_addressRAM+'1';				-- increment the read address of the RAM
					else													-- if the counter from 0 to N-1 didn't reach N-1
						newraw   <= '0';									-- we are still filling in the same raw
						counterN <= counterN+1;								-- keep incrementing this counter
						if conv_integer(counterD)=DECIM_BLOCK-1 then		-- if the decimation counter reached its maximum value
							counterD <= (others=>'0');						-- restart it
							temp_addressRAM := temp_addressRAM+'1';			-- increment the read address of the RAM					
						else												-- but if this counter didn't reach D-1
							counterD <= counterD+1;							-- increment it
						end if;										
					end if;				
				end if;
			end if;			
			rawindex <= temp_rawindex;										-- modification of the rawindex signal
			delay_addressRAM <= temp_addressRAM;							-- modification of the 1 sample delay for the addressRAM signal
			addressRAM <= delay_addressRAM;									-- modification of the read address for all the subfilters RAMs						
			--*************** DECIMATION ************************--
			for i in 0 to DECIM_BLOCK-1 loop								-- for each block
				if conv_integer(counterD)=i then
					allblocs_first(i)  <= temp_first_inputsignal;			-- decimate the first input
					allblocs_second(i) <= temp_second_inputsignal;			-- decimate the second input
					enablocs(i)        <= '1';								-- enable to write in a RAM
				else
					enablocs(i) <= '0';
				end if;
			end loop;
			--***************************************************--										
		end if;								
	end process;				
end archidecim;