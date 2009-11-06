LIBRARY ieee;
USE ieee.std_logic_1164.all;

LIBRARY lpm;
USE lpm.lpm_components.all;

LIBRARY work;
USE work.pack.all;

ENTITY lpm_ram_dp_prefilter IS
	PORT
	(
		data						: IN STD_LOGIC_VECTOR (BIT_IN-1 DOWNTO 0);
		wraddress					: IN STD_LOGIC_VECTOR (BIT_RAM_DECIM-1 DOWNTO 0);
		rdaddress					: IN STD_LOGIC_VECTOR (BIT_RAM_DECIM-1 DOWNTO 0);
		wren						: IN STD_LOGIC  := '1';
		wrclock						: IN STD_LOGIC ;
		rdclock						: IN STD_LOGIC ;
		q							: OUT STD_LOGIC_VECTOR (BIT_IN-1 DOWNTO 0)
	);
END lpm_ram_dp_prefilter;

ARCHITECTURE SYN OF lpm_ram_dp_prefilter IS

	SIGNAL sub_wire0				: STD_LOGIC_VECTOR (BIT_IN-1 DOWNTO 0);

	COMPONENT lpm_ram_dp
	GENERIC (
		lpm_width					: NATURAL;
		lpm_widthad					: NATURAL;
		rden_used					: STRING;
		intended_device_family		: STRING;
		lpm_indata					: STRING;
		lpm_wraddress_control		: STRING;
		lpm_rdaddress_control		: STRING;
		lpm_outdata					: STRING;
		use_eab						: STRING;
		lpm_type					: STRING
	);
	PORT (
			rdclock					: IN STD_LOGIC ;
			wren					: IN STD_LOGIC ;
			wrclock					: IN STD_LOGIC ;
			q						: OUT STD_LOGIC_VECTOR (BIT_IN-1 DOWNTO 0);
			data					: IN STD_LOGIC_VECTOR (BIT_IN-1 DOWNTO 0);
			rdaddress				: IN STD_LOGIC_VECTOR (BIT_RAM_DECIM-1 DOWNTO 0);
			wraddress				: IN STD_LOGIC_VECTOR (BIT_RAM_DECIM-1 DOWNTO 0)
	);
	END COMPONENT;

BEGIN
	q    							<= sub_wire0(BIT_IN-1 DOWNTO 0);

	lpm_ram_dp_component : lpm_ram_dp
	GENERIC MAP (
		lpm_width 					=> BIT_IN,
		lpm_widthad 				=> BIT_RAM_DECIM,
		rden_used 					=> "FALSE",
		intended_device_family 		=> "UNUSED",
		lpm_indata 					=> "REGISTERED",
		lpm_wraddress_control 		=> "REGISTERED",
		lpm_rdaddress_control 		=> "REGISTERED",
		lpm_outdata			 		=> "UNREGISTERED",
		use_eab 					=> "ON",
		lpm_type 					=> "LPM_RAM_DP"
	)
	PORT MAP (
		rdclock 					=> rdclock,
		wren 						=> wren,
		wrclock 					=> wrclock,
		data 						=> data,
		rdaddress 					=> rdaddress,
		wraddress 					=> wraddress,
		q 							=> sub_wire0
	);

END SYN;