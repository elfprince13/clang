//===--- SkeletonSearchOptions.h ----------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//


#ifndef LLVM_SkeletonSearchOptions_h
#define LLVM_SkeletonSearchOptions_h

#include "clang/Basic/LLVM.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/StringRef.h"
#include <string>
#include <vector>

namespace clang {
	
	
	/// SkeletonSearchOptions - Helper class for storing options related to the
	/// initialization of the SkeletonSearch object.
	class SkeletonSearchOptions : public RefCountedBase<SkeletonSearchOptions> {
	public:
		struct Entry {
			std::string Path;
			
			/// IgnoreSysRoot - This is false if an absolute path should be treated
			/// relative to the sysroot, or true if it should always be the absolute
			/// path.
			unsigned IgnoreSysRoot : 1;
			
			Entry(StringRef path, bool ignoreSysRoot)
			: Path(path), IgnoreSysRoot(ignoreSysRoot) {}
		};
		/// If non-empty, the directory to use as a "virtual system root" for include
		/// paths.
		std::string Sysroot;
		
		/// User specified include entries.
		std::vector<Entry> UserEntries;
	public:
		SkeletonSearchOptions(StringRef _Sysroot = "/")
		: Sysroot(_Sysroot) {}
		
		/// AddPath - Add the \p Path path to the specified \p Group list.
		void AddPath(StringRef Path, bool IgnoreSysRoot) {
			UserEntries.push_back(Entry(Path, IgnoreSysRoot));
		}
		
	};
	
} // end namespace clang

#endif
