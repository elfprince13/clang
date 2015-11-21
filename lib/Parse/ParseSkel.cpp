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


Parser::DeclGroupPtrTy Parser::ParseTopLevelSkeleton(ParsingDeclSpec *DS) {
	SourceLocation AtLoc = ConsumeToken();
	
	DeclGroupPtrTy ret = DeclGroupPtrTy();
	ExposedSkeletonDecl * skelDecl = Actions.ActOnExposedSkeleton(getCurScope(), *DS, nullptr);
	
	ParseScope BodyScope(this, Scope::FnScope|Scope::DeclScope);
	Actions.PushDeclContext(getCurScope(), skelDecl);
	
	StmtResult skelStmt = ParseSkeletonStmt(AtLoc);
	if(skelStmt.isUsable()){
		SkeletonStmt * body = (SkeletonStmt*)(skelStmt.get());
		skelDecl->setBody(body);
		skelDecl->setLocation(body->getLocStart());
		skelDecl->setDeclName(DeclarationName(body->getHeader()->getName()));
	} else {
		Diag(AtLoc, diag::err_expected_statement);
	}
	Actions.PopDeclContext();
	ret = Actions.ConvertDeclToDeclGroup(skelDecl);
	
	BodyScope.Exit();
	return ret; // ConvertDeclToDeclGroup
}

ExprResult Parser::ParseSkeletonExpr(SourceLocation AtLoc){
	ExprResult ret = ExprResult();
	SmallVector<IdentifierInfo*, 16> paramNames;
	SmallVector<SkeletonArg, 16> params;
	IdentifierInfo *is = nullptr;
	
	if (Tok.is(tok::identifier)) {
		IdentifierInfo *ii = Tok.getIdentifierInfo();
		SourceLocation SkelLoc = ConsumeToken();
		SourceLocation EndLoc = SkelLoc;
		if(Tok.is(tok::l_paren)) {
			// Skeletons may attach a name for their local block of code
			BalancedDelimiterTracker T(*this, tok::l_paren);
			T.consumeOpen();
			
			if(Tok.is(tok::identifier)) {
				is = Tok.getIdentifierInfo();
				ConsumeToken();
			} else {
				ret = ExprError(Diag(Tok, diag::err_expected_unqualified_id) << getLangOpts().CPlusPlus);
			}
			
			T.consumeClose();
		}
		
		
		for(size_t argNum = 0; Tok.is(tok::l_square); argNum++){
			BalancedDelimiterTracker T(*this, tok::l_square);
			T.consumeOpen();
			
			SkeletonArg argHere;
			switch(Tok.getKind()){
				case tok::at: {
					ConsumeToken();
					if(Tok.is(tok::identifier)) {
						IdentifierInfo *ip = Tok.getIdentifierInfo();
						ConsumeToken();
						
						argHere.data.ident = ip;
						argHere.type = ARG_IS_IDENT;
					} else {
						ret = ExprError(Diag(Tok, diag::err_expected_unqualified_id) << getLangOpts().CPlusPlus);
						argHere.type = NO_SUCH_ARG;
						argHere.data.ident = nullptr; // data is a union type, so they should all be nulled.
					}
				} break;
				case tok::equal: {
					ConsumeToken();
					ExprResult er = ParseExpression();
					if(er.isUsable()) {
						FullExprArg FullExp(Actions.MakeFullExpr(er.get(), SkelLoc));
						
						// We really need to test if this is evaluatable, and if so, do constant folding ourselves.
						
						argHere.data.expr = FullExp.get();
						argHere.type = ARG_IS_EXPR;
					} else {
						ret = ExprError(Diag(er.get()->getLocStart(), diag::err_expected_expression));
						argHere.type = NO_SUCH_ARG;
						argHere.data.expr = nullptr; // data is a union type, so they should all be nulled.
					}
				} break;
				default: {
					StmtResult sr = ParseStatement();
					if(sr.isUsable()) {
						argHere.data.stmt = sr.get();
						argHere.type = ARG_IS_STMT;
					} else {
						ret = ExprError(Diag(sr.get()->getLocStart(), diag::err_expected_statement));
						argHere.type = NO_SUCH_ARG;
						argHere.data.stmt = nullptr; // data is a union type, so they should all be nulled.
					}
				}
					
			}
			
			T.consumeClose();
			if(!ret.isInvalid()) {
				params.push_back(argHere);
			} else {
				break;
			}
		}
		EndLoc = PrevTokLocation;
		
		if (!ret.isInvalid()){
			ret = Actions.ActOnSkeletonExpr(AtLoc, SkelLoc, EndLoc, ii, is, params);
		}
	} else {
		ret = ExprError(Diag(Tok, diag::err_unexpected_at));
	}
	return ret;
}

StmtResult Parser::ParseSkeletonStmt(SourceLocation AtLoc){
	StmtResult ret = StmtResult();
	bool C99orCXX = getLangOpts().C99 || getLangOpts().CPlusPlus;
	ParseScope SkelScope(this, Scope::DeclScope | Scope::ControlScope, C99orCXX);
	
	ExprResult header = ParseSkeletonExpr(AtLoc);
	StmtResult body = StmtResult();
	//if(header.isInvalid()){
		ParseScope InnerScope(this, Scope::DeclScope, C99orCXX, Tok.is(tok::l_brace));
		body = ParseStatement();
		InnerScope.Exit();
	/*} else {
		ret = StmtError(Diag(Tok, diag::err_expected_expression));
	}*/
	
	SkelScope.Exit();
	
	if(!ret.isInvalid()){
		ret = Actions.ActOnSkeletonStmt((SkeletonExpr*)(header.get()), body.get());
	}
	
	return ret;
}
