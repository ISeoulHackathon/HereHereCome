#include<stdio.h>
#include<time.h>
//#include<sqlite3.h>
#include<mysql.h>

#include<string.h>
#include<errno.h>
#include<stdlib.h>
#include<wiringPi.h>
#include<wiringSerial.h>

char device[] = "/dev/ttyACM0";
int fd;
unsigned long baud = 9600;
unsigned long time_val = 0;

int count;

void insertDataToServer(MYSQL*, struct tm *);
void loop(MYSQL*);
void setup();

int main() 
{
//	sqlite3 *conn;
//	sqlite3_stmt *statement;
	int rc;
	int col;
	int i;
//////////////////////////// MYSQL Setting ////////////////////
	MYSQL *myConn;
	MYSQL_RES *myRes;
	MYSQL_ROW myRow;

	char *server = "128.199.114.194";
	char *user = "hackathon";
	char *pass = "hackathon@like";
	char *db = "hackathon";
	int mysql_port = 3306;
////////////////////MySQL Connection/////////////////////////////////

	if(!(myConn = mysql_init((MYSQL*)NULL))) {
		printf("MySQL init fail..\n");
		return 0;
	}
	printf("MySQL init success.\n");
	if(!mysql_real_connect(myConn, server, user, pass, NULL, mysql_port, NULL, 0))
	{
		printf("MySQL connect error.\n");
		return 0;
	}
	printf("MySQL connect success.\n");

	if(mysql_select_db(myConn, db)!=0) {
		mysql_close(myConn);
		printf("select_db fail.\n");
		return 0;
	}
	printf("select db success.\n");


///////////////////////////Init sqlite///////////////////////////////////
	/*rc = sqlite3_open("test.db", &conn);

	if(rc) {
		fprintf(stderr, "Can\'t open database: %s\n", sqlite3_errmsg(conn));
		return 0;
	} else {
		fprintf(stderr, "Opened database successfully\n");
	}

///////////////////////////////////////////////////////////////
//////////////// get sqlite datas. /////////////////
	rc = sqlite3_prepare_v2(conn, "select * from seoul", -1, &statement, NULL);
	if(rc != SQLITE_OK) {
		printf("We did not get any data\n");
		return 0;
	}

///////////////// get mysql datas. //////////////////
	if(mysql_query(myConn, "SELECT * FROM user_cnt")){
		printf("mysql query fail..");
		return 0;
	}
	myRes = mysql_store_result(myConn);
	printf("mysql result store success.\n");

	while((myRow = mysql_fetch_row(myRes)!=NULL) {
		printf("%s %s\n", myRow[0], myRow[1], myRow[2]);
	}
//////////////////////////////////////////////
*/

/////////////////// sqlite3 ///////////////////
/*	col = sqlite3_column_count(statement);

	for(i=0;i<col;i++) {
		printf("=%s=\t\t", sqlite3_column_name(statement, i));
	}printf("\n");

	while(1) {
		rc = sqlite3_step(statement);
		if(rc == SQLITE_ROW) {
			for(i=0;i<col;i++) {
				switch(sqlite3_column_type(statement, i)) {
					case SQLITE_INTEGER:
						printf("%d\t\t", sqlite3_column_int(statement, i));
						break;
					case SQLITE_TEXT:
						printf("%s\t\t", sqlite3_column_text(statement, i));
						break;
					case SQLITE_NULL:
						printf("NULL\t\t");
				}
			} printf("\n");
		} else if(rc == SQLITE_DONE) {
			printf("DONE\n");
			break;
		} else {
			printf("rc: %d\n", rc);
			break;
		}

	}*/
	setup();
	loop(myConn);

	//sqlite3_finalize(statement);
	//sqlite3_close(conn);

	mysql_close(myConn);

	return 0;
}

void setup()
{
	if((fd = serialOpen(device, baud)) < 0) {
		fprintf(stderr, "Unable to open serial device: %s\n", strerror(errno));
		exit(1);
	}

	if(wiringPiSetup() == -1) {
		fprintf(stdout, "Unable to start wiringPi: %s\n", strerror(errno));
		exit(1);
	}
}

void loop(MYSQL *myConn)
{
	struct tm *t;
	time_t timer;
	int temp_minutes, temp_seconds;

/*	if(millis() - time_val>=3000) {
		serialPuts(fd, "Pong!\n");
		serialPutchar(fd, 65);
		time_val = millis();
	}
*/

	while(1) {

		timer = time(NULL);
		t = localtime(&timer);
	
		// copy string 'sql' every 5 minutes.
		temp_minutes = t->tm_min%5;
		temp_seconds = t->tm_sec;
		if(temp_minutes == 0 && temp_seconds == 0) 
			insertDataToServer(myConn, t);

		if(serialDataAvail(fd)) {
			char newChar = serialGetchar(fd);
//			printf("%c ", newChar);
			if(newChar == '0') {
				if(count > 0) 
					count--;
			} else if(newChar == '1') {
				count++;
			}
printf("count:%d\n", count);
			fflush(stdout);
		}
	}

	
}

void insertDataToServer(MYSQL *myConn, struct tm *t)
{	
	char zone[8] = "ID-1010";
	char sql[100];

	sprintf(sql, "INSERT INTO user_cnt VALUES(\'%d-%d-%d %d:%d:%d\',%d,\'%s\');", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, count, zone);
	printf("sql:%s\n",sql);
	if(mysql_query(myConn, sql)) {
		printf("MySQL query fail..\n");			
	}
	else{
		printf("MySQL data insert success.\n");
		count = 0;
	}
	sleep(1);
}
