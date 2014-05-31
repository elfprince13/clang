//
//  ParseSkel.cpp
//  LLVM
//
//  Created by Thomas Dickerson on 5/29/14.
//
//

#include "clang/Parse/Parser.h"
#include "RAIIObjectsForParser.h"
#include "clang/Basic/CharInfo.h"
#include "clang/Parse/ParseDiagnostic.h"
#include "clang/Sema/DeclSpec.h"
#include "clang/Sema/PrettyDeclStackTrace.h"
#include "clang/Sema/SemaDiagnostic.h"
#include "clang/Sema/Scope.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/DynamicLibrary.h"
using namespace clang;


Parser::DeclGroupPtrTy Parser::ParseTopLevelSkeleton() {
	return DeclGroupPtrTy();
}


Parser::StmtResult Parser::ParseSkeleton(SourceLocation AtLoc){
	StmtResult ret = StmtError();
	if (Tok.is(tok::identifier)) {
		IdentifierInfo *ii = Tok.getIdentifierInfo();
		SkeletonHandler handler = GetHandlerForSkeleton(*ii);
		if (handler == nullptr) {
			// TODO: this blows up if uncommented, but I can't even step
			// in with the debugger for some reason to see why.
			// I think I must need access to a Sema or something.
			//Diag(Tok, diag::err_undeclared_var_use);
		} else {
			ret = handler(ConsumeToken());
		}
	} else {
		Diag(Tok, diag::err_unexpected_at);
	}
	return ret;
}


Parser::StmtResult doParseSkeletonWithName_Loop2dAccumulate(SourceLocation kindLoc){
	return StmtResult();
}

Parser::SkeletonHandler Parser::GetHandlerForSkeleton(IdentifierInfo &skelIdent){
	// Hard-code this as a test;
	llvm::sys::DynamicLibrary::AddSymbol("doParseSkeletonWithName_Loop2dAccumulate", (void*)&doParseSkeletonWithName_Loop2dAccumulate);
	
	std::string symName("doParseSkeletonWithName_");
	StringRef skelName = skelIdent.getName();
	symName += skelName;
	
	return (SkeletonHandler)llvm::sys::DynamicLibrary::SearchForAddressOfSymbol(symName);
}
