#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <iostream>  
#include <string>  
#include <errno.h>  
#include <iconv.h>
#include <cstring>
#include <errno.h>
#include <ctime>

#include "Utils.h"

using namespace std;


string Utils::getNowTimeNs() {
	//printf("---------------------------时间输出----------------------------------------\n");
	//printf("[time(NULL)]     :     %ld\n", time(NULL));
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	//printf("clock_gettime : tv_sec=%ld, tv_nsec=%ld\n", ts.tv_sec, ts.tv_nsec);

	struct tm t;
	char date_time[64];
	strftime(date_time, sizeof(date_time), "%Y-%m-%d %H:%M:%S", localtime_r(&ts.tv_sec, &t));
	//printf("Utils::getNowTimeNs() clock_gettime : date_time=%s, tv_nsec=%ld, tv_usec=%ld, compare_str=%s\n", date_time, ts.tv_nsec, ts.tv_nsec / 1000, (":"
	//	+ std::to_string(ts.tv_nsec)).c_str());
	string s = ", ";
	return (date_time + s + std::to_string(ts.tv_sec) + s + std::to_string(ts.tv_nsec));
}

string Utils::getNowTime() {
	//printf("---------------------------时间输出----------------------------------------\n");
	//printf("[time(NULL)]     :     %ld\n", time(NULL));
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	//printf("clock_gettime : tv_sec=%ld, tv_nsec=%ld\n", ts.tv_sec, ts.tv_nsec);

	struct tm t;
	char date_time[64];
	//strftime(date_time, sizeof(date_time), "%Y%m%d %H:%M:%S", localtime_r(&ts.tv_sec, &t));
	strftime(date_time, sizeof(date_time), "%Y%m%d%X", localtime_r(&ts.tv_sec, &t));
	//strftime(date_time, sizeof(date_time), "%Y%m%d%H%M%S", localtime_r(&ts.tv_sec, &t));
	/*printf("clock_gettime : date_time=%s, tv_nsec=%ld, tv_usec=%ld, compare_str=%s\n", date_time, ts.tv_nsec, ts.tv_nsec / 1000, (":"
	+ std::to_string(ts.tv_nsec)).c_str());*/
	string str_date_time = date_time;
	return str_date_time;
}

string Utils::getNowFileTimeName() {
	//printf("---------------------------时间输出----------------------------------------\n");
	//printf("[time(NULL)]     :     %ld\n", time(NULL));
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	//printf("clock_gettime : tv_sec=%ld, tv_nsec=%ld\n", ts.tv_sec, ts.tv_nsec);

	struct tm t;
	char date_time[64];
	//strftime(date_time, sizeof(date_time), "%Y%m%d %H:%M:%S", localtime_r(&ts.tv_sec, &t));
	//strftime(date_time, sizeof(date_time), "%Y%m%d%X", localtime_r(&ts.tv_sec, &t));
	strftime(date_time, sizeof(date_time), "%Y%m%d%H%M", localtime_r(&ts.tv_sec, &t));
	/*printf("clock_gettime : date_time=%s, tv_nsec=%ld, tv_usec=%ld, compare_str=%s\n", date_time, ts.tv_nsec, ts.tv_nsec / 1000, (":"
	+ std::to_string(ts.tv_nsec)).c_str());*/
	string str_date_time = date_time;
	return str_date_time;
}

bool Utils::isOrdTime() {
	bool flag = false;
	// 精确到纳秒
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);

	if (ts.tv_sec % 2 == 0)
	{
		flag = false;
	}
	else {
		flag = true;
	}

	return flag;

#if 0

	// 精确到微秒
	struct timeval tv;
	gettimeofday(&tv, NULL);
	printf("%d, Seconds, ", tv.tv_sec); //输出秒
	printf("%ld, Micorseconds\n", (long)tv.tv_usec); //输出微秒
	

	// 偶数秒,并且微秒500000, 发单
	if ((tv.tv_sec % 2 == 0) && (tv.tv_usec == 500000))
	{
		return 11;
	}
	// 奇数秒,并且微秒100000， 撤单
	else if ((tv.tv_sec % 2 != 0) && (tv.tv_usec == 500000)){
		return 1;
	}
	else {
		return 0;
	}
#endif

}

string Utils::getYMDDate() {
	//std::cout << "Utils::getYMDDate()" << std::endl;
	time_t rawtime;
	time(&rawtime);
	char time_char[20];
	string time_str; //
	strftime(time_char, 20, "%Y%m%d", localtime(&rawtime));
	//cout << "\t当前日期 = " << time_char << endl;
	time_str = time_char;
	return time_str;
}

long long Utils::strtolonglong(string str) {
	long long result;
	istringstream is(str);
	is >> result;
	return result;
}

string Utils::longlongtostr(long long num) {
	ostringstream os;
	os << num;
	string result;
	istringstream is(os.str());
	is >> result;
	return result;
}

bool Utils::compareTradingDaySeconds(const char *compare_day, const char *today) {

	//时间格式：20161216 13:30:00

	struct tm tm_time = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	struct tm tm_time2 = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	/*std::cout << "Utils::compareTradingDaySeconds()" << std::endl;
	std::cout << "\t现在时间 = " << compare_day << std::endl;
	std::cout << "\t对比时间 = " << today << std::endl;*/

	//strptime(compare_day, "%Y%m%d%H%M%S", &tm_time); //时间格式：20161216 13:30:00
	strptime(compare_day, "%Y%m%d%X", &tm_time);//时间格式：20161216 13:30:00
	time_t l_time1 = mktime(&tm_time);

	strptime(today, "%Y%m%d%X", &tm_time2);
	time_t l_time2 = mktime(&tm_time2);


	if (l_time1 >= l_time2) {
		//USER_PRINT("日期相等");
		//std::cout << "\t对比结果 = " << true << std::endl;
		return true; // 等于当前日期
	}
	else {
		//USER_PRINT("日期偏小了");
		//std::cout << "\t对比结果 = " << false << std::endl;
		return false; // 小于当前日期
	}
}

int Utils::code_convert(const char *from_charset, const char *to_charset, char *inbuf,
	size_t inlen, char *outbuf, size_t outlen) {
	iconv_t cd;
	size_t ret;
	char **pin = &inbuf;
	char **pout = &outbuf;

	cd = iconv_open(to_charset, from_charset);
	if (cd == 0)
		return -1;
	memset(outbuf, 0, outlen);
	if ((ret = iconv(cd, pin, &inlen, pout, &outlen)) != 0)
		printf("Utils::code_convert() error errno = %d ret = %d out=%s", errno, ret, pout);
	iconv_close(cd);
	return -1;
	iconv_close(cd);
	return 0;
}

int Utils::u2g(char *inbuf, int inlen, char *outbuf, size_t outlen) {
	return code_convert("utf-8//IGNORE", "gb2312", inbuf, inlen, outbuf, outlen);
}

int Utils::g2u(char *inbuf, size_t inlen, char *outbuf, size_t outlen) {
	return code_convert("gb2312", "utf-8", inbuf, inlen, outbuf, outlen);
}