#ifndef FILLHVTVV_CXX
#define FILLHVTVV_CXX

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

EL::StatusCode Analysisllqq::fillHVTVV(){

  // truth particles
  std::vector<const xAOD::TruthParticle*> truth_Wll, truth_Zll, truth_H, truth_lep, truth_neu;
  std::vector<const xAOD::TruthParticle*> truth_Wqq, truth_Zqq;

  //std::cout << "Check=> doTruth()" << std::endl;
  // loop all truth particles
  for (const xAOD::TruthParticle* particle : *m_truthParts) {
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

  return EL::StatusCode::SUCCESS;
}

#endif
