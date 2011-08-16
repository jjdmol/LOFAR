--**************************************************************************************************--
--*****************           SELECTION OF THE OUTPUT OF THE FFT CORES	    	********************--
--*****************  Date: 12/12/02 Author: J.L. 								********************--
--*****************  This file selects the real part, imaginary part and  		********************--
--*****************  exponent at the output of the Altera FFT Cores				********************--
--*****************  It generates the control signals for the separation block	********************--
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
entity myselect is
	port(
			clockselect		: 	in std_logic;								-- write clock
			clear			:	in std_logic;								-- clear signal
			counter			:	in std_logic_vector(CODE_N_FFT-1 downto 0);	-- counter corresponding to the read address of the fft controler
			fftindex		:	in std_logic_vector(CODE_CORES-1 downto 0);	-- index of the fft that we are reading
			realfftblocks	:	in outputfft;								-- array of fft real part output
			imagfftblocks	:	in outputfft;								-- array of fft imag. part output
			expofftblocks	:	in fftexpo;									-- array of fft exponent output
			RSfinaloutreal	:	out std_logic_vector(FFT_DATA_WIDTH-1 downto 0);-- selected real part output
			RSfinaloutima	:	out std_logic_vector(FFT_DATA_WIDTH-1 downto 0);-- selected imaginary part output
			exponent		:	out std_logic_vector(EXPO_WIDTH-1 downto 0);-- selected exponent output
			bincounter		:	out std_logic_vector(CODE_N_FFT-1 downto 0);-- index of the output channel
			clock_adder 	:	out std_logic								-- clock
		);		
begin
end myselect;
--*************************************************************************************************--

--*********************** architecture ************************************************************--
architecture archiselect of myselect is	
		--*** no disgnals ***--
begin
	process(clear,clockselect)
		--**** variables ****--
		variable init		:	std_logic;									-- one sample is required to initialize the controler
		--*******************--
	begin
		--*********** clear: re init signals and variables *****************--
		if clear='1' then												
			init:='0';														-- the controler will be re initialized
			RSfinaloutreal <= (others=>'0');
			RSfinaloutima  <= (others=>'0');										
			bincounter	   <= (others=>'0');
		--******************************************************************--
		elsif (clockselect'event and clockselect='1')  then					-- if the clock is high
			--*** initialization ***--
			if init='0' then												-- if the initialization has not been done
				init := '1';												-- then we do it
			--**********************--												
			else															-- if the initialization already took place						
			--*********** this is the selection ****************************--
				bincounter		<= counter;									-- index a the output channel
				clock_adder 	<= not(counter(0));							-- clock for the adders of the separarion block
				RSfinaloutreal 	<= realfftblocks(conv_integer(fftindex));	-- real part signal for the first D/A converter
				RSfinaloutima  	<= imagfftblocks(conv_integer(fftindex));	-- imaginary part signal for the first D/A converter
				exponent	   	<= expofftblocks(conv_integer(fftindex));	-- exponent of the output channel
			--**************************************************************--		
			end if;
		end if;
	end process;
end archiselect;