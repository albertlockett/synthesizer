// HEADER FILE FOR THE FLUTE:
// Created March 16, 2013 - AGL

// Structures ---------------------------------------------------------------------------

struct note_s{
	int Finger[6];
	int Njet;
	int defined;	// Because not all notes are
};

struct Flute_s{
	double BoundaryNum[3];
	double BoundaryDen[3];
	double OpenEndNum[2];
	double OpenEndDen[3];
	double OpenToneHoleNum[2];
	double OpenToneHoleDen[2];
	double ClosedToneHoleNum[2];
	double ClosedToneHoleDen[2];
	double ToneHoler0;
	int DelayTotal;
	int Dsum[6];
	int NumToneHoles;
	double kv;
	double kn;
	double kvib;
	double fvib;
	double kfb;
	double kdfb;
	double JetGain;
	double scale;
};

struct sample_create_s{
	struct note_s note;
	struct Flute_s Flute;
	double *channel;
	int active_flag;
	int note_num;
	double delay_up[56];
	double delay_low[56];
	double * delay_jet;
	double z_branch[6];
	double p_branch[6];
	double z_open[2];
	double z_boundary[2][2];
	double noise_filt[2];
	double f[2];
	double c[2];
	int dup_zer, dup_dto;	// Pointers for upper delay line
	int dlo_zer, dlo_dto;	// Pointers for lower delay line
	int djt_zer, djt_njt;	// Pointers for jet delay line
	int cycle_count;
	int fade;
	double old_out;
	double old_out2;
};

	
// Functions -----------------------------------------------------------------------------
	
struct Flute_s flute_init(){
	struct Flute_s Flute;

	Flute.BoundaryNum[0] = 0.90496106280386546000;
	Flute.BoundaryNum[1] = -0.30898048357671820000;
	Flute.BoundaryNum[2] = -0.38711114873121610000;
	Flute.BoundaryDen[0] = 1.00000000000000000000;
	Flute.BoundaryDen[1] = -0.42312673883642404000;
	Flute.BoundaryDen[2] = -0.36705482673716577000;
	Flute.OpenEndNum[0] = -0.19252733878840766000;
	Flute.OpenEndNum[1] = -0.32788342098989970000;
	Flute.OpenEndDen[0] = 1.00000000000000000000;
	Flute.OpenEndDen[1] 		= -0.5496946657562632;
	Flute.OpenEndDen[2] 		=  0.0700217255890797;
	Flute.OpenToneHoleNum[0] = 0.09170000000000000400;
	Flute.OpenToneHoleNum[1] = -0.67959999999999998000;
	Flute.OpenToneHoleDen[0] = 1.00000000000000000000;
	Flute.OpenToneHoleDen[1] = -0.41230000000000000000;
	Flute.ClosedToneHoleNum[0] = 0.00000000000000000000;
	Flute.ClosedToneHoleNum[1] = 1.00000000000000000000;
	Flute.ClosedToneHoleDen[0] = 1.00000000000000000000;
	Flute.ClosedToneHoleDen[1] = 0.00000000000000000000;
	Flute.ToneHoler0 = -0.11215777896529013000;
	Flute.DelayTotal = 56;
	Flute.Dsum[0] = 31;
	Flute.Dsum[1] = 35;
	Flute.Dsum[2] = 40;
	Flute.Dsum[3] = 41;
	Flute.Dsum[4] = 46;
	Flute.NumToneHoles = 5;
	Flute.kv = 0.08000000000000000200;
	Flute.kn = 1.00000000000000000000;
	Flute.kvib = 0.10000000000000001000;
	Flute.fvib = 3.50000000000000000000;
	Flute.kfb = -0.96999999999999997000;
	Flute.kdfb = -0.10000000000000001000;
	Flute.JetGain = 4.00000000000000000000;
	Flute.scale = 0.10000000000000001000;

	
	return Flute;

}


struct Flute_s flute_1_init(){

        struct Flute_s Flute;
        // Flute 1:
// Notes: G4, A4, B4, C5, D5, E5, F5
Flute.BoundaryNum[0] = 0.91676928748785735000;
Flute.BoundaryNum[1] = -0.31701188754791282000;
Flute.BoundaryNum[2] = -0.38871471627702231000;
Flute.BoundaryDen[0] = 1.00000000000000000000;
Flute.BoundaryDen[1] = -0.41686077844727665000;
Flute.BoundaryDen[2] = -0.37126143309914272000;
Flute.OpenEndNum[0] = -0.19252733878840766000;
Flute.OpenEndNum[1] = -0.32788342098989970000;
Flute.OpenEndDen[0] = 1.00000000000000000000;
Flute.OpenEndDen[1] = -0.54969466575626325000;
Flute.OpenEndDen[2] = 0.07002172558907969500;
Flute.OpenToneHoleNum[0] = 0.09170000000000000400;
Flute.OpenToneHoleNum[1] = -0.67959999999999998000;
Flute.OpenToneHoleDen[0] = 1.00000000000000000000;
Flute.OpenToneHoleDen[1] = -0.41230000000000000000;
Flute.ClosedToneHoleNum[0] = 0.00000000000000000000;
Flute.ClosedToneHoleNum[1] = 1.00000000000000000000;
Flute.ClosedToneHoleDen[0] = 1.00000000000000000000;
Flute.ClosedToneHoleDen[1] = 0.00000000000000000000;
Flute.ToneHoler0 = -0.11215777896529013000;
Flute.DelayTotal = 56;
Flute.Dsum[0] = 27;
Flute.Dsum[1] = 31;
Flute.Dsum[2] = 34;
Flute.Dsum[3] = 39;
Flute.Dsum[4] = 41;
Flute.Dsum[5] = 44;
Flute.NumToneHoles = 6;
Flute.kv = 0.08000000000000000200;
Flute.kn = 1.00000000000000000000;
Flute.kvib = 0.10000000000000001000;
Flute.fvib = 3.50000000000000000000;
Flute.kfb = -0.96999999999999997000;
Flute.kdfb = -0.10000000000000001000;
Flute.JetGain = 4.00000000000000000000;
Flute.scale = 0.10000000000000001000;

return Flute;
}


struct Flute_s flute_2_init(){

        struct Flute_s Flute;
        
        // Flute 2:
// C5, A5, B5, C6, D6 E6
Flute.BoundaryNum[0] = 0.90790082454511223000;
Flute.BoundaryNum[1] = -0.31097636819108948000;
Flute.BoundaryNum[2] = -0.38751303676318311000;
Flute.BoundaryDen[0] = 1.00000000000000000000;
Flute.BoundaryDen[1] = -0.42155971498119688000;
Flute.BoundaryDen[2] = -0.36810810399319144000;
Flute.OpenEndNum[0] = -0.19252733878840766000;
Flute.OpenEndNum[1] = -0.32788342098989970000;
Flute.OpenEndDen[0] = 1.00000000000000000000;
Flute.OpenEndDen[1] = -0.54969466575626325000;
Flute.OpenEndDen[2] = 0.07002172558907969500;
Flute.OpenToneHoleNum[0] = 0.09170000000000000400;
Flute.OpenToneHoleNum[1] = -0.67959999999999998000;
Flute.OpenToneHoleDen[0] = 1.00000000000000000000;
Flute.OpenToneHoleDen[1] = -0.41230000000000000000;
Flute.ClosedToneHoleNum[0] = 0.00000000000000000000;
Flute.ClosedToneHoleNum[1] = 1.00000000000000000000;
Flute.ClosedToneHoleDen[0] = 1.00000000000000000000;
Flute.ClosedToneHoleDen[1] = 0.00000000000000000000;
Flute.ToneHoler0 = -0.11215777896529013000;
Flute.DelayTotal = 56;
Flute.Dsum[0] = 30;
Flute.Dsum[1] = 32;
Flute.Dsum[2] = 39;
Flute.Dsum[3] = 40;
Flute.Dsum[4] = 45;
Flute.NumToneHoles = 5;
Flute.kv = 0.08000000000000000200;
Flute.kn = 1.00000000000000000000;
Flute.kvib = 0.10000000000000001000;
Flute.fvib = 3.50000000000000000000;
Flute.kfb = -0.96999999999999997000;
Flute.kdfb = -0.10000000000000001000;
Flute.JetGain = 4.00000000000000000000;
Flute.scale = 0.10000000000000001000;
        
        
        return Flute;

}

void note_init(struct note_s * note){
// Since we only have certain notes, I'm going to use the same numbering we did for the 
// piano header file here. It might end up being a waste of memory but at least it will 
// keep things consistant when we decode the MIDI. Plus we have lots of memory :D.
// This also makes life a bit easier when/if we add some more notes ( .. in the future )

// TL;DR This consistant init method is a reckless waste of memory

// Set all Non-Defined notes to 0 so they don't get played.
// NOTE: If you change the array of notes size to less (or more) this might (will)
// cause problems
	int i;
	for(i=0;i<88;i++){
		note[i].defined = 0;
	}

// G4
	note[46].Finger[0] = 1;
	note[46].Finger[1] = 1;
	note[46].Finger[2] = 1;
	note[46].Finger[3] = 1;
	note[46].Finger[4] = 1;
	note[46].Finger[5] = 1;
	note[46].Njet = 50;
	note[46].defined = 1;
	
// A4
	note[48].Finger[0] = 1;
	note[48].Finger[1] = 1;
	note[48].Finger[2] = 1;
	note[48].Finger[3] = 1;
	note[48].Finger[4] = 1;
	note[48].Finger[5] = 0;
	note[48].Njet = 56;
	note[48].defined = 1;
	
// B4
	note[50].Finger[0] = 1;
	note[50].Finger[1] = 1;
	note[50].Finger[2] = 1;
	note[50].Finger[3] = 1;
	note[50].Finger[4] = 0;
	note[50].Finger[5] = 0;
	note[50].Njet = 41;
	note[50].defined = 1;

// C5
	note[51].Finger[0] = 1;
	note[51].Finger[1] = 1;
	note[51].Finger[2] = 1;
	note[51].Finger[3] = 0;
	note[51].Finger[4] = 0;
        note[51].Finger[5] = 0;
	note[51].Njet = 41;
	note[51].defined = 1;
	
// D5
	note[53].Finger[0] = 1;
	note[53].Finger[1] = 1;
	note[53].Finger[2] = 0;
	note[53].Finger[3] = 0;
	note[53].Finger[4] = 0;
	note[53].Finger[5] = 0;
	note[53].Njet = 29;
	note[53].defined = 1;
	
// E5
	note[55].Finger[0] = 1;
	note[55].Finger[1] = 0;
	note[55].Finger[2] = 0;
	note[55].Finger[3] = 0;
	note[55].Finger[4] = 0;
	note[55].Finger[5] = 0;
	note[55].Njet = 26;
	note[55].defined = 1;

// F5
        note[56].Finger[0] = 0;
        note[56].Finger[1] = 0;
        note[56].Finger[2] = 0;
        note[56].Finger[3] = 0;
        note[56].Finger[4] = 0;
        note[56].Finger[5] = 0;
        note[56].Njet = 39;
        note[56].defined = 1;
        

// G5
	note[58].Finger[0] = 1;
	note[58].Finger[1] = 1;
	note[58].Finger[2] = 1;
	note[58].Finger[3] = 1;
	note[58].Finger[4] = 1;
	note[58].Njet = 143;
	note[58].defined = 1;
	
// A5
	note[60].Finger[0] = 1;
	note[60].Finger[1] = 1;
	note[60].Finger[2] = 1;
	note[60].Finger[3] = 1;
	note[60].Finger[4] = 0;
	note[60].Njet = 73;
	note[60].defined = 1;

// B5
	note[62].Finger[0] = 1;
	note[62].Finger[1] = 1;
	note[62].Finger[2] = 1;
	note[62].Finger[3] = 0;
	note[62].Finger[4] = 0;
	note[62].Njet = 68;
	note[62].defined = 1;
	
// C6
	note[63].Finger[0] = 1;
	note[63].Finger[1] = 1;
	note[63].Finger[2] = 0;
	note[63].Finger[3] = 0;
	note[63].Finger[4] = 0;
	note[63].Njet = 62;
	note[63].defined = 1;

// D6
	note[65].Finger[0] = 1;
	note[65].Finger[1] = 0;
	note[65].Finger[2] = 0;
	note[65].Finger[3] = 0;
	note[65].Finger[4] = 0;
	note[65].Njet = 59;
	note[65].defined = 1;
	
// E6
	note[67].Finger[0] = 0;
	note[67].Finger[1] = 0;
	note[67].Finger[2] = 0;
	note[67].Finger[3] = 0;
	note[67].Finger[4] = 0;
	note[67].Njet = 56;
	note[67].defined = 1;

}
