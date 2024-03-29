/*

Copyright (c) 2005, 2008-2010, 2013-2020, Arvid Norberg
Copyright (c) 2015, Steven Siloti
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#ifndef TEST_HPP
#define TEST_HPP

#include "ip2/address.hpp"
#include "ip2/socket.hpp"

#include <boost/config.hpp>
#include <exception>
#include <sstream>
#include <vector>
#include <cstdio> // for std::snprintf
#include <cinttypes> // for PRId64 et.al.

#include <boost/preprocessor/cat.hpp>

#include "ip2/config.hpp"

// tests are expected to even test deprecated functionality. There is no point
// in warning about deprecated use in any of the tests.
// the unreachable code warnings are disabled since the test macros may
// sometimes have conditions that are known at compile time
#include "ip2/aux_/disable_deprecation_warnings_push.hpp"

#if defined TORRENT_BUILDING_TEST_SHARED
#define EXPORT BOOST_SYMBOL_EXPORT
#elif defined TORRENT_LINK_TEST_SHARED
#define EXPORT BOOST_SYMBOL_IMPORT
#else
#define EXPORT
#endif

void EXPORT report_failure(char const* err, char const* file, int line);
int EXPORT print_failures();
int EXPORT test_counter();
void EXPORT reset_output();

using unit_test_fun_t = void (*)();

struct unit_test_t
{
	unit_test_fun_t fun;
	char const* name;
	int num_failures;
	bool run;
	FILE* output;
};

extern unit_test_t EXPORT _g_unit_tests[1024];
extern int EXPORT _g_num_unit_tests;
extern int EXPORT _g_test_failures;
extern int _g_test_idx;

#define TORRENT_TEST(test_name) \
	static void BOOST_PP_CAT(unit_test_, test_name)(); \
	static struct BOOST_PP_CAT(register_class_, test_name) { \
		BOOST_PP_CAT(register_class_, test_name) () { \
			unit_test_t& t = _g_unit_tests[_g_num_unit_tests]; \
			t.fun = &BOOST_PP_CAT(unit_test_, test_name); \
			t.name = __FILE__ "." #test_name; \
			t.num_failures = 0; \
			t.run = false; \
			t.output = nullptr; \
			_g_num_unit_tests++; \
		} \
	} BOOST_PP_CAT(_static_registrar_, test_name); \
	static void BOOST_PP_CAT(unit_test_, test_name)()

#define TEST_REPORT_AUX(x, line, file) \
	report_failure(x, line, file)

#ifdef BOOST_NO_EXCEPTIONS
#define TEST_CHECK(x) \
	do if (!(x)) { \
		TEST_REPORT_AUX("TEST_ERROR: check failed: \"" #x "\"", __FILE__, __LINE__); \
	} while (false)
#define TEST_EQUAL(x, y) \
	do if ((x) != (y)) { \
		std::stringstream s__; \
		s__ << "TEST_ERROR: equal check failed:\n" #x ": " << (x) << "\nexpected: " << (y); \
		TEST_REPORT_AUX(s__.str().c_str(), __FILE__, __LINE__); \
	} while (false)
#define TEST_NE(x, y) \
	do if ((x) == (y)) { \
		std::stringstream s__; \
		s__ << "TEST_ERROR: not equal check failed:\n" #x ": " << (x) << "\nexpected not equal to: " << (y); \
		TEST_REPORT_AUX(s__.str().c_str(), __FILE__, __LINE__); \
	} while (false)
#else
#define TEST_CHECK(x) \
	do try \
	{ \
		if (!(x)) \
			TEST_REPORT_AUX("TEST_ERROR: check failed: \"" #x "\"", __FILE__, __LINE__); \
	} \
	catch (std::exception const& e) \
	{ \
		TEST_ERROR("TEST_ERROR: Exception thrown: " #x " :" + std::string(e.what())); \
	} \
	catch (...) \
	{ \
		TEST_ERROR("TEST_ERROR: Exception thrown: " #x); \
	} while (false)

#define TEST_EQUAL(x, y) \
	do try { \
		if ((x) != (y)) { \
			std::stringstream s__; \
			s__ << "TEST_ERROR: " #x ": " << (x) << " expected: " << (y); \
			TEST_REPORT_AUX(s__.str().c_str(), __FILE__, __LINE__); \
		} \
	} \
	catch (std::exception const& e) \
	{ \
		TEST_ERROR("TEST_ERROR: Exception thrown: " #x " :" + std::string(e.what())); \
	} \
	catch (...) \
	{ \
		TEST_ERROR("TEST_ERROR: Exception thrown: " #x); \
	} while (false)
#define TEST_NE(x, y) \
	do try { \
		if ((x) == (y)) { \
			std::stringstream s__; \
			s__ << "TEST_ERROR: " #x ": " << (x) << " expected not equal to: " << (y); \
			TEST_REPORT_AUX(s__.str().c_str(), __FILE__, __LINE__); \
		} \
	} \
	catch (std::exception const& e) \
	{ \
		TEST_ERROR("TEST_ERROR: Exception thrown: " #x " :" + std::string(e.what())); \
	} \
	catch (...) \
	{ \
		TEST_ERROR("TEST_ERROR: Exception thrown: " #x); \
	} while (false)
#endif

#define TEST_ERROR(x) \
	TEST_REPORT_AUX((std::string("TEST_ERROR: \"") + (x) + "\"").c_str(), __FILE__, __LINE__)

#define TEST_NOTHROW(x) \
	do try \
	{ \
		x; \
	} \
	catch (...) \
	{ \
		TEST_ERROR("TEST_ERROR: Exception thrown: " #x); \
	} while (false)

#define TEST_THROW(x) \
	do try \
	{ \
		x; \
		TEST_ERROR("No exception thrown: " #x); \
	} \
	catch (...) {} while (false)

#endif // TEST_HPP

