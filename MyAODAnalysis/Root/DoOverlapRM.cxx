#ifndef FILLHVTVV_CXX
#define FILLHVTVV_CXX

#include <EventLoop/Job.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/Worker.h>
#include "EventLoop/OutputStream.h"

#include "CxAODTools/ReturnCheck.h"
#include <MyAODAnalysis/Analysisllqq.h>

#include "xAODEventInfo/EventInfo.h"


EL::StatusCode Analysisllqq::doOverlapRM(){
  // proper overlap removal
  //       m_overlapRemoval->removeOverlap(m_electrons, m_photons, m_muons, m_taus, m_jets); 

  // set objects for OR
  if (m_electrons) {
    for (const xAOD::Electron* elec : *m_electrons) {
      Props::passPreSel.set(elec, true);
    }
  }

  if (m_muons) {
    for (const xAOD::Muon* muon : *m_muons) {
      Props::passPreSel.set(muon, true);
    }
  }

  if (m_jets) {
    for (const xAOD::Jet* jet : *m_jets) {
      Props::passPreSel.set(jet, true);
    }
  }

  if (m_taus) {
    for (const xAOD::TauJet* tau : *m_taus) {
      Props::passPreSel.set(tau, true);
    }
  }

  if (m_photons) {
    for (const xAOD::Photon* photon : *m_photons) {
      Props::passPreSel.set(photon, true);
    }
  }
  
  // Set input flag for the OR tool (IntProp::ORInputLabel)  
  if (m_electrons) {
    for (const xAOD::Electron* elec : *m_electrons) {
      Props::ORInputLabel.set(elec,Props::passPreSel.get(elec));
    }
  }

  if (m_muons) {
    for (const xAOD::Muon* muon : *m_muons) {
      Props::ORInputLabel.set(muon,Props::passPreSel.get(muon));
    }
  }
  
  if (m_jets) {
    for (const xAOD::Jet* jet : *m_jets) {
      Props::ORInputLabel.set(jet,Props::passPreSel.get(jet));
    }
  }
  
  if (m_taus) {
    for (const xAOD::TauJet* tau : *m_taus) {
      Props::ORInputLabel.set(tau,Props::passPreSel.get(tau));
    }
  }
  
  if (m_photons) {
    for (const xAOD::Photon* photon : *m_photons) {
      Props::ORInputLabel.set(photon,Props::passPreSel.get(photon));
    }
  }

  EL_CHECK("doOverlapRM()",m_OR->removeOverlaps(m_electrons, m_muons, m_jets, m_taus, nullptr));

  if(m_electrons) {
    for(const xAOD::Electron* el : *m_electrons) {
      bool passOR = Props::ORInputLabel.get(el);
      passOR &= ! Props::OROutputLabel.get(el);
      Props::passOR.set(el, passOR);
    }
  }

  if(m_photons) {
    for(const xAOD::Photon* ph : *m_photons) {
      bool passOR = Props::ORInputLabel.get(ph);
      //passOR &= ! Props::OROutputLabel.get(ph);
      //Props::passOR.set(ph, passOR);
      Props::passOR.set(ph, true);  // set all photons to pass OR
    }
  }

  if(m_muons) {
    for(const xAOD::Muon* mu : *m_muons) {
      bool passOR = Props::ORInputLabel.get(mu);
      passOR &= ! Props::OROutputLabel.get(mu);
      Props::passOR.set(mu, passOR);
    }
  }

  if(m_taus) {
    for(const xAOD::TauJet* tau : *m_taus) {
      bool passOR = Props::ORInputLabel.get(tau);
      passOR &= ! Props::OROutputLabel.get(tau);
      Props::passOR.set(tau, passOR);
    }
  }

  if(m_jets) {
    for(const xAOD::Jet* jet : *m_jets) {
      bool passOR = Props::ORInputLabel.get(jet);
      passOR &= ! Props::OROutputLabel.get(jet);
      Props::passOR.set(jet, passOR);
    }
  }

  return EL::StatusCode::SUCCESS;
}

float Analysisllqq::deltaR(float eta1, float phi1, float eta2, float phi2){
    return sqrt(pow(eta1-eta2,2)+pow(Analysisllqq::deltaPhi(phi1,phi2),2));
}

float Analysisllqq::deltaPhi(float phi1, float phi2){
    float result = fabs(phi1-phi2);
    result = result > 3.141592 ? fabs(2*3.141592-result) : result;
    return result;
}

#endif
