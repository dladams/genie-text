// g2hepmc.cxx

#include <iostream>
#include "TFile.h"
#include "TTree.h"
#include "Ntuple/NtpMCTreeHeader.h"

using std::cout;
using std::endl;
using genie::NtpMCTreeHeader;

int main(int argc, char** argv) {

  if ( argc < 2 )  {
    cout << "Usage: " << argv[0] << " ROOTFILE" << endl;
    return 1;
  }

  TFile* pfile = TFile::Open(argv[1], "R");
  if ( pfile == nullptr || !pfile->IsOpen() ) {
    cout << "Unable to open input file: " << argv[1] << endl;
    return 2;
  }

  TTree* ptree = dynamic_cast <TTree *>(pfile->Get("gtree"));
  if ( ptree == nullptr ) {
    cout << "Unable to find genie tree" << endl;
    return 3;
  }

  NtpMCTreeHeader* phdr = dynamic_cast<NtpMCTreeHeader*>(pfile->Get("header"));
  if ( phdr == nullptr ) {
    cout << "Unable to retrieve header." << endl;
    return 4;
  }

  cout << "Event count: " << ptree->GetEntries() << endl;
  cout << "Input tree header: " << *phdr << endl;

  return 0;
}
  
