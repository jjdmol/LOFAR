LIBRARY ieee;
USE ieee.std_logic_1164.all;
LIBRARY lpm;
USE lpm.lpm_components.all;
LIBRARY work;
USE work.pack.all;

ENTITY lpm_mult_prefilter IS
	PORT
	(
		dataa				: IN STD_LOGIC_VECTOR (BIT_IN-1 DOWNTO 0);
		datab				: IN STD_LOGIC_VECTOR (BIT_IN-1 DOWNTO 0);
		clock				: IN STD_LOGIC ;
		aclr				: IN STD_LOGIC ;
		clken				: IN STD_LOGIC ;
		result				: OUT STD_LOGIC_VECTOR (OUT_MULT_PREFILT-1 DOWNTO 0)
	);
END lpm_mult_prefilter;

ARCHITECTURE SYN OF lpm_mult_prefilter IS

	SIGNAL sub_wire0		: STD_LOGIC_VECTOR (OUT_MULT_PREFILT-1 DOWNTO 0);

	COMPONENT lpm_mult
	GENERIC (
		lpm_widtha			: NATURAL;
		lpm_widthb			: NATURAL;
		lpm_widthp			: NATURAL;
		lpm_widths			: NATURAL;
		lpm_type			: STRING;
		lpm_representation	: STRING;
		lpm_hint			: STRING;
		lpm_pipeline		: NATURAL
	);
	PORT (
			dataa			: IN STD_LOGIC_VECTOR (BIT_IN-1 DOWNTO 0);
			datab			: IN STD_LOGIC_VECTOR (BIT_IN-1 DOWNTO 0);
			clken			: IN STD_LOGIC ;
			aclr			: IN STD_LOGIC ;
			clock			: IN STD_LOGIC ;
			result			: OUT STD_LOGIC_VECTOR (OUT_MULT_PREFILT-1 DOWNTO 0)
	);
	END COMPONENT;

BEGIN
	result    				<= sub_wire0(OUT_MULT_PREFILT-1 DOWNTO 0);

	lpm_mult_component : lpm_mult
	GENERIC MAP (
		lpm_widtha 			=> BIT_IN,
		lpm_widthb 			=> BIT_IN,
		lpm_widthp 			=> OUT_MULT_PREFILT,
		lpm_widths 			=> 2*BIT_IN,
		lpm_type 			=> "LPM_MULT",
		lpm_representation 	=> "SIGNED",
		lpm_hint 			=> "MAXIMIZE_SPEED=1",
		lpm_pipeline 		=> 1
	)
	PORT MAP (
		dataa 				=> dataa,
		datab 				=> datab,
		clken 				=> clken,
		aclr 				=> aclr,
		clock 				=> clock,
		result 				=> sub_wire0
	);

END SYN;