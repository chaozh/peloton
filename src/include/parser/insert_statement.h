//===----------------------------------------------------------------------===//
//
//                         Peloton
//
// statement_insert.h
//
// Identification: src/include/parser/statement_insert.h
//
// Copyright (c) 2015-16, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include "parser/sql_statement.h"
#include "common/sql_node_visitor.h"
#include "select_statement.h"

namespace peloton {
namespace parser {

/**
 * @struct InsertStatement
 * @brief Represents "INSERT INTO students VALUES ('Max', 1112233,
 * 'Musterhausen', 2.3)"
 */
struct InsertStatement : TableRefStatement {
  InsertStatement(InsertType type)
      : TableRefStatement(StatementType::INSERT),
        type(type),
        columns(NULL),
        insert_values(NULL),
        select(NULL) {}

  virtual ~InsertStatement() {

    if (columns) {
      for (auto col : *columns) free(col);
      delete columns;
    }

    if (insert_values) {
      for (auto tuple : *insert_values) {
        for( auto expr : *tuple){
          if (expr->GetExpressionType() != ExpressionType::VALUE_PARAMETER)
            delete expr;
        }
        delete tuple;
      }
      delete insert_values;
    }

    delete select;
  }


  virtual void Accept(SqlNodeVisitor* v) const override {
    v->Visit(this);
  }

  InsertType type;
  std::vector<char*>* columns;
  std::vector<std::vector<peloton::expression::AbstractExpression*>*>* insert_values;
  SelectStatement* select;
};

}  // End parser namespace
}  // End peloton namespace
