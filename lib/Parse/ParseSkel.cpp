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
using namespace clang;


Parser::DeclGroupPtrTy Parser::ParseTopLevelSkeleton() {
	return DeclGroupPtrTy();
}



StmtResult Parser::ParseSkeleton(SourceLocation AtLoc){
	StmtResult ret = StmtResult();
	SmallVector<IdentifierInfo*, 16> paramNames;
	SmallVector<SkeletonStmt::SkeletonArg, 16> params;
	IdentifierInfo *is = nullptr;
	
	if (Tok.is(tok::identifier)) {
		IdentifierInfo *ii = Tok.getIdentifierInfo();
		SourceLocation SkelLoc = ConsumeToken();
		SkeletonHandler handler = SkeletonStmt::GetHandlerForSkeleton(*ii);
		if (handler.isValid) {
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
			
			for(size_t argNum = 0; Tok.is(tok::l_square); argNum++){
				BalancedDelimiterTracker T(*this, tok::l_square);
				T.consumeOpen();
				
				SkeletonStmt::SkeletonArg argHere;
				argHere.type = handler.GetTypeOfNthArg(argNum);
				switch(argHere.type){
					case ARG_IS_IDENT:
					{
						if(Tok.is(tok::identifier)) {
							IdentifierInfo *ip = Tok.getIdentifierInfo();
							ConsumeToken();
							
							argHere.data.ident = ip;
						} else {
							ret = StmtError(Diag(Tok, diag::err_expected_unqualified_id) << getLangOpts().CPlusPlus);
						}
					}
						break;
					case ARG_IS_EXPR:
					{
						ExprResult er = ParseExpression();
						if(er.isUsable()) {
							FullExprArg FullExp(Actions.MakeFullExpr(er.get(), SkelLoc));
							
							argHere.data.expr = FullExp.get();
						} else {
							ret = StmtError(Diag(er.get()->getLocStart(), diag::err_expected_expression));
						}
					}
						
						break;
					case ARG_IS_STMT:
					{
						StmtResult sr = ParseStatement();
						if(sr.isUsable()) {
							argHere.data.stmt = sr.get();
							
						} else {
							ret = StmtError(Diag(sr.get()->getLocStart(), diag::err_expected_statement));
						}
					}
						break;
					case NO_SUCH_ARG:
						ret = StmtError();
						break;
				}
				if(!ret.isInvalid()) {
					params.push_back(argHere);
				}
				
				T.consumeClose();
			}
			
			ParseScope InnerScope(this, Scope::DeclScope, C99orCXX, Tok.is(tok::l_brace));
			
			StmtResult body = ParseStatement();
			
			InnerScope.Exit();
			SkelScope.Exit();
			
			if (!ret.isInvalid()){
				ret = Actions.ActOnSkeletonStmt(AtLoc, SkelLoc, ii, is, paramNames, params, body.get(), handler); //handler(SkelLoc);
			}
		} else {
			ret = StmtError(Diag(SkelLoc, diag::err_undeclared_var_use) << ii);
		}
	} else {
		ret = StmtError(Diag(Tok, diag::err_unexpected_at));
	}
	return ret;
}
