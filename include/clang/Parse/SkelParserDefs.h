//
//  SkelParserDefs.h
//  LLVM
//
//  Created by Thomas Dickerson on 11/3/14.
//
//

#ifndef LLVM_SkelParserDefs_h
#define LLVM_SkelParserDefs_h

typedef enum {
	ARG_IS_DECL,
	ARG_IS_IDENT,
	ARG_IS_EXPR,
	ARG_IS_STMT,
	NO_SUCH_ARG
} SkeletonArgType;
typedef struct _SkeletonHandler {
	bool isValid;
	SkeletonArgType (*GetTypeOfNthArg)(int n);
	_SkeletonHandler(SkeletonArgType (*NthArgHandler)(int n) = nullptr, bool valid = false) : isValid(valid), GetTypeOfNthArg(NthArgHandler) {}
} SkeletonHandler;

#endif
