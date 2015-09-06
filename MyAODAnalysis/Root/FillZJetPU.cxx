#ifndef FILLHVTVV_CXX
#define FILLHVTVV_CXX

#include <EventLoop/Job.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/Worker.h>
#include "EventLoop/OutputStream.h"

#include <MyAODAnalysis/Analysisllqq.h>

#include "xAODBTagging/BTagging.h"
#include "xAODEventInfo/EventInfo.h"

#define EL_RETURN_CHECK( CONTEXT, EXP )                              \
  if (EXP !=  EL::StatusCode::SUCCESS) {                      \
    Error( CONTEXT, "Failed to execute: %s. Exiting.", #EXP); \
    return EL::StatusCode::FAILURE;                           \
  }

EL::StatusCode Analysisllqq::fillZjetsPU(){
  // select jets at reco level
  // for VBF selection: two tag jets ( eta1*eta2 < 0, highest mjj pair)
  // check the jets: mjj > 500 GeV, or detajj > 4
  // then match these reco jets with the truth jets
  
  // select good jets
  std::vector<const xAOD::Jet*> Goodjets, VBFTagjets;
  for(const xAOD::Jet *jet : *m_jets) {
    if(!Props::passOR.get(jet)) continue;
    if(jet->pt()>20.e3 && fabs(jet->eta())<2.5 || jet->pt()>30.e3 && fabs(jet->eta())<4.5) Goodjets.push_back(jet);
  }

  // select VBF tag jets
  if((int)Goodjets.size()<2) return EL::StatusCode::FAILURE;

  float m_tag=0;
  const xAOD::Jet *j_tag1 = 0, *j_tag2 = 0;
  for (const xAOD::Jet *j1 : Goodjets) {
      // tag jet should not be b-jet
      if (decorateJetsBTag(j1)) continue;
      for (const xAOD::Jet *j2 : Goodjets) {
          if (decorateJetsBTag(j2)) continue;
          if (j1 == j2) continue;
          if (j1->eta() * j2->eta() < 0) {
              // if more than one-pair of forward jet found, choose the pair with highest mass
              TLorentzVector tag_jj = j1->p4() + j2->p4();
              if (tag_jj.M() > m_tag) {
                  j_tag1 = j1;
                  j_tag2 = j2;
                  if(j1->pt()>j2->pt()) { j_tag1 = j1; j_tag2 = j2; }
                  else                  { j_tag1 = j2; j_tag2 = j1; }
                  m_tag = tag_jj.M();
              }
          }
      }
  } 

  if(!j_tag1 || !j_tag2) return EL::StatusCode::FAILURE;

  HM->Var["Mjjtag"]=m_tag/1000.;  HM->Var["dEtajjtag"]=fabs(j_tag1->eta() - j_tag2->eta());

  // pass the VBFtag jet selection
  if( m_tag>500.e3 || fabs(j_tag1->eta() - j_tag2->eta()) > 4.) {
    VBFTagjets.push_back(j_tag1);
    VBFTagjets.push_back(j_tag2);
  }
  for( const xAOD::Jet* reco_jet : VBFTagjets) {
    float reco_eta = reco_jet->eta();
    float reco_phi = reco_jet->phi();

    // do the truth matching here
    // 1. match reco jet with turth jet
    // 2. match truth jet with the partons
    // loop all truth jets

    /* // no AntiKt4Truth jets in HIGG2D4
    float dRRecoTruth = 9999.;
    int parentPDGid = 9999;
    for( const xAOD::Jet* truthJet : *m_truthJets) {
      float truth_eta = truthJet->eta();
      float truth_phi = truthJet->phi();
      float tmp_dR = Analysisllqq::deltaR(reco_eta, reco_phi, truth_eta, truth_phi); 
      if(tmp_dR < dRRecoTruth) {
        dRRecoTruth = tmp_dR;
        //the corresponding parton
        //parentPDGid = truthJet->PartonTruthLabelID();

        // unfortunately PartonTruthLabelID is not found in truthJet container
        // do turth jet to parton matching
        float maxDR = 0.4; // for AntiKt4 truth jets
        // loop all truth particles
        for (const xAOD::TruthParticle* particle : *m_truthParts) {
          int barcode = particle->barcode();
          int status = particle->status();
          int pdgId = particle->pdgId();
          float par_eta = particle->eta();
          float par_phi = particle->phi();
          // Look for the interesting partons...
          if(fabs(pdgId) > 5 && fabs(pdgId) != 21 && fabs(pdgId)!=9) continue;
          if(fabs(pdgId) == 0) continue;
          float dRJetParton=Analysisllqq::deltaR(truth_eta, truth_phi, par_eta, par_phi);
          if(dRJetParton>maxDR) continue;
          parentPDGid = pdgId;
        } // for TruthParticle
      } // if(tmp_dR < dRRecoTruth)
    } // for TruthJet
    if( parentPDGid < 0 )parentPDGid=9999; 
    // check if the reco jet matches with a parton
    if(fabs(parentPDGid) != 0 && fabs(parentPDGid)<=5 || fabs(parentPDGid)==21 || fabs(parentPDGid)==9) {
      HM->Var["Mjjtag_R"]=m_tag/1000.;  HM->Var["dEtajjtag_R"]=fabs(j_tag1->eta() - j_tag2->eta());
    } else {
      HM->Var["Mjjtag_F"]=m_tag/1000.;  HM->Var["dEtajjtag_F"]=fabs(j_tag1->eta() - j_tag2->eta());
    }
    */

  }

  // it seems that for reco jet, there is an Aux: AntiKt4EMTopoJetsAuxDyn.PartonTruthLabelID
  // so one does not need to use any turth matching
  HM->Var["TagJ1TruthID"]=j_tag1->getAttribute<int>("PartonTruthLabelID");
  HM->Var["TagJ2TruthID"]=j_tag2->getAttribute<int>("PartonTruthLabelID");


  return EL::StatusCode::SUCCESS;
}

bool Analysisllqq::decorateJetsBTag(const xAOD::Jet *j) {
    bool isBTagged = false;
    float bTagCut = -0.0436;

    const xAOD::BTagging * tagInfo = j->btagging();
    double discriminant_mv2c20=0;

    if(tagInfo) tagInfo->MVx_discriminant("MV2c20",discriminant_mv2c20);
    // pt > 20, eta < 2.5
    // MV2c20
    if (j->pt()>20.e3 && fabs(j->eta())<2.5 && discriminant_mv2c20 > bTagCut) isBTagged = true;

    return isBTagged;
}

#endif
