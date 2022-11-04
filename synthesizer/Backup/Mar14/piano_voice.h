// This is the header file for the piano script
// Hopefully we'll have a structure that defines all the pre-initialized variables we'll
// be using.
//

// DEBUG
#include <time.h>
#include <sys/time.h>

// Structures ----------------------------------------------------------

// Soundboard Structure
struct sound_board_s{
	double abs;
	int lengths[8];
	double aa[8];
	double gsb[8];
	double a1sb[8];
	double osb[2][8];
	double bsb[8];
	double csb[8];
	double A[8][8];
	int c1sb;
	int c3sb;
	double f0s;
	double isb[8];
	double d1[37];
	double d2[87];
	double d3[181];
	double d4[271];
	double d5[359];
	double d6[592];
	double d7[687];
	double d8[721];
	int dptr[8]; 	// not actually pointers ; -)
};

// Lower Level Note structures
struct hammer_s{
	double K;	// Stiffness Force
	double fsh; // Hammer sampling rate
	double m;	// Mass of Hammer
	double p;	// Stiffness Exponent
};

struct string_s{
	int Min;	// Input to waveguide
	int Mup;	// Length of Upper delay line
	int Mlow[4];	// Lengths of Lower delay lines
	int fs;		// sampling frequency
	double Z;	// String Impedance
	double tau;	// Transmission coefficient
	double Ref;	// Reflection Coefficient
};

struct LP_s{
	double glp;	// gain of LPF
	double a1;	// a
};

struct tune_s{
	double a1;
};

struct allpass_s{
	double a1;
	double a2;
};

struct control_s{
	double MaxV;
	double MinV;
	double scale;
};

// Top Level Note Structures
struct note_s{
	struct hammer_s hammer;
	struct string_s  string;
	struct LP_s	  LP[4];
	struct tune_s    tunning[4];
	struct allpass_s allpass;
	struct control_s control;
	int M;		// number of cycles through all-pass filter
	int Ns;		// Number of strings
	
	
};

struct sample_create_s{
	struct note_s note;		// Note being played
	double *channel;		// buffer for samples
	int active_flag;		// flag if sample_create is active
	int cycle_count;		// how many cylces it will remain active
	double vh[2];			// Hammer Velocity
	double * delay_up[4];	// Upper Delay Line
	double * delay_low[4];	// Lower delay line
	double vin_up[4][2];	// Velocity, upper delay line
	double vin_low[4][2];	// Lower delay line
	double temp[4][8][4];	// temp variables
	double Vi;              // hammer velocity
	double hammer;          // hammer
	double del_y[2];        // change in position
	double Fout[2];         // output forces
	int dup_mup;		// pointer for upper delay lines
	int dup_mzer;
	int dup_min1;
	int dup_min2;
	int dlow_mlow[4];	// pointer for lower delay lines
	int dlow_min1[4];	
	int dlow_min2[4];
	int dlow_mzer[4];
	double f_sc;	// this note's 'sampling rate' as a fraction of fs
	int note_num;
	
	// debug stuff
	int debug_flag[2];
	struct timeval tv[2];
	double t[2];
};


// Functions -------------------------------------------------------------

struct sound_board_s sound_board_init(){
// this one 
	int i,k;
	double b=0, g=0;
	
	struct sound_board_s sb;
	sb.abs = -1.0/4.0;
	sb.lengths[0] = 37;
	sb.lengths[1] = 87;
	sb.lengths[2] = 181;
	sb.lengths[3] = 271;
	sb.lengths[4] = 359;
	sb.lengths[5] = 592;
	sb.lengths[6] = 687;
	sb.lengths[7] = 721;
	sb.aa[0] = sb.abs;
	sb.aa[1] = sb.abs+1;
	sb.aa[2] = sb.abs;
	sb.aa[3] = sb.abs;
	sb.aa[4] = sb.abs;
	sb.aa[5] = sb.abs;
	sb.aa[6] = sb.abs;
	sb.aa[7] = sb.abs;
	sb.c1sb = 20;
	sb.c3sb = 20;
	for(i=0;i<8;i++){		// Initialize all to 0
		sb.gsb[i]		= 0;
		sb.a1sb[i]		= 0;
		sb.osb[0][i]	= 0;
		sb.osb[1][i]	= 0;
		sb.csb[i]		= 0;
		sb.bsb[i]		= 0;
		sb.isb[i]		= 0;
		sb.dptr[i]		= 0;
	}
	
	for(i=0;i<8;i++){
		sb.bsb[i] = 1;
		
		if(i%2 ==0){
			sb.csb[i] = 1.0/8.0;
		} else {
			sb.csb[i] = -1.0/8.0;
		}
		sb.f0s = 44100.0/sb.lengths[i];		// Should be static @ 44100 ??
		g = 1.0 - sb.c1sb/sb.f0s;
		b = 4.0 * sb.c3sb+sb.f0s;
		sb.a1sb[i] = (-b + sqrt(b*b - 16.0*sb.c3sb*sb.c3sb))/(4.0 * sb.c3sb);
		sb.gsb[i] = g*(1+sb.a1sb[i]);
	
		for(k=0;k<8;k++){
			if	(k==i+1)  sb.A[i][k] = sb.aa[1];
			else		  sb.A[i][k] = sb.aa[0];
		}
	}
	
	for(i=0;i<37;i++)  sb.d1[i] = 0;
	for(i=0;i<87;i++)  sb.d2[i] = 0;
	for(i=0;i<181;i++) sb.d3[i] = 0;
	for(i=0;i<271;i++) sb.d4[i] = 0;
	for(i=0;i<359;i++) sb.d5[i] = 0;
	for(i=0;i<592;i++) sb.d6[i] = 0;
	for(i=0;i<687;i++) sb.d7[i] = 0;
	for(i=0;i<721;i++) sb.d8[i] = 0;
	 
	
	return sb;

}

void note_init(struct note_s * notes) {
// This one initializes all the notes for the piano

// A0
	notes[0].hammer.K		 = 83786247.2554055450000000;
	notes[0].hammer.fsh		 = 44100;
	notes[0].hammer.m		 = 0.0269728449281802;
	notes[0].hammer.p		 = 2.0035845258791265;
	notes[0].string.Min		 = 10;
	notes[0].string.Mup		 = 394;
	notes[0].string.Mlow[0]	 	 = 385;
	notes[0].string.fs		 = 22050;
	notes[0].string.Z		 = 7.4705286630239307;
	notes[0].string.tau 	 	 = 0.9962716987638005;
	notes[0].string.Ref		 = 0.0037283012361996;
	notes[0].LP[0].glp		 = 0.7496321683986990;
	notes[0].LP[0].a1		 = -0.2434904722581937;
	notes[0].tunning[0].a1	 	 = 0;
	notes[0].allpass.a1		 = -0.9543117820761717;
	notes[0].allpass.a2		 = 0.9113650309180760;
	notes[0].Ns			 = 1;
	notes[0].M			 = 4;
	notes[0].control.MaxV		 = 20;
	notes[0].control.MinV		 = 1;
	notes[0].control.scale		 = 1;

// AS0
	notes[1].hammer.K		 = 88738436.1018191430000000;
	notes[1].hammer.fsh		 = 44100;
	notes[1].hammer.m		 = 0.0228928463022831;
	notes[1].hammer.p		 = 2.0114893774004514;
	notes[1].string.Min		 = 10;
	notes[1].string.Mup		 = 379;
	notes[1].string.Mlow[0]	         = 370;
	notes[1].string.fs		 = 22050;
	notes[1].string.Z		 = 7.2687859140357878;
	notes[1].string.tau 	         = 0.9963721994693313;
	notes[1].string.Ref		 = 0.0036278005306688;
	notes[1].LP[0].glp		 = 0.7496321683986990;
	notes[1].LP[0].a1		 = -0.2434904722581937;
	notes[1].tunning[0].a1	         = 0;
	notes[1].allpass.a1		 = -0.9542210928101017;
	notes[1].allpass.a2		 = 0.9111944646363253;
	notes[1].Ns		         = 1;
	notes[1].M			 = 4;
	notes[1].control.MaxV		 = 20;
	notes[1].control.MinV		 = 1;
	notes[1].control.scale 		 = 1;

// B0
	notes[2].hammer.K		 = 96466324.0715272870000000;
	notes[2].hammer.fsh		 = 44100;
	notes[2].hammer.m		 = 0.0202286542632413;
	notes[2].hammer.p		 = 2.0229838579176258;
	notes[2].string.Min		 = 10;
	notes[2].string.Mup		 = 358;
	notes[2].string.Mlow[0]	         = 349;
	notes[2].string.fs		 = 22050;
	notes[2].string.Z		 = 7.0022697530803297;
	notes[2].string.tau 	         = 0.9965049833857410;
	notes[2].string.Ref		 = 0.0034950166142590;
	notes[2].LP[0].glp		 = 0.7496321683986990;
	notes[2].LP[0].a1		 = -0.2434904722581937;
	notes[2].tunning[0].a1	         = 0;
	notes[2].allpass.a1		 = -0.9540636465368867;
	notes[2].allpass.a2		 = 0.9108983930898178;
	notes[2].Ns		         = 1;
	notes[2].M			 = 4;
	notes[2].control.MaxV		 = 20;
	notes[2].control.MinV		 = 1;
	notes[2].control.scale 		 = 1;


// C1
	notes[3].hammer.K		 = 104848289.4925714300000000;
	notes[3].hammer.fsh		 = 44100;
	notes[3].hammer.m		 = 0.0185856034399479;
	notes[3].hammer.p		 = 2.0344535075278594;
	notes[3].string.Min		 = 10;
	notes[3].string.Mup		 = 338;
	notes[3].string.Mlow[0]	         = 329;
	notes[3].string.fs		 = 22050;
	notes[3].string.Z		 = 6.7643446687936786;
	notes[3].string.tau 	         = 0.9966235375545388;
	notes[3].string.Ref		 = 0.0033764624454613;
	notes[3].LP[0].glp		 = 0.7496321683986990;
	notes[3].LP[0].a1		 = -0.2434904722581937;
	notes[3].tunning[0].a1	         = 0;
	notes[3].allpass.a1		 = -0.9538792866521453;
	notes[3].allpass.a2		 = 0.9105517918269761;
	notes[3].Ns			 = 1;
	notes[3].M			 = 4;
	notes[3].control.MaxV		 = 20;
	notes[3].control.MinV		 = 1;
	notes[3].control.scale 		 = 1;

// CS1
	notes[4].hammer.K		 = 113993137.1291272500000000;
	notes[4].hammer.fsh		 = 44100;
	notes[4].hammer.m		 = 0.0173743869741583;
	notes[4].hammer.p		 = 2.0459649131071171;
	notes[4].string.Min		 = 10;
	notes[4].string.Mup		 = 319;
	notes[4].string.Mlow[0]		 = 310;
	notes[4].string.fs		 = 22050;
	notes[4].string.Z		 = 6.5501476060865906;
	notes[4].string.tau 		 = 0.9967302804833228;
	notes[4].string.Ref		 = 0.0032697195166771;
	notes[4].LP[0].glp		 = 0.7496321683986990;
	notes[4].LP[0].a1		 = -0.2434904722581937;
	notes[4].tunning[0].a1		 = 0;
	notes[4].allpass.a1		 = -0.9536700312408806;
	notes[4].allpass.a2		 = 0.9101584912497981;
	notes[4].Ns			 = 1;
	notes[4].M			 = 4;
	notes[4].control.MaxV		 = 20;
	notes[4].control.MinV		 = 1;
	notes[4].control.scale 		 = 1;
	
// D1
	notes[5].hammer.K		 = 123876832.4072402400000000;
	notes[5].hammer.fsh		 = 44100;
	notes[5].hammer.m		 = 0.0164159554664840;
	notes[5].hammer.p		 = 2.0574110329565642;
	notes[5].string.Min		 = 10;
	notes[5].string.Mup		 = 301;
	notes[5].string.Mlow[0]	         = 292;
	notes[5].string.fs		 = 22050;
	notes[5].string.Z		 = 6.3586048228022349;
	notes[5].string.tau 	         = 0.9968257435491932;
	notes[5].string.Ref		 = 0.0031742564508069;
	notes[5].LP[0].glp		 = 0.7496321683986990;
	notes[5].LP[0].a1		 = -0.2434904722581937;
	notes[5].tunning[0].a1	         = 0;
	notes[5].allpass.a1		 = -0.9534408859600436;
	notes[5].allpass.a2		 = 0.9097279351081842;
	notes[5].Ns		         = 1;
	notes[5].M			 = 4;
	notes[5].control.MaxV		 = 20;
	notes[5].control.MinV		 = 1;
	notes[5].control.scale 		 = 1;
	
// DS1
	notes[6].hammer.K		 = 134749985.9170595700000000;
	notes[6].hammer.fsh		 = 44100;
	notes[6].hammer.m		 = 0.0156076403395406;
	notes[6].hammer.p		 = 2.0689925772436650;
	notes[6].string.Min		 = 10;
	notes[6].string.Mup		 = 284;
	notes[6].string.Mlow[0]		 = 275;
	notes[6].string.fs		 = 22050;
	notes[6].string.Z		 = 6.1839489894148727;
	notes[6].string.tau 		 = 0.9969127982797820;
	notes[6].string.Ref		 = 0.0030872017202180;
	notes[6].LP[0].glp		 = 0.7496321683986990;
	notes[6].LP[0].a1		 = -0.2434904722581937;
	notes[6].tunning[0].a1		 = 0;
	notes[6].allpass.a1		 = -0.9531905764245826;
	notes[6].allpass.a2		 = 0.9092577648466317;
	notes[6].Ns			 = 1;
	notes[6].M			 = 4;
	notes[6].control.MaxV		 = 20;
	notes[6].control.MinV		 = 1;
	notes[6].control.scale 		 = 1;

// E1
	notes[7].hammer.K		 = 146413763.8854385300000000;
	notes[7].hammer.fsh		 = 44100;
	notes[7].hammer.m		 = 0.0149220109559706;
	notes[7].hammer.p		 = 2.0804202467089898;
	notes[7].string.Min		 = 10;
	notes[7].string.Mup		 = 268;
	notes[7].string.Mlow[0]		 = 259;
	notes[7].string.fs		 = 22050;
	notes[7].string.Z		 = 6.0282886162704585;
	notes[7].string.tau 		 = 0.9969903913891968;
	notes[7].string.Ref		 = 0.0030096086108032;
	notes[7].LP[0].glp		 = 0.7496321683986990;
	notes[7].LP[0].a1		 = -0.2434904722581937;
	notes[7].tunning[0].a1		 = 0;
	notes[7].allpass.a1		 = -0.9529282210274086;
	notes[7].allpass.a2		 = 0.9087651393699705;
	notes[7].Ns			 = 1;
	notes[7].M			 = 4;
	notes[7].control.MaxV		 = 20;
	notes[7].control.MinV		 = 1;
	notes[7].control.scale 		 = 1;

// F1
	notes[8].hammer.K		 = 159160964.2949727800000000;
	notes[8].hammer.fsh		 = 44100;
	notes[8].hammer.m		 = 0.0143158961000053;
	notes[8].hammer.p		 = 2.0919117785857142;
	notes[8].string.Min		 = 10;
	notes[8].string.Mup		 = 253;
	notes[8].string.Mlow[0]		 = 244;
	notes[8].string.fs		 = 22050;
	notes[8].string.Z		 = 5.8865398858640452;
	notes[8].string.tau 		 = 0.9970610551111457;
	notes[8].string.Ref		 = 0.0029389448888544;
	notes[8].LP[0].glp		 = 0.7496321683986990;
	notes[8].LP[0].a1		 = -0.2434904722581937;
	notes[8].tunning[0].a1		 = 0;
	notes[8].allpass.a1		 = -0.9526517929029065;
	notes[8].allpass.a2		 = 0.9082462790104661;
	notes[8].Ns			 = 1;
	notes[8].M			 = 4;
	notes[8].control.MaxV		 = 20;
	notes[8].control.MinV		 = 1;
	notes[8].control.scale 		 = 1;
	

// FS1 - Missing

// G1
	notes[10].hammer.K		 = 188105050.0378597700000000;
	notes[10].hammer.fsh		 = 88200;
	notes[10].hammer.m		 = 0.0132841144485267;
	notes[10].hammer.p		 = 2.1149121425854109;
	notes[10].string.Min		 = 10;
	notes[10].string.Mup		 = 450;
	notes[10].string.Mlow[0]	 = 454;
	notes[10].string.Mlow[1]	 = 442;
	notes[10].string.fs		 = 44100;
	notes[10].string.Z		 = 5.6408375801618718;
	notes[10].string.tau 		 = 0.9971835529899532;
	notes[10].string.Ref		 = 0.0028164470100467;
	notes[10].LP[0].glp		 = 0.7482566965300774;
	notes[10].LP[0].a1		 = -0.2434904722581937;
	notes[10].LP[1].glp		 = 0.7500000000000000;
	notes[10].LP[1].a1		 = -0.2434904722581937;
	notes[10].tunning[0].a1		 = 0.4548773471485953;
	notes[10].tunning[1].a1		 = 0.4429798720075600;
	notes[10].allpass.a1		 = -0.9520716776258444;
	notes[10].allpass.a2		 = 0.9071580246830376;
	notes[10].Ns			 = 2;
	notes[10].M			 = 4;
	notes[10].control.MaxV		 = 10;
	notes[10].control.MinV		 = 1;
	notes[10].control.scale		 = 1;
	
// GS1
	notes[11].hammer.K		 = 204403265.0888533300000000;
	notes[11].hammer.fsh		 = 88200;
	notes[11].hammer.m		 = 0.0128387028058096;
	notes[11].hammer.p		 = 2.1263506513326282;
	notes[11].string.Min		 = 10;
	notes[11].string.Mup		 = 425;
	notes[11].string.Mlow[0]	 = 428;
	notes[11].string.Mlow[1]	 = 417;
	notes[11].string.fs		 = 44100;
	notes[11].string.Z		 = 5.5347566334400726;
	notes[11].string.tau 		 = 0.9972364455835646;
	notes[11].string.Ref		 = 0.0027635544164355;
	notes[11].LP[0].glp		 = 0.7482566965300774;
	notes[11].LP[0].a1		 = -0.2434904722581937;
	notes[11].LP[1].glp		 = 0.7500000000000000;
	notes[11].LP[1].a1		 = -0.2434904722581937;
	notes[11].tunning[0].a1		 = 0.3862728049075780;
	notes[11].tunning[1].a1		 = 0.4388147485387106;
	notes[11].allpass.a1		 = -0.9517754056428607;
	notes[11].allpass.a2		 = 0.9066025700113949;
	notes[11].Ns			 = 2;
	notes[11].M			 = 4;
	notes[11].control.MaxV		 = 8;
	notes[11].control.MinV		 = 1;
	notes[11].control.scale		 = 1;


// A1
	notes[12].hammer.K		 = 222279287.0139198600000000;
	notes[12].hammer.fsh		 = 88200;
	notes[12].hammer.m		 = 0.0124246671935663;
	notes[12].hammer.p		 = 2.1378917916578222;
	notes[12].string.Min		 = 10;
	notes[12].string.Mup		 = 401;
	notes[12].string.Mlow[0]	 = 404;
	notes[12].string.Mlow[1]	 = 393;
	notes[12].string.fs		 = 44100;
	notes[12].string.Z		 = 5.4369686428082131;
	notes[12].string.tau 		 = 0.9972852057414099;
	notes[12].string.Ref		 = 0.0027147942585902;
	notes[12].LP[0].glp		 = 0.7482566965300774;
	notes[12].LP[0].a1		 = -0.2434904722581937;
	notes[12].LP[1].glp		 = 0.7500000000000000;
	notes[12].LP[1].a1		 = -0.2434904722581937;
	notes[12].tunning[0].a1		 = 0.4182861944498294;
	notes[12].tunning[1].a1		 = 0.3729371420823111;
	notes[12].allpass.a1		 = -0.9514750451989918;
	notes[12].allpass.a2		 = 0.9060396780160630;
	notes[12].Ns			 = 2;
	notes[12].M			 = 4;
	notes[12].control.MaxV		 = 10;
	notes[12].control.MinV		 = 1;
	notes[12].control.scale 	 = 1;


// AS1
	notes[13].hammer.K		 = 241807573.8028357300000000;
	notes[13].hammer.fsh		 = 88200;
	notes[13].hammer.m		 = 0.0120390984866240;
	notes[13].hammer.p		 = 2.1494835637439023;
	notes[13].string.Min		 = 10;
	notes[13].string.Mup		 = 379;
	notes[13].string.Mlow[0]	 = 380;
	notes[13].string.Mlow[1]	 = 370;
	notes[13].string.fs		 = 44100;
	notes[13].string.Z		 = 5.3470412837485908;
	notes[13].string.tau 		 = 0.9973300484434253;
	notes[13].string.Ref		 = 0.0026699515565746;
	notes[13].LP[0].glp		 = 0.7482566965300774;
	notes[13].LP[0].a1		 = -0.2434904722581937;
	notes[13].LP[1].glp		 = 0.7500000000000000;
	notes[13].LP[1].a1		 = -0.2434904722581937;
	notes[13].tunning[0].a1		 = 0.3682758848358916;
	notes[13].tunning[1].a1		 = 0.3883317264498187;
	notes[13].allpass.a1		 = -0.9511748720490287;
	notes[13].allpass.a2		 = 0.9054773660762073;
	notes[13].Ns			 = 2;
	notes[13].M			 = 4;
	notes[13].control.MaxV		 = 10;
	notes[13].control.MinV		 = 1;
	notes[13].control.scale 	 = 1;

// B1
	notes[14].hammer.K		 = 262448878.9922285700000000;
	notes[14].hammer.fsh		 = 88200;
	notes[14].hammer.m		 = 0.0116890377103379;
	notes[14].hammer.p		 = 2.1607596106703117;
	notes[14].string.Min		 = 10;
	notes[14].string.Mup		 = 358;
	notes[14].string.Mlow[0]	 = 362;
	notes[14].string.Mlow[1]	 = 347;
	notes[14].string.fs		 = 44100;
	notes[14].string.Z		 = 5.2666220944273121;
	notes[14].string.tau 		 = 0.9973701515572646;
	notes[14].string.Ref		 = 0.0026298484427353;
	notes[14].LP[0].glp		 = 0.7482566965300774;
	notes[14].LP[0].a1		 = -0.2434904722581937;
	notes[14].LP[1].glp		 = 0.7500000000000000;
	notes[14].LP[1].a1		 = -0.2434904722581937;
	notes[14].tunning[0].a1		 = 0.4086092580845171;
	notes[14].tunning[1].a1		 = 0.3916893295531102;
	notes[14].allpass.a1		 = -0.9508872049775841;
	notes[14].allpass.a2		 = 0.9049386964831725;
	notes[14].Ns			 = 2;
	notes[14].M			 = 4;
	notes[14].control.MaxV		 = 10;
	notes[14].control.MinV		 = 1;
	notes[14].control.scale 	 = 1;

// C2
	notes[15].hammer.K		 = 285493637.1547135700000000;
	notes[15].hammer.fsh		 = 88200;
	notes[15].hammer.m		 = 0.0113516698439730;
	notes[15].hammer.p		 = 2.1723452991856815;
	notes[15].string.Min		 = 10;
	notes[15].string.Mup		 = 338;
	notes[15].string.Mlow[0]	 = 339;
	notes[15].string.Mlow[1]	 = 329;
	notes[15].string.fs		 = 44100;
	notes[15].string.Z		 = 5.1903937960559388;
	notes[15].string.tau 		 = 0.9974081662614113;
	notes[15].string.Ref		 = 0.0025918337385887;
	notes[15].LP[0].glp		 = 0.7482566965300774;
	notes[15].LP[0].a1		 = -0.2434904722581937;
	notes[15].LP[1].glp		 = 0.7500000000000000;
	notes[15].LP[1].a1		 = -0.2434904722581937;
	notes[15].tunning[0].a1		 = 0.4828036266057773;
	notes[15].tunning[1].a1		 = 0.3378122304976147;
	notes[15].allpass.a1		 = -0.9505991191181026;
	notes[15].allpass.a2		 = 0.9043994533302359;
	notes[15].Ns			 = 2;
	notes[15].M			 = 4;
	notes[15].control.MaxV		 = 10;
	notes[15].control.MinV		 = 1;
	notes[15].control.scale 	 = 1;


// CS2
	notes[16].hammer.K		 = 309773100.1398096100000000;
	notes[16].hammer.fsh		 = 88200;
	notes[16].hammer.m		 = 0.0110434565895522;
	notes[16].hammer.p		 = 2.1835809157133701;
	notes[16].string.Min		 = 10;
	notes[16].string.Mup		 = 319;
	notes[16].string.Mlow[0]	 = 321;
	notes[16].string.Mlow[1]	 = 311;
	notes[16].string.fs		 = 44100;
	notes[16].string.Z		 = 5.1219184431155451;
	notes[16].string.tau 		 = 0.9974423158408587;
	notes[16].string.Ref		 = 0.0025576841591412;
	notes[16].LP[0].glp		 = 0.7482566965300774;
	notes[16].LP[0].a1		 = -0.2434904722581937;
	notes[16].LP[1].glp		 = 0.7500000000000000;
	notes[16].LP[1].a1		 = -0.2434904722581937;
	notes[16].tunning[0].a1		 = 0.3493503183603975;
	notes[16].tunning[1].a1		 = 0.4688622943692007;
	notes[16].allpass.a1		 = -0.9503299530885486;
	notes[16].allpass.a2		 = 0.9038958150095092;
	notes[16].Ns			 = 2;
	notes[16].M			 = 4;
	notes[16].control.MaxV		 = 10;
	notes[16].control.MinV		 = 1;
	notes[16].control.scale 	 = 1;

// D2
	notes[17].hammer.K		 = 337306861.3165464400000000;
	notes[17].hammer.fsh		 = 88200;
	notes[17].hammer.m		 = 0.0107394965588113;
	notes[17].hammer.p		 = 2.1953028246143864;
	notes[17].string.Min		 = 10;
	notes[17].string.Mup		 = 301;
	notes[17].string.Mlow[0]	 = 303;
	notes[17].string.Mlow[1]	 = 297;
	notes[17].string.fs		 = 44100;
	notes[17].string.Z		 = 5.0554707027183667;
	notes[17].string.tau 		 = 0.9974754553390335;
	notes[17].string.Ref		 = 0.0025245446609664;
	notes[17].LP[0].glp		 = 0.7482566965300774;
	notes[17].LP[0].a1		 = -0.2434904722581937;
	notes[17].LP[1].glp		 = 0.7500000000000000;
	notes[17].LP[1].a1		 = -0.2434904722581937;
	notes[17].tunning[0].a1		 = 0.4753792558603310;
	notes[17].tunning[1].a1		 = 0.3724588290312206;
	notes[17].allpass.a1		 = -0.9500630928282847;
	notes[17].allpass.a2		 = 0.9033966725863323;
	notes[17].Ns			 = 2;
	notes[17].M			 = 4;
	notes[17].control.MaxV		 = 10;
	notes[17].control.MinV		 = 1;
	notes[17].control.scale 	 = 1;
	notes[17].control.MaxV		 = 10;
	notes[17].control.MinV		 = 1;
	notes[17].control.scale 	 = 1;

// DS2
	notes[18].hammer.K		 = 366913602.2360518600000000;
	notes[18].hammer.fsh		 = 88200;
	notes[18].hammer.m		 = 0.0104548940517656;
	notes[18].hammer.p		 = 2.2068843689014872;
	notes[18].string.Min		 = 10;
	notes[18].string.Mup		 = 284;
	notes[18].string.Mlow[0]	 = 287;
	notes[18].string.Mlow[1]	 = 280;
	notes[18].string.fs		 = 44100;
	notes[18].string.Z		 = 4.9941499763727890;
	notes[18].string.tau 		 = 0.9975060388158609;
	notes[18].string.Ref		 = 0.0024939611841392;
	notes[18].LP[0].glp		 = 0.7482566965300774;
	notes[18].LP[0].a1		 = -0.2434904722581937;
	notes[18].LP[1].glp		 = 0.7500000000000000;
	notes[18].LP[1].a1		 = -0.2434904722581937;
	notes[18].tunning[0].a1		 = 0.4906434262219733;
	notes[18].tunning[1].a1		 = 0.3701768229953039;
	notes[18].allpass.a1		 = -0.9498167403351217;
	notes[18].allpass.a2		 = 0.9029360488587124;
	notes[18].Ns			 = 2;
	notes[18].M			 = 4;
	notes[18].control.MaxV		 = 10;
	notes[18].control.MinV		 = 1;
	notes[18].control.scale 	 = 1;
	
// E2
	notes[19].hammer.K		 = 398673151.3071260500000000;
	notes[19].hammer.fsh		 = 88200;
	notes[19].hammer.m		 = 0.0101877955417154;
	notes[19].hammer.p		 = 2.2183120383668120;
	notes[19].string.Min		 = 10;
	notes[19].string.Mup		 = 268;
	notes[19].string.Mlow[0]	 = 272;
	notes[19].string.Mlow[1]	 = 265;
	notes[19].string.fs		 = 44100;
	notes[19].string.Z		 = 4.9372498583037974;
	notes[19].string.tau 		 = 0.9975344183689877;
	notes[19].string.Ref		 = 0.0024655816310123;
	notes[19].LP[0].glp		 = 0.7482566965300774;
	notes[19].LP[0].a1		 = -0.2434904722581937;
	notes[19].LP[1].glp		 = 0.7500000000000000;
	notes[19].LP[1].a1		 = -0.2434904722581937;
	notes[19].tunning[0].a1		 = 0.4013945272350533;
	notes[19].tunning[1].a1		 = 0.4868862037184261;
	notes[19].allpass.a1		 = -0.9495938409911726;
	notes[19].allpass.a2		 = 0.9025194099340734;
	notes[19].Ns			 = 2;
	notes[19].M			 = 4;	
	notes[19].control.MaxV		 = 10;
	notes[19].control.MinV		 = 2;
	notes[19].control.scale 	 = 1;
	
// F2
	notes[20].hammer.K		 = 433382774.3831976100000000;
	notes[20].hammer.fsh		 = 88200;
	notes[20].hammer.m		 = 0.0099316048946994;
	notes[20].hammer.p		 = 2.2298035702435364;
	notes[20].string.Min		 = 37;
	notes[20].string.Mup		 = 253;
	notes[20].string.Mlow[0]	 = 254;
	notes[20].string.Mlow[1]	 = 250;
	notes[20].string.Mlow[2]	 = 245;
	notes[20].string.fs		 = 44100;
	notes[20].string.Z		 = 4.8830735806002608;
	notes[20].string.tau 		 = 0.9975614401265230;
	notes[20].string.Ref		 = 0.0024385598734769;
	notes[20].LP[0].glp		 = 0.7500000000000000;
	notes[20].LP[0].a1		 = -0.2434904722581937;
	notes[20].LP[1].glp		 = 0.7500000000000000;
	notes[20].LP[1].a1		 = -0.2434904722581937;
	notes[20].LP[2].glp		 = 0.7500000000000000;
	notes[20].LP[2].a1		 = -0.2434904722581937;
	notes[20].tunning[0].a1		 = 0.4864425247060247;
	notes[20].tunning[1].a1		 = 0.4972586853395724;
	notes[20].tunning[2].a1		 = 0.4063143353979180;
	notes[20].allpass.a1		 = -0.9493932625763278;
	notes[20].allpass.a2		 = 0.9021446005692616;
	notes[20].Ns			 = 3;
	notes[20].M			 = 4;
	notes[20].control.MaxV		 = 10;
	notes[20].control.MinV		 = 2;
	notes[20].control.scale 	 = 1;
	
// FS2
	notes[21].hammer.K		 = 471177744.1717655100000000;
	notes[21].hammer.fsh		 = 88200;
	notes[21].hammer.m		 = 0.0096863088447134;
	notes[21].hammer.p		 = 2.2413136341328306;
	notes[21].string.Min		 = 35;
	notes[21].string.Mup		 = 239;
	notes[21].string.Mlow[0]	 = 239;
	notes[21].string.Mlow[1]	 = 235;
	notes[21].string.Mlow[2]	 = 231;
	notes[21].string.fs		 = 44100;
	notes[21].string.Z		 = 4.8313316757310814;
	notes[21].string.tau 		 = 0.9975872483629867;
	notes[21].string.Ref		 = 0.0024127516370131;
	notes[21].LP[0].glp		 = 0.7500000000000000;
	notes[21].LP[0].a1		 = -0.2434904722581937;
	notes[21].LP[1].glp		 = 0.7500000000000000;
	notes[21].LP[1].a1		 = -0.2434904722581937;
	notes[21].LP[2].glp		 = 0.7500000000000000;
	notes[21].LP[2].a1		 = -0.2434904722581937;
	notes[21].tunning[0].a1		 = 0.4119273637110305;
	notes[21].tunning[1].a1		 = 0.3831013685462439;
	notes[21].tunning[2].a1		 = 0.4261981298963928;
	notes[21].allpass.a1		 = -0.9492194572405194;
	notes[21].allpass.a2		 = 0.9018199030337128;
	notes[21].Ns			 = 3;
	notes[21].M			 = 4;
	notes[21].control.MaxV		 = 10;
	notes[21].control.MinV		 = 2;
	notes[21].control.scale 	 = 1;


// G2
	notes[22].hammer.K		 = 512195240.9751313900000000;
	notes[22].hammer.fsh		 = 88200;
	notes[22].hammer.m		 = 0.0094517207096985;
	notes[22].hammer.p		 = 2.2528039342432331;
	notes[22].string.Min		 = 33;
	notes[22].string.Mup		 = 225;
	notes[22].string.Mlow[0]	 = 225;
	notes[22].string.Mlow[1]	 = 222;
	notes[22].string.Mlow[2]	 = 219;
	notes[22].string.fs		 = 44100;
	notes[22].string.Z		 = 4.7816919139484391;
	notes[22].string.tau 		 = 0.9976120087026950;
	notes[22].string.Ref		 = 0.0023879912973050;
	notes[22].LP[0].glp		 = 0.7500000000000000;
	notes[22].LP[0].a1		 = -0.2434904722581937;
	notes[22].LP[1].glp		 = 0.7500000000000000;
	notes[22].LP[1].a1		 = -0.2434904722581937;
	notes[22].LP[2].glp		 = 0.7500000000000000;
	notes[22].LP[2].a1		 = -0.2434904722581937;
	notes[22].tunning[0].a1		 = 0.3692090184844816;
	notes[22].tunning[1].a1		 = 0.3507350693837752;
	notes[22].tunning[2].a1		 = 0.3880988008384601;
	notes[22].allpass.a1		 = -0.9490764332128481;
	notes[22].allpass.a2		 = 0.9015527675999580;
	notes[22].Ns			 = 3;
	notes[22].M			 = 4;
	notes[22].control.MaxV		 = 7;
	notes[22].control.MinV		 = 2;
	notes[22].control.scale 	 = 1;
	
// GS2
	notes[23].hammer.K		 = 556573997.3340277700000000;
	notes[23].hammer.fsh		 = 88200;
	notes[23].hammer.m		 = 0.0092275347353978;
	notes[23].hammer.p		 = 2.2642424429904504;
	notes[23].string.Min		 = 31;
	notes[23].string.Mup		 = 213;
	notes[23].string.Mlow[0]	 = 212;
	notes[23].string.Mlow[1]	 = 209;
	notes[23].string.Mlow[2]	 = 206;
	notes[23].string.fs		 = 44100;
	notes[23].string.Z		 = 4.7338033367163348;
	notes[23].string.tau 		 = 0.9976358961323361;
	notes[23].string.Ref		 = 0.0023641038676639;
	notes[23].LP[0].glp		 = 0.7500000000000000;
	notes[23].LP[0].a1		 = -0.2434904722581937;
	notes[23].LP[1].glp		 = 0.7500000000000000;
	notes[23].LP[1].a1		 = -0.2434904722581937;
	notes[23].LP[2].glp		 = 0.7500000000000000;
	notes[23].LP[2].a1		 = -0.2434904722581937;
	notes[23].tunning[0].a1		 = 0.4162287996188068;
	notes[23].tunning[1].a1		 = 0.3697069414742946;
	notes[23].tunning[2].a1		 = 0.4475939593693235;
	notes[23].allpass.a1		 = -0.9489677146288542;
	notes[23].allpass.a2		 = 0.9013497414073228;
	notes[23].Ns			 = 3;
	notes[23].M			 = 4;
	notes[23].control.MaxV		 = 7;
	notes[23].control.MinV		 = 2;
	notes[23].control.scale 	 = 1;
	
// A2
	notes[24].hammer.K		 = 605248997.5838527700000000;
	notes[24].hammer.fsh		 = 88200;
	notes[24].hammer.m		 = 0.0090100208267321;
	notes[24].hammer.p		 = 2.2757835833156443;
	notes[24].string.Min		 = 29;
	notes[24].string.Mup		 = 201;
	notes[24].string.Mlow[0]	 = 200;
	notes[24].string.Mlow[1]	 = 197;
	notes[24].string.Mlow[2]	 = 194;
	notes[24].string.fs		 = 44100;
	notes[24].string.Z		 = 4.6865805043844730;
	notes[24].string.tau 		 = 0.9976594520394182;
	notes[24].string.Ref		 = 0.0023405479605818;
	notes[24].LP[0].glp		 = 0.7500000000000000;
	notes[24].LP[0].a1		 = -0.2434904722581937;
	notes[24].LP[1].glp		 = 0.7500000000000000;
	notes[24].LP[1].a1		 = -0.2434904722581937;
	notes[24].LP[2].glp		 = 0.7500000000000000;
	notes[24].LP[2].a1		 = -0.2434904722581937;
	notes[24].tunning[0].a1		 = 0.4342406257669617;
	notes[24].tunning[1].a1		 = 0.3626812946735937;
	notes[24].tunning[2].a1		 = 0.3960365984291833;
	notes[24].allpass.a1		 = -0.9488954922501516;
	notes[24].allpass.a2		 = 0.9012148865078822;
	notes[24].Ns			 = 3;
	notes[24].M			 = 4;
	notes[24].control.MaxV		 = 7;
	notes[24].control.MinV		 = 2;
	notes[24].control.scale 	 = 0.5;
	
// AS2
	notes[25].hammer.K		 = 657933406.3478637900000000;
	notes[25].hammer.fsh		 = 88200;
	notes[25].hammer.m		 = 0.0088014727343819;
	notes[25].hammer.p		 = 2.2872729607160958;
	notes[25].string.Min		 = 28;
	notes[25].string.Mup		 = 190;
	notes[25].string.Mlow[0]	 = 188;
	notes[25].string.Mlow[1]	 = 186;
	notes[25].string.Mlow[2]	 = 183;
	notes[25].string.fs		 = 44100;
	notes[25].string.Z		 = 4.6402399826278691;
	notes[25].string.tau 		 = 0.9976825683684145;
	notes[25].string.Ref		 = 0.0023174316315855;
	notes[25].LP[0].glp		 = 0.7500000000000000;
	notes[25].LP[0].a1		 = -0.2434904722581937;
	notes[25].LP[1].glp		 = 0.7500000000000000;
	notes[25].LP[1].a1		 = -0.2434904722581937;
	notes[25].LP[2].glp		 = 0.7500000000000000;
	notes[25].LP[2].a1		 = -0.2434904722581937;
	notes[25].tunning[0].a1		 = 0.3757645067358819;
	notes[25].tunning[1].a1		 = 0.4427780708117810;
	notes[25].tunning[2].a1		 = 0.4453103177813607;
	notes[25].allpass.a1		 = -0.9488643600004633;
	notes[25].allpass.a2		 = 0.9011567599004081;
	notes[25].Ns			 = 3;
	notes[25].M			 = 4;
	notes[25].control.MaxV		 = 7;
	notes[25].control.MinV		 = 2;
	notes[25].control.scale 	 = 0.5;

// B2
	notes[26].hammer.K		 = 715464695.5256630200000000;
	notes[26].hammer.fsh		 = 88200;
	notes[26].hammer.m		 = 0.0085994368949419;
	notes[26].hammer.p		 = 2.2988125492012661;
	notes[26].string.Min		 = 26;
	notes[26].string.Mup		 = 179;
	notes[26].string.Mlow[0]	 = 178;
	notes[26].string.Mlow[1]	 = 175;
	notes[26].string.Mlow[2]	 = 172;
	notes[26].string.fs		 = 44100;
	notes[26].string.Z		 = 4.5939700634111551;
	notes[26].string.tau 		 = 0.9977056500120842;
	notes[26].string.Ref		 = 0.0022943499879157;
	notes[26].LP[0].glp		 = 0.7500000000000000;
	notes[26].LP[0].a1		 = -0.2434904722581937;
	notes[26].LP[1].glp		 = 0.7500000000000000;
	notes[26].LP[1].a1		 = -0.2434904722581937;
	notes[26].LP[2].glp		 = 0.7500000000000000;
	notes[26].LP[2].a1		 = -0.2434904722581937;
	notes[26].tunning[0].a1		 = 0.3581691112207669;
	notes[26].tunning[1].a1		 = 0.3409668602363493;
	notes[26].tunning[2].a1		 = 0.3920838276770738;
	notes[26].allpass.a1		 = -0.9488773594639233;
	notes[26].allpass.a2		 = 0.9011810307262101;
	notes[26].Ns			 = 3;
	notes[26].M			 = 4;
	notes[26].control.MaxV		 = 7;
	notes[26].control.MinV		 = 2;
	notes[26].control.scale 	 = 0.5;

// C3
	notes[27].hammer.K		 = 777376695.8935630300000000;
	notes[27].hammer.fsh		 = 88200;
	notes[27].hammer.m		 = 0.0084062178590839;
	notes[27].hammer.p		 = 2.3102370908435033;
	notes[27].string.Min		 = 25;
	notes[27].string.Mup		 = 169;
	notes[27].string.Mlow[0]	 = 168;
	notes[27].string.Mlow[1]	 = 166;
	notes[27].string.Mlow[2]	 = 162;
	notes[27].string.fs		 = 44100;
	notes[27].string.Z		 = 4.5480619031341698;
	notes[27].string.tau 		 = 0.9977285517202794;
	notes[27].string.Ref		 = 0.0022714482797206;
	notes[27].LP[0].glp		 = 0.7500000000000000;
	notes[27].LP[0].a1		 = -0.2434904722581937;
	notes[27].LP[1].glp		 = 0.7500000000000000;
	notes[27].LP[1].a1		 = -0.2434904722581937;
	notes[27].LP[2].glp		 = 0.7500000000000000;
	notes[27].LP[2].a1		 = -0.2434904722581937;
	notes[27].tunning[0].a1		 = 0.3697798487965726;
	notes[27].tunning[1].a1		 = 0.4988177529365031;
	notes[27].tunning[2].a1		 = 0.3519825522502040;
	notes[27].allpass.a1		 = -0.9489370926338061;
	notes[27].allpass.a2		 = 0.9012925618633791;
	notes[27].Ns			 = 3;
	notes[27].M			 = 4;
	notes[27].control.MaxV		 = 7;
	notes[27].control.MinV		 = 2;
	notes[27].control.scale 	 = 0.5;

	
// CS3
	notes[28].hammer.K		 = 845249846.8822364800000000;
	notes[28].hammer.fsh		 = 88200;
	notes[28].hammer.m		 = 0.0082177167361197;
	notes[28].hammer.p		 = 2.3217599793402517;
	notes[28].string.Min		 = 23;
	notes[28].string.Mup		 = 160;
	notes[28].string.Mlow[0]	 = 158;
	notes[28].string.Mlow[1]	 = 156;
	notes[28].string.Mlow[2]	 = 153;
	notes[28].string.fs		 = 44100;
	notes[28].string.Z		 = 4.5013110193975407;
	notes[28].string.tau 		 = 0.9977518743684707;
	notes[28].string.Ref		 = 0.0022481256315292;
	notes[28].LP[0].glp		 = 0.7520000000000000;
	notes[28].LP[0].a1		 = -0.2434904722581937;
	notes[28].LP[1].glp		 = 0.7520000000000000;
	notes[28].LP[1].a1		 = -0.2434904722581937;
	notes[28].LP[2].glp		 = 0.7520000000000000;
	notes[28].LP[2].a1		 = -0.2434904722581937;
	notes[28].tunning[0].a1		 = 0.3875914343375919;
	notes[28].tunning[1].a1		 = 0.4920319883246386;
	notes[28].tunning[2].a1		 = 0.4822505186138061;
	notes[28].allpass.a1		 = -0.9490476083729125;
	notes[28].allpass.a2		 = 0.9014989358200981;
	notes[28].Ns			 = 3;
	notes[28].M			 = 4;
	notes[28].control.MaxV		 = 7;
	notes[28].control.MinV		 = 2;
	notes[28].control.scale 	 = 0.5;

// D3
	notes[29].hammer.K		 = 918460025.8197242000000000;
	notes[29].hammer.fsh		 = 88200;
	notes[29].hammer.m		 = 0.0080365722885251;
	notes[29].hammer.p		 = 2.3331946162722081;
	notes[29].string.Min		 = 22;
	notes[29].string.Mup		 = 151;
	notes[29].string.Mlow[0]	 = 149;
	notes[29].string.Mlow[1]	 = 147;
	notes[29].string.Mlow[2]	 = 144;
	notes[29].string.fs		 = 44100;
	notes[29].string.Z		 = 4.4541559850452472;
	notes[29].string.tau 		 = 0.9977753991872335;
	notes[29].string.Ref		 = 0.0022246008127665;
	notes[29].LP[0].glp		 = 0.7520000000000000;
	notes[29].LP[0].a1		 = -0.2434904722581937;
	notes[29].LP[1].glp		 = 0.7520000000000000;
	notes[29].LP[1].a1		 = -0.2434904722581937;
	notes[29].LP[2].glp		 = 0.7520000000000000;
	notes[29].LP[2].a1		 = -0.2434904722581937;
	notes[29].tunning[0].a1		 = 0.3749145791569650;
	notes[29].tunning[1].a1		 = 0.4420997896898730;
	notes[29].tunning[2].a1		 = 0.4012527240633238;
	notes[29].allpass.a1		 = -0.9492099963506668;
	notes[29].allpass.a2		 = 0.9018022307021316;
	notes[29].Ns			 = 3;
	notes[29].M			 = 4;
	notes[29].control.MaxV		 = 7;
	notes[29].control.MinV		 = 2;
	notes[29].control.scale 	 = 0.5;
	
// DS3
	notes[30].hammer.K		 = 999076850.2840477200000000;
	notes[30].hammer.fsh		 = 88200;
	notes[30].hammer.m		 = 0.0078587166210982;
	notes[30].hammer.p		 = 2.3447761605593094;
	notes[30].string.Min		 = 21;
	notes[30].string.Mup		 = 142;
	notes[30].string.Mlow[0]	 = 141;
	notes[30].string.Mlow[1]	 = 139;
	notes[30].string.Mlow[2]	 = 136;
	notes[30].string.fs		 = 44100;
	notes[30].string.Z		 = 4.4053242760367031;
	notes[30].string.tau 		 = 0.9977997610534928;
	notes[30].string.Ref		 = 0.0022002389465073;
	notes[30].LP[0].glp		 = 0.7510000000000000;
	notes[30].LP[0].a1		 = -0.2434904722581937;
	notes[30].LP[1].glp		 = 0.7510000000000000;
	notes[30].LP[1].a1		 = -0.2434904722581937;
	notes[30].LP[2].glp		 = 0.7510000000000000;
	notes[30].LP[2].a1		 = -0.2434904722581937;
	notes[30].tunning[0].a1		 = 0.3929054500338225;
	notes[30].tunning[1].a1		 = 0.4393993274858100;
	notes[30].tunning[2].a1		 = 0.3725536328520425;
	notes[30].allpass.a1		 = -0.9494306331708247;
	notes[30].allpass.a2		 = 0.9022144251182507;
	notes[30].Ns			 = 3;
	notes[30].M			 = 4;
	notes[30].control.MaxV		 = 7;
	notes[30].control.MinV		 = 2;
	notes[30].control.scale 	 = 0.5;

// E3
	notes[31].hammer.K		 = 1085555601.8457220000000000;
	notes[31].hammer.fsh		 = 88200;
	notes[31].hammer.m		 = 0.0076884182477003;
	notes[31].hammer.p		 = 2.3562038300246342;
	notes[31].string.Min		 = 20;
	notes[31].string.Mup		 = 134;
	notes[31].string.Mlow[0]	 = 134;
	notes[31].string.Mlow[1]	 = 131;
	notes[31].string.Mlow[2]	 = 128;
	notes[31].string.fs		 = 44100;
	notes[31].string.Z		 = 4.3558213544859301;
	notes[31].string.tau 		 = 0.9978244583904071;
	notes[31].string.Ref		 = 0.0021755416095928;
	notes[31].LP[0].glp		 = 0.7520000000000000;
	notes[31].LP[0].a1		 = -0.2434904722581937;
	notes[31].LP[1].glp		 = 0.7520000000000000;
	notes[31].LP[1].a1		 = -0.2434904722581937;
	notes[31].LP[2].glp		 = 0.7520000000000000;
	notes[31].LP[2].a1		 = -0.2434904722581937;
	notes[31].tunning[0].a1		 = 0.4626629953809280;
	notes[31].tunning[1].a1		 = 0.4067697210416531;
	notes[31].tunning[2].a1		 = 0.4014552625155435;
	notes[31].allpass.a1		 = -0.9497060451887353;
	notes[31].allpass.a2		 = 0.9027291239854188;
	notes[31].Ns			 = 3;
	notes[31].M			 = 4;
	notes[31].control.MaxV		 = 7;
	notes[31].control.MinV		 = 2;
	notes[31].control.scale 	 = 0.5;

// F3
	notes[32].hammer.K		 = 1180067172.6516428000000000;
	notes[32].hammer.fsh		 = 88200;
	notes[32].hammer.m		 = 0.0075220563646877;
	notes[32].hammer.p		 = 2.3676953619013585;
	notes[32].string.Min		 = 19;
	notes[32].string.Mup		 = 127;
	notes[32].string.Mlow[0]	 = 125;
	notes[32].string.Mlow[1]	 = 123;
	notes[32].string.Mlow[2]	 = 120;
	notes[32].string.fs		 = 44100;
	notes[32].string.Z		 = 4.3044849594217487;
	notes[32].string.tau 		 = 0.9978500711044379;
	notes[32].string.Ref		 = 0.0021499288955622;
	notes[32].LP[0].glp		 = 0.7522000000000000;
	notes[32].LP[0].a1		 = -0.2434904722581937;
	notes[32].LP[1].glp		 = 0.7522000000000000;
	notes[32].LP[1].a1		 = -0.2434904722581937;
	notes[32].LP[2].glp		 = 0.7522000000000000;
	notes[32].LP[2].a1		 = -0.2434904722581937;
	notes[32].tunning[0].a1		 = 0.3338432734935156;
	notes[32].tunning[1].a1		 = 0.4091444359372261;
	notes[32].tunning[2].a1		 = 0.3759324491413596;
	notes[32].allpass.a1		 = -0.9500427872067351;
	notes[32].allpass.a2		 = 0.9033586998194310;
	notes[32].Ns			 = 3;
	notes[32].M			 = 4;
	notes[32].control.MaxV		 = 7;
	notes[32].control.MinV		 = 2;
	notes[32].control.scale 	 = 0.5;
	
// FS3
	notes[33].hammer.K		 = 1282979899.6337576000000000;
	notes[33].hammer.fsh		 = 88200;
	notes[33].hammer.m		 = 0.0073600524551570;
	notes[33].hammer.p		 = 2.3792054257906528;
	notes[33].string.Min		 = 18;
	notes[33].string.Mup		 = 120;
	notes[33].string.Mlow[0]	 = 118;
	notes[33].string.Mlow[1]	 = 116;
	notes[33].string.Mlow[2]	 = 113;
	notes[33].string.fs		 = 44100;
	notes[33].string.Z		 = 4.2512970987135859;
	notes[33].string.tau 		 = 0.9978766082429472;
	notes[33].string.Ref		 = 0.0021233917570527;
	notes[33].LP[0].glp		 = 0.7522000000000000;
	notes[33].LP[0].a1		 = -0.2434904722581937;
	notes[33].LP[1].glp		 = 0.7522000000000000;
	notes[33].LP[1].a1		 = -0.2434904722581937;
	notes[33].LP[2].glp		 = 0.7522000000000000;
	notes[33].LP[2].a1		 = -0.2434904722581937;
	notes[33].tunning[0].a1		 = 0.3760004304868942;
	notes[33].tunning[1].a1		 = 0.4441086188291752;
	notes[33].tunning[2].a1		 = 0.3789095158440672;
	notes[33].allpass.a1		 = -0.9504417981726392;
	notes[33].allpass.a2		 = 0.9041050667430425;
	notes[33].Ns			 = 3;
	notes[33].M			 = 4;
	notes[33].control.MaxV		 = 7;
	notes[33].control.MinV		 = 2;
	notes[33].control.scale 	 = 0.5;

// G3
	notes[34].hammer.K		 = 1394667313.9544652000000000;
	notes[34].hammer.fsh		 = 88200;
	notes[34].hammer.m		 = 0.0072026822681313;
	notes[34].hammer.p		 = 2.3906957259010548;
	notes[34].string.Min		 = 17;
	notes[34].string.Mup		 = 113;
	notes[34].string.Mlow[0]	 = 111;
	notes[34].string.Mlow[1]	 = 109;
	notes[34].string.Mlow[2]	 = 107;
	notes[34].string.fs		 = 44100;
	notes[34].string.Z		 = 4.1962644058836407;
	notes[34].string.tau 		 = 0.9979040665697708;
	notes[34].string.Ref		 = 0.0020959334302292;
	notes[34].LP[0].glp		 = 0.7535000000000000;
	notes[34].LP[0].a1		 = -0.2434904722581937;
	notes[34].LP[1].glp		 = 0.7535000000000000;
	notes[34].LP[1].a1		 = -0.2434904722581937;
	notes[34].LP[2].glp		 = 0.7535000000000000;
	notes[34].LP[2].a1		 = -0.2434904722581937;
	notes[34].tunning[0].a1		 = 0.3727987116781103;
	notes[34].tunning[1].a1		 = 0.3471189006049185;
	notes[34].tunning[2].a1		 = 0.3469888638539945;
	notes[34].allpass.a1		 = -0.9509029507777896;
	notes[34].allpass.a2		 = 0.9049681757677996;
	notes[34].Ns			 = 3;
	notes[34].M			 = 4;
	notes[34].control.MaxV		 = 7;
	notes[34].control.MinV		 = 2;
	notes[34].control.scale 	 = 0.5;

// GS3
	notes[35].hammer.K		 = 1516562268.7597120000000000;
	notes[35].hammer.fsh		 = 88200;
	notes[35].hammer.m		 = 0.0070488442743627;
	notes[35].hammer.p		 = 2.4022300380712673;
	notes[35].string.Min		 = 16;
	notes[35].string.Mup		 = 107;
	notes[35].string.Mlow[0]	 = 104;
	notes[35].string.Mlow[1]	 = 103;
	notes[35].string.Mlow[2]	 = 101;
	notes[35].string.fs		 = 44100;
	notes[35].string.Z		 = 4.1389384321571967;
	notes[35].string.tau 		 = 0.9979326699218993;
	notes[35].string.Ref		 = 0.0020673300781005;
	notes[35].LP[0].glp		 = 0.7535000000000000;
	notes[35].LP[0].a1		 = -0.2434904722581937;
	notes[35].LP[1].glp		 = 0.7535000000000000;
	notes[35].LP[1].a1		 = -0.2434904722581937;
	notes[35].LP[2].glp		 = 0.7535000000000000;
	notes[35].LP[2].a1		 = -0.2434904722581937;
	notes[35].tunning[0].a1		 = 0.3430523928541756;
	notes[35].tunning[1].a1		 = 0.4514807353505312;
	notes[35].tunning[2].a1		 = 0.4272402450357157;
	notes[35].allpass.a1		 = -0.9514297654120448;
	notes[35].allpass.a2		 = 0.9059548410927053;
	notes[35].Ns			 = 3;
	notes[35].M			 = 4;
	notes[35].control.MaxV		 = 7;
	notes[35].control.MinV		 = 2;
	notes[35].control.scale 	 = 0.5;
	
// A3
	notes[36].hammer.K		 = 1656713031.4855716000000000;
	notes[36].hammer.fsh		 = 88200;
	notes[36].hammer.m		 = 0.0068908069593422;
	notes[36].hammer.p		 = 2.4143974657989564;
	notes[36].string.Min		 = 15;
	notes[36].string.Mup		 = 100;
	notes[36].string.Mlow[0]	 = 99;
	notes[36].string.Mlow[1]	 = 97;
	notes[36].string.Mlow[2]	 = 95;
	notes[36].string.fs		 = 44100;
	notes[36].string.Z		 = 4.0760933851821202;
	notes[36].string.tau 		 = 0.9979640280103987;
	notes[36].string.Ref		 = 0.0020359719896013;
	notes[36].LP[0].glp		 = 0.7540000000000000;
	notes[36].LP[0].a1		 = -0.2434904722581937;
	notes[36].LP[1].glp		 = 0.7540000000000000;
	notes[36].LP[1].a1		 = -0.2434904722581937;
	notes[36].LP[2].glp		 = 0.7540000000000000;
	notes[36].LP[2].a1		 = -0.2434904722581937;
	notes[36].tunning[0].a1		 = 0.4490039433709537;
	notes[36].tunning[1].a1		 = 0.3805257903416301;
	notes[36].tunning[2].a1		 = 0.3831781742561978;
	notes[36].allpass.a1		 = -0.9520551737683578;
	notes[36].allpass.a2		 = 0.9071270771606891;
	notes[36].Ns			 = 3;
	notes[36].M			 = 4;
	notes[36].control.MaxV		 = 7;
	notes[36].control.MinV		 = 2;
	notes[36].control.scale 	 = 0.5;
	

// AS3
	notes[37].hammer.K		 = 1791500863.7041183000000000;
	notes[37].hammer.fsh		 = 88200;
	notes[37].hammer.m		 = 0.0067544007866648;
	notes[37].hammer.p		 = 2.4251647523739175;
	notes[37].string.Min		 = 14;
	notes[37].string.Mup		 = 95;
	notes[37].string.Mlow[0]	 = 93;
	notes[37].string.Mlow[1]	 = 92;
	notes[37].string.Mlow[2]	 = 90;
	notes[37].string.fs		 = 44100;
	notes[37].string.Z		 = 4.0183895074392701;
	notes[37].string.tau 		 = 0.9979928216523833;
	notes[37].string.Ref		 = 0.0020071783476167;
	notes[37].LP[0].glp		 = 0.7550000000000000;
	notes[37].LP[0].a1		 = -0.2434904722581937;
	notes[37].LP[1].glp		 = 0.7550000000000000;
	notes[37].LP[1].a1		 = -0.2434904722581937;
	notes[37].LP[2].glp		 = 0.7550000000000000;
	notes[37].LP[2].a1		 = -0.2434904722581937;
	notes[37].tunning[0].a1		 = 0.4316840630475204;
	notes[37].tunning[1].a1		 = 0.4727873111660317;
	notes[37].tunning[2].a1		 = 0.3833346590087813;
	notes[37].allpass.a1		 = -0.9526680418892614;
	notes[37].allpass.a2		 = 0.9082767732595407;
	notes[37].Ns			 = 3;
	notes[37].M			 = 4;
	notes[37].control.MaxV		 = 5;
	notes[37].control.MinV		 = 2;
	notes[37].control.scale 	 = 0.5;


// B3
	notes[38].hammer.K		 = 1947014305.1731987000000000;
	notes[38].hammer.fsh		 = 88200;
	notes[38].hammer.m		 = 0.0066126047337104;
	notes[38].hammer.p		 = 2.4366237837395408;
	notes[38].string.Min		 = 13;
	notes[38].string.Mup		 = 90;
	notes[38].string.Mlow[0]	 = 87;
	notes[38].string.Mlow[1]	 = 86;
	notes[38].string.Mlow[2]	 = 84;
	notes[38].string.fs		 = 44100;
	notes[38].string.Z		 = 3.9547968752664224;
	notes[38].string.tau 		 = 0.9980245546836080;
	notes[38].string.Ref		 = 0.0019754453163921;
	notes[38].LP[0].glp		 = 0.7545000000000000;
	notes[38].LP[0].a1		 = -0.2434904722581937;
	notes[38].LP[1].glp		 = 0.7545000000000000;
	notes[38].LP[1].a1		 = -0.2434904722581937;
	notes[38].LP[2].glp		 = 0.7545000000000000;
	notes[38].LP[2].a1		 = -0.2434904722581937;
	notes[38].tunning[0].a1		 = 0.3371185666827390;
	notes[38].tunning[1].a1		 = 0.3942973023965352;
	notes[38].tunning[2].a1		 = 0.3515968819072398;
	notes[38].allpass.a1		 = -0.9533808171531326;
	notes[38].allpass.a2		 = 0.9096150899945191;
	notes[38].Ns			 = 3;
	notes[38].M			 = 4;
	notes[38].control.MaxV		 = 4.5;
	notes[38].control.MinV		 = 2;
	notes[38].control.scale 	 = 0.5;

// C4
	notes[39].hammer.K		 = 2116735536.8796103000000000;
	notes[39].hammer.fsh		 = 88200;
	notes[39].hammer.m		 = 0.0064735689529495;
	notes[39].hammer.p		 = 2.4481288825013254;
	notes[39].string.Min		 = 13;
	notes[39].string.Mup		 = 85;
	notes[39].string.Mlow[0]	 = 82;
	notes[39].string.Mlow[1]	 = 81;
	notes[39].string.Mlow[2]	 = 79;
	notes[39].string.fs		 = 44100;
	notes[39].string.Z		 = 3.8886956195292468;
	notes[39].string.tau 		 = 0.9980575405985768;
	notes[39].string.Ref		 = 0.0019424594014233;
	notes[39].LP[0].glp		 = 0.7535000000000000;
	notes[39].LP[0].a1		 = -0.2434904722581937;
	notes[39].LP[1].glp		 = 0.7535000000000000;
	notes[39].LP[1].a1		 = -0.2434904722581937;
	notes[39].LP[2].glp		 = 0.7535000000000000;
	notes[39].LP[2].a1		 = -0.2434904722581937;
	notes[39].tunning[0].a1		 = 0.3494306174012524;
	notes[39].tunning[1].a1		 = 0.3980886636133134;
	notes[39].tunning[2].a1		 = 0.3412798417653118;
	notes[39].allpass.a1		 = -0.9541579444132254;
	notes[39].allpass.a2		 = 0.9110757089642891;
	notes[39].Ns			 = 3;
	notes[39].M			 = 4;
	notes[39].control.MaxV		 = 4.5;
	notes[39].control.MinV		 = 2;
	notes[39].control.scale 	 = 0.5;

// CS4
	notes[40].hammer.K		 = 2301548783.0916958000000000;
	notes[40].hammer.fsh		 = 88200;
	notes[40].hammer.m		 = 0.0063375017482974;
	notes[40].hammer.p		 = 2.4596517709980734;
	notes[40].string.Min		 = 12;
	notes[40].string.Mup		 = 80;
	notes[40].string.Mlow[0]	 = 78;
	notes[40].string.Mlow[1]	 = 77;
	notes[40].string.Mlow[2]	 = 75;
	notes[40].string.fs		 = 44100;
	notes[40].string.Z		 = 3.8202754929352691;
	notes[40].string.tau 		 = 0.9980916848259555;
	notes[40].string.Ref		 = 0.0019083151740446;
	notes[40].LP[0].glp		 = 0.7535000000000000;
	notes[40].LP[0].a1		 = -0.2434904722581937;
	notes[40].LP[1].glp		 = 0.7535000000000000;
	notes[40].LP[1].a1		 = -0.2434904722581937;
	notes[40].LP[2].glp		 = 0.7535000000000000;
	notes[40].LP[2].a1		 = -0.2434904722581937;
	notes[40].tunning[0].a1		 = 0.3796839268174616;
	notes[40].tunning[1].a1		 = 0.4933946197524216;
	notes[40].tunning[2].a1		 = 0.4490596129412641;
	notes[40].allpass.a1		 = -0.9542754510483712;
	notes[40].allpass.a2		 = 0.9112966978418624;
	notes[40].Ns			 = 3;
	notes[40].M			 = 4;
	notes[40].control.MaxV		 = 4.5;
	notes[40].control.MinV		 = 2;
	notes[40].control.scale 	 = 0.5;

// D4 -- THIS IS SAME AS E4 TELL GRAHAM!!!!!
	notes[41].hammer.K		 = 2955882433.6048627000000000;
	notes[41].hammer.fsh		 = 88200;
	notes[41].hammer.m		 = 0.0059483328007129;
	notes[41].hammer.p		 = 2.4940956216824564;
	notes[41].string.Min		 = 10;
	notes[41].string.Mup		 = 67;
	notes[41].string.Mlow[0]	 = 66;
	notes[41].string.Mlow[1]	 = 64;
	notes[41].string.Mlow[2]	 = 63;
	notes[41].string.fs		 = 44100;
	notes[41].string.Z		 = 3.6035002357465058;
	notes[41].string.tau 		 = 0.9981998715729296;
	notes[41].string.Ref		 = 0.0018001284270704;
	notes[41].LP[0].glp		 = 0.7540000000000000;
	notes[41].LP[0].a1		 = -0.2434904722581937;
	notes[41].LP[1].glp		 = 0.7540000000000000;
	notes[41].LP[1].a1		 = -0.2434904722581937;
	notes[41].LP[2].glp		 = 0.7540000000000000;
	notes[41].LP[2].a1		 = -0.2434904722581937;
	notes[41].tunning[0].a1		 = 0.4803106486151413;
	notes[41].tunning[1].a1		 = 0.3662382055296397;
	notes[41].tunning[2].a1		 = 0.4450945057377992;
	notes[41].allpass.a1		 = -0.9540549769902237;
	notes[41].allpass.a2		 = 0.9108820921817902;
	notes[41].Ns			 = 3;
	notes[41].M			 = 4;
	notes[41].control.MaxV		 = 4.5;
	notes[41].control.MinV		 = 2;
	notes[41].control.scale 	 = 1;

// DS4
	notes[42].hammer.K		 = 2719144385.2904954000000000;
	notes[42].hammer.fsh		 = 88200;
	notes[42].hammer.m		 = 0.0060753808189886;
	notes[42].hammer.p		 = 2.4826040165572514;
	notes[42].string.Min		 = 11;
	notes[42].string.Mup		 = 71;
	notes[42].string.Mlow[0]	 = 70;
	notes[42].string.Mlow[1]	 = 68;
	notes[42].string.Mlow[2]	 = 67;
	notes[42].string.fs		 = 44100;
	notes[42].string.Z		 = 3.6777361798103918;
	notes[42].string.tau 		 = 0.9981628210749451;
	notes[42].string.Ref		 = 0.0018371789250548;
	notes[42].LP[0].glp		 = 0.7540000000000000;
	notes[42].LP[0].a1		 = -0.2434904722581937;
	notes[42].LP[1].glp		 = 0.7540000000000000;
	notes[42].LP[1].a1		 = -0.2434904722581937;
	notes[42].LP[2].glp		 = 0.7540000000000000;
	notes[42].LP[2].a1		 = -0.2434904722581937;
	notes[42].tunning[0].a1		 = 0.4719649273662410;
	notes[42].tunning[1].a1		 = 0.3721743824287104;
	notes[42].tunning[2].a1		 = 0.4750751773676833;
	notes[42].allpass.a1		 = -0.9540433523347754;
	notes[42].allpass.a2		 = 0.9108602352334144;
	notes[42].Ns			 = 3;
	notes[42].M			 = 4;
	notes[42].control.MaxV		 = 4.5;
	notes[42].control.MinV		 = 2;
	notes[42].control.scale 	 = 0.5;

// E4
	notes[43].hammer.K		 = 2955882433.6048627000000000;
	notes[43].hammer.fsh		 = 88200;
	notes[43].hammer.m		 = 0.0059483328007129;
	notes[43].hammer.p		 = 2.4940956216824564;
	notes[43].string.Min		 = 10;
	notes[43].string.Mup		 = 67;
	notes[43].string.Mlow[0]	 = 66;
	notes[43].string.Mlow[1]	 = 64;
	notes[43].string.Mlow[2]	 = 63;
	notes[43].string.fs		 = 44100;
	notes[43].string.Z		 = 3.6035002357465058;
	notes[43].string.tau 		 = 0.9981998715729296;
	notes[43].string.Ref		 = 0.0018001284270704;
	notes[43].LP[0].glp		 = 0.7550000000000000;
	notes[43].LP[0].a1		 = -0.2434904722581937;
	notes[43].LP[1].glp		 = 0.7550000000000000;
	notes[43].LP[1].a1		 = -0.2434904722581937;
	notes[43].LP[2].glp		 = 0.7550000000000000;
	notes[43].LP[2].a1		 = -0.2434904722581937;
	notes[43].tunning[0].a1		 = 0.4803106486151413;
	notes[43].tunning[1].a1		 = 0.3662382055296397;
	notes[43].tunning[2].a1		 = 0.4450945057377992;
	notes[43].allpass.a1		 = -0.9540549769902237;
	notes[43].allpass.a2		 = 0.9108820921817902;
	notes[43].Ns			 = 3;
	notes[43].M			 = 4;
	notes[43].control.MaxV		 = 4.5;
	notes[43].control.MinV		 = 2;
	notes[43].control.scale 	 = 0.5;
	
// F4
	notes[44].hammer.K		 = 3213628943.3607535000000000;
	notes[44].hammer.fsh		 = 88200;
	notes[44].hammer.m		 = 0.0058237345216189;
	notes[44].hammer.p		 = 2.5056042435298971;
	notes[44].string.Min		 = 10;
	notes[44].string.Mup		 = 64;
	notes[44].string.Mlow[0]	 = 61;
	notes[44].string.Mlow[1]	 = 60;
	notes[44].string.Mlow[2]	 = 58;
	notes[44].string.fs		 = 44100;
	notes[44].string.Z		 = 3.5274564726423558;
	notes[44].string.tau 		 = 0.9982378257619093;
	notes[44].string.Ref		 = 0.0017621742380907;
	notes[44].LP[0].glp		 = 0.7550000000000000;
	notes[44].LP[0].a1		 = -0.2434904722581937;
	notes[44].LP[1].glp		 = 0.7550000000000000;
	notes[44].LP[1].a1		 = -0.2434904722581937;
	notes[44].LP[2].glp		 = 0.7550000000000000;
	notes[44].LP[2].a1		 = -0.2434904722581937;
	notes[44].tunning[0].a1		 = 0.4021347005382232;
	notes[44].tunning[1].a1		 = 0.4522491123887120;
	notes[44].tunning[2].a1		 = 0.3547052373610100;
	notes[44].allpass.a1		 = -0.9541508572081936;
	notes[44].allpass.a2		 = 0.9110623815357266;
	notes[44].Ns			 = 3;
	notes[44].M			 = 4;
	notes[44].control.MaxV		 = 4.5;
	notes[44].control.MinV		 = 2;
	notes[44].control.scale 	 = 0.5;
	



// FS4
	notes[45].hammer.K		 = 3490724617.3797431000000000;
	notes[45].hammer.fsh		 = 88200;
	notes[45].hammer.m		 = 0.0057029569742076;
	notes[45].hammer.p		 = 2.5169896555080316;
	notes[45].string.Min		 = 9;
	notes[45].string.Mup		 = 60;
	notes[45].string.Mlow[0]	 = 58;
	notes[45].string.Mlow[1]	 = 57;
	notes[45].string.Mlow[2]	 = 56;
	notes[45].string.fs		 = 44100;
	notes[45].string.Z		 = 3.4507493325644569;
	notes[45].string.tau 		 = 0.9982761125096195;
	notes[45].string.Ref		 = 0.0017238874903805;
	notes[45].LP[0].glp		 = 0.7552000000000000;
	notes[45].LP[0].a1		 = -0.2434904722581937;
	notes[45].LP[1].glp		 = 0.7552000000000000;
	notes[45].LP[1].a1		 = -0.2434904722581937;
	notes[45].LP[2].glp		 = 0.7552000000000000;
	notes[45].LP[2].a1		 = -0.2434904722581937;
	notes[45].tunning[0].a1		 = 0.4662817356665419;
	notes[45].tunning[1].a1		 = 0.4566722841587789;
	notes[45].tunning[2].a1		 = 0.4781415131705332;
	notes[45].allpass.a1		 = -0.9543269234728301;
	notes[45].allpass.a2		 = 0.9113935105497857;
	notes[45].Ns			 = 3;
	notes[45].M			 = 4;
	notes[45].control.MaxV		 = 5;
	notes[45].control.MinV		 = 2;
	notes[45].control.scale 	 = 1;

// G4
	notes[46].hammer.K		 = 3798969387.2490492000000000;
	notes[46].hammer.fsh		 = 88200;
	notes[46].hammer.m		 = 0.0055818400593746;
	notes[46].hammer.p		 = 2.5286382600164927;
	notes[46].string.Min		 = 9;
	notes[46].string.Mup		 = 57;
	notes[46].string.Mlow[0]	 = 54;
	notes[46].string.Mlow[1]	 = 53;
	notes[46].string.Mlow[2]	 = 52;
	notes[46].string.fs		 = 44100;
	notes[46].string.Z		 = 3.3709707648243770;
	notes[46].string.tau 		 = 0.9983159338520254;
	notes[46].string.Ref		 = 0.0016840661479745;
	notes[46].LP[0].glp		 = 0.7550000000000000;
	notes[46].LP[0].a1		 = -0.2434904722581937;
	notes[46].LP[1].glp		 = 0.7550000000000000;
	notes[46].LP[1].a1		 = -0.2434904722581937;
	notes[46].LP[2].glp		 = 0.7550000000000000;
	notes[46].LP[2].a1		 = -0.2434904722581937;
	notes[46].tunning[0].a1		 = 0.3925200363998607;
	notes[46].tunning[1].a1		 = 0.4146734827845150;
	notes[46].tunning[2].a1		 = 0.4634983600879342;
	notes[46].allpass.a1		 = -0.9545882060813307;
	notes[46].allpass.a2		 = 0.9118850521318498;
	notes[46].Ns			 = 3;
	notes[46].M			 = 4;
	notes[46].control.MaxV		 = 4.5;
	notes[46].control.MinV		 = 2;
	notes[46].control.scale 	 = 0.75;

// GS4
	notes[47].hammer.K		 = 4128043104.9143238000000000;
	notes[47].hammer.fsh		 = 88200;
	notes[47].hammer.m		 = 0.0054652514206174;
	notes[47].hammer.p		 = 2.5400739337847136;
	notes[47].string.Min		 = 8;
	notes[47].string.Mup		 = 54;
	notes[47].string.Mlow[0]	 = 50;
	notes[47].string.Mlow[1]	 = 50;
	notes[47].string.Mlow[2]	 = 49;
	notes[47].string.fs		 = 44100;
	notes[47].string.Z		 = 3.2916091332347688;
	notes[47].string.tau		 = 0.9983555486561482;
	notes[47].string.Ref		 = 0.0016444513438517;
	notes[47].LP[0].glp		 = 0.7555000000000000;
	notes[47].LP[0].a1		 = -0.2434904722581937;
	notes[47].LP[1].glp		 = 0.7555000000000000;
	notes[47].LP[1].a1		 = -0.2434904722581937;
	notes[47].LP[2].glp		 = 0.7555000000000000;
	notes[47].LP[2].a1		 = -0.2434904722581937;
	notes[47].tunning[0].a1		 = 0.3351112861732178;
	notes[47].tunning[1].a1		 = 0.4686433818604029;
	notes[47].tunning[2].a1		 = 0.4605615548812393;
	notes[47].allpass.a1		 = -0.9549215363717331;
	notes[47].allpass.a2		 = 0.9125123870581635;
	notes[47].Ns			 = 3;
	notes[47].M			 = 4;
	notes[47].control.MaxV		 = 4.5;
	notes[47].control.MinV		 = 2;
	notes[47].control.scale 	 = 0.75;
	
	
// A4
	notes[48].hammer.K   		= 4.487497776184154e+9;
	notes[48].hammer.fsh 		= 88200;
	notes[48].hammer.m   		= 0.005350293466813135;
	notes[48].hammer.p   		= 2.551567166631288;
	notes[48].string.Min 		= 8;
	notes[48].string.Mup		= 51;
	notes[48].string.Mlow[0]	= 48;
	notes[48].string.Mlow[1]	= 47;
	notes[48].string.Mlow[2]	= 46;
	notes[48].string.fs		= 44100;
	notes[48].string.Z		= 3.211057923171025;
	notes[48].string.tau		= 0.998395758866215;
	notes[48].string.Ref		= 0.001604241133785;
	notes[48].LP[0].glp		= 0.756200000000000;
	notes[48].LP[0].a1		= -0.24349047225819373;
	notes[48].LP[1].glp		= 0.756200000000000;
	notes[48].LP[1].a1		= -0.243490472258194;
	notes[48].LP[2].glp		= 0.756200000000000;
	notes[48].LP[2].a1		= -0.243490472258194;
	notes[48].tunning[0].a1		= 0.456260334907079;
	notes[48].tunning[1].a1		= 0.458863721503665;
	notes[48].tunning[2].a1		= 0.481839688395739;
	notes[48].allpass.a1		= -0.955329655458589;
	notes[48].allpass.a2		= 0.913280861962960;
	notes[48].Ns			= 3;
	notes[48].M			= 4;
	notes[48].control.MaxV		 = 4.5;
	notes[48].control.MinV		 = 2;
	notes[48].control.scale 	 = 1;

// AS4
	notes[49].hammer.K		 = 4878115799.6949186000000000;
	notes[49].hammer.fsh		 = 88200;
	notes[49].hammer.m		 = 0.0052375090268997;
	notes[49].hammer.p		 = 2.5630565440317397;
	notes[49].string.Min		 = 7;
	notes[49].string.Mup		 = 48;
	notes[49].string.Mlow[0]	 = 45;
	notes[49].string.Mlow[1]	 = 44;
	notes[49].string.Mlow[2]	 = 43;
	notes[49].string.fs		 = 44100;
	notes[49].string.Z		 = 3.1299999198259880;
	notes[49].string.tau 		 = 0.9984362236950144;
	notes[49].string.Ref		 = 0.0015637763049857;
	notes[49].LP[0].glp		 = 0.7535000000000000;
	notes[49].LP[0].a1		 = -0.2434904722581937;
	notes[49].LP[1].glp		 = 0.7535000000000000;
	notes[49].LP[1].a1		 = -0.2434904722581937;
	notes[49].LP[2].glp		 = 0.7535000000000000;
	notes[49].LP[2].a1		 = -0.2434904722581937;
	notes[49].tunning[0].a1		 = 0.3971442337496230;
	notes[49].tunning[1].a1		 = 0.3902665071613015;
	notes[49].tunning[2].a1		 = 0.3958392034441853;
	notes[49].allpass.a1		 = -0.9558068149490092;
	notes[49].allpass.a2		 = 0.9141798765009499;
	notes[49].Ns			 = 3;
	notes[49].M			 = 4;
	notes[49].control.MaxV		 = 4.5;
	notes[49].control.MinV		 = 2;
	notes[49].control.scale 	 = 1;

// B4
	notes[50].hammer.K		 = 5303118283.2390280000000000;
	notes[50].hammer.fsh		 = 88200;
	notes[50].hammer.m		 = 0.0051266822548850;
	notes[50].hammer.p		 = 2.5745558580347394;
	notes[50].string.Min		 = 7;
	notes[50].string.Mup		 = 45;
	notes[50].string.Mlow[0]	 = 43;
	notes[50].string.Mlow[1]	 = 42;
	notes[50].string.Mlow[2]	 = 41;
	notes[50].string.fs		 = 44100;
	notes[50].string.Z		 = 3.0486010843408606;
	notes[50].string.tau 		 = 0.9984768603191502;
	notes[50].string.Ref		 = 0.0015231396808498;
	notes[50].LP[0].glp		 = 0.7540000000000000;
	notes[50].LP[0].a1		 = -0.2434904722581937;
	notes[50].LP[1].glp		 = 0.7540000000000000;
	notes[50].LP[1].a1		 = -0.2434904722581937;
	notes[50].LP[2].glp		 = 0.7540000000000000;
	notes[50].LP[2].a1		 = -0.2434904722581937;
	notes[50].tunning[0].a1		 = 0.4630501016763717;
	notes[50].tunning[1].a1		 = 0.4429415515151265;
	notes[50].tunning[2].a1		 = 0.4376354318496882;
	notes[50].allpass.a1		 = -0.9563490995490833;
	notes[50].allpass.a2		 = 0.9152022985136331;
	notes[50].Ns			 = 3;
	notes[50].M			 = 4;
	notes[50].control.MaxV		 = 4.5;
	notes[50].control.MinV		 = 2;
	notes[50].control.scale 	 = 1;

// C5
	notes[51].hammer.K		 = 5764500234.5210695000000000;
	notes[51].hammer.fsh		 = 176400;
	notes[51].hammer.m		 = 0.0050179794053932;
	notes[51].hammer.p		 = 2.5860396846999434;
	notes[51].string.Min		 = 10;
	notes[51].string.Mup		 = 85;
	notes[51].string.Mlow[0]	 = 83;
	notes[51].string.Mlow[1]	 = 82;
	notes[51].string.Mlow[2]	 = 80;
	notes[51].string.fs		 = 88200;
	notes[51].string.Z		 = 2.9673058736660103;
	notes[51].string.tau 		 = 0.9999970326985287;
	notes[51].string.Ref		 = 0.0000029673014712;
	notes[51].LP[0].glp		 = 0.7530000000000000;
	notes[51].LP[0].a1		 = -0.2434904722581937;
	notes[51].LP[1].glp		 = 0.7530000000000000;
	notes[51].LP[1].a1		 = -0.2434904722581937;
	notes[51].LP[2].glp		 = 0.7530000000000000;
	notes[51].LP[2].a1		 = -0.2434904722581937;
	notes[51].tunning[0].a1		 = 0.3565257602824724;
	notes[51].tunning[1].a1		 = 0.4756918428234565;
	notes[51].tunning[2].a1		 = 0.4519760256165217;
	notes[51].allpass.a1		 = -0.9659021360867613;
	notes[51].allpass.a2		 = 0.9333370695658205;
	notes[51].Ns			 = 3;
	notes[51].M			 = 1;
	notes[51].control.MaxV		 = 2.8;
	notes[51].control.MinV		 = 0.5;
	notes[51].control.scale 	 = 90;
	
// CS5
	notes[52].hammer.K		 = 6266446050.9977016000000000;
	notes[52].hammer.fsh		 = 176400;
	notes[52].hammer.m		 = 0.0049110914044579;
	notes[52].hammer.p		 = 2.5975327974401674;
	notes[52].string.Min		 = 10;
	notes[52].string.Mup		 = 80;
	notes[52].string.Mlow[0]	 = 79;
	notes[52].string.Mlow[1]	 = 77;
	notes[52].string.Mlow[2]	 = 75;
	notes[52].string.fs		 = 88200;
	notes[52].string.Z		 = 2.8862007936421099;
	notes[52].string.tau 		 = 0.9999971138033715;
	notes[52].string.Ref		 = 0.0000028861966286;
	notes[52].LP[0].glp		 = 0.7540000000000000;
	notes[52].LP[0].a1		 = -0.2434904722581937;
	notes[52].LP[1].glp		 = 0.7540000000000000;
	notes[52].LP[1].a1		 = -0.2434904722581937;
	notes[52].LP[2].glp		 = 0.7540000000000000;
	notes[52].LP[2].a1		 = -0.2434904722581937;
	notes[52].tunning[0].a1		 = 0.4446040282773688;
	notes[52].tunning[1].a1		 = 0.3784829902253591;
	notes[52].tunning[2].a1		 = 0.3484084760919510;
	notes[52].allpass.a1		 = -0.9664602910828104;
	notes[52].allpass.a2		 = 0.9344038817854586;
	notes[52].Ns			 = 3;
	notes[52].M			 = 1;
	notes[52].control.MaxV		 = 2.8;
	notes[52].control.MinV		 = 0.5;
	notes[52].control.scale 	 = 100;
	
// D5
	notes[53].hammer.K		 = 6811917203.3453197000000000;
	notes[53].hammer.fsh		 = 176400;
	notes[53].hammer.m		 = 0.0048060712536507;
	notes[53].hammer.p		 = 2.6090222370404952;
	notes[53].string.Min		 = 10;
	notes[53].string.Mup		 = 76;
	notes[53].string.Mlow[0]	 = 74;
	notes[53].string.Mlow[1]	 = 72;
	notes[53].string.Mlow[2]	 = 71;
	notes[53].string.fs		 = 88200;
	notes[53].string.Z		 = 2.8056332943354052;
	notes[53].string.tau 		 = 0.9999971943706414;
	notes[53].string.Ref		 = 0.0000028056293586;
	notes[53].LP[0].glp		 = 0.7540000000000000;
	notes[53].LP[0].a1		 = -0.2434904722581937;
	notes[53].LP[1].glp		 = 0.7540000000000000;
	notes[53].LP[1].a1		 = -0.2434904722581937;
	notes[53].LP[2].glp		 = 0.7540000000000000;
	notes[53].LP[2].a1		 = -0.2434904722581937;
	notes[53].tunning[0].a1		 = 0.4478183817685142;
	notes[53].tunning[1].a1		 = 0.3681656868315941;
	notes[53].tunning[2].a1		 = 0.4868359052060843;
	notes[53].allpass.a1		 = -0.9670592739562339;
	notes[53].allpass.a2		 = 0.9355496229387258;
	notes[53].Ns			 = 3;
	notes[53].M			 = 1;
	notes[53].control.MaxV		 = 3;
	notes[53].control.MinV		 = 0.5;
	notes[53].control.scale 	 = 120;

// DS5
	notes[54].hammer.K		 = 7404875935.5416241000000000;
	notes[54].hammer.fsh		 = 176400;
	notes[54].hammer.m		 = 0.0047028182760570;
	notes[54].hammer.p		 = 2.6205117940565383;
	notes[54].string.Min		 = 10;
	notes[54].string.Mup		 = 71;
	notes[54].string.Mlow[0]	 = 70;
	notes[54].string.Mlow[1]	 = 69;
	notes[54].string.Mlow[2]	 = 67;
	notes[54].string.fs		 = 88200;
	notes[54].string.Z		 = 2.7258234022899712;
	notes[54].string.tau 		 = 0.9999972741803127;
	notes[54].string.Ref		 = 0.0000027258196872;
	notes[54].LP[0].glp		 = 0.7540000000000000;
	notes[54].LP[0].a1		 = -0.2434904722581937;
	notes[54].LP[1].glp		 = 0.7540000000000000;
	notes[54].LP[1].a1		 = -0.2434904722581937;
	notes[54].LP[2].glp		 = 0.7540000000000000;
	notes[54].LP[2].a1		 = -0.2434904722581937;
	notes[54].tunning[0].a1		 = 0.3672318563447337;
	notes[54].tunning[1].a1		 = 0.4364294013497312;
	notes[54].tunning[2].a1		 = 0.3660616544493046;
	notes[54].allpass.a1		 = -0.9676943038225324;
	notes[54].allpass.a2		 = 0.9367653263668303;
	notes[54].Ns			 = 3;
	notes[54].M			 = 1;
	notes[54].control.MaxV		 = 3;
	notes[54].control.MinV		 = 0.5;
	notes[54].control.scale 	 = 120;
	
// E5
	notes[55].hammer.K		 = 8049693854.3251266000000000;
	notes[55].hammer.fsh		 = 176400;
	notes[55].hammer.m		 = 0.0046012353147981;
	notes[55].hammer.p		 = 2.6320055195372785;
	notes[55].string.Min		 = 10;
	notes[55].string.Mup		 = 67;
	notes[55].string.Mlow[0]	 = 66;
	notes[55].string.Mlow[1]	 = 65;
	notes[55].string.Mlow[2]	 = 63;
	notes[55].string.fs		 = 88200;
	notes[55].string.Z		 = 2.6469788145801552;
	notes[55].string.tau 		 = 0.9999973530246886;
	notes[55].string.Ref		 = 0.0000026469753113;
	notes[55].LP[0].glp		 = 0.7540000000000000;
	notes[55].LP[0].a1		 = -0.2434904722581937;
	notes[55].LP[1].glp		 = 0.7540000000000000;
	notes[55].LP[1].a1		 = -0.2434904722581937;
	notes[55].LP[2].glp		 = 0.7540000000000000;
	notes[55].LP[2].a1		 = -0.2434904722581937;
	notes[55].tunning[0].a1		 = 0.3720079266380707;
	notes[55].tunning[1].a1		 = 0.4279470381390308;
	notes[55].tunning[2].a1		 = 0.3480667552611850;
	notes[55].allpass.a1		 = -0.9683605813479476;
	notes[55].allpass.a2		 = 0.9380419699218218;
	notes[55].Ns			 = 3;
	notes[55].M			 = 1;
	notes[55].control.MaxV		 = 2.8;
	notes[55].control.MinV		 = 0.5;
	notes[55].control.scale 	 = 120;
	
// F5
	notes[56].hammer.K		 = 8750458590.6546803000000000;
	notes[56].hammer.fsh		 = 176400;
	notes[56].hammer.m		 = 0.0045013292942680;
	notes[56].hammer.p		 = 2.6434960351877193;
	notes[56].string.Min		 = 10;
	notes[56].string.Mup		 = 64;
	notes[56].string.Mlow[0]	 = 62;
	notes[56].string.Mlow[1]	 = 60;
	notes[56].string.Mlow[2]	 = 59;
	notes[56].string.fs		 = 88200;
	notes[56].string.Z		 = 2.5693719118864120;
	notes[56].string.tau 		 = 0.9999974306313889;
	notes[56].string.Ref		 = 0.0000025693686111;
	notes[56].LP[0].glp		 = 0.7540000000000000;
	notes[56].LP[0].a1		 = -0.2434904722581937;
	notes[56].LP[1].glp		 = 0.7540000000000000;
	notes[56].LP[1].a1		 = -0.2434904722581937;
	notes[56].LP[2].glp		 = 0.7540000000000000;
	notes[56].LP[2].a1		 = -0.2434904722581937;
	notes[56].tunning[0].a1		 = 0.4747114200536811;
	notes[56].tunning[1].a1		 = 0.3532308125313790;
	notes[56].tunning[2].a1		 = 0.4070342335523526;
	notes[56].allpass.a1		 = -0.9690526197491232;
	notes[56].allpass.a2		 = 0.9393691879925458;
	notes[56].Ns			 = 3;
	notes[56].M			 = 1;
	notes[56].control.MaxV		 = 2.1;
	notes[56].control.MinV		 = 0.5;
	notes[56].control.scale 	 = 100;
	
// FS5
	notes[57].hammer.K		 = 9512212384.3701649000000000;
	notes[57].hammer.fsh		 = 176400;
	notes[57].hammer.m		 = 0.0044030180806397;
	notes[57].hammer.p		 = 2.6549863207664570;
	notes[57].string.Min		 = 10;
	notes[57].string.Mup		 = 60;
	notes[57].string.Mlow[0]	 = 58;
	notes[57].string.Mlow[1]	 = 57;
	notes[57].string.Mlow[2]	 = 56;
	notes[57].string.fs		 = 88200;
	notes[57].string.Z		 = 2.4931882112862542;
	notes[57].string.tau 		 = 0.9999975068148967;
	notes[57].string.Ref		 = 0.0000024931851033;
	notes[57].LP[0].glp		 = 0.7553000000000000;
	notes[57].LP[0].a1		 = -0.2434904722581937;
	notes[57].LP[1].glp		 = 0.7553000000000000;
	notes[57].LP[1].a1		 = -0.2434904722581937;
	notes[57].LP[2].glp		 = 0.7553000000000000;
	notes[57].LP[2].a1		 = -0.2434904722581937;
	notes[57].tunning[0].a1		 = 0.3388843469203522;
	notes[57].tunning[1].a1		 = 0.3640450814895203;
	notes[57].tunning[2].a1		 = 0.4071028129058303;
	notes[57].allpass.a1		 = -0.9697655608163855;
	notes[57].allpass.a2		 = 0.9407377890785994;
	notes[57].Ns			 = 3;
	notes[57].M			 = 1;
	notes[57].control.MaxV		 = 2.3;
	notes[57].control.MinV		 = 0.5;
	notes[57].control.scale 	 = 150;

// G5
	notes[58].hammer.K		 = 10340291566.6095940000000000;
	notes[58].hammer.fsh		 = 176400;
	notes[58].hammer.m		 = 0.0043062455967246;
	notes[58].hammer.p		 = 2.6664767717540370;
	notes[58].string.Min		 = 10;
	notes[58].string.Mup		 = 57;
	notes[58].string.Mlow[0]	 = 55;
	notes[58].string.Mlow[1]	 = 54;
	notes[58].string.Mlow[2]	 = 52;
	notes[58].string.fs		 = 88200;
	notes[58].string.Z		 = 2.4186145706943019;
	notes[58].string.tau 		 = 0.9999975813883540;
	notes[58].string.Ref		 = 0.0000024186116458;
	notes[58].LP[0].glp		 = 0.7553000000000000;
	notes[58].LP[0].a1		 = -0.2434904722581937;
	notes[58].LP[1].glp		 = 0.7553000000000000;
	notes[58].LP[1].a1		 = -0.2434904722581937;
	notes[58].LP[2].glp		 = 0.7553000000000000;
	notes[58].LP[2].a1		 = -0.2434904722581937;
	notes[58].tunning[0].a1		 = 0.4551858742231165;
	notes[58].tunning[1].a1		 = 0.4853125877335367;
	notes[58].tunning[2].a1		 = 0.3476613849508995;
	notes[58].allpass.a1		 = -0.9704944916021200;
	notes[58].allpass.a2		 = 0.9421384447561718;
	notes[58].Ns			 = 3;
	notes[58].M			 = 1;
	notes[58].control.MaxV		 = 2.3;
	notes[58].control.MinV		 = 0.5;
	notes[58].control.scale 	 = 100;
	
// GS5
	notes[59].hammer.K		 = 11240531927.3764020000000000;
	notes[59].hammer.fsh		 = 176400;
	notes[59].hammer.m		 = 0.0042109558148172;
	notes[59].hammer.p		 = 2.6779681205136465;
	notes[59].string.Min		 = 10;
	notes[59].string.Mup		 = 54;
	notes[59].string.Mlow[0]	 = 51;
	notes[59].string.Mlow[1]	 = 50;
	notes[59].string.Mlow[2]	 = 49;
	notes[59].string.fs		 = 88200;
	notes[59].string.Z		 = 2.3458184668867683;
	notes[59].string.tau 		 = 0.9999976541842844;
	notes[59].string.Ref		 = 0.0000023458157155;
	notes[59].LP[0].glp		 = 0.7553000000000000;
	notes[59].LP[0].a1		 = -0.2434904722581937;
	notes[59].LP[1].glp		 = 0.7553000000000000;
	notes[59].LP[1].a1		 = -0.2434904722581937;
	notes[59].LP[2].glp		 = 0.7553000000000000;
	notes[59].LP[2].a1		 = -0.2434904722581937;
	notes[59].tunning[0].a1		 = 0.3543920329946447;
	notes[59].tunning[1].a1		 = 0.3637502447889337;
	notes[59].tunning[2].a1		 = 0.3794836616223649;
	notes[59].allpass.a1		 = -0.9712346578083030;
	notes[59].allpass.a2		 = 0.9435620974176638;
	notes[59].Ns			 = 3;
	notes[59].M			 = 1;
	notes[59].control.MaxV		 = 2.3;
	notes[59].control.MinV		 = 0.5;
	notes[59].control.scale 	 = 100;
	
// A5
	notes[60].hammer.K		 = 12219103125.5428620000000000;
	notes[60].hammer.fsh		 = 176400;
	notes[60].hammer.m		 = 0.0041171128618461;
	notes[60].hammer.p		 = 2.6894589582891104;
	notes[60].string.Min		 = 10;
	notes[60].string.Mup		 = 51;
	notes[60].string.Mlow[0]	 = 48;
	notes[60].string.Mlow[1]	 = 47;
	notes[60].string.Mlow[2]	 = 46;
	notes[60].string.fs		 = 88200;
	notes[60].string.Z		 = 2.2749629672136864;
	notes[60].string.tau 		 = 0.9999977250396205;
	notes[60].string.Ref		 = 0.0000022749603795;
	notes[60].LP[0].glp		 = 0.7553000000000000;
	notes[60].LP[0].a1		 = -0.2434904722581937;
	notes[60].LP[1].glp		 = 0.7553000000000000;
	notes[60].LP[1].a1		 = -0.2434904722581937;
	notes[60].LP[2].glp		 = 0.7553000000000000;
	notes[60].LP[2].a1		 = -0.2434904722581937;
	notes[60].tunning[0].a1		 = 0.3565261775849871;
	notes[60].tunning[1].a1		 = 0.3581087116304812;
	notes[60].tunning[2].a1		 = 0.3643907386069202;
	notes[60].allpass.a1		 = -0.9719813423253555;
	notes[60].allpass.a2		 = 0.9449997260518915;
	notes[60].Ns			 = 3;
	notes[60].M			 = 1;
	notes[60].control.MaxV		 = 2.5;
	notes[60].control.MinV		 = 0.5;
	notes[60].control.scale 	 = 100;

// AS5
	notes[61].hammer.K		 = 13282931139.7888220000000000;
	notes[61].hammer.fsh		 = 176400;
	notes[61].hammer.m		 = 0.0040246617466324;
	notes[61].hammer.p		 = 2.7009504694498281;
	notes[61].string.Min		 = 10;
	notes[61].string.Mup		 = 48;
	notes[61].string.Mlow[0]	 = 46;
	notes[61].string.Mlow[1]	 = 45;
	notes[61].string.Mlow[2]	 = 44;
	notes[61].string.fs		 = 88200;
	notes[61].string.Z		 = 2.2061768457907611;
	notes[61].string.tau 		 = 0.9999977938255877;
	notes[61].string.Ref		 = 0.0000022061744122;
	notes[61].LP[0].glp		 = 0.7545000000000000;
	notes[61].LP[0].a1		 = -0.2434904722581937;
	notes[61].LP[1].glp		 = 0.7545000000000000;
	notes[61].LP[1].a1		 = -0.2434904722581937;
	notes[61].LP[2].glp		 = 0.7545000000000000;
	notes[61].LP[2].a1		 = -0.2434904722581937;
	notes[61].tunning[0].a1		 = 0.4704754447914179;
	notes[61].tunning[1].a1		 = 0.4608424786069530;
	notes[61].tunning[2].a1		 = 0.4558882493218019;
	notes[61].allpass.a1		 = -0.9727302056606408;
	notes[61].allpass.a2		 = 0.9464430018848337;
	notes[61].Ns			 = 3;
	notes[61].M			 = 1;
	notes[61].control.MaxV		 = 2.3;
	notes[61].control.MinV		 = 0.5;
	notes[61].control.scale 	 = 100;
	
// B5
	notes[62].hammer.K		 = 14439339781.2727070000000000;
	notes[62].hammer.fsh		 = 176400;
	notes[62].hammer.m		 = 0.0039335677328989;
	notes[62].hammer.p		 = 2.7124416078169302;
	notes[62].string.Min		 = 10;
	notes[62].string.Mup		 = 45;
	notes[62].string.Mlow[0]	 = 43;
	notes[62].string.Mlow[1]	 = 42;
	notes[62].string.Mlow[2]	 = 41;
	notes[62].string.fs		 = 88200;
	notes[62].string.Z		 = 2.1395846857174248;
	notes[62].string.tau 		 = 0.9999978604176032;
	notes[62].string.Ref		 = 0.0000021395823968;
	notes[62].LP[0].glp		 = 0.7545000000000000;
	notes[62].LP[0].a1		 = -0.2434904722581937;
	notes[62].LP[1].glp		 = 0.7545000000000000;
	notes[62].LP[1].a1		 = -0.2434904722581937;
	notes[62].LP[2].glp		 = 0.7545000000000000;
	notes[62].LP[2].a1		 = -0.2434904722581937;
	notes[62].tunning[0].a1		 = 0.3616267880638732;
	notes[62].tunning[1].a1		 = 0.3492419250839998;
	notes[62].tunning[2].a1		 = 0.3396929041568076;
	notes[62].allpass.a1		 = -0.9734770025580085;
	notes[62].allpass.a2		 = 0.9478837444821507;
	notes[62].Ns			 = 3;
	notes[62].M			 = 1;
	notes[62].control.MaxV		 = 2;
	notes[62].control.MinV		 = 0.5;
	notes[62].control.scale 	 = 100;
	
	
// C6
	notes[63].hammer.K		 = 15696280275.9811840000000000;
	notes[63].hammer.fsh		 = 176400;
	notes[63].hammer.m		 = 0.0038437964488618;
	notes[63].hammer.p		 = 2.7239314763577651;
	notes[63].string.Min		 = 10;
	notes[63].string.Mup		 = 43;
	notes[63].string.Mlow[0]	 = 40;
	notes[63].string.Mlow[1]	 = 39;
	notes[63].string.Mlow[2]	 = 38;
	notes[63].string.fs		 = 88200;
	notes[63].string.Z		 = 2.0752921505779409;
	notes[63].string.tau 		 = 0.9999979247100029;
	notes[63].string.Ref		 = 0.0000020752899972;
	notes[63].LP[0].glp		 = 0.7560000000000000;
	notes[63].LP[0].a1		 = -0.2434904722581937;
	notes[63].LP[1].glp		 = 0.7560000000000000;
	notes[63].LP[1].a1		 = -0.2434904722581937;
	notes[63].LP[2].glp		 = 0.7560000000000000;
	notes[63].LP[2].a1		 = -0.2434904722581937;
	notes[63].tunning[0].a1		 = 0.3694674275646077;
	notes[63].tunning[1].a1		 = 0.3502284550081110;
	notes[63].tunning[2].a1		 = 0.3345320678705913;
	notes[63].allpass.a1		 = -0.9742177604580250;
	notes[63].allpass.a2		 = 0.9493142673996708;
	notes[63].Ns			 = 3;
	notes[63].M			 = 1;
	notes[63].control.MaxV		 = 1.7;
	notes[63].control.MinV		 = 0.5;
	notes[63].control.scale 	 = 100;

// CS6
	notes[64].hammer.K		 = 17062816811.1742000000000000;
	notes[64].hammer.fsh		 = 176400;
	notes[64].hammer.m		 = 0.0037552873609434;
	notes[64].hammer.p		 = 2.7354227948387297;
	notes[64].string.Min		 = 10;
	notes[64].string.Mup		 = 40;
	notes[64].string.Mlow[0]	 = 38;
	notes[64].string.Mlow[1]	 = 38;
	notes[64].string.Mlow[2]	 = 37;
	notes[64].string.fs		 = 88200;
	notes[64].string.Z		 = 2.0133684759776243;
	notes[64].string.tau 		 = 0.9999979866335509;
	notes[64].string.Ref		 = 0.0000020133664492;
	notes[64].LP[0].glp		 = 0.7561000000000000;
	notes[64].LP[0].a1		 = -0.2434904722581937;
	notes[64].LP[1].glp		 = 0.7561000000000000;
	notes[64].LP[1].a1		 = -0.2434904722581937;
	notes[64].LP[2].glp		 = 0.7561000000000000;
	notes[64].LP[2].a1		 = -0.2434904722581937;
	notes[64].tunning[0].a1		 = 0.3411385063774800;
	notes[64].tunning[1].a1		 = 0.4699636005482567;
	notes[64].tunning[2].a1		 = 0.4325117727021247;
	notes[64].allpass.a1		 = -0.9749490173150041;
	notes[64].allpass.a2		 = 0.9507278411186603;
	notes[64].Ns			 = 3;
	notes[64].M			 = 1;
	notes[64].control.MaxV		 = 1.7;
	notes[64].control.MinV		 = 0.5;
	notes[64].control.scale 	 = 100;
	
// D6
	notes[65].hammer.K		 = 18548314214.6787300000000000;
	notes[65].hammer.fsh		 = 176400;
	notes[65].hammer.m		 = 0.0036680149681070;
	notes[65].hammer.p		 = 2.7469140286983169;
	notes[65].string.Min		 = 10;
	notes[65].string.Mup		 = 38;
	notes[65].string.Mlow[0]	 = 36;
	notes[65].string.Mlow[1]	 = 35;
	notes[65].string.Mlow[2]	 = 34;
	notes[65].string.fs		 = 88200;
	notes[65].string.Z		 = 1.9538911249559991;
	notes[65].string.tau 		 = 0.9999980461107838;
	notes[65].string.Ref		 = 0.0000019538892161;
	notes[65].LP[0].glp		 = 0.7561000000000000;
	notes[65].LP[0].a1		 = -0.2434904722581937;
	notes[65].LP[1].glp		 = 0.7561000000000000;
	notes[65].LP[1].a1		 = -0.2434904722581937;
	notes[65].LP[2].glp		 = 0.7561000000000000;
	notes[65].LP[2].a1		 = -0.2434904722581937;
	notes[65].tunning[0].a1		 = 0.4124907598292154;
	notes[65].tunning[1].a1		 = 0.3751132720912874;
	notes[65].tunning[2].a1		 = 0.3451383614329858;
	notes[65].allpass.a1		 = -0.9756673249227570;
	notes[65].allpass.a2		 = 0.9521177370783357;
	notes[65].Ns			 = 3;
	notes[65].M			 = 1;
	notes[65].control.MaxV		 = 1.7;
	notes[65].control.MinV		 = 0.5;
	notes[65].control.scale 	 = 100;
	
// DS6
	notes[66].hammer.K		 = 20163128295.1182370000000000;
	notes[66].hammer.fsh		 = 176400;
	notes[66].hammer.m		 = 0.0035819433116557;
	notes[66].hammer.p		 = 2.7584051842278581;
	notes[66].string.Min		 = 10;
	notes[66].string.Mup		 = 36;
	notes[66].string.Mlow[0]	 = 34;
	notes[66].string.Mlow[1]	 = 33;
	notes[66].string.Mlow[2]	 = 32;
	notes[66].string.fs		 = 88200;
	notes[66].string.Z		 = 1.8969141989381499;
	notes[66].string.tau 		 = 0.9999981030876002;
	notes[66].string.Ref		 = 0.0000018969123998;
	notes[66].LP[0].glp		 = 0.7570000000000000;
	notes[66].LP[0].a1		 = -0.2434904722581937;
	notes[66].LP[1].glp		 = 0.7570000000000000;
	notes[66].LP[1].a1		 = -0.2434904722581937;
	notes[66].LP[2].glp		 = 0.7570000000000000;
	notes[66].LP[2].a1		 = -0.2434904722581937;
	notes[66].tunning[0].a1		 = 0.4607584322374345;
	notes[66].tunning[1].a1		 = 0.4074201206020061;
	notes[66].tunning[2].a1		 = 0.3669889637582603;
	notes[66].allpass.a1		 = -0.9763696289922983;
	notes[66].allpass.a2		 = 0.9534779652542629;
	notes[66].Ns			 = 3;
	notes[66].M			 = 1;
	notes[66].control.MaxV		 = 1.5;
	notes[66].control.MinV		 = 0.2;
	notes[66].control.scale 	 = 150;
	

// E6
	notes[67].hammer.K		 = 21918442342.2028120000000000;
	notes[67].hammer.fsh		 = 176400;
	notes[67].hammer.m		 = 0.0034970414181179;
	notes[67].hammer.p		 = 2.7698958024082909;
	notes[67].string.Min		 = 10;
	notes[67].string.Mup		 = 34;
	notes[67].string.Mlow[0]	 = 32;
	notes[67].string.Mlow[1]	 = 31;
	notes[67].string.Mlow[2]	 = 30;
	notes[67].string.fs		 = 88200;
	notes[67].string.Z		 = 1.8424805151423580;
	notes[67].string.tau 		 = 0.9999981575211823;
	notes[67].string.Ref		 = 0.0000018424788178;
	notes[67].LP[0].glp		 = 0.7570000000000000;
	notes[67].LP[0].a1		 = -0.2434904722581937;
	notes[67].LP[1].glp		 = 0.7570000000000000;
	notes[67].LP[1].a1		 = -0.2434904722581937;
	notes[67].LP[2].glp		 = 0.7570000000000000;
	notes[67].LP[2].a1		 = -0.2434904722581937;
	notes[67].tunning[0].a1		 = 0.4639493604357612;
	notes[67].tunning[1].a1		 = 0.4032712666938180;
	notes[67].tunning[2].a1		 = 0.3580949265380914;
	notes[67].allpass.a1		 = -0.9770531425562138;
	notes[67].allpass.a2		 = 0.9548030334065620;
	notes[67].Ns			 = 3;
	notes[67].M			 = 1;
	notes[67].control.MaxV		 = 1.2;
	notes[67].control.MinV		 = 0.1;
	notes[67].control.scale 	 = 140;
	
// F6
	notes[68].hammer.K		 = 23826563288.0129170000000000;
	notes[68].hammer.fsh		 = 176400;
	notes[68].hammer.m		 = 0.0034132725262823;
	notes[68].hammer.p		 = 2.7813864027373887;
	notes[68].string.Min		 = 10;
	notes[68].string.Mup		 = 32;
	notes[68].string.Mlow[0]	 = 30;
	notes[68].string.Mlow[1]	 = 29;
	notes[68].string.Mlow[2]	 = 29;
	notes[68].string.fs		 = 88200;
	notes[68].string.Z		 = 1.7906159369668315;
	notes[68].string.tau 		 = 0.9999982093856662;
	notes[68].string.Ref		 = 0.0000017906143338;
	notes[68].LP[0].glp		 = 0.7570000000000000;
	notes[68].LP[0].a1		 = -0.2434904722581937;
	notes[68].LP[1].glp		 = 0.7570000000000000;
	notes[68].LP[1].a1		 = -0.2434904722581937;
	notes[68].LP[2].glp		 = 0.7570000000000000;
	notes[68].LP[2].a1		 = -0.2434904722581937;
	notes[68].tunning[0].a1		 = 0.4226722041407341;
	notes[68].tunning[1].a1		 = 0.3665506200492869;
	notes[68].tunning[2].a1		 = 0.4805715262929506;
	notes[68].allpass.a1		 = -0.9777154284737187;
	notes[68].allpass.a2		 = 0.9560881109105999;
	notes[68].Ns			 = 3;
	notes[68].M			 = 1;
	notes[68].control.MaxV		 = 1.2;
	notes[68].control.MinV		 = 0.1;
	notes[68].control.scale 	 = 140;
	
// FS6
	notes[69].hammer.K		 = 25901005387.3539540000000000;
	notes[69].hammer.fsh		 = 176400;
	notes[69].hammer.m		 = 0.0033305971280496;
	notes[69].hammer.p		 = 2.7928781124242787;
	notes[69].string.Min		 = 10;
	notes[69].string.Mup		 = 30;
	notes[69].string.Mlow[0]	 = 28;
	notes[69].string.Mlow[1]	 = 28;
	notes[69].string.Mlow[2]	 = 27;
	notes[69].string.fs		 = 88200;
	notes[69].string.Z		 = 1.7413332571169831;
	notes[69].string.tau 		 = 0.9999982586682590;
	notes[69].string.Ref		 = 0.0000017413317410;
	notes[69].LP[0].glp		 = 0.7570000000000000;
	notes[69].LP[0].a1		 = -0.2434904722581937;
	notes[69].LP[1].glp		 = 0.7570000000000000;
	notes[69].LP[1].a1		 = -0.2434904722581937;
	notes[69].LP[2].glp		 = 0.7570000000000000;
	notes[69].LP[2].a1		 = -0.2434904722581937;
	notes[69].tunning[0].a1		 = 0.3585764655891193;
	notes[69].tunning[1].a1		 = 0.4572806282987429;
	notes[69].tunning[2].a1		 = 0.3880380936956421;
	notes[69].allpass.a1		 = -0.9783543620645883;
	notes[69].allpass.a2		 = 0.9573289605139445;
	notes[69].Ns			 = 3;
	notes[69].M			 = 1;
	notes[69].control.MaxV		 = 0.8;
	notes[69].control.MinV		 = 0.1;
	notes[69].control.scale 	 = 150;
	
// G6
	notes[70].hammer.K		 = 28155799802.5397610000000000;
	notes[70].hammer.fsh		 = 176400;
	notes[70].hammer.m		 = 0.0032490021037045;
	notes[70].hammer.p		 = 2.8043685634118587;
	notes[70].string.Min		 = 10;
	notes[70].string.Mup		 = 29;
	notes[70].string.Mlow[0]	 = 26;
	notes[70].string.Mlow[1]	 = 25;
	notes[70].string.Mlow[2]	 = 25;
	notes[70].string.fs		 = 88200;
	notes[70].string.Z		 = 1.6946503334159255;
	notes[70].string.tau 		 = 0.9999983053511026;
	notes[70].string.Ref		 = 0.0000016946488975;
	notes[70].LP[0].glp		 = 0.7570000000000000;
	notes[70].LP[0].a1		 = -0.2434904722581937;
	notes[70].LP[1].glp		 = 0.7570000000000000;
	notes[70].LP[1].a1		 = -0.2434904722581937;
	notes[70].LP[2].glp		 = 0.7570000000000000;
	notes[70].LP[2].a1		 = -0.2434904722581937;
	notes[70].tunning[0].a1		 = 0.4144380680087277;
	notes[70].tunning[1].a1		 = 0.3515275176650278;
	notes[70].tunning[2].a1		 = 0.4404854311157807;
	notes[70].allpass.a1		 = -0.9789678981177036;
	notes[70].allpass.a2		 = 0.9585214891968447;
	notes[70].Ns			 = 3;
	notes[70].M			 = 1;
	notes[70].control.MaxV		 = 0.63;
	notes[70].control.MinV		 = 0.2;
	notes[70].control.scale 	 = 150;
	
// GS6
	notes[71].hammer.K		 = 30607083425.3117520000000000;
	notes[71].hammer.fsh		 = 176400;
	notes[71].hammer.m		 = 0.0031684431926749;
	notes[71].hammer.p		 = 2.8158599121714687;
	notes[71].string.Min		 = 10;
	notes[71].string.Mup		 = 27;
	notes[71].string.Mlow[0]	 = 25;
	notes[71].string.Mlow[1]	 = 24;
	notes[71].string.Mlow[2]	 = 24;
	notes[71].string.fs		 = 88200;
	notes[71].string.Z		 = 1.6505574929970321;
	notes[71].string.tau 		 = 0.9999983494438691;
	notes[71].string.Ref		 = 0.0000016505561308;
	notes[71].LP[0].glp		 = 0.7590000000000000;
	notes[71].LP[0].a1		 = -0.2434904722581937;
	notes[71].LP[1].glp		 = 0.7590000000000000;
	notes[71].LP[1].a1		 = -0.2434904722581937;
	notes[71].LP[2].glp		 = 0.7590000000000000;
	notes[71].LP[2].a1		 = -0.2434904722581937;
	notes[71].tunning[0].a1		 = 0.4491806468713099;
	notes[71].tunning[1].a1		 = 0.3717485472710588;
	notes[71].tunning[2].a1		 = 0.4654053914571211;
	notes[71].allpass.a1		 = -0.9795544977189181;
	notes[71].allpass.a2		 = 0.9596625807459770;
	notes[71].Ns			 = 3;
	notes[71].M			 = 1;
	notes[71].control.MaxV		 = 0.45;
	notes[71].control.MinV		 = 0.2;
	notes[71].control.scale 	 = 900;




}


