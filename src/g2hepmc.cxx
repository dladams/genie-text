// g2hepmc.cxx

// Converts a GENIE event file to HEPEVT format.
// For the latter, see e.g.
// http://http://cepa.fnal.gov/psm/simulation/mcgen/lund/pythia_manual/pythia6.3/pythia6301/node39.html

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
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
using std::istringstream;
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

  // Open the input file.
  TFile* pfile = TFile::Open(argv[1], "R");
  if ( pfile == nullptr || !pfile->IsOpen() ) {
    cout << "Unable to open input file: " << argv[1] << endl;
    return 2;
  }

  // Open the output file.
  ofstream fout(argv[2], ofstream::out|ofstream::trunc);
  fout.precision(9);

  // Read arguments of form NAME=VAL.
  double x0 = 0.0;  // Units are mm.
  double y0 = 0.0;
  double z0 = 0.0;
  double t0 = 0.0;  // Units are mm/c
  int posopt = 1;  // 0=particle, 1=vertex, 2=command line
  for ( int iarg=3; iarg<argc; ++iarg ) {
    string arg = argv[iarg];
    string::size_type ipos = arg.find("=");
    if ( ipos == string::npos ) {
      cout << "Invalid command line argument: " << arg << endl;
      return 3;
    }
    string name = arg.substr(0, ipos);
    string sval = arg.substr(ipos+1);
    istringstream ssval(sval);
    double val;
    ssval >> val;
    int ival = val + 0.49;
    if      ( name == "x" ) { posopt = true; x0 = val; }
    else if ( name == "y" ) { posopt = true; y0 = val; }
    else if ( name == "z" ) { posopt = true; z0 = val; }
    else if ( name == "t" ) { posopt = true; t0 = val; }
    else if ( name == "posopt" ) posopt = ival;
    else {
      cout << "Invalid name for command line argument: " << arg << endl;
      return 4;
    }
  }

  if ( posopt == 0 ) {
    cout << "Particle positions taken from particles." << endl;
  } else if ( posopt == 1 ) {
    cout << "Particle positions taken from vertices." << endl;
  } else if ( posopt == 2 ) {
    cout << "Particle positions fixed:" << endl;
    cout << "Position option:" << endl;
    cout << "  x0 = " << x0 << " mm " << endl;
    cout << "  y0 = " << x0 << " mm " << endl;
    cout << "  z0 = " << x0 << " mm " << endl;
    cout << "  t0 = " << x0 << " mm/c" << endl;
  }

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
    unsigned int npar = event.GetEntries();
    cout << "Processing event " << evhdr.ievent << endl;
    fout << evhdr.ievent << " " << npar << endl;
    unsigned int ipar = 0;
    TObjArrayIter iobj(&event);
    TLorentzVector* pvtx = event.Vertex();
    double x = x0;
    double y = y0;
    double z = z0;
    double t = t0;
    if ( posopt == 1 ) {
      double x = pvtx->X();
      double y = pvtx->Y();
      double z = pvtx->Z();
      double t = pvtx->T();
    }
    while ( TObject* pobj = iobj.Next() ) {
      GHepParticle* ppar = dynamic_cast<GHepParticle*>(pobj);
      if ( ppar == nullptr ) {
        cout << "Unexpected invalid particle!" << endl;
        return 5;
      }
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
      if ( posopt == 0 ) {
        x = posfac*ppar->Vx();
        y = posfac*ppar->Vy();
        z = posfac*ppar->Vz();
        t = timfac*ppar->Vt();
      }
      fout << sep << x;
      fout << sep << y;
      fout << sep << z;
      fout << sep << t;
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
  
