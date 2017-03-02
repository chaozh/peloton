//===----------------------------------------------------------------------===//
//
//                         Peloton
//
// statement_delete.h
//
// Identification: src/include/parser/statement_delete.h
//
// Copyright (c) 2015-16, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include "common/logger.h"
#include "common/sql_node_visitor.h"
#include "parser/sql_statement.h"

namespace peloton {
namespace parser {

/**
 * @struct DeleteStatement
 * @brief Represents "DELETE FROM students WHERE grade > 3.0"
 *
 * If expr == NULL => delete all rows (truncate)
 */
struct DeleteStatement : TableRefStatement {
  DeleteStatement()
      : TableRefStatement(StatementType::DELETE), expr(NULL) {};

  virtual ~DeleteStatement() {
    delete expr;
  }

  virtual void Accept(SqlNodeVisitor* v) const override {
    v->Visit(this);
  }

  expression::AbstractExpression* expr = nullptr;
};

}  // End parser namespace
}  // End peloton namespace
