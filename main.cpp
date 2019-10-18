#include <iostream>
#include "lex.h"
#include "parse_expression.h"
#include "parse_select_stmt.h"
#include "parse_exception.h"
#include "sql_select_stmt.h"
#include "sql_table_ref.h"
#include "sql_expression.h"
#include <time.h>
#include <map>
#include <stack>
#include "relational_algebra.h"
#include "translate.h"

enum BOOL_CONSTANT { BC_TRUE = 1, BC_FALSE=0, BC_UNKNOWN=-1 };

enum OPER { OP_OR, OP_AND, OP_GT, OP_LT };

const char* op2str(OPER op) {
    switch (op) {
        case OP_OR: return "OR";
        case OP_AND: return "AND";
        case OP_GT: return ">";
        case OP_LT: return "<";
    }
}

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

void dump(const std::vector<Instruction*>& instructions) {
    for (auto it : instructions) {
        switch (it->_type) {
            case Instruction::PUSH_CONSTANT: {
                printf("     PUSH_CONSTANT %d\n", it->u._constant);
            } break;
            case Instruction::PUSH_VARIABLE: {
                printf("     PUSH VARIABLE %s\n", it->u._var_name);
            } break;
            case Instruction::EXEC_OPERATOR: {
                printf("     EXEC_OPERATOR %s\n", op2str(it->u._op));
            } break;
            case Instruction::JUMP: {
                printf("     JUMP      LAB%d\n", it->u._jump_label);
            } break;
            case Instruction::C_JUMP: {
                printf("     CJUMP  %d LAB%d\n", it->u._c_jump._when, it->u._c_jump._c_jump_label);
            } break;
            case Instruction::LAB: {
                printf("LAB%d\n", it->u._lab);
            } break;
            default: assert(false);
        }
    }
}

Instruction *make_push_var(const char *var) {
    Instruction *ins = new Instruction;
    ins->_type = Instruction::PUSH_VARIABLE;
    ins->u._var_name = strdup(var);
    return ins;
}

Instruction *make_push_int(int d) {
    Instruction *ins = new Instruction;
    ins->_type = Instruction::PUSH_CONSTANT;
    ins->u._constant = d;
    return ins;
}

Instruction *make_exec_op(OPER op) {
    Instruction *ins = new Instruction;
    ins->_type = Instruction::EXEC_OPERATOR;
    ins->u._op = op;
    return ins;
}

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


Instruction *make_lb(unsigned int label) {
    Instruction *ins = new Instruction;
    ins->_type = Instruction::LAB;
    ins->u._lab = label;
    return ins;
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
            //unsigned int lab2 = make_new_label();
            //instructions.push_back(make_jump(lab2));
            instructions.push_back(make_lb(lab1));
            //instructions.push_back(make_lb(lab2));
        } break;
        case GSP::AstSearchCondition::AND: {
            GSP::AstBinaryOpExpr *or_expr = dynamic_cast<GSP::AstBinaryOpExpr*>(condition);
            translate(or_expr->GetLeft(), instructions);
            unsigned int lab1 = make_new_label();
            Instruction *cjmp = make_c_jump(BC_FALSE, lab1);
            instructions.push_back(cjmp);
            translate(or_expr->GetRight(), instructions);
            instructions.push_back(make_exec_op(OP_AND));
            //unsigned int lab2 = make_new_label();
            //instructions.push_back(make_jump(lab2));
            instructions.push_back(make_lb(lab1));
            //instructions.push_back(make_lb(lab2));
        } break;
        case GSP::AstSearchCondition::COMP_GT: {
            GSP::AstBinaryOpExpr *gt_expr = dynamic_cast<GSP::AstBinaryOpExpr*>(condition);
            translate(gt_expr->GetLeft(), instructions);
            translate(gt_expr->GetRight(), instructions);
            instructions.push_back(make_exec_op(OP_GT));
        } break;
        case GSP::AstSearchCondition::COMP_LT: {
            GSP::AstBinaryOpExpr *gt_expr = dynamic_cast<GSP::AstBinaryOpExpr*>(condition);
            translate(gt_expr->GetLeft(), instructions);
            translate(gt_expr->GetRight(), instructions);
            instructions.push_back(make_exec_op(OP_LT));
        } break;
        case GSP::AstSearchCondition::EXPR_COLUMN_REF: {
            const GSP::AstIds& col = dynamic_cast<GSP::AstColumnRef*>(condition)->GetColumn();
            GSP::AstId *id = col[0];
            instructions.push_back(make_push_var(id->GetId().c_str()));
        } break;
        case GSP::AstSearchCondition::C_NUMBER: {
            GSP::AstConstantValue *value = dynamic_cast<GSP::AstConstantValue*>(condition);
            instructions.push_back(make_push_int(value->GetValueAsInt()));
        } break;
        default: { assert(false); } break;
    }
}

void link(std::vector<Instruction*>& instructions) {
    std::map<int, int> labs; // labid, lineid
    for (int i = instructions.size()-1; i > -1; --i) {
        if (instructions[i]->_type == Instruction::LAB) labs[instructions[i]->u._lab] = i;
        if (instructions[i]->_type == Instruction::C_JUMP) {
            auto fd = labs.find(instructions[i]->u._c_jump._c_jump_label);
            assert(fd != labs.end());
            instructions[i]->u._c_jump._c_jump_label = fd->second;
        }
    }
}
const std::string M = "M";
const std::string N = "N";

#define M_V -5
#define N_V 10

#define MN_CND "M>3 OR (M<0 AND N>3) AND (N < 100 OR M > 10 OR M > 109 AND N < 231) "

int value(std::vector<Instruction*>& instructions) {
    std::stack<int> stk;

    for (int i = 0; i < instructions.size();) {
        Instruction *it = instructions[i];
        switch (it->_type) {
            case Instruction::LAB: { ++i; } break;
            case Instruction::PUSH_VARIABLE: {

                if (it->u._var_name == M)
                    stk.push(M_V);
                else if (it->u._var_name == N)
                    stk.push(N_V);
                ++i;
            } break;
            case Instruction::PUSH_CONSTANT: {
                stk.push(it->u._constant);
                ++i;
            } break;
            case Instruction::C_JUMP: {
                int top = stk.top();
                if (top == it->u._c_jump._when) {
                    i = it->u._c_jump._c_jump_label;
                }
                else
                    ++i;
            } break;
            case Instruction::EXEC_OPERATOR: {
                int right = stk.top(); stk.pop();
                int left = stk.top(); stk.pop();
                switch (it->u._op) {
                    case OP_LT: {
                        if (left == BC_UNKNOWN) {
                            stk.push(BC_UNKNOWN);
                            ++i;
                            continue;
                        }
                        stk.push(left < right ? BC_TRUE : BC_FALSE);
                        ++i;
                    } break;
                    case OP_GT: {
                        if (left == BC_UNKNOWN) {
                            stk.push(BC_UNKNOWN);
                            ++i;
                            continue;
                        }
                        stk.push(left > right ? BC_TRUE : BC_FALSE);
                        ++i;
                    } break;
                    case OP_AND: {
                        if (left == BC_FALSE || right == BC_FALSE) {
                            stk.push(BC_FALSE);
                            ++i;
                            continue;
                        } else if (left == BC_UNKNOWN || right == BC_UNKNOWN) {
                            stk.push(BC_UNKNOWN);
                            ++i;
                            continue;
                        } else {
                            stk.push(BC_TRUE);
                            ++i;
                            continue;
                        }
                    } break;
                    case OP_OR: {
                        if (left == BC_TRUE || right == BC_TRUE) {
                            stk.push(BC_TRUE);
                            ++i;
                            continue;
                        } else if (left == BC_UNKNOWN || right == BC_UNKNOWN) {
                            stk.push(BC_UNKNOWN);
                            ++i;
                            continue;
                        } else {
                            stk.push(BC_FALSE);
                            ++i;
                            continue;
                        }
                    }
                }

            }
        }
    }
    assert(stk.size() == 1);
    return stk.top();
}

int value1(GSP::AstSearchCondition *condition) {
    switch (condition->GetExprType()) {
        case GSP::AstSearchCondition::OR: {
            auto or_exp = dynamic_cast<GSP::AstBinaryOpExpr *>(condition);
            int left = value1(or_exp->GetLeft());
            if (left == BC_TRUE) return BC_TRUE;
            int right = value1(or_exp->GetRight());
            if (right == BC_TRUE) return BC_TRUE;
            if (left == BC_UNKNOWN || right == BC_UNKNOWN) return BC_UNKNOWN;
            return false;
        }
            break;
        case GSP::AstSearchCondition::AND: {
            auto and_exp = dynamic_cast<GSP::AstBinaryOpExpr *>(condition);
            int left = value1(and_exp->GetLeft());
            if (left == BC_FALSE) return BC_FALSE;
            int right = value1(and_exp->GetRight());
            if (right == BC_FALSE) return BC_FALSE;
            if (left == BC_UNKNOWN || right == BC_UNKNOWN) return BC_UNKNOWN;
            return true;
        }
            break;
        case GSP::AstSearchCondition::COMP_GT: {
            auto predicate = dynamic_cast<GSP::AstBinaryOpExpr *>(condition);
            int left = value1(predicate->GetLeft());
            if (left == BC_UNKNOWN) return BC_UNKNOWN;
            int right = value1(predicate->GetRight());
            if (left > right) return BC_TRUE;
            return BC_FALSE;
        }
            break;
        case GSP::AstSearchCondition::COMP_LT: {
            auto predicate = dynamic_cast<GSP::AstBinaryOpExpr *>(condition);
            int left = value1(predicate->GetLeft());
            if (left == BC_UNKNOWN) return BC_UNKNOWN;
            int right = value1(predicate->GetRight());
            if (left < right) return BC_TRUE;
            return BC_FALSE;
        }
            break;
        case GSP::AstSearchCondition::EXPR_COLUMN_REF: {
            const GSP::AstIds &col = dynamic_cast<GSP::AstColumnRef *>(condition)->GetColumn();
            GSP::AstId *id = col[0];
            if (id->GetId() == "M") return M_V;
            else if (id->GetId() == "N") return N_V;
        }
            break;
        case GSP::AstSearchCondition::C_NUMBER: {
            return dynamic_cast<GSP::AstConstantValue *>(condition)->GetValueAsInt();
        }
            break;
        default: {
            assert(false);
        }
            break;
    }
}

void dump(GSP::AstSearchCondition *condition, int lvl = 0) {
    for (int i = 0; i < lvl; ++i)
        printf("   ");
    switch (condition->GetExprType()) {
        case GSP::AstSearchCondition::COMP_EQ:
        case GSP::AstSearchCondition::OR:
        case GSP::AstSearchCondition::AND:
        case GSP::AstSearchCondition::COMP_GT:
        case GSP::AstSearchCondition::COMP_LT: {
            switch (condition->GetExprType()){
                case GSP::AstSearchCondition::COMP_EQ: printf("|-EQ\n"); break;
                case GSP::AstSearchCondition::OR: printf("|-OR\n"); break;
                case GSP::AstSearchCondition::AND: printf("|-AND\n"); break;
                case GSP::AstSearchCondition::COMP_GT: printf("|-GT\n"); break;
                case GSP::AstSearchCondition::COMP_LT: printf("|-LT\n"); break;
            }
            dump(dynamic_cast<GSP::AstBinaryOpExpr*>(condition)->GetLeft(), lvl+1);
            dump(dynamic_cast<GSP::AstBinaryOpExpr*>(condition)->GetRight(), lvl+1);
        }
            break;
        case GSP::AstSearchCondition::EXPR_COLUMN_REF: {
            const GSP::AstIds &col = dynamic_cast<GSP::AstColumnRef *>(condition)->GetColumn();
            std::string id;
            for (int i = 0; i < col.size(); ++i) {
                if (i == 0) {
                    id += col[i]->GetId();
                } else {
                    id += "." + col[i]->GetId();
                }
            }
            printf("|-%s\n", id.c_str());
        }
            break;
        case GSP::AstSearchCondition::C_NUMBER: {
            printf("|-%d\n", dynamic_cast<GSP::AstConstantValue *>(condition)->GetValueAsInt());
        }
            break;
        case GSP::AstSearchCondition::C_STRING: {
            printf("|-%s\n", dynamic_cast<GSP::AstConstantValue *>(condition)->GetValue());
        } break;
        default: {
            assert(false);
        }
            break;
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

   /*
   sql = "SELECT CNTRYCODE, CO1UNT(*) AS NUMCUST, S1UM(C_ACCTBAL) AS TOTACCTBAL\n"
         "FROM (SELECT SUBSTRING(C_PHONE,1,2) AS CNTRYCODE, C_ACCTBAL\n"
         " FROM CUSTOMER WHERE SUBSTRING(C_PHONE,1,2) IN ('13', '31', '23', '29', '30', '18', '17') AND\n"
         " C_ACCTBAL > (SELECT AV1G(C_ACCTBAL) FROM CUSTOMER WHERE C_ACCTBAL > 0.00 AND\n"
         "  SUBSTRING(C_PHONE,1,2) IN ('13', '31', '23', '29', '30', '18', '17')) AND\n"
         " NOT EXISTS ( SELECT * FROM ORDERS WHERE O_CUSTKEY = C_CUSTKEY)) AS CUSTSALE\n"
         "GROUP BY CNTRYCODE\n"
         "ORDER BY CNTRYCODE;";*/
   
    {



        std::string condition = MN_CND;

        clock_t start = clock();
        GSP::ILex *lex = GSP::make_lex(condition.c_str());
        lex->next();
        GSP::ParseException e;
        GSP::AstSearchCondition *search_condition = GSP::parse_search_condition(lex, &e);
        //printf("total: %d\n", clock() - start);
        dump(search_condition);
        std::vector<Instruction*> instructions;
        translate(search_condition, instructions);
        dump(instructions);
        link(instructions);


        //assert(1==0);
        assert(value1(search_condition) == value(instructions));

#if 0
        start = clock();
        for (int i = 0; i < 100000; ++i) {
            int v1 = value1(search_condition);
            //int v = value(instructions);
            //assert(v == v1);
        }
        printf("total1: %d\n", clock() - start);
#else
        start = clock();
        for (int i = 0; i < 10; ++i) {
            //int v1 = value1(search_condition);
            int v = value(instructions);
            //assert(v == v1);
        }
        printf("total: %d\n", clock() - start);
#endif



        delete (lex);
        delete (search_condition);
        //return 0;
    }

    sql = "select cou1nt(*) \n"
          "from ((select distinct c_last_name, c_first_name, d_date\n"
          "       from store_sales, date_dim, customer\n"
          "       where store_sales.ss_sold_date_sk = date_dim.d_date_sk\n"
          "         and store_sales.ss_customer_sk = customer.c_customer_sk\n"
          "         and d_month_seq between 1202 and 1202+11)\n"
          "       except\n"
          "      (select distinct c_last_name, c_first_name, d_date\n"
          "       from catalog_sales, date_dim, customer\n"
          "       where catalog_sales.cs_sold_date_sk = date_dim.d_date_sk\n"
          "         and catalog_sales.cs_bill_customer_sk = customer.c_customer_sk\n"
          "         and d_month_seq between 1202 and 1202+11)\n"
          "       except\n"
          "      (select distinct c_last_name, c_first_name, d_date\n"
          "       from web_sales, date_dim, customer\n"
          "       where web_sales.ws_sold_date_sk = date_dim.d_date_sk\n"
          "         and web_sales.ws_bill_customer_sk = customer.c_customer_sk\n"
          "         and d_month_seq between 1202 and 1202+11)\n"
          ") cool_cust";

    sql = "select i_brand_id brand_id, i_brand brand,t_hour,t_minute,\n"
          " \tsu1m(ext_price) ext_price\n"
          " from item, (select ws_ext_sales_price as ext_price, \n"
          "                        ws_sold_date_sk as sold_date_sk,\n"
          "                        ws_item_sk as sold_item_sk,\n"
          "                        ws_sold_time_sk as time_sk  \n"
          "                 from web_sales,date_dim\n"
          "                 where d_date_sk = ws_sold_date_sk\n"
          "                   and d_moy=12\n"
          "                   and d_year=2002\n"
          "                 union all\n"
          "                 select cs_ext_sales_price as ext_price,\n"
          "                        cs_sold_date_sk as sold_date_sk,\n"
          "                        cs_item_sk as sold_item_sk,\n"
          "                        cs_sold_time_sk as time_sk\n"
          "                 from catalog_sales,date_dim\n"
          "                 where d_date_sk = cs_sold_date_sk\n"
          "                   and d_moy=12\n"
          "                   and d_year=2002\n"
          "                 union all\n"
          "                 select ss_ext_sales_price as ext_price,\n"
          "                        ss_sold_date_sk as sold_date_sk,\n"
          "                        ss_item_sk as sold_item_sk,\n"
          "                        ss_sold_time_sk as time_sk\n"
          "                 from store_sales,date_dim\n"
          "                 where d_date_sk = ss_sold_date_sk\n"
          "                   and d_moy=12\n"
          "                   and d_year=2002\n"
          "                 ) tmp,time_dim\n"
          " where\n"
          "   sold_item_sk = i_item_sk\n"
          "   and i_manager_id=1\n"
          "   and time_sk = t_time_sk\n"
          "   and (t_meal_time = 'breakfast' or t_meal_time = 'dinner')\n"
          " group by i_brand, i_brand_id,t_hour,t_minute\n"
          " order by ext_price desc, i_brand_id\n"
          " ;";
    sql = "SELECT 5-1 FROM dummy";
    clock_t start = clock();
    for (int i = 0; i < 1; ++i) {
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