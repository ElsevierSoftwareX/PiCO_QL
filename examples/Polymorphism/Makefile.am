bin_PROGRAMS = polymorphism

bindata_DATA = polymorphism_test_success.txt
bindatadir = $(bindir)

AM_CFLAGS = -I../../src
AM_CXXFLAGS = -I../../src

if ENABLE_TESTS
AM_CFLAGS += -O3 -Wall -Werror -g -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -D_GLIBCXX_DEBUG -D_GLIBCXX_CONCEPT_CHECKS -D_GLIBCXX_FULLY_DYNAMIC_STRING -DPICO_QL_TEST
else
AM_CFLAGS += -D_NDEBUG -O2
endif

if ENABLE_TESTS
AM_CXXFLAGS += -O3 -Wall -Werror -g -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -D_GLIBCXX_DEBUG -D_GLIBCXX_CONCEPT_CHECKS -D_GLIBCXX_FULLY_DYNAMIC_STRING -DPICO_QL_TEST
else
AM_CXXFLAGS += -D_NDEBUG -O2
endif

AM_CXXFLAGS += -DPICO_QL_SINGLE_THREADED -DPICO_QL_HANDLE_TEXT_ARRAY -DPICO_QL_HANDLE_POLYMORPHISM

polymorphism_SOURCES = pico_ql_search.cpp pico_ql_internal.cpp pico_ql_test.c main.cpp Account.cpp PremiumAccount.cpp SavingsAccount.cpp

pico_ql_search.cpp:
	cp ../../src/pico_ql_generator.rb .
	cp -R ../../src/pico_ql_erb_templates .
	ruby pico_ql_generator.rb pico_ql_dsl.sql

pico_ql_internal.cpp: pico_ql_search.cpp

polymorphism_LDADD = ../../lib/libpicoql.la -lpthread
