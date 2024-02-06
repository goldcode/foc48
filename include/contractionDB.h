#pragma once

//#define USE_DB

#ifdef USE_DB

#include <cstdio>
#include <memory>
#include <vector>
#include <cmath>
#include <math.h> 
#include <atlstr.h>


//#include <mysqlx/xdevapi.h>
//using namespace ::mysqlx;


//#include <mysql\jdbc.h>

//#include "mysql_connection.h"

//#include <cppconn\\driver.h>
//#include <cppconn\\exception.h>
//#include <cppconn\\resultset.h>
//#include <cppconn\\statement.h>
//#include <cppconn\\prepared_statement.h>

class db_experiment {
public:
	CString    name = "19990101a";
	int          id = 0;
	int    idParent = 0;
private:
};

class db_measurement {
public:
	CString    name = "19990101a";
	int          id = 0;
	int    idParent = 0;

private:
};

class db_host {
public:
	CString    host = L"mysql-unigoe.gwdg.de";   //url or IP to Database
	unsigned   port = 3307;                      //database Port e.g. 3306
	CString    name = L"contractiondb";          //database Name e.g. contractionDb
	CString    user = L"analyzeMmt";             //database Username with write priviledges tp forces table
	CString    pass = L"ana!zeMe@mnt";           //database password, TDO: store secretely:)	
	//
	db_host(CString _host = L"127.0.0.1", unsigned _port=3007, CString _name = L"contractionDb", CString _user=L"analyzeMeasurement", CString _pass = L"analyzeMeasurement") {
		host = _host;
		port = _port;
		name = _name;
		user = _user;
		pass = _pass;
	};

	void set(CString _host, unsigned _port, CString _name, CString _user, CString _pass) {
		host = _host;
		port = _port;
		name = _name;
		user = _user;
		pass = _pass;
	};
private:
};


class contractionDB {
public:
	db_host host; 
	db_host local_host;
	db_host global_host;
	
	mysqlx::Session*    session;


	contractionDB()
	{
		global_host.set(L"mysql-unigoe.gwdg.de", 3307, L"contractiondb", L"analyzeMmt", L"ana!zeMe@mnt"); 
		local_host.set(L"127.0.0.1", 3007, L"contractionDb", L"analyzeMeasurement", L"analyzeMeasurement");		
		//
		host = local_host;
	};

	virtual ~contractionDB() { free(); };

	void free() {
		
	}

	void reset() {
		
	}


	void initialize() {
	}
	
	void     put() {};
	//void     put_t(double dIn) { put(dIn); };
	//
	db_measurement measurement;
	db_experiment  experiment; 
	
	double   get() {  };
	int      connect() ;
	int      myTest();
	int      prepareStatement(CString stmt);

	void     myPrintTable(CString table_name);

	int      executeQuery();
	int      executeQuery(CString stmt);


	int      getPersons();
	int      getExperimentId(CString expName, bool bCreate);
	int      getMeasurementId(CString expName, CString meaName, bool bCreate);
	int      getForcesId(CString expName, CString meaName, CString wellName, bool bCreate);
	int      addExperiment(CString expName);


	int      measurementId() { return measurement.id; };
	int      experimentId()  { return experiment.id; };
	CString  meaName()       { return measurement.name; };
	CString  ExpName()       { return experiment.name; };
	bool bConnected = false;


	//sql::Driver* driver;
	//sql::Connection* con;
	//sql::Statement* stmt;
	//sql::ResultSet* result;
	//sql::PreparedStatement* pstmt;

	CString stm;   // database statement 
	CString sel;   // database selection statement 
	CString frm;   // database from statement
	CString join1, join2, join3, join4;
	CString whr;   // database where statement
	
	int rowsCount=0;
	int id;

	std::string std_str;



private:
	

};


class db_labware {
public:
	CString name;
	int id;
	int idParent;

private:
};


class db_ehm {
public:
	CString name;
	int id;
	int idParent;

private:
};

class db_person {
public:
	CString name;
	int id;

private:
};

#endif