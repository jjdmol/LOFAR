LIBRARY ieee;
USE ieee.std_logic_1164.all;
LIBRARY lpm;
USE lpm.lpm_components.all;
LIBRARY work;
USE work.pack.all;

ENTITY lpm_rom_prefilter IS
	GENERIC(	file_name	: string := work.pack.FILE_ROM_1
			);
	PORT
	(
		address				: IN STD_LOGIC_VECTOR (BIT_ROM-1 DOWNTO 0);
		inclock				: IN STD_LOGIC ;
		outclock			: IN STD_LOGIC ;
		q					: OUT STD_LOGIC_VECTOR (BIT_IN-1 DOWNTO 0)
	);
END lpm_rom_prefilter;

ARCHITECTURE SYN OF lpm_rom_prefilter IS

	SIGNAL sub_wire0		: STD_LOGIC_VECTOR (BIT_IN-1 DOWNTO 0);

	COMPONENT lpm_rom
	GENERIC (
		lpm_width			: NATURAL;
		lpm_widthad			: NATURAL;
		lpm_address_control	: STRING;
		lpm_outdata			: STRING;
		lpm_file			: STRING;
		lpm_type			: STRING
	);
	PORT (
			outclock		: IN STD_LOGIC ;
			address			: IN STD_LOGIC_VECTOR (BIT_ROM-1 DOWNTO 0);
			inclock			: IN STD_LOGIC ;
			q				: OUT STD_LOGIC_VECTOR (BIT_IN-1 DOWNTO 0)
	);
	END COMPONENT;

BEGIN
	q    					<= sub_wire0(BIT_IN-1 DOWNTO 0);

	lpm_rom_component : lpm_rom
	GENERIC MAP (
		lpm_width 			=> BIT_IN,
		lpm_widthad 		=> BIT_ROM,
		lpm_address_control => "REGISTERED",
		lpm_outdata 		=> "REGISTERED",
		lpm_file 			=> file_name,
		lpm_type 			=> "LPM_ROM"
	)
	PORT MAP (
		outclock 			=> outclock,
		address 			=> address,
		inclock 			=> inclock,
		q 					=> sub_wire0
	);

END SYN;