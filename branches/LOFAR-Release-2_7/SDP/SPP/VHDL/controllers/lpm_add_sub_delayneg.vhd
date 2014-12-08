LIBRARY ieee;
USE ieee.std_logic_1164.all;
LIBRARY lpm;
USE lpm.lpm_components.all;
library work;
use work.pack.all;

ENTITY lpm_add_sub_delayneg IS
	PORT
	(
		dataa		: IN STD_LOGIC_VECTOR (FFT_DATA_WIDTH-1 DOWNTO 0);
		datab		: IN STD_LOGIC_VECTOR (FFT_DATA_WIDTH-1 DOWNTO 0);
		clock		: IN STD_LOGIC ;
		result		: OUT STD_LOGIC_VECTOR (FFT_DATA_WIDTH-1 DOWNTO 0)
	);
END lpm_add_sub_delayneg;

ARCHITECTURE SYN OF lpm_add_sub_delayneg IS

	SIGNAL sub_wire0	: STD_LOGIC_VECTOR (FFT_DATA_WIDTH-1 DOWNTO 0);

	COMPONENT lpm_add_sub
	GENERIC (
		lpm_width		: NATURAL;
		lpm_direction	: STRING;
		lpm_type		: STRING;
		lpm_hint		: STRING;
		lpm_pipeline	: NATURAL
	);
	PORT (
			dataa	: IN STD_LOGIC_VECTOR (FFT_DATA_WIDTH-1 DOWNTO 0);
			datab	: IN STD_LOGIC_VECTOR (FFT_DATA_WIDTH-1 DOWNTO 0);
			clock	: IN STD_LOGIC ;
			result	: OUT STD_LOGIC_VECTOR (FFT_DATA_WIDTH-1 DOWNTO 0)
	);
	END COMPONENT;

BEGIN
	result    <= sub_wire0(FFT_DATA_WIDTH-1 DOWNTO 0);

	lpm_add_sub_component : lpm_add_sub
	GENERIC MAP (
		lpm_width => FFT_DATA_WIDTH,
		lpm_direction => "SUB",
		lpm_type => "LPM_ADD_SUB",
		lpm_hint => "ONE_INPUT_IS_CONSTANT=NO",
		lpm_pipeline => 1
	)
	PORT MAP (
		dataa => dataa,
		datab => datab,
		clock => clock,
		result => sub_wire0
	);

END SYN;