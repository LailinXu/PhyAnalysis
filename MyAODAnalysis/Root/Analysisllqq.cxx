#include <EventLoop/Job.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/Worker.h>
#include "EventLoop/OutputStream.h"

#include <MyAODAnalysis/Analysisllqq.h>

#include "xAODEventInfo/EventInfo.h"

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
  HM->InitVar("TruthPtL1,TruthPtL2",5000,0,5000);
  HM->InitVar("TruthWllM,TruthWqqM,TruthZllM,TruthZqqM,TruthVVM",5000,0,5000);

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
  const xAOD::EventInfo * eventInfo = 0;
  EL_RETURN_CHECK( "retrieve_eventInfo()", m_event->retrieve(eventInfo, "EventInfo"));
  if (!eventInfo) {
    Error("doTruth()", "EventInfo not found!");
    return EL::StatusCode::FAILURE;
  }

  // retrieve truth particle container
  const xAOD::TruthParticleContainer* truthParts = NULL;
  EL_RETURN_CHECK( "retrieve_xTruthParticleContainer()", m_event->retrieve( truthParts, "TruthParticles"));
  if (!truthParts) {
    Error("doTruth()", "Did not find truth particles!");
    return EL::StatusCode::FAILURE;
  }

  // truth particles
  std::vector<const xAOD::TruthParticle*> truth_Wll, truth_Zll, truth_H, truth_lep, truth_neu;
  std::vector<const xAOD::TruthParticle*> truth_Wqq, truth_Zqq;

  //std::cout << "Check=> doTruth()" << std::endl;
  // loop all truth particles
  for (const xAOD::TruthParticle* particle : *truthParts) {
    int barcode = particle->barcode();
    int status = particle->status();
    int pdgId = particle->pdgId();

    float px = particle->px();

    //std::cout << "Check=> doTruth() pdgId: " << pdgId << " status: " << status << std::endl;
    // find the child particles from V boson
    if((pdgId==23 || pdgId==24 || pdgId==25) && status==22) {
      int nChild=particle->nChildren();
      int nChild_l=0, nChild_q=0;
      for(int ich=0; ich<nChild; ich++) {
        const xAOD::TruthParticle* ch = particle->child(ich);
        int tmp_barcode = ch->barcode();
        int tmp_status = ch->status();
        int tmp_pdgId = ch->pdgId();
        if(fabs(tmp_pdgId)==11 || fabs(tmp_pdgId)==13 || fabs(tmp_pdgId)==15) nChild_l++;
        if(fabs(tmp_pdgId)==12 || fabs(tmp_pdgId)==14 || fabs(tmp_pdgId)==16) nChild_l++;
        if(fabs(tmp_pdgId)<=6) nChild_q++;
      }
      int decay_mode=-1;
      if(nChild_l==2 && nChild_q==0) decay_mode=1; //leptonic decay
      if(nChild_l==0 && nChild_q==2) decay_mode=2; //hadronic decay
      //std::cout << "Check=> doTruth: pdgId= " << pdgId << " nChild_l= " << nChild_l << " nChild_q= " << nChild_q << std::endl;

      if(pdgId==23 && decay_mode==1)      truth_Zll.push_back(particle);
      else if(pdgId==23 && decay_mode==2) truth_Zqq.push_back(particle);
      else if(pdgId==24 && decay_mode==1) truth_Wll.push_back(particle);
      else if(pdgId==24 && decay_mode==2) truth_Wqq.push_back(particle);
      if(pdgId==25) truth_H.push_back(particle);

    }

    // find the parent particles 
    if((fabs(pdgId)==11 || fabs(pdgId)==13 || fabs(pdgId)==15) && status==1) {
      if(TraceUp(particle)) truth_lep.push_back(particle);
      //if(TraceUp(particle)) std::cout << "Check=> doTruth: found leptons from V decay" << std::endl;
    } 
    if((fabs(pdgId)==12 || fabs(pdgId)==14 || fabs(pdgId)==16) && status==1) {
      if(TraceUp(particle)) truth_neu.push_back(particle);
      //if(TraceUp(particle)) std::cout << "Check=> doTruth: found neutrino from V decay" << std::endl;
    } 
  }

  // sort the particle vectors by pt
  std::sort(truth_lep.begin(), truth_lep.end(), sort_par_pt);
  std::sort(truth_Wll.begin(), truth_Wll.end(), sort_par_pt);
  std::sort(truth_Zll.begin(), truth_Zll.end(), sort_par_pt);
  std::sort(truth_Wqq.begin(), truth_Wqq.end(), sort_par_pt);
  std::sort(truth_Zqq.begin(), truth_Zqq.end(), sort_par_pt);

  // get the interesting variables
  TLorentzVector tlz_VV, tlz_Vll, tlz_Vqq;
  if(truth_Wll.size()>0) {
    const xAOD::TruthParticle* par = truth_Wll.at(0);
    //float mV = par->m()/1000.;  HM->Var["TruthWllM"]=mV;
    float mV = par->p4().M()/1000.;  HM->Var["TruthWllM"]=mV;
  }
  if(truth_Wqq.size()>0) {
    const xAOD::TruthParticle* par = truth_Wqq.at(0);
    tlz_Vqq = par->p4();
    //float mV = par->m()/1000.;  HM->Var["TruthWqqM"]=mV;
    float mV = par->p4().M()/1000.;  HM->Var["TruthWqqM"]=mV;
  }
  if(truth_Zll.size()>0) {
    const xAOD::TruthParticle* par = truth_Zll.at(0);
    tlz_Vll = par->p4();
    //float mV = par->m()/1000.;  HM->Var["TruthZllM"]=mV;
    float mV = par->p4().M()/1000.;  HM->Var["TruthZllM"]=mV;
  }
  if(truth_Zqq.size()>0) {
    const xAOD::TruthParticle* par = truth_Zqq.at(0);
    //float mV = par->m()/1000.;  HM->Var["TruthZqqM"]=mV;
    float mV = par->p4().M()/1000.;  HM->Var["TruthZqqM"]=mV;
  }
  if(truth_lep.size()>0) {
    const xAOD::TruthParticle* par = truth_lep.at(0);
    float truth_pt = par->pt()/1000.;  HM->Var["TruthL1Pt"]=truth_pt;
    if(truth_lep.size()>1) {
      par = truth_lep.at(1);
      truth_pt = par->pt()/1000.;  HM->Var["TruthL2Pt"]=truth_pt;
    }
  }

  if(tlz_Vll.M()>0 && tlz_Vqq.M()>0) {
    tlz_VV = tlz_Vll + tlz_Vqq;
    HM->Var["TruthVVM"]=tlz_VV.M()/1000.;
  }

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
