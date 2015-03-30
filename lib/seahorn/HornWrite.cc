#include "seahorn/HornWrite.hh"
#include "seahorn/HornifyModule.hh"
#include "seahorn/HornClauseDBTransf.hh"
#include "seahorn/ClpWrite.hh"

#include "llvm/Support/CommandLine.h"

static llvm::cl::opt<bool>
InternalWriter("horn-fp-internal-writer",
               llvm::cl::desc("Use internal writer for Horn SMT2 format. (Default)"),
               llvm::cl::init(true),llvm::cl::Hidden);

enum HCFormat { SMT2, CLP};
static llvm::cl::opt<HCFormat>
HornClauseFormat("horn-format",
       llvm::cl::desc ("Specify the format for Horn Clauses"),
       llvm::cl::values 
       (clEnumValN (SMT2,"smt2",
                    "SMT2 (default)"),
        clEnumValN (CLP, "clp",
                    "CLP (Constraint Logic Programming)"),
        clEnumValEnd),
       llvm::cl::init (SMT2));

namespace seahorn
{
  char HornWrite::ID = 0;
  
  void HornWrite::getAnalysisUsage (AnalysisUsage &AU) const
  {
    AU.addRequired<HornifyModule> ();
    AU.setPreservesAll ();
  }
  
  bool HornWrite::runOnModule (Module &M)
  {
    HornifyModule &hm = getAnalysis<HornifyModule> ();
    HornClauseDB &db  = hm.getHornClauseDB ();
    // this will have effect only if -horn-clp-sem is enabled.
    normalizeHornClauseHeads (db);

    ExprFactory &efac = hm.getExprFactory ();

    if (HornClauseFormat == CLP)
    {
      ClpWrite writer (db, efac);
      m_out << writer.toString ();
    }
    else 
    {
      // Use local ZFixedPoint object to translate to SMT2. 
      //
      // When HornWrite is called hm.getZFixedPoint () might be still
      // empty so we need to dump first the content of HornClauseDB
      // into fp.
      ZFixedPoint<EZ3> fp (hm.getZContext ());
      // -- skip constraints since they are not supported.
      // -- do not skip the query
      db.loadZFixedPoint (fp, true, false);

      if (InternalWriter)
        m_out << fp << "\n";
      else
        m_out << fp.toString () << "\n";
    }
    
    m_out.flush ();
    return false;
  }
  
}