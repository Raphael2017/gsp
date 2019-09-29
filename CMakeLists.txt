cmake_minimum_required(VERSION 3.13)
project(gsp)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -pedantic ${CMAKE_CXX_FLAGS}")


add_executable(gsp main.cpp
        lex.cpp
        parse_select_stmt.cpp
        parse_tableref.cpp
        parse_expression.cpp
        parse_exception.cpp
        sql_select_stmt.cpp
        sql_table_ref.cpp)