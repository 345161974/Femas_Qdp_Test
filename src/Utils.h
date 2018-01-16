#ifndef QUANT_CTP_UTILS_H
#define QUANT_CTP_UTILS_H
#include <string>
#include <sstream>
#include <iostream>

using namespace std;

using std::string;

class Utils {

public:

	static string getNowTimeNs();
	static string getNowTime();
	static string getNowFileTimeName();
	static bool isOrdTime();
	static string getYMDDate();
	static long long strtolonglong(string str);
	static string longlongtostr(long long num);
	static bool compareTradingDaySeconds(const char *compare_day, const char *today);
	static int code_convert(const char *from_charset, const char *to_charset, char *inbuf,
		size_t inlen, char *outbuf, size_t outlen);
	static int u2g(char *inbuf, int inlen, char *outbuf, size_t outlen);
	static int g2u(char *inbuf, size_t inlen, char *outbuf, size_t outlen);
};

#endif