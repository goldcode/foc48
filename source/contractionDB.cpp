
#include "contractionDB.h"
//#include <mysqlx/xdevapi.h>

#ifdef USE_DB
using namespace ::mysqlx;

#include <cstdio>

/* Standard C++ includes */
#include <stdlib.h>
#include <iostream>


#include <string>

//#include <http.h>

#include "DBG_print.h"

//#include "WinHttpWrapper.h"
//#include "WinHttpClient.h"


//#include <mysql\jdbc.h>


//#include "mysql_connection.h"

//#include <cppconn\driver.h>
//#include <cppconn\exception.h>
//#include <cppconn\resultset.h>
//#include <cppconn\statement.h>

//#include "..\..\curl\curl\include\curl\curl.h"


//#pragma comment(lib, "C:\\labhub\\Repos\\Debug\\Win32\\mysqlcppconn.dll")

using namespace std;
//using namespace WinHttpWrapper;

int contractionDB::connect()
{
	mysqlx::SessionSettings sett(SessionOption::USER, "myrImager",	SessionOption::PWD, "myrImager",	SessionOption::DB, "contractionDb");
	/*@note
		When using `HOST`, `PORT`and `PRIORITY` options to specify a single
		host, all have to be specified in the same `set()` call.
	*/
	bool bUseNamedPipe = false;
	if (bUseNamedPipe) {
		sett.set(SessionOption::SOCKET, "MySql");
	}
	else {
		sett.set(SessionOption::HOST, "localhost", SessionOption::PORT, 33060);
	}
	session = new mysqlx::Session(sett);
	Schema db = session->getSchema("contractionDb");
	return bConnected;
}



int contractionDB::myTest()
{
	CString table_name = "person";
	stm.Format(L"SELECT * from %s", table_name );	

	SqlStatement mySql = session->sql("SELECT * from person");
	RowResult myRows = mySql.execute();
	const Columns& myCols = myRows.getColumns();
	int nRow = myRows.count();
	int nCol = myRows.getColumnCount();

	Row myRow = myRows.fetchOne();
	for (int iRow = 0; iRow < nRow; iRow++) {
		//int nCol = myRow.get(1);
		for (int iCol = 0; iCol < nCol; iCol++) {
			std::cout << iRow << " " << iCol << " " << myCols[iCol].getColumnName() << " " << myRow[iCol] << std::endl;
		}
		myRow = myRows.fetchOne();
	}
	std::cout << "Done!\n";
	return 1;
}




int contractionDB::getPersons()
{

	//pstmt = con->prepareStatement("SELECT * FROM person;");
	//result = pstmt->executeQuery();
	//while (result->next())
	//	printf("Reading from table=(%d, %s, %d)\n", result->getInt(1), result->getString(2).c_str(), result->getInt(3));
	return 0;
}

int contractionDB::addExperiment(CString expName)
{

	return 0;
}

int contractionDB::getExperimentId(CString expName, bool bCreate)
{
	int id = 0;
	sel.Format(L"select idExperiment ");
	frm.Format(L"from experiment ");
	whr.Format(L"where name=\"%s\" ", expName);

	/*
	if (executeQuery(sel + frm + whr) == 1)
		id = result->getInt(1);
	else if (bCreate)
		addExperiment(expName);
	else
		MessageBox(NULL, expName, L"Experiment is not defined", 1);

		*/

	//pstmt = con->prepareStatement("SELECT * FROM person;");

	/*
	result = pstmt->executeQuery();
	while (result->next())
		printf("Reading from table=(%d, %s, %d)\n", result->getInt(1), result->getString(2).c_str(), result->getInt(3));
	*/
	return 0;
	
}

int contractionDB::executeQuery(CString stmt)
{
	prepareStatement(sel + frm + whr);
	return executeQuery();
}


int contractionDB::executeQuery()
{
	//result = pstmt->executeQuery();
	//return result->rowsCount();
	return 1;
}

int contractionDB::prepareStatement(CString stmt)
{
	//SQL data is expected to be UTF-8. 
	//MFC program UNICODE option is enabled by default (UTF-16).	
	std_str = CW2A(stmt, CP_UTF8);
	//pstmt = con->prepareStatement(std_str);

	return 0;
}

int contractionDB::getMeasurementId(CString expName, CString meaName, bool bCreate)
{
	sel.Format(L"select e.name, m.name, l.name ");
	frm.Format(L"from measurement m ");
	join1.Format(L"join event em using(idEvent) ");
	join2.Format(L"join experiment e on(em.idParent = e.idEvent) ");
	whr.Format(L"where e.name=\"%ls\" and m.name=\"%ls\"", expName, meaName);

	stm = sel + frm + join1 + join2 + whr;

	//pstmt = con->prepareStatement("SELECT * FROM person;");

	//result = pstmt->executeQuery();
	//while (result->next())
	//	printf("Reading from table=(%d, %s, %d)\n", result->getInt(1), result->getString(2).c_str(), result->getInt(3));
	return 0;
}

int contractionDB::getForcesId(CString expName, CString meaName, CString wellName, bool bCreate)
{

	sel.Format(L"select e.name, m.name, l.name ");
	frm.Format(L"from measurement m ");
	join1.Format(L"join event em using(idEvent) ");
	join2.Format(L"join event ew on(m.idEvent = ew.idParent) ");
	join3.Format(L"join labware l on(l.idLabware = ew.idLabware) ");
	join4.Format(L"join experiment e on(em.idParent = e.idEvent) ");
	whr.Format(L"where e.name=\"%ls\" and m.name=\"%ls\" and l.name=\"%ls\" ", expName, meaName, wellName);

	stm.Format(L"%ls %ls %ls %ls %ls %ls %ls", sel, frm, join1, join2, join3, join4, whr);

	//pstmt = con->prepareStatement("SELECT * FROM person;");

	//result = pstmt->executeQuery();
	//while (result->next())
	//	printf("Reading from table=(%d, %s, %d)\n", result->getInt(1), result->getString(2).c_str(), result->getInt(3));
	return 0;
}

#endif