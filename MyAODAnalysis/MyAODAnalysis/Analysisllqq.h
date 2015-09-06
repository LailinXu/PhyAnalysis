#ifndef MyAODAnalysis_Analysisllqq_H
#define MyAODAnalysis_Analysisllqq_H

#include <EventLoop/Algorithm.h>

// Infrastructure include(s):
// EDM includes (which rootcint doesn't like)
#ifndef __MAKECINT__

#ifndef XAODMUON_ELECTRONCONTAINER_H
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/ElectronAuxContainer.h"
#endif

#ifndef XAODMUON_PHOTONCONTAINER_H
#include "xAODEgamma/PhotonContainer.h"
#include "xAODEgamma/PhotonAuxContainer.h"
#endif

#ifndef XAODMUON_JETCONTAINER_H
#include "xAODJet/JetContainer.h"
#include "xAODJet/JetAuxContainer.h"
#endif

#ifndef XAODMUON_MUONCONTAINER_H
#include "xAODMuon/MuonContainer.h"
#include "xAODMuon/MuonAuxContainer.h"
#endif

#ifndef XAODMUON_TAUCONTAINER_H
#include "xAODTau/TauJetContainer.h"
#include "xAODTau/TauJetAuxContainer.h"
#endif

#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"

#include "xAODEventInfo/EventInfo.h"

#include "xAODTruth/TruthEventContainer.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthVertexContainer.h"

#include "AssociationUtils/OverlapRemovalTool.h"

#endif // not __MAKECINT__

#include "CxAODTools/CommonProperties.h"
#include "CxAODTools/OverlapRemoval.h"

#include "MyAODAnalysis/HistoManager.h"


class Analysisllqq : public EL::Algorithm
{
  // put your configuration variables here as public variables.
  // that way they can be set directly from CINT and python.
public:
  // float cutValue;



  // variables that don't get filled at submission time should be
  // protected from being send from the submission node to the worker
  // node (done by the //!)
public:
  // Tree *myTree; //!
  // TH1 *myHist; //!
  xAOD::TEvent * m_event; //!
  long m_maxEvent; //!
  long m_eventCounter; //!

  const xAOD::EventInfo* m_eventInfo ; //!
  const xAOD::ElectronContainer*  m_electrons; //!
  const xAOD::PhotonContainer*    m_photons;   //!
  const xAOD::MuonContainer*      m_muons;     //!
  const xAOD::TauJetContainer*    m_taus;      //!
  const xAOD::JetContainer* m_jets; //!
  const xAOD::JetContainer* m_truthJets; //!
  const xAOD::JetContainer* m_fatjets; //!
  const xAOD::JetContainer* m_truthFatjets; //!
  const xAOD::TruthParticleContainer* m_truthParts ; //!

  OverlapRemovalTool* m_OR; //!
  // histogram handler
  HistoManager *HM; //!

  // this is a standard constructor
  Analysisllqq ();

  // these are the functions inherited from Algorithm
  virtual EL::StatusCode setupJob (EL::Job& job);
  virtual EL::StatusCode fileExecute ();
  virtual EL::StatusCode histInitialize ();
  virtual EL::StatusCode changeInput (bool firstFile);
  virtual EL::StatusCode initialize ();
  virtual EL::StatusCode execute ();
  virtual EL::StatusCode postExecute ();
  virtual EL::StatusCode finalize ();
  virtual EL::StatusCode histFinalize ();

  // initialze events
  EL::StatusCode initializeEvent();
  // initialze tools
  EL::StatusCode initializeTools();
  // trace up truth particles
  int TraceUp(const xAOD::TruthParticle* particle);
  int TraceUpHadron(const xAOD::TruthParticle* particle);
  // overlap removal 
  EL::StatusCode doOverlapRM();
  float deltaR(float eta1, float phi1, float eta2, float phi2);
  float deltaPhi(float phi1, float phi2);

  // HVT VV truth selection
  virtual EL::StatusCode fillHVTVV();
  // Z+jets: jets matching to identify the jets is truth jets or pileup jets
  virtual EL::StatusCode fillZjetsPU();

  bool decorateJetsBTag(const xAOD::Jet *j);

  // this is needed to distribute the algorithm to the workers
  ClassDef(Analysisllqq, 1);
};

// sort particle containter by pt
static bool sort_par_pt(const xAOD::IParticle* parA, const xAOD::IParticle* parB) {
  return parA->pt() > parB->pt();
}

#endif
