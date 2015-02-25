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
		if (handler.isValid) {
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
			
			for(int argNum = 0; Tok.is(tok::l_square); argNum++){
				BalancedDelimiterTracker T(*this, tok::l_square);
				T.consumeOpen();
				
				SkeletonArgType argType = handler.GetTypeOfNthArg(argNum);
				switch(argType){
					case ARG_IS_IDENT:
					{
						if(Tok.is(tok::identifier)) {
							IdentifierInfo *ip = Tok.getIdentifierInfo();
							ConsumeToken();
							
							////////////////////////////
							// need to do some stuff here
							////////////////////////////
							
						} else {
							ret = StmtError(Diag(Tok, diag::err_expected_unqualified_id) << getLangOpts().CPlusPlus);
						}
					}
						break;
					case ARG_IS_DECL:
					{
						StmtVector Stmts;
						ParsedAttributesWithRange Attrs(AttrFactory);
						MaybeParseCXX11Attributes(Attrs, nullptr, /*MightBeObjCMessageSend*/ true);
						SourceLocation DeclStart = Tok.getLocation(), DeclEnd;
						DeclGroupPtrTy Decl = ParseDeclaration(Stmts, Declarator::BlockContext,
															   DeclEnd, Attrs);
						DeclGroupRef dgr = Decl.get();
						if(!dgr.isNull()){
							//////////////////////
							// need to do some stuff here
							//////////////////////
						} else {
							ret = StmtError();
						}
					}
						break;
					case ARG_IS_EXPR:
					{
						ExprResult er = ParseExpression();
						if(er.isUsable()) {
							FullExprArg FullExp(Actions.MakeFullExpr(er.get(), SkelLoc));
							
							////////////////////////////
							// need to do some stuff here
							////////////////////////////
							
							
						} else {
							ret = StmtError(Diag(er.get()->getLocStart(), diag::err_expected_expression));
						}
					}
						
						break;
					case ARG_IS_STMT:
					{
						StmtResult sr = ParseStatement();
						if(sr.isUsable()) {
							
							////////////////////////////
							// need to do some stuff here
							////////////////////////////
							
							
						} else {
							ret = StmtError(Diag(sr.get()->getLocStart(), diag::err_expected_statement));
						}
					}
						break;
					case NO_SUCH_ARG:
						ret = StmtError();
						break;
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

SkeletonHandler Parser::GetHandlerForSkeleton(IdentifierInfo &skelIdent){
	/*
	// Hard-code this as a test;
	llvm::sys::DynamicLibrary::AddSymbol("doParseSkeletonWithName_Loop2dAccumulate", (void*)&doParseSkeletonWithName_Loop2dAccumulate);
	//*/
	
	SkeletonHandler ret;
	
	std::string symName("getArgTypesForSkeletonOfKind_");
	StringRef skelName = skelIdent.getName();
	symName += skelName;
	
	ret.GetTypeOfNthArg = (SkeletonArgType (*)(int))(llvm::sys::DynamicLibrary::SearchForAddressOfSymbol(symName));
	ret.isValid = (ret.GetTypeOfNthArg == nullptr);
	
	return ret;
}
