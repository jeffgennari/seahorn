#ifndef __EXPR__LLVM__HPP_
#define __EXPR__LLVM__HPP_

#include "ufo/Expr.hpp"

#include <boost/functional/hash.hpp>

#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Value.h"
#include "llvm/ADT/APInt.h"
#include "llvm/IR/Constants.h"

#include <boost/lexical_cast.hpp>


namespace expr
{
  using namespace llvm;
  
  inline llvm::raw_ostream &operator<<(llvm::raw_ostream &OS, const Expr &p)
  {
    OS << p.get ();
    return OS;
  }

  inline llvm::raw_ostream &operator<<(llvm::raw_ostream &OS, 
				       const ENode &n)
  {
    OS << boost::lexical_cast<std::string> (n);
    return OS;
  }
  
  using namespace llvm;
  template<> struct TerminalTrait<const Function*>
  {
    static inline void print (std::ostream &OS, const Function *f,
                              int depth, bool brkt)
    {OS << f->getName ().str ();}
    
    static inline bool less (const Function *f1, const Function *f2)
    {return f1 < f2;}
    
    static inline bool equal_to (const Function *f1, const Function *f2)
    {return f1 == f2;}
    
    static inline size_t hash (const Function *f)
    {
      boost::hash<const Function*> hasher;
      return hasher (f);
    }
  };
    
  
  template<> struct TerminalTrait<const BasicBlock*>
  {
    static inline void print (std::ostream &OS, const BasicBlock* s, 
			      int depth, bool brkt) 
    { 
      OS << s->getParent ()->getName ().str () + "@" + s->getName ().str ();
    }
    static inline bool less (const BasicBlock* s1, const BasicBlock* s2)
    { return s1 < s2; }

    static inline bool equal_to (const BasicBlock *b1, const BasicBlock *b2)
    { return b1 == b2; }

    static inline size_t hash (const BasicBlock *b)
    {
      boost::hash<const BasicBlock *> hasher;
      return hasher (b);
    }
    
    
  };
  
  template<> struct TerminalTrait<const Value*>
  {
    static inline void print (std::ostream &OS, const Value* s, 
			      int depth, bool brkt) 
    {
      // -- name instructions uniquely based on the name of their containing function
      if (const Instruction *inst = dyn_cast<const Instruction> (s))
      {
        const BasicBlock *bb = inst->getParent ();
        const Function *fn = bb ? bb->getParent () : NULL;
        if (fn) OS << fn->getName ().str () << "@";
      }
      else if (const Argument *arg = dyn_cast<const Argument> (s))
      {
        const Function *fn = arg->getParent ();
        if (fn) OS << fn->getName ().str () << "@";
      }
      
      if (s->hasName ())
	OS << (isa<GlobalValue> (s) ? '@' : '%')
	   << s->getName ().str ();
      else
	{
          // names of constant expressions
	  std::string ssstr;
	  raw_string_ostream ss(ssstr);
	  ss <<  *s;
          OS << ss.str ();
          
          // std::string str = ss.str();
	  // int f = str.find_first_not_of(' ');
          // std::string s1 = str.substr(f);
	  // f = s1.find_first_of(' ');
	  // OS << s1.substr(0,f);
	}
    }
    static inline bool less (const Value* s1, const Value* s2)
    { return s1 < s2; }

    static inline bool equal_to (const Value *v1, const Value *v2)
    { return v1 == v2; }
    
    static inline size_t hash (const Value *v)
    {
      boost::hash<const Value*> hasher;
      return hasher (v);
    }
  };
  
  typedef expr::Terminal<const llvm::BasicBlock*> BB;
  typedef expr::Terminal<const llvm::Value*> VALUE;
  typedef expr::Terminal<const llvm::Function*> FUNCTION;
  
    /** Converts v to mpz_class. Assumes that v is signed */
  inline mpz_class toMpz (const APInt &v)
  {
    // Based on:
    // https://llvm.org/svn/llvm-project/polly/trunk/lib/Support/GICHelper.cpp
    // return v.getSExtValue ();

    APInt abs;
    abs = v.isNegative () ? v.abs () : v;
    
    const uint64_t *rawdata = abs.getRawData ();
    unsigned numWords = abs.getNumWords ();

    // TODO: Check if this is true for all platforms.
    mpz_class res;
    mpz_import(res.get_mpz_t (), numWords, 1, sizeof (uint64_t), 0, 0, rawdata);

    return v.isNegative () ? mpz_class(-res) : res;
  }
  
  inline mpz_class toMpz (const Value *v)
  {
    if (const ConstantInt *k = dyn_cast<ConstantInt> (v))
      return toMpz (k->getValue ());
    if (isa<ConstantPointerNull> (v)) return 0;
    
    assert (0 && "Not a number");
    return 0;
  }

}


#endif
