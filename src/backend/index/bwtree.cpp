//===----------------------------------------------------------------------===//
//
//                         PelotonDB
//
// bwtree.cpp
//
// Identification: src/backend/index/bwtree.cpp
//
// Copyright (c) 2015, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "backend/index/bwtree.h"

namespace peloton {
namespace index {

	// Add your function definitions here
	template<typename KeyType, typename ValueType>
	bool BWTree::exists(const KeyType &key) const {

	}

	template<typename KeyType, typename ValueType>
    BWTree::iterator BWTree::find(const KeyType &key) {

    }
    
    template<typename KeyType, typename ValueType>
    BWTree::const_iterator BWTree::find(const KeyType &key) const {

    }

    template<typename KeyType, typename ValueType>
    BWTree::size_type count(const KeyType &key) const {

    }

    template<typename KeyType, typename ValueType>
    BWTree::pair_type insert(const BWTree::pair_type &) {

    }

    template<typename KeyType, typename ValueType>
    BWTree::size_type erase(const KeyType &key){

    }

}  // End index namespace
}  // End peloton namespace
