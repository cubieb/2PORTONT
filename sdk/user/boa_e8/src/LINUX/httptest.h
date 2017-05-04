#ifndef LINUX_HTTPTEST_H
#define LINUX_HTTPTEST_H

#define MAX_HEADER_LEN 256
#define MAX_TIME_LEN 64

#define MAX_DOWNLOAD_THREAD 6
#define MAX_URL_PER_THREAD  5
#define MAX_URL_NUM (MAX_DOWNLOAD_THREAD * MAX_URL_PER_THREAD)
#define DOWNLOAD_PATH  "/var"
#define HTPP_DOWNLOAD_RESULT_FILE "/var/httpdownloadtest"

struct downloadCfg{
	char user[64];
	char passwd[64];
	int num;
	char url[MAX_URL_PER_THREAD][MAX_URL_LEN];
};

typedef struct http_test_entry {
	char id[64];
	int  isHttp; /* 1:HTTP or 0:HTTPS */
	char url[MAX_URL_LEN];
	int reqType;  /* GET or POST */
	char header[MAX_HEADER_LEN];
	char startTime[MAX_TIME_LEN]; /* format hour:minute */
	char endTime[MAX_TIME_LEN]; /* format hour:minute */
	int timePeriod; /* in unit of hour, default 0 */
	int repeat;  /* True or False */
	int repeatNum;
	int lagTime;  /* in unit of second */
	int interval; /* in unit of second */
	int freqNum;  /* total request number */
} HTTP_TEST_ENTRY;

typedef struct http_test_result {
	int code; /* http response status code */
	char elapseTime[MAX_TIME_LEN]; /* http reply receive time  format second:ucsecond*/
	char requestTime[MAX_TIME_LEN]; /* http request send time  format second:ucsecond*/
	char cacheTime[MAX_TIME_LEN]; /* TODO: http cache time, don't know meaning*/
}HTTP_TEST_RESULT;

int httpRequestTest();
int getHttpTestResult(char * buf, int len);

int httpDownloadTest(char * user, char * passwd, char * url);
int httpDownloadResultCheck();

#endif
