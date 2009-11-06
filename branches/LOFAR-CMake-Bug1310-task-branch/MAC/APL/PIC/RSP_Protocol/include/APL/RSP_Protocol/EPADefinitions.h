#include <Common/LofarTypes.h>
#include <Common/lofar_complex.h>
#include <APL/EPA_Protocol/MEPHeader.h>
#include <APL/EPA_Protocol/MEPData.h>

namespace LOFAR {
  namespace EPA_Protocol {

#undef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))

//
// Protocol constants.
//

//@{
//
// Constants used in the EPA protocol.
// These constants are all derived from the leading
// constants in MEPHeader.h.
//
static const int N_AP            = 4; // number of AP's per board
static const int N_COEF          = MIN(MEPHeader::FRAGMENT_SIZE, MEPHeader::BF_XROUT_SIZE) / sizeof(int16);
static const int BF_N_FRAGMENTS  = 1; // MEPHeader::BF_XROUT_SIZE / MEPHeader::FRAGMENT_SIZE
static const int N_WAVE_SAMPLES  = MEPHeader::DIAG_WGXWAVE_SIZE / sizeof(int32);
static const int BST_N_FRAGMENTS = 1; // MEPHeader::BST_POWER_SIZE / MEPHeader::FRAGMENT_SIZE;
static const int SST_N_FRAGMENTS = MEPHeader::SST_POWER_SIZE / MEPHeader::FRAGMENT_SIZE;
static const int N_SST_STATS     = MEPHeader::FRAGMENT_SIZE / sizeof(uint32);
static const int N_BST_STATS     = MIN(MEPHeader::FRAGMENT_SIZE, MEPHeader::BST_POWER_SIZE) / sizeof(uint32);
static const int XST_N_FRAGMENTS = (MEPHeader::N_GLOBAL_XLETS > MEPHeader::MAX_XLETS_PER_FRAGMENT ? 2 : 1);
static const int N_XST_STATS     = MEPHeader::XST_FRAGMENT_SIZE / sizeof(uint32);
//@}

//
// RSR(0x01) 0x00: get boardstatus
//
// struct: BoardStatus containting several other structs
//
typedef struct RSPStatus // total size 8 bytes
{
#if 1
	uint8	voltage_1_5; // measured voltage on 1.5V circuit
	uint8	voltage_3_3; // measured voltage on 3.3V circuit
#else
	uint8	voltage_1_2; // measured voltage on 1.2V circuit 2.5/192 V/unit
	uint8	voltage_2_5; // measured voltage on 2.5V circuit 3.3/192 V/unit
	uint8	voltage_3_3; // measured voltage on 3.3V circuit 5.0/192 V/unit
	uint8	pcb_temp;    // board temperature
#endif
	uint8	bp_temp;     // board processor temperature
	uint8	ap0_temp;    // antenna processor 0 temperature
	uint8	ap1_temp;    // antenna processor 1 temperature
	uint8	ap2_temp;    // antenna processor 2 temperature
	uint8	ap3_temp;    // antenna processor 3 temperature
	uint8	bp_clock;    // board processor clock speed (125/160/200Mhz)
} __attribute__ ((packed));

typedef struct ETHStatus // total size 12 bytes
{
	uint32	nof_frames; // number of ETH frame received
	uint32	nof_errors; // number of incorrect ETH frames
	uint8	last_error; // error status of last received ETH frame
	uint8	ffi0;
	uint8	ffi1;
	uint8	ffi2;
};

typedef struct MEPStatus // total size 4 bytes
{
	uint16	seqnr;  // sequence number of previously received message
	uint8	error;  // error status of previously received message
	uint8	ffi0;
};

typedef struct DIAGStatus // total size 24 bytes
{
	uint8	interface;     // Interface under test
	uint8	mode;          // Test mode
	uint16	ri_errors;     // Number of detected errors
	uint16	rcux_errors;   // Number of detected errors
	uint16	rcuy_errors;   // Number of detected errors
	uint16	lcu_errors;    // Number of detected errors
	uint16	cep_errors;    // Number of detected errors
	uint16	serdes_errors; // Number of detected errors
	uint16	ap0_ri_errors; // Number of detected errors
	uint16	ap1_ri_errors; // Number of detected errors
	uint16	ap2_ri_errors; // Number of detected errors
	uint16	ap3_ri_errors; // Number of detected errors
	uint16	ffi;
};

typedef struct BSStatus // total size 64 bytes
{
	uint32	ext_count;     // number of cycles between external and internal sync, reset 
	uint32	sync_count;    // number of internal sync events
	uint32	sample_offset; // number of samples processed since last reset (wraps)
	uint32	slice_count;   // number of slices in the previous internal sync interval
};

typedef struct APStatus // total size 48 bytes
{
	struct {
	  uint8	pllx:1;
	  uint8	plly:1;
	  uint8	ffi :6;
	};
	uint8	ffi0;
	uint8	ffi1;
	uint8	ffi2;
	uint32	nof_overflowx;
	uint32	nof_overflowy;
};

typedef struct RSUStatus // total size 4 bytes
{
	uint8	rdy:1;  // 0 = Configuration ongoing, 1 = Configuration done
	uint8	err:1;  // 0 = Configuration was succesful, 1 = Error during configuration
	uint8	fpga:1; // 0 = BP was reconfigured, 1 = AP was reconfigured
	uint8	im:1;   // 0 = Factory image, 1 = Application image
	uint8	trig:3; // 000 = Board reset
					// 001 = User reconfiguration request
					// 010 = User reset request
					// 100 = Watchdog timer timeout
	uint8	ffi:1;  // not used

	uint8	ffi1;	// align to 4 byte
	uint8	ffi2;
	uint8	ffi3;
};

// Finally the BoardStatus structure that glues it all together.
typedef struct BoardStatus // total size 164 bytes
{
	RSPStatus	rsp;
	ETHStatus	eth;
	MEPStatus	mep;
	DIAGStatus	diag;
	BSStatus	ap0_sync;
	BSStatus	ap1_sync;
	BSStatus	ap2_sync;
	BSStatus	ap3_sync;
	APStatus	ap0_rcu;
	APStatus	ap1_rcu;
	APStatus	ap2_rcu;
	APStatus	ap3_rcu;
	RSUStatus	cp_status;

} __attribute__ ((packed));

//
// RSR(0x01) 0x01: get version
//
typedef struct RSRVersion // total size 2 bytes
{
	uint8	rsp;         // RSP hardware version, undefined for AP's
	uint8	fpga_min :4; // FPGA firmware version (minor)
	uint8	fpga_maj :4; // FPGA firmware version (major)
};

//
// RSU(0x02) 0x01: read/write flash
//
typedef struct RSUFlashRW	// total size 1024 bytes
{
	uint8	flash[MEPHeader::RSU_FLASHRW_SIZE];
};

//
// RSU(0x02) 0x02: Flash erase
//
// Note: The offsetfield of the MEPheader is used to address the sectors.
// 1 sector = 128 blocks = 128 x 1024 bytes.
// To erase sector 5 of the flash the offset field of the MEPheader 
// should contain the value 5 x 128 = 2480. (sectors and blocks start 
// counting at 0). Therefor the RSUFlashErase structure can always the value 1.
//
typedef struct RSUFlashErase	// total size 1 byte
{
	uint8	erase;	// = 1
};

//
// RSU(0x02) 0x03: Reconfigure FPGA
//
typedef struct RSUReconfig	// total size 1 byte
{
	uint8	page      :3;	// page selection: selects which program to load
	uint8	ffi       :4;
	uint8	fpgaselect:1;	// fpga selection (0 == reconfigure AP, 1 == reconfigure BP)
};

//
// RSU(0x02) 0x04: clear control
//
typedef struct RSUReset	// total size 1 byte
{
	uint8	sync :1;	// send sync pulse to all FPGA's
	uint8	clear:1;	// clear all FPGA's
	uint8	reset:1;	// reset all FPGA's and reconfigure BP with factory image
	uint8	ffi  :5;
};

//
// DIAG(0x03) 0x00: Waveform generator settings X
// DIAG(0x03) 0x01: Waveform generator settings Y
//
typedef struct WGSettings	// total size 12 bytes
{
	uint8	mode;			// 0:off, 1:calc, 3:single, 5:repeat
	uint8	phase;			// 0..255: 0.. 2pi
	uint16	nof_samples;	// #samples in waveform buffer <= 1024
	uint32	frequency;		// Fgen / Fsys x 2^32
	uint32	amplitude;		// 0..2^15 (32768)
};

//
// DIAG(0x03) 0x02: User waveform X
// DIAG(0x03) 0x03: User waveform Y
//
// a WGwave structure can contain only 1024/4=256 points.
// to write a wave of 1024 samples 4 messages should be send
// using the offset in the MEPheader to position them right.
// (offset field should be resp. 0, 1024, 2048, 3072).
//
typedef struct WGWave		// total size 1024
{
	uint32	wave[MEPHeader::DIAG_WGXWAVE_SIZE / sizeof(uint32)];
};

//
// DIAG(0x03) 0x04: Bypass enable
//
typedef struct DIAGBypass	// total size 1 byte
{
	uint8	ffi0:1;
	uint8	pfs :1;  // bypass PFS processing
	uint8	pft :1;  // bypass PFT processing
	uint8	bf  :1;  // bypass BF  processing
	uint8	ffi1:4;
};

//
// DIAG(0x03) 0x05: Data processing results
//
typedef struct DIAGResults	// total size 4096 bytes
{
	uint32	result[MEPHeader::DIAG_RESULTS_SIZE / sizeof(uint32)];
};

//
// DIAG(0x03) 0x06: SelfTest
//
typedef struct DIAGSelftest	// total size 1 byte
{
	uint8	interface;	// 0:ring, 1:rcux, 2:rcuy, 3:lcu, 4:cep, 5:serdes
	uint8	mode;		// 1:local loopback, 2: remote loopback
						// 3:transmit, 4:receive, 5:lane, 6:lane single
	uint8	duration;	// 0: debug, 1:short, 2:normal, 3:long
	uint8	lane;		// LVDS bit lane index
};

//
// SS(0x04) 0x00: Subband selection
//
typedef struct SubbandSelect // total size (4+54) * 4 for FTS2 test (232)
							 // total size (4+210) * 4 final (856)
{
	struct SBselect_t {
		uint16		x;
		uint16		y;
	};
	SBselect_t	xlet    [MEPHeader::N_XLETS];
	SBselect_t	beamlet [MEPHeader::N_BEAMLETS];
};

//
// BF(0x05) 0x00-0x03: Coefficients for XR,XI,YR,YI output
//
typedef struct BeamletWeights // total size 8 x 128 - 1024
{
	struct BFcoeff_t {
		std::complex<uint16>	x;
		std::complex<uint16>	y;
	};
	BFcoeff_t	 xlet    [MEPHeader::N_XLETS];
	BFcoeff_t	 beamlet [MEPHeader::N_BEAMLETS];
};

//
// Define general struct used by all Statistic commands
//
typedef struct powerStat {
	uint32		x;
	uint32		y;
};

//
// BST(0x06) 0x00: Beamformer statistics - power --> N_BEAMLETS
//
typedef struct BFStatistics 	// total size 1024
{
	powerStat	beam[MEPHeader::N_BEAMLETS];
};

//
// SST(0x07) 0x00: Subband statistics - power	--> N_SUBBANDS
//
typedef struct SBStatistics 	// total size 1024
{
	powerStat	subband[MEPHeader::N_SUBBANDS];
};

//
// RCU(0x08) 0x00: RCU Handler settings
//
typedef struct RCUHandler		// total size 3
{
	uint8	spec_inv_x   :1; // spectral inversion bit for X-receiver
	uint8	spec_inv_y   :1; // spectral inversion bit for Y-receiver
	uint8	ffi0         :6;
	uint8	input_delay_x:7; // input delay for X-receiver
	uint8	ffi1         :1;
	uint8	input_delay_y:7; // input delay for Y-receiver
	uint8	ffi2         :1;
};

//
// RCU(0x08) 0x01,0x03: RCU protocol list
//
typedef struct RCUI2CCmdList	// total size 128
{
	uint8	command[MEPHeader::RCU_PROTOCOLX_SIZE];
};

//
// RCU(0x08) 0x02,0x04: RCU protocol results
//
typedef struct RCUI2CResult	// total size 128
{
	uint8	result[MEPHeader::RCU_RESULTSX_SIZE];
};

//
// CR(0x09) 0x00: Clock Control
//
typedef struct CRControl	// total size 1
{
	uint8	sync        :1; // generate soft sync pulse for a specific FPGA
	uint8	clear       :1; // soft clear a specific FPGA
	uint8	sync_disable:1; // disable the external sync pulse
	uint8	ffi         :5;
};

//
// XST(0x0A) 0x00: Crosslet statistics
//
typedef struct XletStatistics // total size 3424
{
	typedef struct XletStat {
		std::complex<uint32>		x;
		std::complex<uint32>		y;
	};

	XletStat		Crosslet [MEPHeader::N_XLETS];
	XletStat		Beamlet  [MEPHeader::N_BEAMLETS];
};

//
// CDO(0x0B) 0x00: CEP Data Output Registers
//
typedef struct CDOSettings
{
	uint32		station_id;
	uint32		configuration_id;
	uint32		format;	
	uint32		antenna_id;
    uint8		destination_mac[ETH_ALEN];
} __attribute__((packed));

//
// CDO(0x0B) 0x01: CDO beamlet selection
//
typedef struct CDOBeamlets
{
	typedef struct single_bit {
		uint8	i:1;
	};
	single_bit	beamlet[MEPHeader::N_BEAMLETS];
//	uint8	beamlet:1 [MEPHeader::N_BEAMLETS];
} __attribute__((packed));

//
// BS(0x0C) 0x00: Samples per Sync
//
typedef struct BSSettings // total size 4
{
	uint32	samples_per_sync;
};

//
// SERDES(0x0D)
//
// NOT YET DEFINED

//
// TDS(0x0E) 0x01: TDS I2C command
//
typedef struct TDSI2CCmdList	// total size 128
{
	uint8	command[MEPHeader::TDS_PROTOCOL_SIZE];
};

//
// TDS(0x0E) 0x02: TDS protocol results
//
typedef struct TDSI2CResult	// total size 128
{
	uint8	result[MEPHeader::TDS_RESULTS_SIZE];
};

//
// TBB(0x0F)
//
// NOT YET DEFINED

} // namespace EPA_Protocol
} // namespace LOFAR
