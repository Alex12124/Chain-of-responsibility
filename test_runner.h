#pragma once
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <iostream>
#include <vector>

template <typename Collection>
std::string Join(const Collection& c, char d);

template <typename First, typename Second>
std::ostream& operator << (std::ostream& out, const std::pair<First, Second>& p);

template <typename T>
std::ostream& operator << (std::ostream& out, const std::set<T>& vi);

template <typename Key, typename Value>
std::ostream& operator << (std::ostream& out, const std::map<Key, Value>& m);

template<typename T, typename U>
void AssertEqual(const T& t, const U& u, const std::string& hint);

void Assert(bool b , const std::string& hint);

class TestRunner{
public:
	template<typename Func>
	void RunTest(const Func& func, const std::string& func_name);
	~TestRunner();
private:
	int count_fails = 0;
};

template <typename Collection>
std::string Join(const Collection& c, char d) {
	  std::stringstream ss;
	  bool first = true;
	  for (const auto& i : c) {
		if (!first) {
		  ss << d;
		}
		first = false;
		ss << i;
	  }
	  return ss.str();
}

template <typename First, typename Second>
std::ostream& operator << (std::ostream& out, const std::pair<First, Second>& p) {
	return out << p.first << ' ' << p.second;
}
template <typename T>
std::ostream& operator << (std::ostream& out, const std::set<T>& vi) {
  return out << '[' << Join(vi, ' ') << ']';
}

template <typename T>
std::ostream& operator << (std::ostream& out, const std::vector<T>& vi) {
  return out << '[' << Join(vi, ',') << ']';
}

template <typename Key, typename Value>
std::ostream& operator << (std::ostream& out, const std::map<Key, Value>& m) {
	return out << '{' << Join(m, ',') << '}';
}
template<typename T, typename U>
void AssertEqual(const T& t, const U& u, const std::string& hint){
	if(t != u){
		std::ostringstream os;
		os << "Assertion failed:" << t << " != " << u << " Hint: " << hint;
		throw std::runtime_error(os.str());
	}
}

template<typename Func>
void TestRunner::RunTest(const Func& func, const std::string& func_name){
	try{
		func();
		std::cerr << func_name << " is OK" << std::endl;
	}
	catch(std::runtime_error& e){
		std::cerr << func_name << " is failed: " << e.what() << std::endl;
		count_fails++;
	}
}

#define RUN_TEST(tr, func) tr.RunTest(func, #func)
#define ASSERT_EQUAL(x, y) {  		                                             \
		std::ostringstream assert_os; 			                                 \
		assert_os << #x << " != " << #y << ", " <<__FILE__ << ":" << __LINE__;   \
		AssertEqual(x, y, assert_os.str());                                      \
}

#define ASSERT(x) {  		                                                     \
		std::ostringstream assert_os; 	 			                             \
		assert_os << #x << " is false, " <<__FILE__ << ":" << __LINE__;          \
		Assert(x, assert_os.str());                                              \
}
