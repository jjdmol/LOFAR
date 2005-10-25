--**************************************************************************************************--
--*****************        		PREFILTER										********************--
--*****************  Date: 16/12/02 Author: J.L. 								********************--
--*****************  This file is a prefilter: it reads the decimated samples	********************--
--*****************  from a RAM, the coefficients from a ROM, multilplies and 	********************--
--*****************  accumulates this data before truncating the result for		********************--
--*****************  the fft cores												********************--
--**************************************************************************************************--

--************************** libraries declaration *************************************************--
LIBRARY ieee;
USE ieee.std_logic_1164.all; 
LIBRARY work;
USE work.pack.all;
--*************************************************************************************************--

--*********************** entity ******************************************************************--
ENTITY PREFILTER IS 
	--** parameter: name of the .hex file to write in the ROM **--
	--** NOTE: this file is created with MATLAB **--
	generic(file_rom	:	string:= work.pack.FILE_ROM_A
			);
	port
	(
		clear 				:  IN  STD_LOGIC;
		multenable 			:  IN  STD_LOGIC;
		adder1enable 		:  IN  STD_LOGIC;
		clockdecim 			:  IN  STD_LOGIC;
		clocksync 			:  IN  STD_LOGIC;
		adder1clear 		:  IN  STD_LOGIC;
		adder1latch 		:  IN  STD_LOGIC;
		adder2enable 		:  IN  STD_LOGIC;
		adder2clear 		:  IN  STD_LOGIC;
		adder2latch 		:  IN  STD_LOGIC;
		writeRAMena 		:  IN  STD_LOGIC;
		firstinput 			:  IN  STD_LOGIC_VECTOR(BIT_IN-1 downto 0);
		readaddressRAM 		:  IN  STD_LOGIC_VECTOR(BIT_RAM_DECIM-1 downto 0);
		readaddressROM 		:  IN  STD_LOGIC_VECTOR(BIT_ROM-1 downto 0);
		secondinput 		:  IN  STD_LOGIC_VECTOR(BIT_IN-1 downto 0);
		writeaddressRAM 	:  IN  STD_LOGIC_VECTOR(BIT_RAM_DECIM-1 downto 0);
		outfirst 			:  OUT  STD_LOGIC_VECTOR(FFT_DATA_WIDTH-1 downto 0);
		outsecond 			:  OUT  STD_LOGIC_VECTOR(FFT_DATA_WIDTH-1 downto 0)
	);
END PREFILTER;
--*************************************************************************************************--

--*********************** architecture ************************************************************--
ARCHITECTURE bdf_type OF PREFILTER IS 
--*********************** components **************************************************************--
	
	--*** adder *************--
	component lpm_add_prefilter
		PORT(clock 			: IN STD_LOGIC;
			 clken 			: IN STD_LOGIC;
			 aclr 			: IN STD_LOGIC;
			 dataa 			: IN STD_LOGIC_VECTOR(OUT_MULT_PREFILT-1 downto 0);
			 datab 			: IN STD_LOGIC_VECTOR(OUT_MULT_PREFILT-1 downto 0);
			 result 		: OUT STD_LOGIC_VECTOR(OUT_MULT_PREFILT-1 downto 0)
		);
	end component;

	--*** multiplexer *******--
	component mux_prefilter
		PORT(sel 			: IN STD_LOGIC;
			 infirst 		: IN STD_LOGIC_VECTOR(FFT_DATA_WIDTH-1 downto 0);
			 insecond 		: IN STD_LOGIC_VECTOR(FFT_DATA_WIDTH-1 downto 0);
			 result 		: OUT STD_LOGIC_VECTOR(FFT_DATA_WIDTH-1 downto 0)
		);
	end component;

	--*** multiplier ********--
	component lpm_mult_prefilter
		PORT(clock 			: IN STD_LOGIC;
			 aclr 			: IN STD_LOGIC;
			 clken 			: IN STD_LOGIC;
			 dataa 			: IN STD_LOGIC_VECTOR(BIT_IN-1 downto 0);
			 datab 			: IN STD_LOGIC_VECTOR(BIT_IN-1 downto 0);
			 result 		: OUT STD_LOGIC_VECTOR(OUT_MULT_PREFILT-1 downto 0)
		);
	end component;

	--*** dual port RAM *****--
	component lpm_ram_dp_prefilter
		PORT(wren 			: IN STD_LOGIC;
			 wrclock 		: IN STD_LOGIC;
			 rdclock 		: IN STD_LOGIC;
			 data 			: IN STD_LOGIC_VECTOR(BIT_IN-1 downto 0);
			 rdaddress 		: IN STD_LOGIC_VECTOR(BIT_RAM_DECIM-1 downto 0);
			 wraddress 		: IN STD_LOGIC_VECTOR(BIT_RAM_DECIM-1 downto 0);
			 q 				: OUT STD_LOGIC_VECTOR(BIT_IN-1 downto 0)
		);
	end component;

	--*** ROM ***************--
	component lpm_rom_prefilter
		GENERIC(file_name	: string	:= work.pack.FILE_ROM_1
				);
		PORT(inclock 		: IN STD_LOGIC;
			 outclock 		: IN STD_LOGIC;
			 address 		: IN STD_LOGIC_VECTOR(BIT_ROM-1 downto 0);
			 q 				: OUT STD_LOGIC_VECTOR(BIT_IN-1 downto 0)
		);
	end component;
--*************************************************************************************************--

	--********************** signals **************************************************************--
	signal	add1cl 			:  STD_LOGIC;
	signal	add1ena 		:  STD_LOGIC;
	signal	add1lat 		:  STD_LOGIC;
	signal	add2cl 			:  STD_LOGIC;
	signal	add2ena 		:  STD_LOGIC;
	signal	add2lat 		:  STD_LOGIC;
	signal	addressROM 		:  STD_LOGIC_VECTOR(BIT_ROM-1 downto 0);
	signal	busaddfirst1 	:  STD_LOGIC_VECTOR(OUT_MULT_PREFILT-1 downto 0);
	signal	busaddfirst2 	:  STD_LOGIC_VECTOR(OUT_MULT_PREFILT-1 downto 0);
	signal	busaddsecond1 	:  STD_LOGIC_VECTOR(OUT_MULT_PREFILT-1 downto 0);
	signal	busaddsecond2 	:  STD_LOGIC_VECTOR(OUT_MULT_PREFILT-1 downto 0);
	signal	firstinp 		:  STD_LOGIC_VECTOR(BIT_IN-1 downto 0);
	signal	multena 		:  STD_LOGIC;
	signal	out1_latchfirst :  STD_LOGIC_VECTOR(FFT_DATA_WIDTH-1 downto 0);
	signal	out1_latchsecond:  STD_LOGIC_VECTOR(FFT_DATA_WIDTH-1 downto 0);
	signal	out2_latchfirst :  STD_LOGIC_VECTOR(FFT_DATA_WIDTH-1 downto 0);
	signal	out2_latchsecond:  STD_LOGIC_VECTOR(FFT_DATA_WIDTH-1 downto 0);
	signal	outmultfirst 	:  STD_LOGIC_VECTOR(OUT_MULT_PREFILT-1 downto 0);
	signal	outmultsecond 	:  STD_LOGIC_VECTOR(OUT_MULT_PREFILT-1 downto 0);
	signal	outRAMfirst 	:  STD_LOGIC_VECTOR(BIT_IN-1 downto 0);
	signal	outRAMsecond 	:  STD_LOGIC_VECTOR(BIT_IN-1 downto 0);
	signal	outROM 			:  STD_LOGIC_VECTOR(BIT_IN-1 downto 0);
	signal	readRAM 		:  STD_LOGIC_VECTOR(BIT_RAM_DECIM-1 downto 0);
	signal	secondinp 		:  STD_LOGIC_VECTOR(BIT_IN-1 downto 0);
	signal	writeRAM 		:  STD_LOGIC_VECTOR(BIT_RAM_DECIM-1 downto 0);
	signal	writeRAMen 		:  STD_LOGIC;
	signal	TEMP_SIGNAL_0 	:  STD_LOGIC_VECTOR(FFT_DATA_WIDTH-1 downto 0);
	signal	TEMP_SIGNAL_1 	:  STD_LOGIC_VECTOR(FFT_DATA_WIDTH-1 downto 0);
	signal	TEMP_SIGNAL_2 	:  STD_LOGIC_VECTOR(FFT_DATA_WIDTH-1 downto 0);
	signal	TEMP_SIGNAL_3 	:  STD_LOGIC_VECTOR(FFT_DATA_WIDTH-1 downto 0);
	--*********************************************************************************************--

BEGIN 

--******************** instanciation of the components ********************************************--	
	
	--****** first adder, first input signal ******--
	add11 : lpm_add_prefilter
	PORT MAP(clock 		=> clocksync,
			 clken 		=> add1ena,
			 aclr 		=> add1cl,
			 dataa 		=> busaddfirst1,
			 datab 		=> outmultfirst,
			 result 	=> busaddfirst1);
	
	--****** second adder, first input signal *****--
	add12 : lpm_add_prefilter
	PORT MAP(clock 		=> clocksync,
			 clken 		=> add2ena,
			 aclr 		=> add2cl,
			 dataa 		=> busaddfirst2,
			 datab 		=> outmultfirst,
			 result 	=> busaddfirst2);

	--****** first adder, second input signal *****--
	add13 : lpm_add_prefilter
	PORT MAP(clock 		=> clocksync,
			 clken 		=> add1ena,
			 aclr 		=> add1cl,
			 dataa 		=> busaddsecond1,
			 datab 		=> outmultsecond,
			 result 	=> busaddsecond1);

	--****** first adder, second input signal *****--
	add14 : lpm_add_prefilter
	PORT MAP(clock 		=> clocksync,
			 clken 		=> add2ena,
			 aclr 		=> add2cl,
			 dataa 		=> busaddsecond2,
			 datab 		=> outmultsecond,
			 result 	=> busaddsecond2);

	--****** multilpexer, first signal
	mux1 : mux_prefilter
	PORT MAP(sel 		=> add1ena,
			 infirst 	=> out1_latchfirst,
			 insecond 	=> out2_latchfirst,
			 result 	=> outfirst);

	--****** multiplexer, second signal
	mux2 : mux_prefilter
	PORT MAP(sel 		=> add1ena,
			 infirst 	=> out1_latchsecond,
			 insecond 	=> out2_latchsecond,
			 result 	=> outsecond);

	--****** multiplier, first signal
	mult1 : lpm_mult_prefilter
	PORT MAP(clock 		=> clocksync,
			 aclr 		=> clear,
			 clken 		=> multena,
			 dataa 		=> outRAMfirst,
			 datab 		=> outROM,
			 result 	=> outmultfirst);

	--****** multiplier, second signal
	mult2 : lpm_mult_prefilter
	PORT MAP(clock 		=> clocksync,
			 aclr 		=> clear,
			 clken 		=> multena,
			 dataa 		=> outRAMsecond,
			 datab 		=> outROM,
			 result 	=> outmultsecond);

	--****** DP RAM, first signal
	ram1 : lpm_ram_dp_prefilter
	PORT MAP(wren 		=> writeRAMen,
			 wrclock 	=> clockdecim,
			 rdclock 	=> clocksync,
			 data 		=> firstinp,
			 rdaddress 	=> readRAM,
			 wraddress 	=> writeRAM,
			 q 			=> outRAMfirst);

	--****** DP RAM, second signal
	ram2 : lpm_ram_dp_prefilter
	PORT MAP(wren 		=> writeRAMen,
			 wrclock 	=> clockdecim,
			 rdclock 	=> clocksync,
			 data 		=> secondinp,
			 rdaddress 	=> readRAM,
			 wraddress 	=> writeRAM,
			 q 			=> outRAMsecond);

	--****** ROM, filter coefficients
	rom : lpm_rom_prefilter GENERIC MAP(file_rom)
	PORT MAP(inclock 	=> clocksync,
			 outclock 	=> clocksync,
			 address 	=> addressROM,
			 q 			=> outROM);

--*************************************************************************************************--

--******************** latches: control of the signals ********************************************--	
	
	--***** first latch, first signal ***********--
	process(add1lat,TEMP_SIGNAL_0)
	begin
	if (add1lat = '1') then
		out1_latchfirst <= TEMP_SIGNAL_0;
	end if;
	end process;

	--***** second latch, first signal **********--	
	process(add2lat,TEMP_SIGNAL_1)
	begin
	if (add2lat = '1') then
		out2_latchfirst <= TEMP_SIGNAL_1;
	end if;
	end process;

	--***** first latch, second signal **********--
	process(add1lat,TEMP_SIGNAL_2)
	begin
	if (add1lat = '1') then
		out1_latchsecond <= TEMP_SIGNAL_2;
	end if;
	end process;

	--***** second latch, second signal *********--
	process(add2lat,TEMP_SIGNAL_3)
	begin
	if (add2lat = '1') then
		out2_latchsecond <= TEMP_SIGNAL_3;
	end if;
	end process;

--*************************************************************************************************--
				
--************* end of processes: modification of the signals *************************************--
	
	--********* truncation at the output of the multipliers ***************************************--
	TEMP_SIGNAL_0 <= (busaddfirst1(OUT_MULT_PREFILT-1) & busaddfirst1(OUT_MULT_PREFILT-TRUNCATION-1 downto OUT_MULT_PREFILT-TRUNCATION-FFT_DATA_WIDTH+1));
	TEMP_SIGNAL_1 <= (busaddfirst2(OUT_MULT_PREFILT-1) & busaddfirst2(OUT_MULT_PREFILT-TRUNCATION-1 downto OUT_MULT_PREFILT-TRUNCATION-FFT_DATA_WIDTH+1));
	TEMP_SIGNAL_2 <= (busaddsecond1(OUT_MULT_PREFILT-1) & busaddsecond1(OUT_MULT_PREFILT-TRUNCATION-1 downto OUT_MULT_PREFILT-TRUNCATION-FFT_DATA_WIDTH+1));
	TEMP_SIGNAL_3 <= (busaddsecond2(OUT_MULT_PREFILT-1) & busaddsecond2(OUT_MULT_PREFILT-TRUNCATION-1 downto OUT_MULT_PREFILT-TRUNCATION-FFT_DATA_WIDTH+1));
	--*********************************************************************************************--
	add2ena 			<= adder2enable;
	add2cl 				<= adder2clear;
	multena 			<= multenable;
	writeRAMen 			<= writeRAMena;
	firstinp 			<= firstinput;
	readRAM 			<= readaddressRAM;
	writeRAM 			<= writeaddressRAM;
	addressROM 			<= readaddressROM;
	add2lat 			<= adder2latch;
	add1ena 			<= adder1enable;
	add1cl 				<= adder1clear;
	add1lat 			<= adder1latch;
	secondinp 			<= secondinput;
--**************************************************************************************************--
END; 