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
	StmtResult ret = StmtResult();
	SmallVector<IdentifierInfo*, 16> paramNames;
	SmallVector<Expr*, 16> paramExprs;
	IdentifierInfo *is = nullptr;
	
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
					is = Tok.getIdentifierInfo();
					ConsumeToken();
				} else {
					ret = StmtError(Diag(Tok, diag::err_expected_unqualified_id) << getLangOpts().CPlusPlus);
				}
				
				T.consumeClose();
			}
			
			bool C99orCXX = getLangOpts().C99 || getLangOpts().CPlusPlus;
			ParseScope SkelScope(this, Scope::DeclScope | Scope::ControlScope, C99orCXX);
			
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
		
						if(er.isUsable() &&
						   (colon ||
							// We've parsed out an identifier that we're going to perform
							// macro-style substitutions on. Any expression will do.
							er.get()->isEvaluatable(Actions.Context)
							// We've parsed out an identifier that's going to be a named
							// Constant. Only a constant expression will do.
							)
						   ) {
							FullExprArg FullExp(Actions.MakeFullExpr(er.get(), SkelLoc));
							paramNames.push_back(ip);
							paramExprs.push_back(FullExp.get());
						} else {
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
			
			ParseScope InnerScope(this, Scope::DeclScope, C99orCXX, Tok.is(tok::l_brace));
			
			StmtResult body = ParseStatement();
			
			InnerScope.Exit();
			SkelScope.Exit();
			
			if (!ret.isInvalid()){
				ret = Actions.ActOnSkeletonStmt(AtLoc, SkelLoc, ii, is, paramNames, paramExprs, body.get(), handler); //handler(SkelLoc);
			}
		}
	} else {
		ret = StmtError(Diag(Tok, diag::err_unexpected_at));
	}
	return ret;
}

/*
Parser::StmtResult doParseSkeletonWithName_Loop2dAccumulate(SourceLocation kindLoc){
	return StmtResult();
}
//*/

Parser::SkeletonHandler Parser::GetHandlerForSkeleton(IdentifierInfo &skelIdent){
	/*
	// Hard-code this as a test;
	llvm::sys::DynamicLibrary::AddSymbol("doParseSkeletonWithName_Loop2dAccumulate", (void*)&doParseSkeletonWithName_Loop2dAccumulate);
	//*/
	
	std::string symName("doParseSkeletonWithName_");
	StringRef skelName = skelIdent.getName();
	symName += skelName;
	
	return (SkeletonHandler)llvm::sys::DynamicLibrary::SearchForAddressOfSymbol(symName);
}
