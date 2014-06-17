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
	StmtResult ret;
	if (Tok.is(tok::identifier)) {
		IdentifierInfo *ii = Tok.getIdentifierInfo();
		SourceLocation SkelLoc = ConsumeToken();
		SkeletonHandler handler = GetHandlerForSkeleton(*ii);
		if (handler == nullptr) {
			ret = StmtError(Diag(SkelLoc, diag::err_undeclared_var_use) << ii);
		} else {
			if(Tok.is(tok::l_paren)) {
				// Skeletons may attach a name for their local block of code
				BalancedDelimiterTracker T(*this, tok::l_paren);
				T.consumeOpen();
				
				if(Tok.is(tok::identifier)) {
					IdentifierInfo *is = Tok.getIdentifierInfo();
					ConsumeToken();
				} else {
					ret = StmtError(Diag(Tok, diag::err_expected_unqualified_id) << getLangOpts().CPlusPlus);
				}
				
				T.consumeClose();
			}
			
			while (Tok.is(tok::l_square)){
				BalancedDelimiterTracker T(*this, tok::l_square);
				T.consumeOpen();
				
				if(Tok.is(tok::identifier)) {
					IdentifierInfo *ip = Tok.getIdentifierInfo();
					ConsumeToken();
					ExprResult er;
					bool colon = Tok.is(tok::colon);
					bool equal = Tok.is(tok::equal);
					assert((!colon || !equal) && "Can't have parsed both a colon and an equal");
					if (colon || equal) {
						ConsumeToken();
						
						er = ParseExpression();
		
						if(colon) {
							// We've parsed out an identifier that we're going to perform
							// macro-style substitutions on. Any expression will do.
						} else if (er.isUsable() && er.get()->isEvaluatable(Actions.Context)) {
							// We've parsed out an identifier that's going to be a named
							// Constant. Only a constant expression will do.
							
						} else {
							//er = ExprError(Diag(er.get()->getLocStart(), diag::err_expected_expression));
							ret = StmtError(Diag(er.get()->getLocStart(), diag::err_expected_expression));
						}
					} else {
						ret = StmtError(Diag(Tok, diag::err_expected_either) << tok::colon << tok::equal);
					}
				} else {
					ret = StmtError(Diag(Tok, diag::err_expected_unqualified_id) << getLangOpts().CPlusPlus);
				}
				
				T.consumeClose();
			}
			
			ret = handler(SkelLoc);
		}
	} else {
		ret = StmtError(Diag(Tok, diag::err_unexpected_at));
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
