#ifndef MyAODAnalysis_Analysisllqq_H
#define MyAODAnalysis_Analysisllqq_H

#include <EventLoop/Algorithm.h>

// Infrastructure include(s):
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"

#include "xAODTruth/TruthEventContainer.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthVertexContainer.h"

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

  EL::StatusCode initializeEvent();
  int TraceUp(const xAOD::TruthParticle* particle);
  int TraceUpHadron(const xAOD::TruthParticle* particle);

  // this is needed to distribute the algorithm to the workers
  ClassDef(Analysisllqq, 1);
};

// sort particle containter by pt
static bool sort_par_pt(const xAOD::IParticle* parA, const xAOD::IParticle* parB) {
  return parA->pt() > parB->pt();
}

#endif
