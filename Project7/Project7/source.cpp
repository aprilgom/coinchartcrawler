#include <iostream>
#include <curl/curl.h>
#include<WinSock2.h>
#include<mysql.h>
#pragma comment(lib,"libmysql.lib")
#pragma warning(disable:4996)
using namespace std;
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <stdint.h>
#include <curl/curl.h>
#include <mysql.h>
#include <time.h>
#include <windows.h> 
#include <winsock2.h>
#include <json/json.h>


int now_start_sec = 59;
int now_end_sec = 0;
int now_t_min = 0;
int now_start_price = 0;
int now_high_price = 0;
int now_low_price = INT_MAX;
int now_end_price = 0;

struct url_data {
	size_t size;
	char *data;
};

void getDat();
void ReadJson(string data);

size_t write_data(void *ptr, size_t size, size_t nmemb, struct url_data *data)
{
	size_t index = data->size;
	size_t n = (size * nmemb);
	char *tmp;

	data->size += (size * nmemb);


	//fprintf(stderr, "data at %p size=%ld nmemb=%ld\n", ptr,	size, nmemb);


	tmp = (char *)realloc(data->data, data->size + 1); // + 1 for \n
	
	if (tmp)
	{
		data->data = tmp;
	}
	else
	{
		if (data->data)
		{
			free(data->data);
		}
		fprintf(stderr, "Failed to allocate memory.\n");
		return 0;
	}

	memcpy((data->data + index), ptr, n);
	data->data[data->size] = '\0';
	
	return size * nmemb;
}


void main() {
	cout << mysql_get_client_info()<<endl;
	MYSQL mysql;
	MYSQL* SQL;
	char query[128];
	int querystat = 0;
	
	mysql_init(&mysql);
	SQL = mysql_real_connect(&mysql, "127.0.0.1", "root", "4321", "coin_chart", NULL, NULL,0);
	if (SQL == NULL) {
		fprintf(stderr, "Mysql connection error : %s", mysql_error(&mysql));
	}

	querystat = mysql_query(&mysql, "use coin_chart;");
	if (querystat != 0)cout << mysql_error(&mysql)<<endl;
	//cout << data.data << endl;
	time_t now;
	struct tm tstruct;
	int temp_t_sec;
	int temp_t_min;
	int now_t_sec;
	now = time(0);
	localtime_s(&tstruct, &now);
	temp_t_sec = tstruct.tm_sec;
	temp_t_min = tstruct.tm_min;
	while (1) {
		now = time(0);
		localtime_s(&tstruct, &now);
		now_t_sec = tstruct.tm_sec;
		
		if (now_t_sec >= temp_t_sec + 1|| temp_t_sec == now_t_sec + 59) {
			//cout << now_t_min<<now_t_sec << endl;
			temp_t_sec = now_t_sec;	
			getDat();
			
			
			now_t_min = tstruct.tm_min;
			if (now_t_min >= temp_t_min + 1 || temp_t_min == now_t_min + 59) {
				temp_t_min = now_t_min;
				//sqlsend
				cout<<tstruct.tm_mon+1<<"-"<<tstruct.tm_mday << "-" << tstruct.tm_hour << "-" << tstruct.tm_min - 1 << endl;
				cout << now_high_price << endl;
				cout << now_low_price << endl;
				cout << now_start_price << endl;
				cout << now_end_price << endl;
				//
				char nowtime_s[20];
				sprintf(nowtime_s, "%d-%d-%d-%d:%d", tstruct.tm_year+1900, tstruct.tm_mon + 1, tstruct.tm_mday, tstruct.tm_hour, (tstruct.tm_min != 0) ? tstruct.tm_min - 1 : 59);
				sprintf(query, "insert into min_charts values ('%s',%d,%d,%d,%d);", nowtime_s, now_high_price, now_low_price, now_start_price, now_end_price);
				querystat = mysql_query(&mysql, query);
				if (querystat != 0)cout << mysql_error(&mysql);
				now_start_sec = 59;
				now_end_sec = 0;
				now_start_price = 0;
				now_high_price = 0;
				now_low_price = INT_MAX;
				now_start_price = 0;
				now_end_price = 0;
			}

		}
	}
	mysql_close(&mysql);
}


void getDat(){
	struct url_data data;
	data.size = 0;
	data.data = (char*)malloc(4096);
	data.data[0] = '\0';
	CURL *curl;
	CURLcode res;
	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, "https://api.bithumb.com/public/recent_transactions/BTC?count=100");
		//curl_easy_setopt(curl, CURLOPT_POST, 1);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
		res = curl_easy_perform(curl);
		//ReadJson(data.data);
		if (res != CURLE_OK)
		{
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		}
	}
	//cout << data.data;
	ReadJson(data.data);
	curl_easy_cleanup(curl);
}

void ReadJson(string data) {
	Json::Value root;
	Json::Reader reader;
	reader.parse(data, root);
	int check_min = 0;
	int check_sec = 0;
	int check_price = 0;
	string check_time_str;

	for (int i = 99; i >=0; i--) {
		/*
		cout << root["data"][i]["transaction_date"].asCString() << endl;
		cout << root["data"][i]["type"].asCString() << endl;
		cout << root["data"][i]["units_traded"].asCString() << endl;
		cout << root["data"][i]["price"].asCString() << endl;
		cout << root["data"][i]["total"].asCString() << endl;
		*/
		check_price = stoi(root["data"][i]["price"].asCString(),nullptr);
		check_time_str = root["data"][i]["transaction_date"].asCString();
		check_min = (check_time_str[13] - '0')*10
			+ (check_time_str)[14] - '0';
		check_sec = (check_time_str[16] - '0')*10
			+ (check_time_str)[17] - '0';
	//	cout << "now_t_min" << now_t_min << endl;
		//cout << check_time_str << endl;
		if (now_t_min == check_min) {
			if (now_start_sec > check_sec ||   check_sec == now_start_sec + 59) {
				now_start_sec = check_sec;
				now_start_price = check_price;
				//cout << "sigaset" << check_time_str << endl;
			}
			if (now_end_sec <= check_sec ) {
				now_end_sec = check_sec;
				now_end_price = check_price;
				//cout << "jonggaset" << endl;
			}

			if (now_high_price < check_price)now_high_price = check_price;
			if (now_low_price > check_price)now_low_price = check_price;
		}
		
	}
	
}