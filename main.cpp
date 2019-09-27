#include <iostream>
#include "lex.h"
#include "parse_select_stmt.h"
#include "parse_exception.h"
#include "sql_select_stmt.h"
#include "sql_table_ref.h"
#include <time.h>
#include <hash_map>



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



    //sql = "SELECT a FROM B WHERE \"mmp\" = (SELECT * FROM C GROUP BY mmm ORDER BY qwe)";

    //sql = "   (SELECT * FROM SALES INTERSECT SELECT * FROM SALES) INTERSECT ((SELECT * FROM SALES))";

    sql = "SELECT * FROM A CROSS JOIN B LEFT JOIN C ON m=n";
    sql = "SELECT * FROM QAZ WHERE 1 > ( (SELECT 1 UNION SELECT 1) INTERSECT SELECT 2 )";

    sql = "SELECT * FROM MY_TABLE1, MY_TABLE2, (SELECT * FROM MY_TABLE3) P LEFT OUTER JOIN MY_TABLE4 ON mm=p"
          " WHERE ID = (SELECT MA1X(ID) FROM MY_TABLE5) AND ID2 IN (SELECT * FROM MY_TABLE6)";

    clock_t start = 0, finish = 0;
    start = clock();
    for (int i = 0; i < 10000; ++i) {
        GSP::ILex *lex = GSP::make_lex(sql.c_str());
        GSP::ParseException e;
        lex->next();
        GSP::AstSelectStmt *stmt = GSP::parse_select_stmt(lex, &e);
        //for (;lex->token()->type() != GSP::END_P; lex->next());
        int qq = 0;
    }
    finish = clock();
    auto dur = finish-start;
    printf("%d", dur);
    return 0;
}