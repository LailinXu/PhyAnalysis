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
#include "xAODCutFlow/CutBookkeeper.h"
#include "xAODCutFlow/CutBookkeeperContainer.h"

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

      bool metafailed=false;

      xAOD::TEvent m_event;
      if(!m_event.readFrom(file, true, "MetaData").isSuccess()) {
        std::cout << "Error=> could not read MetaData" << std::endl;
        metafailed=true;
      }
     
      // get the MetaData tree once a new file is opened, with
      TTree *MetaData = (TTree*) file->Get("MetaData");
      if (!MetaData) {
        std::cout << "fileExecute() MetaData not found! Exiting." << std::endl;
        metafailed=true;
      }
      MetaData->LoadTree(0);
      
      //check if file is from a DxAOD
      bool m_isDerivation = !MetaData->GetBranch("StreamAOD");
      
      if(m_isDerivation ){
        const xAOD::CutBookkeeperContainer* incompleteCBC = nullptr;
        if(!m_event.retrieveMetaInput(incompleteCBC, "IncompleteCutBookkeepers").isSuccess()){
          std::cout << "initializeEvent() Failed to retrieve IncompleteCutBookkeepers from MetaData! Exiting." << std::endl;
          metafailed=true;
        }
        if ( !metafailed && incompleteCBC->size() != 0 ) {
          std::cout << "initializeEvent() Found incomplete Bookkeepers! Check file for corruption." << std::endl;
          metafailed=true;
        }
        // Now, let's find the actual information
        const xAOD::CutBookkeeperContainer* completeCBC = 0;
        if(!m_event.retrieveMetaInput(completeCBC, "CutBookkeepers").isSuccess()){
          std::cout << "initializeEvent() Failed to retrieve CutBookkeepers from MetaData! Exiting." << std::endl;
          metafailed=true;
        }
      
        if(!metafailed) {
          // First, let's find the smallest cycle number,
          // i.e., the original first processing step/cycle
          int minCycle = 10000;
          for ( auto cbk : *completeCBC ) {
            if ( ! cbk->name().empty()  && minCycle > cbk->cycle() ){ minCycle = cbk->cycle(); }
          }
          // Now, let's actually find the right one that contains all the needed info...
          const xAOD::CutBookkeeper* allEventsCBK=0;
          const xAOD::CutBookkeeper* DxAODEventsCBK=0;
          std::string derivationName = "HIGG2D4Kernel"; //need to replace by appropriate name
          for ( auto cbk :  *completeCBC ) {
            if ( minCycle == cbk->cycle() && cbk->name() == "AllExecutedEvents" ){
              allEventsCBK = cbk;
            }
            if ( cbk->name() == derivationName){
              DxAODEventsCBK = cbk;
            }
          }
          uint64_t nEventsProcessed  = allEventsCBK->nAcceptedEvents();
          double sumOfWeights        = allEventsCBK->sumOfEventWeights();
          double sumOfWeightsSquared = allEventsCBK->sumOfEventWeightsSquared();
        
          uint64_t nEventsDxAOD           = DxAODEventsCBK->nAcceptedEvents();
          double sumOfWeightsDxAOD        = DxAODEventsCBK->sumOfEventWeights();
          double sumOfWeightsSquaredDxAOD = DxAODEventsCBK->sumOfEventWeightsSquared();
          /*
          std::cout << "nEventsProcessed:  " << nEventsProcessed << std::endl;
          std::cout << "sumOfWeights:      " << sumOfWeights << std::endl;
          std::cout << "nEventsDxAOD:      " << nEventsDxAOD << std::endl;
          std::cout << "sumOfWeightsDxAOD: " << sumOfWeightsDxAOD << std::endl;
          */
          nTotDataset += nEventsProcessed;
          nTotDataset_Wt += sumOfWeights;
        } else {
          TH1F* hEventCount = (TH1F*) file->Get("MetaData_EventCount");
          uint64_t nEventsProcessed  = 0;
          double sumOfWeights        = 0;
          if(hEventCount) {
            nEventsProcessed  = hEventCount->GetBinContent(1);  // raw number of events
            sumOfWeights  = hEventCount->GetBinContent(4);   // sum of weights (MC weight)
          }
          nTotDataset += nEventsProcessed;
          nTotDataset_Wt += sumOfWeights;
           
        }
          
      }

      file->Close(); // ROOT could not open too many files

    } // loop all DAOD files for each dataset
    
    dict_nTot[list]=nTotDataset;
    foutput << list << " " << nTotDataset << " " << nTotDataset_Wt << std::endl;
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
