
#include "qcat.h"
#include "qcatdatasource.h"
#include "qcatpqresult.h"
#include "qcatfield.h"
#include <boost/lexical_cast.hpp>

QCATPQResult::QCATPQResult(PGresult* result)
{
	m_result = result;
}

QCATPQResult::~QCATPQResult()
{
	PQclear(m_result);
}
	
QCATPQResult::QCATPQResult(const QCATPQResult& other) {
	m_result = PQcopyResult(other.m_result, 0);
}

QCATPQResult& QCATPQResult::operator=(const QCATPQResult& other) {
	m_result = PQcopyResult(other.m_result, PG_COPYRES_ATTRS | PG_COPYRES_TUPLES);
	return *this;
}

std::vector<std::string> QCATPQResult::colNames() const
{
	int nFields = PQnfields(m_result);
	std::vector<std::string> colNames(nFields);
	for (int i = 0; i < nFields; i++)
		colNames[i] = std::string(PQfname(m_result, i));
	return colNames;
}

int QCATPQResult::ncols() const
{
	return PQnfields(m_result);
}

int QCATPQResult::nrows() const
{
	return PQntuples(m_result);
}

bool QCATPQResult::hasRows() const
{
	return nrows() != 0;
}

std::string QCATPQResult::colName(int col) const
{
	return std::string(PQfname(m_result, col));
}

int QCATPQResult::colForName(std::string name) const
{
	return PQfnumber(m_result, name.c_str());
}

bool QCATPQResult::hasCol(std::string name) const
{
	return colForName(name) != -1;
}

char* QCATPQResult::get(int row, int col) const
{
	return PQgetvalue(m_result, row, col); 
}

int QCATPQResult::getInt(int row, int col) const
{
	return atoi(PQgetvalue(m_result, row, col));
}

double QCATPQResult::getDouble(int row, int col) const
{
	return atof(PQgetvalue(m_result, row, col)); 
}

char* QCATPQResult::get(int row, std::string col) const
{
	return PQgetvalue(m_result, row, colForName(col)); 
}

int QCATPQResult::getInt(int row, std::string col) const
{
	return atoi(PQgetvalue(m_result, row, colForName(col)));
}

double QCATPQResult::getDouble(int row, std::string col) const
{
	return atof(PQgetvalue(m_result, row, colForName(col))); 
}

QCATRecordValue QCATPQResult::getRecordValue(int row, std::string col, QCATDataSource* ds) const
{
	return getRecordValue(row, colForName(col), ds); 
}

QCATRecordValue QCATPQResult::getRecordValue(int row, int col, QCATDataSource* ds) const
{
	QCATRecordValue val;
	switch(ds->fields()->at(col)->type()) {
		case fft_integer:
		case fft_double:
			val.v_double = this->getDouble(row,col);
			break;
		case fft_string:
		case fft_date:
		case fft_time:
		case fft_boolean:
			val.v_string = boost::lexical_cast<std::string>(this->get(row,col));
			break;
	}
	return val;
}
