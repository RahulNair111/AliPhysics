/////////////////////////////////////////////////////////////////////////////////////////////
//
// AddTask* macro for flow analysis
// Creates a Flow Event task and adds it to the analysis manager.
// Sets the cuts using the correction framework (CORRFW) classes.
// Also creates Flow Analysis tasks and connects them to the output of the flow event task.
//
/////////////////////////////////////////////////////////////////////////////////////////////

AliAnalysisTaskFlowEvent *AddTaskFlowCentralityBayesian(Float_t centrMin=0.,
                                                        Float_t centrMax=100.,
                                                        TString fileNameBase="output",
                                                        Int_t specie=2, 
                                                        Int_t charge=0, 
                                                        Int_t harmonic=2,
                                                        Bool_t doQA=kFALSE,
                                                        TString uniqueStr="" )
{
  // Define the range for eta subevents (for SP method)
  Double_t minA = -0.9;
  Double_t maxA = -0.5;
  Double_t minB = 0.5;
  Double_t maxB = 0.9;
  
  // AFTERBURNER
  Bool_t useAfterBurner=kFALSE;
  Double_t v1=0.0;
  Double_t v2=0.0;
  Double_t v3=0.0;
  Double_t v4=0.0;
  Int_t numberOfTrackClones=0; //non-flow
  
  // Define a range of the detector to exclude
  Bool_t ExcludeRegion = kFALSE;
  Double_t excludeEtaMin = -0.;
  Double_t excludeEtaMax = 0.;
  Double_t excludePhiMin = 0.;
  Double_t excludePhiMax = 0.;
  
  // use physics selection class
  Bool_t  UsePhysicsSelection = kTRUE;
  
  // QA
  Bool_t runQAtask=kFALSE;
  Bool_t FillQAntuple=kFALSE;
  Bool_t DoQAcorrelations=kFALSE;
  
  // RUN SETTINGS
  // Flow analysis method can be:(set to kTRUE or kFALSE)
  Bool_t MCEP     = kFALSE;  // correlation with Monte Carlo reaction plane
  Bool_t SP       = kTRUE;  // scalar product method (similar to eventplane method)
  Bool_t GFC      = kTRUE;  // cumulants based on generating function
  Bool_t QC       = kTRUE;  // cumulants using Q vectors
  Bool_t FQD      = kTRUE;  // fit of the distribution of the Q vector (only integrated v)
  Bool_t LYZ1SUM  = kTRUE;  // Lee Yang Zeroes using sum generating function (integrated v)
  Bool_t LYZ1PROD = kFALSE;  // Lee Yang Zeroes using product generating function (integrated v)
  Bool_t LYZ2SUM  = kFALSE; // Lee Yang Zeroes using sum generating function (second pass differential v)
  Bool_t LYZ2PROD = kFALSE; // Lee Yang Zeroes using product generating function (second pass differential v)
  Bool_t LYZEP    = kFALSE; // Lee Yang Zeroes Event plane using sum generating function (gives eventplane + weight)
  Bool_t MH       = kFALSE;  // azimuthal correlators in mixed harmonics  
  Bool_t NL       = kFALSE;  // nested loops (for instance distribution of phi1-phi2 for all distinct pairs)
  
  Bool_t METHODS[] = {SP,LYZ1SUM,LYZ1PROD,LYZ2SUM,LYZ2PROD,LYZEP,GFC,QC,FQD,MCEP,MH,NL};
  
  // Boolean to use/not use weights for the Q vector
  Bool_t WEIGHTS[] = {kFALSE,kFALSE,kFALSE}; //Phi, v'(pt), v'(eta)
  
  // SETTING THE CUTS
  
  //---------Data selection----------
  //kMC, kGlobal, kESD_TPConly, kESD_SPDtracklet, kPMD
  AliFlowTrackCuts::trackParameterType rptype = AliFlowTrackCuts::kGlobal;
  AliFlowTrackCuts::trackParameterType poitype = AliFlowTrackCuts::kGlobal;
  
  //---------Parameter mixing--------
  //kPure - no mixing, kTrackWithMCkine, kTrackWithMCPID, kTrackWithMCpt
  AliFlowTrackCuts::trackParameterMix rpmix = AliFlowTrackCuts::kPure;
  AliFlowTrackCuts::trackParameterMix poimix = AliFlowTrackCuts::kPure;
  
  
  const char* rptypestr = AliFlowTrackCuts::GetParamTypeName(rptype);
  const char* poitypestr = AliFlowTrackCuts::GetParamTypeName(poitype);

  TString fileName(fileNameBase);
  fileName.Append(".root");
  //===========================================================================
  printf("CREATE CUTS\n");
  cout << "Used for RP: "<< rptypestr << endl;  
  cout << "Used for POI: "<< poitypestr << endl;  

  // EVENTS CUTS:
  AliFlowEventCuts* cutsEvent = new AliFlowEventCuts("event cuts");
  cutsEvent->SetCentralityPercentileRange(centrMin,centrMax);
  cutsEvent->SetCentralityPercentileMethod(AliFlowEventCuts::kV0);
  cutsEvent->SetRefMultMethod(AliFlowEventCuts::kV0);
  //cutsEvent->SetCentralityPercentileMethod(AliFlowEventCuts::kSPD1tracklets);
  cutsEvent->SetNContributorsRange(2);
  cutsEvent->SetPrimaryVertexZrange(-7.,7.);
  cutsEvent->SetCutSPDvertexerAnomaly(); //"Francesco's cut"
  cutsEvent->SetCutZDCtiming();
  cutsEvent->SetCutTPCmultiplicityOutliers();
  //cutsEvent->SetUseCentralityUnchecked();
  //cutsEvent->SetQA(doQA);
 
  // RP TRACK CUTS:
  AliFlowTrackCuts* cutsRP = new AliFlowTrackCuts("TPConlyRP");
  cutsRP->SetParamType(rptype);
  cutsRP->SetParamMix(rpmix);
  cutsRP->SetPtRange(0.2,5.);
  cutsRP->SetEtaRange(-0.8,0.8);
  cutsRP->SetMinNClustersTPC(70);
  cutsRP->SetMinChi2PerClusterTPC(0.1);
  cutsRP->SetMaxChi2PerClusterTPC(4.0);
  cutsRP->SetMinNClustersITS(2);
  cutsRP->SetRequireITSRefit(kTRUE);
  cutsRP->SetRequireTPCRefit(kTRUE);
  cutsRP->SetMaxDCAToVertexXY(0.3);
  cutsRP->SetMaxDCAToVertexZ(0.3);
  cutsRP->SetAcceptKinkDaughters(kFALSE);
  //  cutsRP->SetMinimalTPCdedx(10.);
  //  cutsRP->SetQA(doQA);

  // POI TRACK CUTS:
  AliFlowTrackCuts* cutsPOI = new AliFlowTrackCuts("GlobalPOI");

  // Set priors as a function of centrality
  Float_t centr = (centrMin+centrMax)*0.5;
  cutsPOI->SetPriors(centr);

  cutsPOI->SetParamType(poitype);
  cutsPOI->SetParamMix(poimix);
  cutsPOI->SetPtRange(0.0,10.);
  cutsPOI->SetEtaRange(-0.8,0.8);
  //cutsPOI->SetRequireCharge(kTRUE);
  //cutsPOI->SetPID(PdgRP);

  // track cuts for PID
  cutsPOI->SetMinNClustersTPC(70);
  cutsPOI->SetMinChi2PerClusterTPC(0.1);
  cutsPOI->SetMaxChi2PerClusterTPC(4.0);
  cutsPOI->SetRequireITSRefit(kTRUE);
  cutsPOI->SetRequireTPCRefit(kTRUE);
  cutsPOI->SetMinNClustersITS(2);
  //cutsPOI->SetMaxChi2PerClusterITS(1.e+09);
  cutsPOI->SetMaxDCAToVertexXY(0.3);
  cutsPOI->SetMaxDCAToVertexZ(0.3);
  //cutsPOI->SetDCAToVertex2D(kTRUE);
  //cutsPOI->SetMaxNsigmaToVertex(1.e+10);
  //cutsPOI->SetRequireSigmaToVertex(kFALSE);
  cutsPOI->SetAcceptKinkDaughters(kFALSE);
  //cutsPOI->SetPID(AliPID::kProton, AliFlowTrackCuts::kTOFpid);
  //cutsPOI->SetPID(AliPID::kPion, AliFlowTrackCuts::kTPCpid);
  //cutsPOI->SetPID(AliPID::kProton, AliFlowTrackCuts::kTPCdedx);
  //cutsPOI->SetPID(AliPID::kProton, AliFlowTrackCuts::kTOFbeta);
  cutsPOI->SetRequireStrictTOFTPCagreement();

  // Bayesian PID
  if(specie==0)
    cutsPOI->SetPID(AliPID::kElectron, AliFlowTrackCuts::kTOFbayesian,0.8);
  else if(specie==1)
    cutsPOI->SetPID(AliPID::kMuon, AliFlowTrackCuts::kTOFbayesian,0.8);
  else if(specie==2)
    cutsPOI->SetPID(AliPID::kPion, AliFlowTrackCuts::kTOFbayesian,0.8);
  else if(specie==3)
    cutsPOI->SetPID(AliPID::kKaon, AliFlowTrackCuts::kTOFbayesian,0.8);
  else if(specie==4)
    cutsPOI->SetPID(AliPID::kProton, AliFlowTrackCuts::kTOFbayesian,0.8);

  //not implemented yet
  //  else if(specie==10)
  //    cutsPOI->SetPID(AliPID::kDeuteron, AliFlowTrackCuts::kTOFbayesian,0.8);

  if(charge !=0) cutsPOI->SetCharge(charge);

  //iexample: francesco's tunig TPC Bethe Bloch for data (for TOF+TPC analysis):
  Float_t centrCur = (centrMax + centrMin) * 0.5;
  Float_t AlephParameters[5];
  Float_t mip=51;

  if(centrCur < 10){
    AlephParameters[0] = 7.68595e-02;
    AlephParameters[1] = 1.01781e+01;
    AlephParameters[2] = 9.34864e-06;
    AlephParameters[3] = 2.38588e+00;
    AlephParameters[4] = 2.13599e+00;
  }
  else if(centrCur < 20){
    AlephParameters[0] = 7.79393e-02;
    AlephParameters[1] = 1.00337e+01;
    AlephParameters[2] = 9.34864e-06;
    AlephParameters[3] = 2.40323e+00;
    AlephParameters[4] = 2.13072e+00;
  }
  else if(centrCur < 30){
    AlephParameters[0] = 7.87563e-02;
    AlephParameters[1] = 9.91265e+00;
    AlephParameters[2] = 9.34864e-06;
    AlephParameters[3] = 2.42280e+00;
    AlephParameters[4] = 2.13296e+00;
  }
  else if(centrCur < 40){
    AlephParameters[0] = 8.23869e-02;
    AlephParameters[1] = 9.50211e+00;
    AlephParameters[2] = 1.40230e-05;
    AlephParameters[3] = 2.42899e+00;
    AlephParameters[4] = 2.05572e+00;
  }
  else if(centrCur < 50){
    AlephParameters[0] = 8.25626e-02;
    AlephParameters[1] = 9.47698e+00;
    AlephParameters[2] = 1.40230e-05;
    AlephParameters[3] = 2.43731e+00;
    AlephParameters[4] = 2.06060e+00;
  }
  else if(centrCur < 60){
    AlephParameters[0] = 8.27528e-02;
    AlephParameters[1] = 9.44676e+00;
    AlephParameters[2] = 1.40230e-05;
    AlephParameters[3] = 2.44433e+00;
    AlephParameters[4] = 2.06498e+00;
  }
  else if(centrCur < 70){
    AlephParameters[0] = 8.29615e-02;
    AlephParameters[1] = 9.41909e+00;
    AlephParameters[2] = 1.40230e-05;
    AlephParameters[3] = 2.44894e+00;
    AlephParameters[4] = 2.06676e+00;
  }
  else if(centrCur < 80){
    AlephParameters[0] = 8.31397e-02;
    AlephParameters[1] = 9.41126e+00;
    AlephParameters[2] = 1.40230e-05;
    AlephParameters[3] = 2.44848e+00;
    AlephParameters[4] = 2.06326e+00;
  }
  else{
    AlephParameters[0] = 8.38910e-02;
    AlephParameters[1] = 9.30736e+00;
    AlephParameters[2] = 1.40230e-05;
    AlephParameters[3] = 2.45844e+00;
    AlephParameters[4] = 2.07334e+00;
  }

  cutsPOI->GetESDpid().GetTPCResponse().SetBetheBlochParameters(AlephParameters[0],AlephParameters[1],AlephParameters[2],AlephParameters[3],AlephParameters[4]);
  cutsPOI->GetESDpid().GetTPCResponse().SetMip(mip);

  TString outputSlotName("");
  outputSlotName+=uniqueStr;  
  outputSlotName+=Form("V%i ",harmonic);
  outputSlotName+=cutsRP->GetName();
  outputSlotName+=" ";
  outputSlotName+=cutsPOI->GetName();
  outputSlotName+=Form(" %.0f-",centrMin);
  outputSlotName+=Form("%.0f ",centrMax);
  outputSlotName+=AliFlowTrackCuts::PIDsourceName(sourcePID);
  outputSlotName+=" ";
  outputSlotName+=AliPID::ParticleName(particleType);
  if (charge<0) outputSlotName+="-";
  if (charge>0) outputSlotName+="+";

  Bool_t useWeights  = WEIGHTS[0] || WEIGHTS[1] || WEIGHTS[2];
  if (useWeights) cout<<"Weights are used"<<endl;
  else cout<<"Weights are not used"<<endl;
  
  // Get the pointer to the existing analysis manager via the static access method.
  //==============================================================================
  AliAnalysisManager *mgr = AliAnalysisManager::GetAnalysisManager();
  if (!mgr) {
    Error("AddTaskFlowEvent", "No analysis manager to connect to.");
    return NULL;
  }
  
  // Check the analysis type using the event handlers connected to the analysis
  // manager. The availability of MC handler can also be checked here.
  //==============================================================================
  if (!mgr->GetInputEventHandler()) {
    ::Error("AddTaskFlowEvent", "This task requires an input event handler");
    return NULL;
  }  

  // Open external input files
  //===========================================================================
  //weights: 
  TFile *weightsFile = NULL;
  TList *weightsList = NULL;

  if(useWeights) {
    //open the file with the weights:
    weightsFile = TFile::Open("weights.root","READ");
    if(weightsFile) {
      //access the list which holds the histos with weigths:
      weightsList = (TList*)weightsFile->Get("weights");
    }
    else {
      cout<<" WARNING: the file <weights.root> with weights from the previous run was not available."<<endl;
      break;
    } 
  }
  
  //LYZ2
  if (LYZ2SUM || LYZ2PROD) {
    //read the outputfile of the first run
    TString outputFileName = "AnalysisResults1.root";
    TString pwd(gSystem->pwd());
    pwd+="/";
    pwd+=outputFileName.Data();
    TFile *outputFile = NULL;
    if(gSystem->AccessPathName(pwd.Data(),kFileExists)) {
      cout<<"WARNING: You do not have an output file:"<<endl;
      cout<<"         "<<pwd.Data()<<endl;
      exit(0);
    } else { outputFile = TFile::Open(pwd.Data(),"READ");}
    
    if (LYZ2SUM){  
      // read the output directory from LYZ1SUM 
      TString inputFileNameLYZ2SUM = "outputLYZ1SUManalysis" ;
      inputFileNameLYZ2SUM += rptypestr;
      cout<<"The input directory is "<<inputFileNameLYZ2SUM.Data()<<endl;
      TFile* fInputFileLYZ2SUM = (TFile*)outputFile->FindObjectAny(inputFileNameLYZ2SUM.Data());
      if(!fInputFileLYZ2SUM || fInputFileLYZ2SUM->IsZombie()) { 
	cerr << " ERROR: To run LYZ2SUM you need the output file from LYZ1SUM. This file is not there! Please run LYZ1SUM first." << endl ; 
	break;
      }
      else {
	TList* fInputListLYZ2SUM = (TList*)fInputFileLYZ2SUM->Get("cobjLYZ1SUM");
	if (!fInputListLYZ2SUM) {cout<<"list is NULL pointer!"<<endl;}
      }
      cout<<"LYZ2SUM input file/list read..."<<endl;
    }

    if (LYZ2PROD){  
      // read the output directory from LYZ1PROD 
      TString inputFileNameLYZ2PROD = "outputLYZ1PRODanalysis" ;
      inputFileNameLYZ2PROD += rptypestr;
      cout<<"The input directory is "<<inputFileNameLYZ2PROD.Data()<<endl;
      TFile* fInputFileLYZ2PROD = (TFile*)outputFile->FindObjectAny(inputFileNameLYZ2PROD.Data());
      if(!fInputFileLYZ2PROD || fInputFileLYZ2PROD->IsZombie()) { 
	cerr << " ERROR: To run LYZ2PROD you need the output file from LYZ1PROD. This file is not there! Please run LYZ1PROD first." << endl ; 
	break;
      }
      else {
	TList* fInputListLYZ2PROD = (TList*)fInputFileLYZ2PROD->Get("cobjLYZ1PROD");
	if (!fInputListLYZ2PROD) {cout<<"list is NULL pointer!"<<endl;}
      }
      cout<<"LYZ2PROD input file/list read..."<<endl;
    }
  }

  if (LYZEP) {
    //read the outputfile of the second run
    TString outputFileName = "AnalysisResults2.root";
    TString pwd(gSystem->pwd());
    pwd+="/";
    pwd+=outputFileName.Data();
    TFile *outputFile = NULL;
    if(gSystem->AccessPathName(pwd.Data(),kFileExists)) {
      cout<<"WARNING: You do not have an output file:"<<endl;
      cout<<"         "<<pwd.Data()<<endl;
      exit(0);
    } else {
      outputFile = TFile::Open(pwd.Data(),"READ");
    }
    
    // read the output file from LYZ2SUM
    TString inputFileNameLYZEP = "outputLYZ2SUManalysis" ;
    inputFileNameLYZEP += rptypestr;
    cout<<"The input file is "<<inputFileNameLYZEP.Data()<<endl;
    TFile* fInputFileLYZEP = (TFile*)outputFile->FindObjectAny(inputFileNameLYZEP.Data());
    if(!fInputFileLYZEP || fInputFileLYZEP->IsZombie()) { 
      cerr << " ERROR: To run LYZEP you need the output file from LYZ2SUM. This file is not there! Please run LYZ2SUM first." << endl ; 
      break;
    }
    else {
      TList* fInputListLYZEP = (TList*)fInputFileLYZEP->Get("cobjLYZ2SUM");
      if (!fInputListLYZEP) {cout<<"list is NULL pointer!"<<endl;}
    }
    cout<<"LYZEP input file/list read..."<<endl;
  }
  
  
  // Create the FMD task and add it to the manager
  //===========================================================================
  if (rptypestr == "FMD") {
    AliFMDAnalysisTaskSE *taskfmd = NULL;
    if (rptypestr == "FMD") {
      taskfmd = new AliFMDAnalysisTaskSE("TaskFMD");
      mgr->AddTask(taskfmd);
      
      AliFMDAnaParameters* pars = AliFMDAnaParameters::Instance();
      pars->Init();
      pars->SetProcessPrimary(kTRUE); //for MC only
      pars->SetProcessHits(kFALSE);
      
      //pars->SetRealData(kTRUE); //for real data
      //pars->SetProcessPrimary(kFALSE); //for real data
    }
  }
  
  // Create the task, add it to the manager.
  //===========================================================================
  AliAnalysisTaskFlowEvent *taskFE = NULL;

  if(useAfterBurner)
    { 
      taskFE = new AliAnalysisTaskFlowEvent(Form("TaskFlowEvent %s",outputSlotName.Data()),"",doQA,1);
      taskFE->SetFlow(v1,v2,v3,v4); 
      taskFE->SetNonFlowNumberOfTrackClones(numberOfTrackClones);
      taskFE->SetAfterburnerOn();
    }
  else {taskFE = new AliAnalysisTaskFlowEvent(Form("TaskFlowEvent %s",outputSlotName.Data()),"",doQA); }
  if (ExcludeRegion) {
    taskFE->DefineDeadZone(excludeEtaMin, excludeEtaMax, excludePhiMin, excludePhiMax); 
  }
  taskFE->SetSubeventEtaRange(minA, maxA, minB, maxB);
  if (UsePhysicsSelection) {
    //taskFE->SelectCollisionCandidates(AliVEvent::kUserDefined);
    taskFE->SelectCollisionCandidates(AliVEvent::kMB);
    cout<<"Using Physics Selection"<<endl;
  }
  mgr->AddTask(taskFE);
  
  // Pass cuts for RPs and POIs to the task:
  taskFE->SetCutsEvent(cutsEvent);

  // Pass cuts for RPs and POIs to the task:
  taskFE->SetCutsRP(cutsRP);
  taskFE->SetCutsPOI(cutsPOI);
  if (cutsRP->GetParamType()==AliFlowTrackCuts::kV0)
  { 
    //TODO: since this is set in a static object all analyses in an analysis train
    //will be affected.
    taskFE->SetHistWeightvsPhiMin(0.);
    taskFE->SetHistWeightvsPhiMax(200.);
  }

  // Create the analysis tasks, add them to the manager.
  //===========================================================================
  if (SP){
    AliAnalysisTaskScalarProduct *taskSP = new AliAnalysisTaskScalarProduct("TaskScalarProduct",WEIGHTS[0]);
    taskSP->SetRelDiffMsub(1.0);
    taskSP->SetApplyCorrectionForNUA(kFALSE);
    taskSP->SetHarmonic(harmonic); // default is v2
    mgr->AddTask(taskSP);
  }
  if (LYZ1SUM){
    AliAnalysisTaskLeeYangZeros *taskLYZ1SUM = new AliAnalysisTaskLeeYangZeros("TaskLeeYangZerosSUM",kTRUE);
    taskLYZ1SUM->SetFirstRunLYZ(kTRUE);
    taskLYZ1SUM->SetUseSumLYZ(kTRUE);
    mgr->AddTask(taskLYZ1SUM);
  }
  if (LYZ1PROD){
    AliAnalysisTaskLeeYangZeros *taskLYZ1PROD = new AliAnalysisTaskLeeYangZeros("TaskLeeYangZerosPROD",kTRUE);
    taskLYZ1PROD->SetFirstRunLYZ(kTRUE);
    taskLYZ1PROD->SetUseSumLYZ(kFALSE);
    mgr->AddTask(taskLYZ1PROD);
  }
  if (LYZ2SUM){
    AliAnalysisTaskLeeYangZeros *taskLYZ2SUM = new AliAnalysisTaskLeeYangZeros("TaskLeeYangZerosSUM",kFALSE);
    taskLYZ2SUM->SetFirstRunLYZ(kFALSE);
    taskLYZ2SUM->SetUseSumLYZ(kTRUE);
    mgr->AddTask(taskLYZ2SUM);
  }
  if (LYZ2PROD){
    AliAnalysisTaskLeeYangZeros *taskLYZ2PROD = new AliAnalysisTaskLeeYangZeros("TaskLeeYangZerosPROD",kFALSE);
    taskLYZ2PROD->SetFirstRunLYZ(kFALSE);
    taskLYZ2PROD->SetUseSumLYZ(kFALSE);
    mgr->AddTask(taskLYZ2PROD);
  }
  if (LYZEP){
    AliAnalysisTaskLYZEventPlane *taskLYZEP = new AliAnalysisTaskLYZEventPlane("TaskLYZEventPlane");
    mgr->AddTask(taskLYZEP);
  }
  if (GFC){
    AliAnalysisTaskCumulants *taskGFC = new AliAnalysisTaskCumulants("TaskCumulants",useWeights);
    taskGFC->SetUsePhiWeights(WEIGHTS[0]); 
    taskGFC->SetUsePtWeights(WEIGHTS[1]);
    taskGFC->SetUseEtaWeights(WEIGHTS[2]); 
    taskGFC->SetHarmonic(harmonic);
    mgr->AddTask(taskGFC);
  }
  if (QC){
    AliAnalysisTaskQCumulants *taskQC = new AliAnalysisTaskQCumulants("TaskQCumulants",useWeights);
    taskQC->SetUsePhiWeights(WEIGHTS[0]); 
    taskQC->SetUsePtWeights(WEIGHTS[1]);
    taskQC->SetUseEtaWeights(WEIGHTS[2]); 
    taskQC->SetCalculateCumulantsVsM(kFALSE);
    taskQC->SetnBinsMult(10000);
    taskQC->SetMinMult(0.);
    taskQC->SetMaxMult(10000.);
    taskQC->SetHarmonic(harmonic); // default is v2
    taskQC->SetApplyCorrectionForNUA(kFALSE);
    taskQC->SetFillMultipleControlHistograms(kFALSE);     
    mgr->AddTask(taskQC);
  }
  if (FQD){
    AliAnalysisTaskFittingQDistribution *taskFQD = new AliAnalysisTaskFittingQDistribution("TaskFittingQDistribution",kFALSE);
    taskFQD->SetUsePhiWeights(WEIGHTS[0]); 
    taskFQD->SetqMin(0.);
    taskFQD->SetqMax(1000.);
    taskFQD->SetqNbins(10000);
    mgr->AddTask(taskFQD);
  }
  if (MCEP){
    AliAnalysisTaskMCEventPlane *taskMCEP = new AliAnalysisTaskMCEventPlane("TaskMCEventPlane");
    taskMCEP->SetHarmonic(harmonic);
    mgr->AddTask(taskMCEP);
  }
  if (MH){
    AliAnalysisTaskMixedHarmonics *taskMH = new AliAnalysisTaskMixedHarmonics("TaskMixedHarmonics",useWeights);
    taskMH->SetHarmonic(harmonic); // n in cos[n(phi1+phi2-2phi3)] and cos[n(psi1+psi2-2phi3)]
    taskMH->SetNoOfMultipicityBins(10000);
    taskMH->SetMultipicityBinWidth(1.);
    taskMH->SetMinMultiplicity(1.);
    taskMH->SetCorrectForDetectorEffects(kTRUE);
    taskMH->SetEvaluateDifferential3pCorrelator(kFALSE); // evaluate <<cos[n(psi1+psi2-2phi3)]>> (Remark: two nested loops)    
    taskMH->SetOppositeChargesPOI(kFALSE); // POIs psi1 and psi2 in cos[n(psi1+psi2-2phi3)] will have opposite charges  
    mgr->AddTask(taskMH);
  }  
  if (NL){
    AliAnalysisTaskNestedLoops *taskNL = new AliAnalysisTaskNestedLoops("TaskNestedLoops",useWeights);
    taskNL->SetHarmonic(harmonic); // n in cos[n(phi1+phi2-2phi3)] and cos[n(psi1+psi2-2phi3)]
    taskNL->SetEvaluateNestedLoopsForRAD(kTRUE); // RAD = Relative Angle Distribution
    taskNL->SetEvaluateNestedLoopsForMH(kTRUE); // evalaute <<cos[n(phi1+phi2-2phi3)]>> (Remark: three nested loops)   
    taskNL->SetEvaluateDifferential3pCorrelator(kFALSE); // evaluate <<cos[n(psi1+psi2-2phi3)]>>  (Remark: three nested loops)   
    taskNL->SetOppositeChargesPOI(kFALSE); // POIs psi1 and psi2 in cos[n(psi1+psi2-2phi3)] will have opposite charges  
    mgr->AddTask(taskNL);
  }

  // Create the output container for the data produced by the task
  // Connect to the input and output containers
  //===========================================================================
  AliAnalysisDataContainer *cinput1 = mgr->GetCommonInputContainer();
  
  if (rptypestr == "FMD") {
    AliAnalysisDataContainer *coutputFMD = 
      mgr->CreateContainer(Form("BackgroundCorrected_%s",centralityName.Data()), TList::Class(), AliAnalysisManager::kExchangeContainer);
    //input and output taskFMD     
    mgr->ConnectInput(taskfmd, 0, cinput1);
    mgr->ConnectOutput(taskfmd, 1, coutputFMD);
    //input into taskFE
    mgr->ConnectInput(taskFE,1,coutputFMD);
  }
  
  AliAnalysisDataContainer *coutputFE = 
  mgr->CreateContainer(Form("cobjFlowEventSimple_%s",centralityName.Data()),AliFlowEventSimple::Class(),AliAnalysisManager::kExchangeContainer);
  mgr->ConnectInput(taskFE,0,cinput1); 
  mgr->ConnectOutput(taskFE,1,coutputFE);

  if (taskFE->GetQAOn())
  {
    TString outputQA = fileName;
    outputQA += ":QA";
    AliAnalysisDataContainer* coutputFEQA = 
    mgr->CreateContainer(Form("QA %s",outputSlotName.Data()), TList::Class(),AliAnalysisManager::kOutputContainer,outputQA);
    mgr->ConnectOutput(taskFE,2,coutputFEQA);
  }

  // Create the output containers for the data produced by the analysis tasks
  // Connect to the input and output containers
  //===========================================================================
  if (useWeights) {    
    AliAnalysisDataContainer *cinputWeights = mgr->CreateContainer(Form("cobjWeights_%s",centralityName.Data()),
								   TList::Class(),AliAnalysisManager::kInputContainer); 
  }

  if(SP) {
    TString outputSP = fileName;
    outputSP += ":outputSPanalysis";
    outputSP+= rptypestr;
    AliAnalysisDataContainer *coutputSP = mgr->CreateContainer(Form("cobjSP_%s",centralityName.Data()), 
							       TList::Class(),AliAnalysisManager::kOutputContainer,outputSP); 
    mgr->ConnectInput(taskSP,0,coutputFE); 
    mgr->ConnectOutput(taskSP,1,coutputSP); 
    if (WEIGHTS[0]) {
      mgr->ConnectInput(taskSP,1,cinputWeights);
      cinputWeights->SetData(weightsList);
    }
  }
  if(LYZ1SUM) {
    TString outputLYZ1SUM = fileName;
    outputLYZ1SUM += ":outputLYZ1SUManalysis";
    outputLYZ1SUM+= rptypestr;
    AliAnalysisDataContainer *coutputLYZ1SUM = mgr->CreateContainer(Form("cobjLYZ1SUM_%s",centralityName.Data()), 
								    TList::Class(),AliAnalysisManager::kOutputContainer,outputLYZ1SUM); 
    mgr->ConnectInput(taskLYZ1SUM,0,coutputFE);
    mgr->ConnectOutput(taskLYZ1SUM,1,coutputLYZ1SUM);
  }
  if(LYZ1PROD) {
    TString outputLYZ1PROD = fileName;
    outputLYZ1PROD += ":outputLYZ1PRODanalysis";
    outputLYZ1PROD+= rptypestr;
    AliAnalysisDataContainer *coutputLYZ1PROD = mgr->CreateContainer(Form("cobjLYZ1PROD_%s",centralityName.Data()), 
								     TList::Class(),AliAnalysisManager::kOutputContainer,outputLYZ1PROD); 
    mgr->ConnectInput(taskLYZ1PROD,0,coutputFE); 
    mgr->ConnectOutput(taskLYZ1PROD,1,coutputLYZ1PROD);
  }
  if(LYZ2SUM) {
    AliAnalysisDataContainer *cinputLYZ2SUM = mgr->CreateContainer(Form("cobjLYZ2SUMin_%s",centralityName.Data()),
								   TList::Class(),AliAnalysisManager::kInputContainer);
    TString outputLYZ2SUM = fileName;
    outputLYZ2SUM += ":outputLYZ2SUManalysis";
    outputLYZ2SUM+= rptypestr;
    
    AliAnalysisDataContainer *coutputLYZ2SUM = mgr->CreateContainer(Form("cobjLYZ2SUM_%s",centralityName.Data()), 
								    TList::Class(),AliAnalysisManager::kOutputContainer,outputLYZ2SUM); 
    mgr->ConnectInput(taskLYZ2SUM,0,coutputFE); 
    mgr->ConnectInput(taskLYZ2SUM,1,cinputLYZ2SUM);
    mgr->ConnectOutput(taskLYZ2SUM,1,coutputLYZ2SUM); 
    cinputLYZ2SUM->SetData(fInputListLYZ2SUM);
  }
  if(LYZ2PROD) {
    AliAnalysisDataContainer *cinputLYZ2PROD = mgr->CreateContainer(Form("cobjLYZ2PRODin_%s",centralityName.Data()),
								    TList::Class(),AliAnalysisManager::kInputContainer);
    TString outputLYZ2PROD = fileName;
    outputLYZ2PROD += ":outputLYZ2PRODanalysis";
    outputLYZ2PROD+= rptypestr;
    
    AliAnalysisDataContainer *coutputLYZ2PROD = mgr->CreateContainer(Form("cobjLYZ2PROD_%s",centralityName.Data()), 
								     TList::Class(),AliAnalysisManager::kOutputContainer,outputLYZ2PROD); 
    mgr->ConnectInput(taskLYZ2PROD,0,coutputFE); 
    mgr->ConnectInput(taskLYZ2PROD,1,cinputLYZ2PROD);
    mgr->ConnectOutput(taskLYZ2PROD,1,coutputLYZ2PROD); 
    cinputLYZ2PROD->SetData(fInputListLYZ2PROD);
  }
  if(LYZEP) {
    AliAnalysisDataContainer *cinputLYZEP = mgr->CreateContainer(Form("cobjLYZEPin_%s",centralityName.Data()),
								 TList::Class(),AliAnalysisManager::kInputContainer);
    TString outputLYZEP = fileName;
    outputLYZEP += ":outputLYZEPanalysis";
    outputLYZEP+= rptypestr;
    
    AliAnalysisDataContainer *coutputLYZEP = mgr->CreateContainer(Form("cobjLYZEP_%s",centralityName.Data()), 
								  TList::Class(),AliAnalysisManager::kOutputContainer,outputLYZEP); 
    mgr->ConnectInput(taskLYZEP,0,coutputFE); 
    mgr->ConnectInput(taskLYZEP,1,cinputLYZEP);
    mgr->ConnectOutput(taskLYZEP,1,coutputLYZEP); 
    cinputLYZEP->SetData(fInputListLYZEP);
  }
  if(GFC) {
    TString outputGFC = fileName;
    outputGFC += ":outputGFCanalysis";
    outputGFC+= rptypestr;
    
    AliAnalysisDataContainer *coutputGFC = mgr->CreateContainer(Form("cobjGFC_%s",centralityName.Data()), 
								TList::Class(),AliAnalysisManager::kOutputContainer,outputGFC); 
    mgr->ConnectInput(taskGFC,0,coutputFE); 
    mgr->ConnectOutput(taskGFC,1,coutputGFC);
    if (useWeights) {
      mgr->ConnectInput(taskGFC,1,cinputWeights);
      cinputWeights->SetData(weightsList);
    } 
  }
  if(QC) {
    TString outputQC = fileName;
    outputQC += ":outputQCanalysis";
    outputQC+= rptypestr;

    AliAnalysisDataContainer *coutputQC = mgr->CreateContainer(Form("cobjQC_%s",centralityName.Data()), 
							       TList::Class(),AliAnalysisManager::kOutputContainer,outputQC); 
    mgr->ConnectInput(taskQC,0,coutputFE); 
    mgr->ConnectOutput(taskQC,1,coutputQC);
    if (useWeights) {
      mgr->ConnectInput(taskQC,1,cinputWeights);
      cinputWeights->SetData(weightsList);
    }
  }
  if(FQD) {
    TString outputFQD = fileName;
    outputFQD += ":outputFQDanalysis";
    outputFQD+= rptypestr;
    
    AliAnalysisDataContainer *coutputFQD = mgr->CreateContainer(Form("cobjFQD_%s",centralityName.Data()), 
								TList::Class(),AliAnalysisManager::kOutputContainer,outputFQD); 
    mgr->ConnectInput(taskFQD,0,coutputFE); 
    mgr->ConnectOutput(taskFQD,1,coutputFQD);
    if(useWeights) {
      mgr->ConnectInput(taskFQD,1,cinputWeights);
      cinputWeights->SetData(weightsList);
    } 
  }
  if(MCEP) {
    TString outputMCEP = fileName;
    outputMCEP += ":outputMCEPanalysis";
    outputMCEP+= rptypestr;
    
    AliAnalysisDataContainer *coutputMCEP = mgr->CreateContainer(Form("cobjMCEP_%s",centralityName.Data()), 
								 TList::Class(),AliAnalysisManager::kOutputContainer,outputMCEP); 
    mgr->ConnectInput(taskMCEP,0,coutputFE);
    mgr->ConnectOutput(taskMCEP,1,coutputMCEP); 
  }
  if(MH) {
    TString outputMH = fileName;
    outputMH += ":outputMHanalysis";
    outputMH += rptypestr;
        
    AliAnalysisDataContainer *coutputMH = mgr->CreateContainer(Form("cobjMH_%s",centralityName.Data()), 
							       TList::Class(),AliAnalysisManager::kOutputContainer,outputMH); 
    mgr->ConnectInput(taskMH,0,coutputFE); 
    mgr->ConnectOutput(taskMH,1,coutputMH); 
    //if (useWeights) {
    //  mgr->ConnectInput(taskMH,1,cinputWeights);
    //  cinputWeights->SetData(weightsList);
    //} 
  }
  if(NL) {
    TString outputNL = fileName;
    outputNL += ":outputNLanalysis";
    outputNL += rptypestr;

    AliAnalysisDataContainer *coutputNL = mgr->CreateContainer(Form("cobjNL_%s",centralityName.Data()), 
							       TList::Class(),AliAnalysisManager::kOutputContainer,outputNL); 
    mgr->ConnectInput(taskNL,0,coutputFE);
    mgr->ConnectOutput(taskNL,1,coutputNL);
    //if (useWeights) {
    //  mgr->ConnectInput(taskNL,1,cinputWeights);
    //  cinputWeights->SetData(weightsList);
    //} 
  }

  ///////////////////////////////////////////////////////////////////////////////////////////
  if (runQAtask)
  {
    AliAnalysisTaskQAflow* taskQAflow = new AliAnalysisTaskQAflow("TaskQAflow");
    taskQAflow->SetEventCuts(cutsEvent);
    taskQAflow->SetTrackCuts(cutsRP);
    taskQAflow->SetFillNTuple(FillQAntuple);
    taskQAflow->SetDoCorrelations(DoQAcorrelations);
    mgr->AddTask(taskQAflow);
    
    Printf("centralityName %s",centralityName.Data());
    TString taskQAoutputFileName(fileNameBase);
    taskQAoutputFileName.Append("_QA.root");
    AliAnalysisDataContainer* coutputQAtask = mgr->CreateContainer(Form("flowQA_%s",centralityName.Data()),
                                              TObjArray::Class(),
                                              AliAnalysisManager::kOutputContainer,
                                              taskQAoutputFileName);
    AliAnalysisDataContainer* coutputQAtaskTree = mgr->CreateContainer(Form("flowQAntuple_%s",centralityName.Data()),
                                              TNtuple::Class(),
                                              AliAnalysisManager::kOutputContainer,
                                              taskQAoutputFileName);
    mgr->ConnectInput(taskQAflow,0,mgr->GetCommonInputContainer());
    mgr->ConnectInput(taskQAflow,1,coutputFE);
    mgr->ConnectOutput(taskQAflow,1,coutputQAtask);
    if (FillQAntuple) mgr->ConnectOutput(taskQAflow,2,coutputQAtaskTree);
  }

  return taskFE;
}





