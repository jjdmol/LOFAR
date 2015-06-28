--**************************************************************************************************--
--*****************        MULTIPLEXER										    ********************--
--*****************  Date: 16/12/02 Author: J.L. 								********************--
--*****************  This file is a 2 to 1 multiplexer for the prefilters		********************--
--*****************  NOTE: this file has been developped to be independent 		********************--
--*****************        from any library										********************--
--**************************************************************************************************--

--************************** libraries declaration *************************************************--
LIBRARY ieee;
USE ieee.std_logic_1164.all;
LIBRARY lpm;
USE lpm.lpm_components.all;
LIBRARY work;
USE work.pack.all;
--*************************************************************************************************--

--*********************** entity ******************************************************************--
ENTITY mux_prefilter IS
	PORT
	(
		infirst		: IN STD_LOGIC_VECTOR (FFT_DATA_WIDTH-1 DOWNTO 0);	-- first input
		insecond	: IN STD_LOGIC_VECTOR (FFT_DATA_WIDTH-1 DOWNTO 0);	-- second input
		sel			: IN STD_LOGIC ;						-- selector
		result		: OUT STD_LOGIC_VECTOR (FFT_DATA_WIDTH-1 DOWNTO 0)	-- multiplexer output
	);
END mux_prefilter;
--*************************************************************************************************--

--*********************** architecture ************************************************************--
ARCHITECTURE archimux OF mux_prefilter IS
	--*** no signals ***--
BEGIN
	PROCESS(sel)
	--*** no variables **--
	BEGIN
		IF sel='0' THEN
			result <= infirst;	-- selection of the first input
		ELSE
			result <= insecond;	-- selection of the second input
		END IF;
	END PROCESS;
END archimux;