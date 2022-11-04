// HEADER FILE FOR THE CLARINET
// Created March 25, 2013 - AGL


// STRUCTURES ----------------------------------------------------------

struct note_s{
    int Finger[11];
    int defined;
};


struct Clarinet_s{
 	double BoundaryNum[3];
	double BoundaryDen[3];
	double OpenEndNum[2];
	double OpenEndDen[3];
	double OpenToneHoleNum[2];
	double OpenToneHoleDen[2];
	double ClosedToneHoleNum[2];
	double ClosedToneHoleDen[2];
	double ToneHoler0;
	int Dsum[11];
	int NumToneHoles;   
   	int DelayTotal; 

};


struct sample_create_s{
    struct note_s note;
    struct Clarinet_s Clarinet;
    double * channel;
    int active_flag;
    int note_num;
    
    double z_branch[11];
    double p_branch[11];
    double z_open[2];
    double z_boundary[2][2];
    double delay_up[84];
    double delay_low[84];
    double p_oc;
 	int fade;   
    double cycle_count;
};


// FUNCTIONS ----------------------------------------------------------

struct Clarinet_s clarinet_init(){
    // It's confusing that the clarinet in this isntance is a variable called flute
    // but it made coding easier because I could copy and paste..
    // Also my text editor doesn't have find and replace
    struct Clarinet_s Flute;
    
    Flute.BoundaryNum[0] = 0.87600861637198602000;
    Flute.BoundaryNum[1] = -0.28945225772580335000;
    Flute.BoundaryNum[2] = -0.38305834503783598000;
    Flute.BoundaryDen[0] = 1.00000000000000000000;
    Flute.BoundaryDen[1] = -0.43881728419806310000;
    Flute.BoundaryDen[2] = -0.35646169971076291000;
    Flute.OpenEndNum[0] = -0.19252733878840766000;
    Flute.OpenEndNum[1] = -0.32788342098989970000;
    Flute.OpenEndDen[0] = 1.00000000000000000000;
    Flute.OpenEndDen[1] = -0.54969466575626325000;
    Flute.OpenEndDen[2] = 0.07002172558907969500;
    Flute.OpenToneHoleNum[0] = 0.09329999999999999400;
    Flute.OpenToneHoleNum[1] = -0.74370000000000003000;
    Flute.OpenToneHoleDen[0] = 1.00000000000000000000;
    Flute.OpenToneHoleDen[1] = -0.34970000000000001000;
    Flute.ClosedToneHoleNum[0] = 0.00000000000000000000;
    Flute.ClosedToneHoleNum[1] = 1.00000000000000000000;
    Flute.ClosedToneHoleDen[0] = 1.00000000000000000000;
    Flute.ClosedToneHoleDen[1] = 0.00000000000000000000;
    Flute.ToneHoler0 = -0.08221782585236760100;
    Flute.DelayTotal = 84;
    Flute.Dsum[0] = 41;
    Flute.Dsum[1] = 44;
    Flute.Dsum[2] = 46;
    Flute.Dsum[3] = 49;
    Flute.Dsum[4] = 52;
    Flute.Dsum[5] = 55;
    Flute.Dsum[6] = 59;
    Flute.Dsum[7] = 62;
    Flute.Dsum[8] = 66;
    Flute.Dsum[9] = 71;
    Flute.Dsum[10] = 75;
    Flute.NumToneHoles = 11;
    
    return Flute;
    
}

void note_init(struct note_s * note){
    
    int i;
    for(i=0;i<88;i++){
        note[i].defined = 0;
    }
  
//    C3=[1 1 1 1 1 1 1 1 1 1 1]; % = note 27
    note[27].Finger[0] = 1;
    note[27].Finger[1] = 1;
    note[27].Finger[2] = 1;
    note[27].Finger[3] = 1;
    note[27].Finger[4] = 1;
    note[27].Finger[5] = 1;
    note[27].Finger[6] = 1;
    note[27].Finger[7] = 1;
    note[27].Finger[8] = 1;
    note[27].Finger[9] = 1;
    note[27].Finger[10] = 1;
    note[27].defined = 1;
    
//    CS3=[1 1 1 1 1 1 1 1 1 1 0];
    note[28].Finger[0] = 1;
    note[28].Finger[1] = 1;
    note[28].Finger[2] = 1;
    note[28].Finger[3] = 1;
    note[28].Finger[4] = 1;
    note[28].Finger[5] = 1;
    note[28].Finger[6] = 1;
    note[28].Finger[7] = 1;
    note[28].Finger[8] = 1;
    note[28].Finger[9] = 1;
    note[28].Finger[10] = 0;
    note[28].defined = 1;
    
//    D3=[1 1 1 1 1 1 1 1 1 0 0];
    note[29].Finger[0] = 1;
    note[29].Finger[1] = 1;
    note[29].Finger[2] = 1;
    note[29].Finger[3] = 1;
    note[29].Finger[4] = 1;
    note[29].Finger[5] = 1;
    note[29].Finger[6] = 1;
    note[29].Finger[7] = 1;
    note[29].Finger[8] = 1;
    note[29].Finger[9] = 0;
    note[29].Finger[10] = 0;
    note[29].defined = 1;
    
//    DS3=[1 1 1 1 1 1 1 1 0 0 0];
    note[30].Finger[0] = 1;
    note[30].Finger[1] = 1;
    note[30].Finger[2] = 1;
    note[30].Finger[3] = 1;
    note[30].Finger[4] = 1;
    note[30].Finger[5] = 1;
    note[30].Finger[6] = 1;
    note[30].Finger[7] = 1;
    note[30].Finger[8] = 0;
    note[30].Finger[9] = 0;
    note[30].Finger[10] = 0;
    note[30].defined = 1;
    
//    E3=[1 1 1 1 1 1 1 0 0 0 0];
    note[31].Finger[0] = 1;
    note[31].Finger[1] = 1;
    note[31].Finger[2] = 1;
    note[31].Finger[3] = 1;
    note[31].Finger[4] = 1;
    note[31].Finger[5] = 1;
    note[31].Finger[6] = 1;
    note[31].Finger[7] = 0;
    note[31].Finger[8] = 0;
    note[31].Finger[9] = 0;
    note[31].Finger[10] = 0;
    note[31].defined = 1;
    
//    F3=[1 1 1 1 1 1 0 0 0 0 0];
    note[32].Finger[0] = 1;
    note[32].Finger[1] = 1;
    note[32].Finger[2] = 1;
    note[32].Finger[3] = 1;
    note[32].Finger[4] = 1;
    note[32].Finger[5] = 1;
    note[32].Finger[6] = 0;
    note[32].Finger[7] = 0;
    note[32].Finger[8] = 0;
    note[32].Finger[9] = 0;
    note[32].Finger[10] = 0;
    note[32].defined = 1;
    
//    FS3=[1 1 1 1 1 0 0 0 0 0 0];
    note[33].Finger[0] = 1;
    note[33].Finger[1] = 1;
    note[33].Finger[2] = 1;
    note[33].Finger[3] = 1;
    note[33].Finger[4] = 1;
    note[33].Finger[5] = 0;
    note[33].Finger[6] = 0;
    note[33].Finger[7] = 0;
    note[33].Finger[8] = 0;
    note[33].Finger[9] = 0;
    note[33].Finger[10] = 0;
    note[33].defined = 1;
    
//    G3=[1 1 1 1 0 0 0 0 0 0 0];
    note[34].Finger[0] = 1;
    note[34].Finger[1] = 1;
    note[34].Finger[2] = 1;
    note[34].Finger[3] = 1;
    note[34].Finger[4] = 0;
    note[34].Finger[5] = 0;
    note[34].Finger[6] = 0;
    note[34].Finger[7] = 0;
    note[34].Finger[8] = 0;
    note[34].Finger[9] = 0;
    note[34].Finger[10] = 0;
    note[34].defined = 1;
    
//    GS3=[1 1 1 0 0 0 0 0 0 0 0];
    note[35].Finger[0] = 1;
    note[35].Finger[1] = 1;
    note[35].Finger[2] = 1;
    note[35].Finger[3] = 0;
    note[35].Finger[4] = 0;
    note[35].Finger[5] = 0;
    note[35].Finger[6] = 0;
    note[35].Finger[7] = 0;
    note[35].Finger[8] = 0;
    note[35].Finger[9] = 0;
    note[35].Finger[10] = 0;
    note[35].defined = 1;
    
//    A3=[1 1 0 0 0 0 0 0 0 0 0];
    note[36].Finger[0] = 1;
    note[36].Finger[1] = 1;
    note[36].Finger[2] = 0;
    note[36].Finger[3] = 0;
    note[36].Finger[4] = 0;
    note[36].Finger[5] = 0;
    note[36].Finger[6] = 0;
    note[36].Finger[7] = 0;
    note[36].Finger[8] = 0;
    note[36].Finger[9] = 0;
    note[36].Finger[10] = 0;
    note[36].defined = 1;
    
//    AS3=[1 0 0 0 0 0 0 0 0 0 0];
    note[37].Finger[0] = 1;
    note[37].Finger[1] = 0;
    note[37].Finger[2] = 0;
    note[37].Finger[3] = 0;
    note[37].Finger[4] = 0;
    note[37].Finger[5] = 0;
    note[37].Finger[6] = 0;
    note[37].Finger[7] = 0;
    note[37].Finger[8] = 0;
    note[37].Finger[9] = 0;
    note[37].Finger[10] = 0;
    note[37].defined = 1;
    
//    B3=[0 0 0 0 0 0 0 0 0 0 0];
    note[38].Finger[0] = 0;
    note[38].Finger[1] = 0;
    note[38].Finger[2] = 0;
    note[38].Finger[3] = 0;
    note[38].Finger[4] = 0;
    note[38].Finger[5] = 0;
    note[38].Finger[6] = 0;
    note[38].Finger[7] = 0;
    note[38].Finger[8] = 0;
    note[38].Finger[9] = 0;
    note[38].Finger[10] = 0;
    note[38].defined = 1;
    
}
