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
#include "clang/Sema/Scope.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/DynamicLibrary.h"
using namespace clang;


Parser::DeclGroupPtrTy Parser::ParseTopLevelSkeleton() {
	return DeclGroupPtrTy();
}


Parser::StmtResult Parser::ParseSkeleton(SourceLocation AtLoc){
	if (Tok.is(tok::identifier)) {
		IdentifierInfo *ii = Tok.getIdentifierInfo();
		SkeletonHandler handler = GetHandlerForSkeleton(*ii);
		return (handler == nullptr) ? StmtError() : handler(ConsumeToken());
	} else {
		return StmtError();
	}
}

Parser::SkeletonHandler Parser::GetHandlerForSkeleton(IdentifierInfo &skelIdent){
	std::string symName("doParseSkeletonWithName-");
	StringRef skelName = skelIdent.getName();
	symName += skelName;
	
	return (SkeletonHandler)llvm::sys::DynamicLibrary::SearchForAddressOfSymbol(symName);
}