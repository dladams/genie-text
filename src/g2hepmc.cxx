// g2hepmc.cxx

// Converts a GENIE event file to HEPEVT format.
// For the latter, see e.g.
// http://http://cepa.fnal.gov/psm/simulation/mcgen/lund/pythia_manual/pythia6.3/pythia6301/node39.html

#include <iostream>
#include <fstream>
#include <string>
#include "TFile.h"
#include "TTree.h"
#include "EVGCore/EventRecord.h"
#include "GHEP/GHepParticle.h"
#include "Ntuple/NtpMCTreeHeader.h"
#include "Ntuple/NtpMCEventRecord.h"
#include "Ntuple/NtpMCRecHeader.h"

using std::string;
using std::cout;
using std::endl;
using std::ofstream;
using genie::EventRecord;
using genie::NtpMCTreeHeader;
using genie::NtpMCEventRecord;
using genie::NtpMCRecHeader;
using genie::GHepParticle;

int main(int argc, char** argv) {

  if ( argc < 3 )  {
    cout << "Usage: " << argv[0] << " ROOTFILE HEPEVTFILE" << endl;
    return 1;
  }
  cout << "  Input file: " << argv[1] << endl;
  cout << " Output file: " << argv[2] << endl;

  TFile* pfile = TFile::Open(argv[1], "R");
  if ( pfile == nullptr || !pfile->IsOpen() ) {
    cout << "Unable to open input file: " << argv[1] << endl;
    return 2;
  }

  ofstream fout(argv[2], ofstream::out|ofstream::trunc);
  fout.precision(9);

  TTree* ptree = dynamic_cast <TTree *>(pfile->Get("gtree"));
  if ( ptree == nullptr ) {
    cout << "Unable to find genie tree" << endl;
    return 3;
  }

  // Fetch the header.
  NtpMCTreeHeader* phdr = dynamic_cast<NtpMCTreeHeader*>(pfile->Get("header"));
  if ( phdr == nullptr ) {
    cout << "Unable to retrieve header." << endl;
    return 4;
  }

  cout << "Event count: " << ptree->GetEntries() << endl;
  cout << "Input tree header: " << *phdr << endl;

  // Fetch the event record.
  NtpMCEventRecord* pmcrec = 0;
  ptree->SetBranchAddress("gmcrec", &pmcrec);

  // Loop over events.
  for ( unsigned int ient=0; ient<ptree->GetEntries(); ++ient ) {
    ptree->GetEntry(ient);
    NtpMCRecHeader evhdr = pmcrec->hdr;
    EventRecord& event = *(pmcrec->event);
    TObjArrayIter iobj(&event);
    unsigned int npar = event.GetEntries();
    fout << evhdr.ievent << " " << npar << endl;
    unsigned int ipar = 0;
    while ( TObject* pobj = iobj.Next() ) {
      GHepParticle* ppar = dynamic_cast<GHepParticle*>(pobj);
      if ( ppar == nullptr ) {
        cout << "Unexpected invalid particle!" << endl;
        return 5;
      }
      cout << "  Particle " << ipar << ": " << long(pobj) << endl;
      string sep = " ";
      double posfac = 1000.0;  // Convert m to mm
      double timfac = 1000.0*299792458.0;  // Convert s to mm/c.
      fout << sep << ppar->Status();
      fout << sep << ppar->Pdg();
      fout << sep << ppar->FirstMother() + 1;
      fout << sep << ppar->LastMother() + 1;
      fout << sep << ppar->FirstDaughter() + 1;
      fout << sep << ppar->LastDaughter() + 1;
      fout << sep << ppar->Px();
      fout << sep << ppar->Py();
      fout << sep << ppar->Pz();
      fout << sep << ppar->E();
      fout << sep << ppar->Mass();
      fout << sep << posfac*ppar->Vx();
      fout << sep << posfac*ppar->Vy();
      fout << sep << posfac*ppar->Vz();
      fout << sep << timfac*ppar->Vt();
      fout << endl;
      ++ipar;
    }
    if ( ipar != npar ) {
      cout << "Unexpected particle count" << endl;
      return 6;
    }
  }

  return 0;
}
  
