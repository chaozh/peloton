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
		BWTree::EpochGuard(epoch);
	}

	template<typename KeyType, typename ValueType>
    BWTree::iterator BWTree::find(const KeyType &key) {
    	BWTree::EpochGuard(epoch);
    }
    
    template<typename KeyType, typename ValueType>
    BWTree::const_iterator BWTree::find(const KeyType &key) const {
    	BWTree::EpochGuard(epoch);
    }

    template<typename KeyType, typename ValueType>
    BWTree::size_type count(const KeyType &key) const {

    }

    template<typename KeyType, typename ValueType>
    BWTree::pair_type insert(const BWTree::pair_type &record) {
    	BWTree::EpochGuard(epoch);
        //start insert
        //find record page by key
        auto page = findPage(record.first);
        auto node = getNodeByPID(page->pid);
        BWTree::DeltaInsert* newNode = BWTree::DeltaInsert::initialize(page, record);
        if(!node.compare_exchange_weak(page, newNode)) {
            freeNode(newNode);
            //try again
        }else{
            //may split
            //may consolidation
        }
    }

    template<typename KeyType, typename ValueType>
    BWTree::size_type erase(const KeyType &key){
    	BWTree::EpochGuard(epoch);
    }

    template<typename KeyType, typename ValueType>
    void splitPage(const PID splitPage, const PID splitPage) {
    
    }

    template<typename KeyType, typename ValueType>
    void consolidateLeafPage(const PID page, Node* startNode) {
    
    }
    
    template<typename KeyType, typename ValueType>
    void consolidateInnerPage(const PID page, Node* startNode) {
    
    }

}  // End index namespace
}  // End peloton namespace
