--**************************************************************************************************--
--*****************        SEPARATION OF FOUR SIGNALS AFTER A COMPLEX FFT	    ********************--
--*****************  Date: 12/12/02 Author: J.L. 								********************--
--*****************  This file separates two real parts and two imaginary 		********************--
--*****************  parts after a complex FFT, for the first Nyquist zone 		********************--
--*****************  only because it needs two complex channels to compute 		********************--
--*****************  one separation 											********************--
--*****************  This file also sends synchronisation information for NIOS  ********************--
--**************************************************************************************************--

--************************** libraries declaration *************************************************--
LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE IEEE.std_logic_arith.all;
USE IEEE.std_logic_unsigned.all;
--**************** NOTE: THIS CODE IS BASED ON THE LIBRARY OF ALERA ********************************--
--****************  The components have to be compiled separately   ********************************--
LIBRARY lpm;
USE lpm.lpm_components.all;
--**************************************************************************************************--
library work;
use work.pack.all;																-- use package
--*************************************************************************************************--

--*********************** entity ******************************************************************--
ENTITY separate IS
	PORT
	(
		clk						: IN STD_LOGIC;									-- clock
		clk_adder				: IN STD_LOGIC;									-- clock for the adders, coming from the selection block
		clear					: IN STD_LOGIC;									-- clear	
		counter_bin				: IN STD_LOGIC_VECTOR(CODE_N_FFT-1 DOWNTO 0);	-- channel index, coming from the selection block
		realpart_fftoutput		: IN STD_LOGIC_VECTOR(FFT_DATA_WIDTH-1 DOWNTO 0);-- real part of the complex FFT
		imagpart_fftoutput		: IN STD_LOGIC_VECTOR(FFT_DATA_WIDTH-1 DOWNTO 0);-- imag. part of the complex FFT
		expo					: IN STD_LOGIC_VECTOR(EXPO_WIDTH-1 DOWNTO 0);	-- exponent of the complex FFT
		real_first_comb			: OUT STD_LOGIC_VECTOR (FFT_DATA_WIDTH-1 DOWNTO 0);-- real part of the first signal after separation
		real_sec_comb			: OUT STD_LOGIC_VECTOR (FFT_DATA_WIDTH-1 DOWNTO 0);-- real part of the second signal after separation
		imag_first_comb			: OUT STD_LOGIC_VECTOR (FFT_DATA_WIDTH-1 DOWNTO 0);-- imag. part of the first signal after separation
		imag_sec_comb			: OUT STD_LOGIC_VECTOR (FFT_DATA_WIDTH-1 DOWNTO 0);-- imag. part of the second signal after separation
		exponent				: OUT STD_LOGIC_VECTOR(EXPO_WIDTH-1 DOWNTO 0);	-- exponent (for the NIOS interface)
		start_acq				: OUT STD_LOGIC;								-- synchronisation for NIOS
		ena_acq					: OUT STD_LOGIC									-- acquisition enable for NIOS
	);
END separate;
--*************************************************************************************************--

--*********************** architecture ************************************************************--
ARCHITECTURE SYN OF separate IS
	--******************* signals **********************************--
	SIGNAL  temp_realpart 		: STD_LOGIC_VECTOR (FFT_DATA_WIDTH-1 DOWNTO 0);	-- register of the real part of the complex FFT
	SIGNAL  temp_imagpart 		: STD_LOGIC_VECTOR (FFT_DATA_WIDTH-1 DOWNTO 0);	-- register of the imag. part of the complex FFT
	SIGNAL	real_first_nyq		: STD_LOGIC_VECTOR (FFT_DATA_WIDTH-1 DOWNTO 0);	-- register of the real part of the first signal
	SIGNAL	real_sec_nyq		: STD_LOGIC_VECTOR (FFT_DATA_WIDTH-1 DOWNTO 0);	-- register of the real part of the second signal
	SIGNAL	imag_first_nyq		: STD_LOGIC_VECTOR (FFT_DATA_WIDTH-1 DOWNTO 0);	-- register of the imag. part of the first signal
	SIGNAL	imag_sec_nyq		: STD_LOGIC_VECTOR (FFT_DATA_WIDTH-1 DOWNTO 0);	-- register of the imag. part of the second signal
	--**************************************************************--
	
--*************************** components declaration***********************************************--
	COMPONENT lpm_add_sub_delaypos									-- adder
	PORT (
			dataa	: IN STD_LOGIC_VECTOR (FFT_DATA_WIDTH-1 DOWNTO 0);
			datab	: IN STD_LOGIC_VECTOR (FFT_DATA_WIDTH-1 DOWNTO 0);
			clock	: IN STD_LOGIC;	
			result	: OUT STD_LOGIC_VECTOR (FFT_DATA_WIDTH-1 DOWNTO 0)
	);
	END COMPONENT;

	COMPONENT lpm_add_sub_delayneg									-- substracter
	PORT (
			dataa	: IN STD_LOGIC_VECTOR (FFT_DATA_WIDTH-1 DOWNTO 0);
			datab	: IN STD_LOGIC_VECTOR (FFT_DATA_WIDTH-1 DOWNTO 0);
			clock	: IN STD_LOGIC;	
			result	: OUT STD_LOGIC_VECTOR (FFT_DATA_WIDTH-1 DOWNTO 0)
	);
	END COMPONENT;
--*************************************************************************************************--
BEGIN
	PROCESS(clear,clk)
	--**** no variables ****--	
	BEGIN
		--******* clear: re init signals and variables *************--
		IF clear='1' THEN
			temp_realpart	<= (others=>'0');
			real_first_nyq	<= (others=>'0');
			real_sec_nyq	<= (others=>'0');
			temp_imagpart	<= (others=>'0');
			imag_first_nyq  <= (others=>'0');
			imag_sec_nyq	<= (others=>'0');
			exponent		<= (others=>'0');
			start_acq		<= '0';
			ena_acq			<= '0';
		--**********************************************************--
		ELSIF (clk'event AND clk='1')  THEN							-- if the clock is high
			--************** control of the i/o signals ************--	
			IF conv_integer(counter_bin)=0 THEN						-- special case: first channel: reset siganls
				temp_realpart  <= realpart_fftoutput;				
				real_first_nyq <= (others=>'0');					
				real_sec_nyq   <= (others=>'0');					
				temp_imagpart  <= imagpart_fftoutput;				
				imag_first_nyq <= (others=>'0');					
				imag_sec_nyq   <= (others=>'0');							
			ELSIF conv_integer(counter_bin)=1 THEN					-- second channel: start separation
				temp_realpart  <= realpart_fftoutput;
				real_first_nyq <= temp_realpart;
				real_sec_nyq   <= temp_realpart;
				temp_imagpart  <= imagpart_fftoutput;
				imag_first_nyq <= temp_imagpart;
				imag_sec_nyq   <= temp_imagpart;					
			ELSIF conv_integer(counter_bin)=HALF_N_FFT THEN 		-- special case: middle of the spectrum
				temp_realpart  <= realpart_fftoutput;
				real_first_nyq <= temp_realpart;
				real_sec_nyq   <= temp_realpart;
				temp_imagpart  <= imagpart_fftoutput;
				imag_first_nyq <= temp_imagpart;
				imag_sec_nyq   <= temp_imagpart;												
			ELSIF	conv_integer(counter_bin)=HALF_N_FFT+1 THEN 	-- keep separating 
				temp_realpart  <= realpart_fftoutput;
				real_first_nyq <= temp_realpart;
				real_sec_nyq   <= temp_realpart;
				temp_imagpart  <= imagpart_fftoutput;
				imag_first_nyq <= temp_imagpart;
				imag_sec_nyq   <= temp_imagpart;												
			ELSE													-- typical separation:
				temp_realpart  <= realpart_fftoutput;				-- real part: store the current channel
				real_first_nyq <= temp_realpart;					-- real part: get the stored value
				real_sec_nyq   <= realpart_fftoutput;				-- real part: get the cuurent value
				temp_imagpart  <= imagpart_fftoutput;				-- imag. part: store the current channel
				imag_first_nyq <= temp_imagpart;					-- imag. part: get the stored value
				imag_sec_nyq   <= imagpart_fftoutput;				-- imag. part: get the cuurent value
			END IF;
			--******************************************************--
			
			--** signals required for the acquisition with NIOS ****--
			IF conv_integer(counter_bin)=1 THEN  					-- start acquisition on the first channel
				start_acq <= '1';				 								 
			ELSE								
				start_acq <= '0';				 								
			END IF;
			ena_acq  <= not(counter_bin(0));						-- store only one sample over two: first nyquist zone only
			exponent <= expo;										-- store the value of the exponent associated to the channel
			--******************************************************--
		END IF;
	END PROCESS;

--************************ instanciation of the components **********************************************--
	lpm_add_sub_pos_real_component : lpm_add_sub_delaypos
	PORT MAP (
		dataa  => real_first_nyq,
		datab  => real_sec_nyq,
		clock  => clk_adder,
		result => real_first_comb
	);

	lpm_add_sub_pos_imag_component : lpm_add_sub_delaypos
	PORT MAP (
		dataa  => imag_first_nyq,
		datab  => imag_sec_nyq,
		clock  => clk_adder,
		result => imag_first_comb
	);

	lpm_add_sub_neg_real_component : lpm_add_sub_delayneg
	PORT MAP (
		dataa => real_first_nyq,
		datab => real_sec_nyq,
		clock => clk_adder,
		result => real_sec_comb
	);

	lpm_add_sub_neg_imag_component : lpm_add_sub_delayneg
	PORT MAP (
		dataa => imag_first_nyq,
		datab => imag_sec_nyq,
		clock => clk_adder,
		result => imag_sec_comb
	);
--*****************************************************************************************************--
END SYN;