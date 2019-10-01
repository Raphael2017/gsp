#include <iostream>
#include "lex.h"
#include "parse_expression.h"
#include "parse_select_stmt.h"
#include "parse_exception.h"
#include "sql_select_stmt.h"
#include "sql_table_ref.h"
#include "sql_expression.h"
#include <time.h>

enum BOOL_CONSTANT { BC_TRUE, BC_FALSE, BC_UNKNOWN };

enum OPER { OP_OR, OP_AND, OP_GT, OP_LT };

unsigned int make_new_label() {
    static unsigned int i = 1;
    return i++;
}

struct Instruction {
    enum { PUSH_CONSTANT, PUSH_VARIABLE, EXEC_OPERATOR, JUMP, C_JUMP, LAB } _type;
    union {
        int _constant;
        char *_var_name;
        OPER _op;
        unsigned int _jump_label;
        unsigned int _lab;
        struct {
            BOOL_CONSTANT _when;
            unsigned int _c_jump_label;
        } _c_jump;
    } u;
};

Instruction *make_c_jump(BOOL_CONSTANT when, unsigned int label) {
    Instruction *ins = new Instruction;
    ins->_type = Instruction::C_JUMP;
    ins->u._c_jump._when = when;
    ins->u._c_jump._c_jump_label = label;
    return ins;
}

Instruction *make_jump(unsigned int label) {
    Instruction *ins = new Instruction;
    ins->_type = Instruction::JUMP;
    ins->u._jump_label = label;
    return ins;
}

Instruction *make_exec_op(OPER op) {
    Instruction *ins = new Instruction;
    ins->_type = Instruction::EXEC_OPERATOR;
    ins->u._op = op;
    return ins;
}

Instruction *make_lb(unsigned int label) {
    Instruction *ins = new Instruction;
    ins->_type = Instruction::LAB;
    ins->u._lab = label;
}

void translate(GSP::AstSearchCondition *condition, std::vector<Instruction*>& instructions) {
    switch (condition->GetExprType()) {
        case GSP::AstSearchCondition::OR: {
            GSP::AstBinaryOpExpr *or_expr = dynamic_cast<GSP::AstBinaryOpExpr*>(condition);
            translate(or_expr->GetLeft(), instructions);
            unsigned int lab1 = make_new_label();
            Instruction *cjmp = make_c_jump(BC_TRUE, lab1);
            instructions.push_back(cjmp);
            translate(or_expr->GetRight(), instructions);
            instructions.push_back(make_exec_op(OP_OR));
            unsigned int lab2 = make_new_label();
            instructions.push_back(make_jump(lab2));
            instructions.push_back(make_lb(lab1));
            instructions.push_back(make_lb(lab2));
        } break;
        case GSP::AstSearchCondition::AND: {
            GSP::AstBinaryOpExpr *or_expr = dynamic_cast<GSP::AstBinaryOpExpr*>(condition);
            translate(or_expr->GetLeft(), instructions);
            unsigned int lab1 = make_new_label();
            Instruction *cjmp = make_c_jump(BC_FALSE, lab1);
            instructions.push_back(cjmp);
            translate(or_expr->GetRight(), instructions);
            instructions.push_back(make_exec_op(OP_AND));
            unsigned int lab2 = make_new_label();
            instructions.push_back(make_jump(lab2));
            instructions.push_back(make_lb(lab1));
            instructions.push_back(make_lb(lab2));
        } break;
        case GSP::AstSearchCondition::COMP_GT: {} break;
        case GSP::AstSearchCondition::COMP_LT: {} break;
        default: { assert(false); } break;
    }
}

int main() {
    std::string sql = "SELECT a FROM (A JOIN B ON m=n), (SELECT m FROM PP) QQ";
    sql = "SELECT a FROM (((SELECT m FROM AA)) UNION SELECT kp) CC";
    //sql = "SELECT a FROM L JOIN L1 ON q=e JOIN L2 ON e=r";
    sql = "SELECT L.* FROM L JOIN L1 JOIN L2 ON q=e ON e=r";
    sql = "select * from\n"
          "(select * from test01.sa.CUSTOMER where c_privilege_level=1) a\n"
          "cross join\n"
          "(select * from test01.sa.CUSTOMER_COPY where c_privilege_level<0) b\n"
          "where a.c_mktsegment =b.c_mktsegment";

    sql = "WITH QQ(m,n) AS (SELECT 1, 1+2) SELECT * FROM QQ GROUP BY ALL m HAVING (SELECT 1) > -1 ORDER BY n ASC, m DESC";

    sql = "SELECT * FROM A NATURAL JOIN B JOIN (SELECT 1) C ON m=n";
    sql = "SELECT * FROM A NATURAL JOIN ((B NATURAL JOIN C))";

    sql = "WITH QQ(m,n) AS (SELECT 1, 1+2), MQ(m,n) AS (SELECT 2,2+3) SELECT * FROM QQ GROUP BY ALL m HAVING (SELECT 1) > -1 ORDER BY n ASC, m DESC";

    sql = "SELECT a1.from_resource_name, a1.allow_count, d1.deny_count, a1.allow_count + d1.deny_count AS total_count\n"
          "FROM (\n"
          "  SELECT rp1.from_resource_name AS from_resource_name, ra1.allow_decision AS decision, ISNULL(ra1.allow_count, 0) AS allow_count\n"
          "  FROM (\n"
          "    SELECT ra.from_resource_name AS from_resource_name, ra.policy_decision AS allow_decision, COU1NT(ra.from_resource_name) AS allow_count\n"
          "    FROM RPA_LOG ra\n"
          "    WHERE policy_decision = 'A'\n"
          "      AND day_nb >= ?\n"
          "      AND day_nb <= ?\n"
          "    GROUP BY ra.from_resource_name, ra.policy_decision\n"
          "  ) ra1\n"
          "    RIGHT JOIN (\n"
          "      SELECT rp.from_resource_name AS from_resource_name, 'A' AS deny_decision\n"
          "      FROM RPA_LOG rp\n"
          "      WHERE day_nb >= ?\n"
          "        AND day_nb <= ?\n"
          "      GROUP BY rp.from_resource_name\n"
          "    ) rp1\n"
          "    ON rp1.from_resource_name = ra1.from_resource_name\n"
          ") a1\n"
          "  INNER JOIN (\n"
          "    SELECT rp1.from_resource_name AS from_resource_name, rp1.deny_decision AS decision, ISNULL(rd1.deny_count, 0) AS deny_count\n"
          "    FROM (\n"
          "      SELECT rd.from_resource_name AS from_resource_name, rd.policy_decision AS deny_decision, COU1NT(rd.from_resource_name) AS deny_count\n"
          "      FROM RPA_LOG rd\n"
          "      WHERE policy_decision = 'D'\n"
          "        AND day_nb >= ?\n"
          "        AND day_nb <= ?\n"
          "      GROUP BY rd.from_resource_name, rd.policy_decision\n"
          "    ) rd1\n"
          "      RIGHT JOIN (\n"
          "        SELECT rp.from_resource_name AS from_resource_name, 'D' AS deny_decision\n"
          "        FROM RPA_LOG rp\n"
          "        WHERE day_nb >= ?\n"
          "          AND day_nb <= ?\n"
          "        GROUP BY rp.from_resource_name\n"
          "      ) rp1\n"
          "      ON rp1.from_resource_name = rd1.from_resource_name\n"
          "  ) d1\n"
          "  ON a1.from_resource_name = d1.from_resource_name\n"
          "ORDER BY total_count DESC;\n"
          "";

    //sql = "SELECT * FROM table1 WHERE NOT EXISTS (SELECT * FROM table2 WHERE table1.field1=table2.field1);";
    //sql = "SELECT (1+2)*3 FROM MN WHERE (SELECT 1 FROM P) > 0";

    //sql = "SELECT a FROM B WHERE \"mmp\" = (SELECT * FROM C GROUP BY mmm ORDER BY qwe)";

    //sql = "   (SELECT * FROM SALES INTERSECT SELECT * FROM SALES) INTERSECT ((SELECT * FROM SALES))";

    //sql = "SELECT * FROM A CROSS JOIN B LEFT JOIN C ON m=n";
    //sql = "SELECT * FROM QAZ WHERE 1 > ( (SELECT 1 UNION SELECT 1) INTERSECT SELECT 2 )";

   // sql = "SELECT * FROM MY_TABLE1, MY_TABLE2, (SELECT * FROM MY_TABLE3) P LEFT OUTER JOIN MY_TABLE4 ON mm=p"
   //       " WHERE ID = (SELECT MA1X(ID) FROM MY_TABLE5) AND ID2 IN (SELECT * FROM MY_TABLE6)";

   sql = "SELECT CNTRYCODE, CO1UNT(*) AS NUMCUST, S1UM(C_ACCTBAL) AS TOTACCTBAL\n"
         "FROM (SELECT SUBSTRING(C_PHONE,1,2) AS CNTRYCODE, C_ACCTBAL\n"
         " FROM CUSTOMER WHERE SUBSTRING(C_PHONE,1,2) IN ('13', '31', '23', '29', '30', '18', '17') AND\n"
         " C_ACCTBAL > (SELECT AV1G(C_ACCTBAL) FROM CUSTOMER WHERE C_ACCTBAL > 0.00 AND\n"
         "  SUBSTRING(C_PHONE,1,2) IN ('13', '31', '23', '29', '30', '18', '17')) AND\n"
         " NOT EXISTS ( SELECT * FROM ORDERS WHERE O_CUSTKEY = C_CUSTKEY)) AS CUSTSALE\n"
         "GROUP BY CNTRYCODE\n"
         "ORDER BY CNTRYCODE;";
   
    {
        std::string condition = "M>3 OR (M<-1 AND N>3)";
        GSP::ILex *lex = GSP::make_lex(condition.c_str());
        lex->next();
        GSP::ParseException e;
        GSP::AstSearchCondition *search_condition = GSP::parse_search_condition(lex, &e);






        delete (lex);
        delete (search_condition);
    }


    clock_t start = clock();
    for (int i = 0; i < 10000; ++i) {
        GSP::ILex *lex = GSP::make_lex(sql.c_str());
        GSP::ParseException e;
        lex->next();
        GSP::AstSelectStmt *stmt = GSP::parse_select_stmt(lex, &e);
        delete (lex);
        delete (stmt);
    }
    printf("total: %d\n", clock() - start);
    return 0;
}