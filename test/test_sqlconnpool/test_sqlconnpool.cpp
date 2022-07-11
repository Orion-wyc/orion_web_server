/*
 * @Author       : Orion
 * @Date         : 2022-06-29
 * @copyleft Apache 2.0
 */

#include <cassert>
#include <cstdio>
#include <iostream>
#include <string>

#include "pool/sqlconnpool.h"

const char *host = "localhost";
const int sql_port = 3306;
const char *sql_user = "orion";
const char *sql_pwd = "orion";
const char *db_name = "yourdb";
const int pool_size = 1;

void TestMysqlConnection() {
  MYSQL *sql = nullptr;
  webserver::SqlConnPool::Instance()->Initialize(host, sql_port, sql_user,
                                                 sql_pwd, db_name, pool_size);

  webserver::SqlConnRAII mysql_conn(&sql, webserver::SqlConnPool::Instance());
  assert(sql);

  char order[256] = {0};
  MYSQL_FIELD *fields = nullptr;
  MYSQL_RES *res = nullptr;
  std::string name = "orion";

  /* 查询用户及密码 */
  snprintf(order, 256,
           "SELECT username, password FROM user WHERE username='%s' LIMIT 1",
           name.c_str());

  if (mysql_query(sql, order)) {
    mysql_free_result(res);
    return;
  }
  res = mysql_store_result(sql);
  fields = mysql_fetch_fields(res);
  /* 输出查询结果 */
  MYSQL_ROW row;
  size_t num_fields = mysql_num_fields(res);
  std::cout << num_fields << " records found!" << std::endl;
  while ((row = mysql_fetch_row(res))) {
    {
      unsigned long *lengths;
      lengths = mysql_fetch_lengths(res);
      for (int i = 0; i < num_fields; i++) {
        printf("[%.*s] ", (int)lengths[i], row[i] ? row[i] : "NULL");
      }
      printf("\n");
    }
  }
  mysql_free_result(res);
}

int main() {
  TestMysqlConnection();
  return 0;
}