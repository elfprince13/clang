#!/bin/bash
mkdir tmp
echo -e "#lang Cxx\n(translation-unit\n" > tmp/$1-tmp.rkt

"/Users/thomas/Documents/Brown/Proteins/llvm/build/Debug/bin/clang" -cc1 -ast-sexp -fdiagnostics-show-option -fcolor-diagnostics -x skeletons $1.skel >> tmp/$1-tmp.rkt
if [ $? -eq 0 ]
then
	echo "Clang succesfully parsed the file into S-Expression format"
	echo -e "\n)" >> tmp/$1-tmp.rkt

	racket tmp/$1-tmp.rkt > tmp/$1-tmp.cu
	if [ $? -eq 0 ]
	then
		echo "Racket succesfully performed skeleton expansion."
		"/Users/thomas/Documents/Brown/Proteins/llvm/build/Debug/bin/clang" tmp/$1-tmp.cu
	else
		echo "Racket failed to perform skeleton expansion.">&2
	fi
else
	echo "Could not parse $1" >&2
fi
