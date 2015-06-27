#ifndef QCATPQResult_H
#define QCATPQResult_H

#include "libpq-fe.h"
#include <memory>
#include <string>
#include <list>

using namespace std;

#include "qcatfield.h"

class QCATDataSource;
struct QCATRecordValue;

class QCATPQResult
{
public:
    QCATPQResult(PGresult* result);
    ~QCATPQResult();

	QCATPQResult(const QCATPQResult& other);
	QCATPQResult& operator=(const QCATPQResult& rhs);
	int ncols() const;
	int nrows() const;
	std::string colName(int col) const;
	bool hasCol(std::string) const;
	bool hasRows() const;

	std::vector<std::string> colNames() const;

	char* get(int row, int col) const;
	int getInt(int row, int col) const;
	double getDouble(int row, int col) const;
	
	char* get(int row, std::string col) const;
	int getInt(int row, std::string col) const;
	double getDouble(int row, std::string col) const;

	QCATRecordValue getRecordValue(int row, int col, QCATDataSource* ds) const;
	QCATRecordValue getRecordValue(int row, std::string col, QCATDataSource* ds) const;
	
	int colForName(std::string name) const;

private:
	PGresult* m_result;
};

#endif // QCATPQResult_H
