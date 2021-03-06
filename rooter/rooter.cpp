/* Rooter: stores MOL output from stdin in a ROOT TTree of 64-bit values
 * 
 * Syntax: cat input.dat | rooter output.root
 */

#include "TFile.h"
#include "TTree.h"
#include <iostream>

using namespace std;

#define GZ 0x5a47 // FEROL "magic number"
#define START (1L<<39) // initial block: bit 31 --> 39 (endian flip), as well as zeroes in 42..32

int main(int argc, char* argv[]) {
  char* rootfile;
  if (argc == 1) rootfile = "mol.root";
  else rootfile = argv[1];
  TFile a(rootfile, "RECREATE");
  TTree tree("moltree","");
  
  const int MAX_WORDS = 100000;

  uint64_t blob[MAX_WORDS];
  int len = 0;
  tree.Branch("nWord64",&len,"nWord64/I");
  tree.Branch("words",blob,"words[nWord64]/l");

  uint64_t buf;
  int n = 0; // fragments
  int m = 0; // blocks
  int success = fread(&buf,sizeof(buf),1,stdin); // read a word!
  while (success) {
    if ((buf & 0xffff) == GZ) { // if it's a new block, increment the block counter
      m++;
      if ((buf & 0x80ffff0000L) == START && len) { // if it's a new fragment, fill the tree.
        tree.Fill();
        len = 0;
        n++;
      }
    }

    if (len == MAX_WORDS) {
      cout << "Too many words in fragment! (" << MAX_WORDS << " 64-bit words)" << endl;
      return 1;
    }

    blob[len] = buf; // store the read word
    len++;
 
    success = fread(&buf,sizeof(buf),1,stdin); // read another word!
  }

  tree.Fill();
  tree.Write();
  a.Close();

  cout << n+1 << " fragments in " << m << " blocks." << endl;
  return 0;
}
