library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_arith.all;
use IEEE.std_logic_unsigned.all;

package pack is
	
	signal clockdecim			: 	std_logic;
	signal clear				:	std_logic :='0'; 

	constant N_FFT				:	integer := 256;						-- fft length				--256
	constant DECIM_BLOCK		:	integer := 16;						-- decimation factor and number of blocks
	constant PREFILTER_L		:	integer := 16;						-- prefilters length
	constant BIT_IN				:	integer := 14;						-- input signals width		
	constant FFT_CORES			:	integer := 7;						-- number of FFT cores
	constant FFT_DATA_WIDTH		:	integer := 16;						-- FFT data width
	constant EXPO_WIDTH			:	integer := 5;						-- FFT exponent width
	constant TRUNCATION			:	integer := 7;						-- truncation width after multiplication in the prefilters
	
	constant BIT_RAM_DECIM		:	integer	:= 9;						-- log2(N_FFT)+1			--9
	constant BIT_ROM			:	integer := 7;						-- log2(N_FFT/2)			--7
	constant BIT_RAW			:	integer := 5;						-- log2(DECIM_BLOCK)+1		--5	
	constant CODE_CORES			:	integer := 3;						-- log2(FFT_CORES)			--3
	constant MAX_MATRIX			:	integer := N_FFT*(PREFILTER_L+1);	-- number of sample per block matrix
	constant SAMPLES_RAW		:	integer := N_FFT/DECIM_BLOCK;		-- number of samples in each raw of the mtrix
	constant SAMPLES_READ_BLOCK	:	integer := SAMPLES_RAW*PREFILTER_L; -- number of samples to read in each block
	constant HALF_READ_BLOCK	:	integer := SAMPLES_READ_BLOCK/2;	-- middle of the matrix: used for the symmetry of the filters
	constant HALF_PREFILTER		:	integer := PREFILTER_L/2;			-- middle of the prefilters: used for an internal counter
	constant HALF_N_FFT			:	integer := N_FFT/2;					-- lenght of a nyquist zone
	constant CODE_N_FFT			:	integer := BIT_RAM_DECIM-1;			-- number of bits required to code all teh fft channels
	constant OUT_MULT_PREFILT	:	integer := 2*BIT_IN+4;				-- number of bits at the output of the multiplier
			
	constant FILE_ROM_1			:	string	:= "C:/user/Lemaitre/prefilters/complete_dsp_board_final_separate/Files/sub1.hex";
	constant FILE_ROM_2			:	string	:= "C:/user/Lemaitre/prefilters/complete_dsp_board_final_separate/Files/sub2.hex";
	constant FILE_ROM_A			:	string	:= "C:/user/Lemaitre/prefilters/complete_dsp_board_final_separate/Files/sub10.hex";
	
	type decimation is array(0 to DECIM_BLOCK-1) of std_logic_vector(BIT_IN-1 downto 0);
	type internalcountersreadRAM is array(1 to PREFILTER_L+1) of std_logic_vector(15 downto 0);
	type inputfft is array(1 to PREFILTER_L) of std_logic_vector(FFT_DATA_WIDTH-1 downto 0);
	type outputfft is array(0 to FFT_CORES-1) of std_logic_vector(FFT_DATA_WIDTH-1 downto 0);
	type fftexpo is array(0 to FFT_CORES-1) of std_logic_vector(EXPO_WIDTH-1 downto 0);
			
end pack;

package body pack is
			
end pack;