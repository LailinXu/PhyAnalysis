#include <EventLoop/Job.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/Worker.h>
#include "EventLoop/OutputStream.h"

#include <MyAODAnalysis/Analysisllqq.h>
#include "CxAODTools/ReturnCheck.h"


#define EL_RETURN_CHECK( CONTEXT, EXP )                              \
  if (EXP !=  EL::StatusCode::SUCCESS) {                      \
    Error( CONTEXT, "Failed to execute: %s. Exiting.", #EXP); \
    return EL::StatusCode::FAILURE;                           \
  }

// this is needed to distribute the algorithm to the workers
ClassImp(Analysisllqq)



Analysisllqq :: Analysisllqq ()
{
  // Here you put any code for the base initialization of variables,
  // e.g. initialize all pointers to 0.  Note that you should only put
  // the most basic initialization here, since this method will be
  // called on both the submission and the worker node.  Most of your
  // initialization code will go into histInitialize() and
  // initialize().
  HM = new HistoManager();
}



EL::StatusCode Analysisllqq :: setupJob (EL::Job& job)
{
  // Here you put code that sets up the job on the submission object
  // so that it is ready to work with your algorithm, e.g. you can
  // request the D3PDReader service or add output files.  Any code you
  // put here could instead also go into the submission script.  The
  // sole advantage of putting it here is that it gets automatically
  // activated/deactivated when you add/remove the algorithm from your
  // job, which may or may not be of value to you.

  Info("setupJob()", "Setting up job.");

  // setup for xAOD
  job.useXAOD();
  // let's initialize the algorithm to use the xAODRootAccess package
  xAOD::Init("AnalysisBase").ignore(); // call before opening first file
  
  // tell EventLoop about our output xAOD:
  EL::OutputStream out("outputLabel","xAODNoMeta");
  job.outputAdd (out);

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode Analysisllqq :: histInitialize ()
{
  // Here you do everything that needs to be done at the very
  // beginning on each worker node, e.g. create histograms and output
  // trees.  This method gets called before any input files are
  // connected.

  Info("histInitialize()", "histInitialize.");

  // book histograms
  // HM->InitVar("Var1,Var2,...",nBin,xMin,xMax);  // use comma "," to separate Var's
  // for HVT VV truth
  HM->InitVar("TruthPtL1,TruthPtL2",5000,0,5000);
  HM->InitVar("TruthWllM,TruthWqqM,TruthZllM,TruthZqqM,TruthVVM",5000,0,5000);
  // for Zjets Sherpa Pileup tag jets
  //HM->InitVar("Mjjtag,Mjjtag_R,Mjjtag_F",5000,0,5000);
  //HM->InitVar("dEtajj,dEtajj_R,dEtajj_F",2000,0,20);
  HM->InitVar("TagJ1TruthID,TagJ2TruthID",200,-100,100);

  // creat histograms defined in InitVar
  HM->CreateHistoMap();

  // write histograms to output
  for (auto h : HM->getHistos()) {
    wk()->addOutput(h);
  }

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode Analysisllqq :: fileExecute ()
{
  // Here you do everything that needs to be done exactly once for every
  // single file, e.g. collect a list of all lumi-blocks processed

  Info("fileExecute()", "fileExecute.");

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode Analysisllqq :: changeInput (bool firstFile)
{
  // Here you do everything you need to do when we change input files,
  // e.g. resetting branch addresses on trees.  If you are using
  // D3PDReader or a similar service this method is not needed.

  Info("changeInput()", "changeInput.");

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode Analysisllqq :: initialize ()
{
  // Here you do everything that you need to do after the first input
  // file has been connected and before the first event is processed,
  // e.g. create additional histograms based on which variables are
  // available in the input files.  You can also create all of your
  // histograms and trees in here, but be aware that this method
  // doesn't get called if no events are processed.  So any objects
  // you create here won't be available in the output if you have no
  // input events.

  Info("initialize()", "Initialize.");

  // initialize xAOD event
  EL_RETURN_CHECK("initialize()",initializeEvent()    );
  // initialize tools
  EL_RETURN_CHECK("initialize()",initializeTools()    );

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode Analysisllqq::initializeEvent() {
 
  Info("initializeEvent()", "Initialize event.");

  // get TEvent
  m_event = wk()->xaodEvent();

  // sample name
  TString sampleName = wk()->metaData()->castString("sample_name");
  Info("initialize()", "Sample name = %s", sampleName.Data());
  // as a check, let's see the number of events in our xAOD (long long int)
  Info("initialize()", "Number of events in file   = %lli", m_event->getEntries());
  if(m_maxEvent < 0) m_maxEvent = m_event->getEntries() ;
  Info("initialize()", "Number of events to run on = %li", m_maxEvent);

  // count number of events
  m_eventCounter = 0;
 
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode Analysisllqq::initializeTools() {
 
  Info("initializeTools()", "Initialize tools.");

  m_OR = new OverlapRemovalTool("OverlapRemovalTool");
  m_OR->msg().setLevel(MSG::INFO);

  // Set name of decoration which tells the OR tool which objects to consider
  // if set to "false", the object automatically passes OR, i.e. the OROutputLabel is 0 (as in "do not remove this object").
  TOOL_CHECK("OverlapRemoval::initialize()", m_OR->setProperty("InputLabel", "ORInputLabel"));
  // Set name of decoration the OR tool will add as decoration
  // if decoration is "false" the object should be removed, while if "true" the object should be kept.
  TOOL_CHECK("OverlapRemoval::initialize()", m_OR->setProperty("OverlapLabel", "OROutputLabel"));
  TOOL_CHECK("OverlapRemoval::initialize()", m_OR->setProperty("WriteSharedTrackFlag", true));

  TOOL_CHECK("OverlapRemoval::initialize()", m_OR->initialize());

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode Analysisllqq :: execute ()
{
  // Here you do everything that needs to be done on every single
  // events, e.g. read input variables, apply cuts, and fill
  // histograms and trees.  This is where most of your actual analysis
  // code will go.

  if( (m_eventCounter % 10000) == 0) {
    Info("execute()", "Event number = %i", m_eventCounter );
  }
  m_eventCounter++;

  HM->ClearVariables();

  // retrieve event info
  EL_RETURN_CHECK( "retrieve_m_eventInfo()", m_event->retrieve(m_eventInfo, "EventInfo"));

  // retrieve truth particle container
  EL_RETURN_CHECK( "retrieve_xTruthParticleContainer()", m_event->retrieve( m_truthParts, "TruthParticles"));
  // retrieve truth thin jets
  //EL_RETURN_CHECK( "retrieve_AntiKt4TruthJets()", m_event->retrieve( m_truthJets, "AntiKt4TruthJets"));
  // retrieve truth fat jets
  //EL_RETURN_CHECK( "retrieve_AntiKt10TruthJets()", m_event->retrieve( m_truthFatjets, "AntiKt10TruthJets"));

  EL_RETURN_CHECK( "retrieve_ele()", m_event->retrieve( m_electrons, "Electrons"));
  EL_RETURN_CHECK( "retrieve_muon()", m_event->retrieve( m_muons, "Muons"));
  EL_RETURN_CHECK( "retrieve_photon()", m_event->retrieve( m_photons, "Photons"));
  EL_RETURN_CHECK( "retrieve_tau()", m_event->retrieve( m_taus, "TauJets"));
  // retrieve reco thin jets
  EL_RETURN_CHECK( "retrieve_AntiKt4EMTopo()", m_event->retrieve( m_jets, "AntiKt4EMTopoJets"));
  // retrieve reco fat jets
  EL_RETURN_CHECK( "retrieve_10LCTpTriPtFr5SmR20()", m_event->retrieve( m_fatjets, "AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets"));

  // check if the MC sample is interesting to us
  int mc_channel = m_eventInfo->mcChannelNumber();
  // Sherpa Z+jets: 361372-361395,361396-361419,361420-361443
  // HVT: 301390-302414
  bool isZjets = false, isHVTVV = false, isPowhegH = false;
  if( mc_channel >=301390 && mc_channel <=302414) isHVTVV = true; 
  if( mc_channel >=361372 && mc_channel <=361443) isZjets = true; 
  if( mc_channel >=341339 && mc_channel <=341344) isPowhegH = true; 

  if( !isHVTVV && !isZjets && !isPowhegH) return EL::StatusCode::SUCCESS;

  // overlap removal for reco jets (thin jets only)
  doOverlapRM();

  // HVT VV truth 
  if(isHVTVV) fillHVTVV();

  // Z+jets pileup VBF tag jets
  if(isZjets || isPowhegH) fillZjetsPU();

  // Fill histograms
  HM->FillHistograms();

  return EL::StatusCode::SUCCESS;

}

int Analysisllqq::TraceUp(const xAOD::TruthParticle* particle) {
    // trace up the parents, until a V boson found

    bool found_mother = false;

    int nPar=particle->nParents();
    // in case no parent particles
    if(nPar==0) {
      return false; 
    }
    for(int ich=0; ich<nPar; ich++) {
      const xAOD::TruthParticle* ch = particle->parent(ich);
      if(!ch) continue;
      int barcode = ch->barcode();
      int status = ch->status();
      int pdgId = ch->pdgId();
      if( pdgId==24 && status==22 ) { // W, MadGraph
          found_mother = true;
          break;
      } else {
          found_mother = TraceUp(ch);
      }

    }

    return found_mother;

}

int Analysisllqq::TraceUpHadron(const xAOD::TruthParticle* particle) {
    // check if the lepton is from a hadron

    bool foundHadron=false;
    int nPar=particle->nParents();
    if(nPar==0) {
      return false; 
    }
    for(int ich=0; ich<nPar; ich++) {
      const xAOD::TruthParticle* ch = particle->parent(ich);
      if(!ch) continue;
      int barcode = ch->barcode();
      int status = ch->status();
      int pdgId = ch->pdgId();
      if( fabs(pdgId) > 100 && status<3) { // hadron
          foundHadron=true;
          break;
      } else {
          foundHadron=TraceUpHadron(ch);
      }

    }

    return foundHadron;

}


EL::StatusCode Analysisllqq :: postExecute ()
{
  // Here you do everything that needs to be done after the main event
  // processing.  This is typically very rare, particularly in user
  // code.  It is mainly used in implementing the NTupleSvc.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode Analysisllqq :: finalize ()
{
  // This method is the mirror image of initialize(), meaning it gets
  // called after the last event has been processed on the worker node
  // and allows you to finish up any objects you created in
  // initialize() before they are written to disk.  This is actually
  // fairly rare, since this happens separately for each worker node.
  // Most of the time you want to do your post-processing on the
  // submission node after all your histogram outputs have been
  // merged.  This is different from histFinalize() in that it only
  // gets called on worker nodes that processed input events.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode Analysisllqq :: histFinalize ()
{
  // This method is the mirror image of histInitialize(), meaning it
  // gets called after the last event has been processed on the worker
  // node and allows you to finish up any objects you created in
  // histInitialize() before they are written to disk.  This is
  // actually fairly rare, since this happens separately for each
  // worker node.  Most of the time you want to do your
  // post-processing on the submission node after all your histogram
  // outputs have been merged.  This is different from finalize() in
  // that it gets called on all worker nodes regardless of whether
  // they processed input events.
  return EL::StatusCode::SUCCESS;
}
