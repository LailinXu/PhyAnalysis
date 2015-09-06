#include <sstream>
#include <iostream>
#include <iomanip>
#include <fstream>

#include <TSystem.h>
#include <TFile.h>
#include <TTree.h>
#include <TH1.h>

#include "AthContainers/AuxElement.h"

#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"

#include "AsgTools/IAsgTool.h"
#include "AsgTools/AsgTool.h"

#include "xAODTruth/TruthEventContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthVertexContainer.h"

#include "xAODEventInfo/EventInfo.h"

#include "AsgTools/StatusCode.h"
#include "EventLoop/StatusCode.h"

#define EL_RETURN_CHECK( CONTEXT, EXP )                              \
  if (EXP !=  EL::StatusCode::SUCCESS) {                      \
    Error( CONTEXT, "Failed to execute: %s. Exiting.", #EXP); \
    return EL::StatusCode::FAILURE;                           \
  }


using namespace std;

bool cmdline(int argc, char** argv, map<string,string> &opts);
void usage();

int main(int argc, char* argv[]) {

  map<string, string> opts;
  if (!cmdline(argc, argv, opts)) return 0;

  // inputlist is a list of list that contains the DAOD root files
  string inputlist = opts["inputlist"];
  string output = opts["output"];
  ofstream foutput(output);
  
  //gROOT->Macro( "$ROOTCOREDIR/scripts/load_packages.C" );
  xAOD::Init().ignore();

  //<Get input list>
  ifstream filelist;
  filelist.open(opts["inputlist"]);
  if(!filelist.good()) {
      cout<<"ERROR: Cannot find the input filelist, now quit!"<<endl;
      return 2;
  }

  std::map<string, uint64_t> dict_nTot;

  std::string m_xaodTruthEventContainerName = "TruthEvents";
  std::string m_xaodTruthParticleContainerName = "TruthParticles";

  string list, data;
  // loop dataset list
  while(!filelist.eof()) {
    getline(filelist,list);
    if(list.size()==0) continue; //remove the blank lines
    cout<<"Read file \""<<list<<"\""<<endl;

    ifstream datalist;
    datalist.open(list); 

    uint64_t nTotDataset=0;
    double   nTotDataset_Wt=0;

    // loop DAOD files for each dataset
    while(!datalist.eof()) {
      getline(datalist,data);
      if(data.size()==0) continue; //remove the blank lines

      TFile *file = TFile::Open(data.c_str());
      cout<<"  Read file \""<<data.c_str()<<"\""<<endl;

      bool metafailed=false;

      xAOD::TEvent m_event;
      if(!m_event.readFrom(file, true, "CollectionTree").isSuccess()) {
        std::cout << "Error=> could not read CollectionTree" << std::endl;
        metafailed=true;
      }
     
      // get the CollectionTree tree once a new file is opened, with
      TTree *CollectionTree = (TTree*) file->Get("CollectionTree");
      if (!CollectionTree) {
        std::cout << "fileExecute() CollectionTree not found! Exiting." << std::endl;
        metafailed=true;
      }

      uint64_t nEntries = CollectionTree->GetEntries();
      int nRead=(nEntries>100) ? 100 : nEntries;
      for( uint64_t iEntry=0; iEntry < nRead; iEntry++) { 
        CollectionTree->LoadTree(iEntry); 
 
        const xAOD::EventInfo * eventInfoIn = 0;
        EL_RETURN_CHECK( "retrieve_eventInfoIn()", m_event.retrieve(eventInfoIn, "EventInfo"));
        cout << "Info=> EvnetNumber: " << eventInfoIn->eventNumber() << endl;

        const xAOD::TruthEventContainer* xTruthEventContainer = NULL;
        EL_RETURN_CHECK( "retrieve_xTruthEventContainer()", m_event.retrieve( xTruthEventContainer, m_xaodTruthEventContainerName));   

        xAOD::TruthEventContainer::const_iterator itr;
        for (itr = xTruthEventContainer->begin(); itr!=xTruthEventContainer->end(); ++itr) {
          /* not available in DAOD
          int nVert = (*itr)->numTruthVertices();
          int nPart = (*itr)->numTruthParticles();

          int id1(0); (*itr)->pdfInfoParameter(id1,xAOD::TruthEvent::id1);
          int id2(0); (*itr)->pdfInfoParameter(id2,xAOD::TruthEvent::id2);
          int pdfId1(0); (*itr)->pdfInfoParameter(pdfId1,xAOD::TruthEvent::pdfId1);
          int pdfId2(0); (*itr)->pdfInfoParameter(pdfId2,xAOD::TruthEvent::pdfId2);
          float x1(0.0); (*itr)->pdfInfoParameter(x1,xAOD::TruthEvent::x1);
          float x2(0.0); (*itr)->pdfInfoParameter(x2,xAOD::TruthEvent::x2);
          float scalePDF(0.0); (*itr)->pdfInfoParameter(scalePDF,xAOD::TruthEvent::scalePDF);
          float pdf1(0.0); (*itr)->pdfInfoParameter(pdf1,xAOD::TruthEvent::pdf1);
          float pdf2(0.0); (*itr)->pdfInfoParameter(pdf2,xAOD::TruthEvent::pdf2);
          */
          /*
          std::vector<int> id1(0); id1=(*itr)->PDGID1();
          std::vector<int> id2(0); id2=(*itr)->PDGID2();
          std::vector<int> pdfId1(0); pdfId1=(*itr)->PDFID1();
          std::vector<int> pdfId2(0); pdfId2=(*itr)->PDFID2();
          std::vector<float> x1(0.0); x1=(*itr)->X1();
          std::vector<float> x2(0.0); x2=(*itr)->X2();
          std::vector<float> scalePDF(0.0); scalePDF=(*itr)->Q();
          */

          //std::cout << "Check=> truth scale: " << scalePDF << " pdfId1: " << pdfId1 << " pdfId2: " << pdfId2 << std::endl;
          /*
          std::pair<const xAOD::TruthParticle*,const xAOD::TruthParticle*> beamParticles = (*itr)->beamParticles();
          const std::vector<float> weights = (*itr)->weights();
          float scale = (*itr)->eventScale();
          float qcd = (*itr)->alphaQCD();
          float qed = (*itr)->alphaQED();

          std::cout << "Check=> truth scale: " << scale << " qcd: " << qcd << " qed: " << qed << std::endl;
          */
          //const xAOD::TruthVertex* vertex = (*itr)->truthVertex(iVtx);
          //const xAOD::TruthParticle* particle = (*itr)->truthParticle(iPart);
        }

        const xAOD::TruthParticleContainer* xTruthParticleContainer = NULL;
        EL_RETURN_CHECK( "retrieve_xTruthParticleContainer()", m_event.retrieve( xTruthParticleContainer, m_xaodTruthParticleContainerName));   
        for (const xAOD::TruthParticle* particle : *xTruthParticleContainer) {
          int barcode = particle->barcode();
          int status = particle->status();
          int pdgId = particle->pdgId();
          
          float px = particle->px();

          //std::cout << "Check=> truth par pdgId: " << pdgId << " status: " << status << std::endl;
          bool hasDecayVtx = particle->hasDecayVtx();
          bool hasProdVtx = particle->hasProdVtx();
          const xAOD::TruthVertex* prodVtx = particle->prodVtx();
          const xAOD::TruthVertex* decayVtx = particle->decayVtx();

          // find the child particles from V boson
          if(fabs(pdgId)>=11 && fabs(pdgId)<=16 || pdgId==23 || pdgId==24 || pdgId==25 || fabs(pdgId)<6 && (status==3 || status==11)) {
            std::cout << "Check=> truth par pdgId: " << pdgId << " status: " << status << std::endl;
            int nChild=particle->nChildren();
            for(int ich=0; ich<nChild; ich++) {
              const xAOD::TruthParticle* ch = particle->child(ich);
              if(!ch) continue;
              int tmp_barcode = ch->barcode();
              int tmp_status = ch->status();
              int tmp_pdgId = ch->pdgId();
              std::cout << "Check=>   child: " << tmp_pdgId << " status: " << tmp_status << std::endl;
            }
          }

          // find the parent particles 
          if(fabs(pdgId)==11 || fabs(pdgId)==13 || fabs(pdgId)==15) {
            std::cout << "Check=> truth par pdgId: " << pdgId << " status: " << status << std::endl;
            int nPar=particle->nParents();
            for(int ich=0; ich<nPar; ich++) {
              const xAOD::TruthParticle* ch = particle->parent(ich);
              if(!ch) continue;
              int tmp_barcode = ch->barcode();
              int tmp_status = ch->status();
              int tmp_pdgId = ch->pdgId();
              std::cout << "Check=>   mother: " << tmp_pdgId << " status: " << tmp_status << std::endl;
            }
          }
        }
 
      }

      file->Close(); // ROOT could not open too many files

    } // loop all DAOD files for each dataset
    
  } // loop all dataset

  return 0;

}


bool cmdline(int argc, char** argv, map<string,string> &opts) {
    opts.clear();

    // defaults
    opts["inputlist"] = "";
    opts["output"] = "";
    opts["debug"] = "0";

    for (int i=1;i<argc;i++) {

        string opt=argv[i];

        if (opt=="--help") {usage(); return false;}

        if (0 != opt.find("-")) {
            cout<<"ERROR: options start with '-'!"<<endl;
            return false;
        }
        opt.erase(0,1);
        if (opts.find(opt) == opts.end()) {
            cout<<"ERROR: invalid option '"<<opt<<"'!"<<endl;
            return false;
        }
        string nxtopt=argv[i+1];
        if (0 == nxtopt.find("-") || i+1 >= argc) {
            cout<<"ERROR: option '"<<opt<<"' requires value!"<<endl;
            return false;
        }

        opts[opt] = nxtopt;
        i++;
    }



    return true;
}

void usage()
{
    cout<<"USAGE: run [-option value]\n\n"
        <<"options [default]:\n\n"
        <<"-inputlist (required!)\n"
        <<"-output (required!)\n"
        <<"-debug <0/1> [0]\n"
        <<endl;

    return;
}
