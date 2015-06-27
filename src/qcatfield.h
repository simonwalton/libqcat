#ifndef FACASFIELD_H
#define FACASFIELD_H

#include <list>
#include <string>
#include <memory>
#include <map>
#include <vector>

#include "qcatbinstrategy.h"

enum QCATFieldType
{
    fft_integer = 0,
    fft_double = 1,
    fft_string = 2,
    fft_boolean = 3,
    fft_date = 4,
	fft_time = 5
};

class QCATDataSource;
class QCATBin;
class QCATFieldStats;
class QCATBinStats;

using namespace std;
using std::shared_ptr;

class QCATFieldInvalidBinException: public exception
{
public:
    QCATFieldInvalidBinException(std::string msg)
        :m_msg(msg) {}
    ~QCATFieldInvalidBinException() throw() {}

    virtual const char* what() const throw()
    {
        return std::string("QCATFieldInvalidBinException: " + m_msg).c_str();
    }
private:
    std::string m_msg;
};

/*!
 * \brief The QCATField class
 */
class QCATField
{
public:
    QCATField(QCATDataSource* db, std::string name, int index, QCATFieldType type);

    void setName(std::string);
    std::string name(bool quoted = false) const;
    std::string nameBinned() const;

    void setIndex(int);
    int index() const;

    void setType(QCATFieldType);
    QCATFieldType type() const;

    QCATFieldStats stats();

    /*!
     * Gives the current database
     */
    QCATDataSource* db() const { return m_db; }


protected:
    void initialise();
	std::string safePad(std::string str);

     /*!
     * \brief Provides a suitable name for a database index
     */
    std::string indexName() const;

    std::string m_name;
    std::string m_table;
    int m_index;
    QCATFieldType m_type;
    QCATDataSource* m_db;

	shared_ptr<QCATFieldStats> m_cachedStats;
};


#endif // FACASFIELD_H
