#ifndef QCATFIELDBINSTRATEGY_H
#define QCATFIELDBINSTRATEGY_H

#include <string>
#include <vector>
#include <boost/assign/list_of.hpp>
#include <boost/lexical_cast.hpp>

class QCATAttribute;

/*!
 * \brief A bin strategy works with a QCATAttribute to define how the space is divided to compute the probability masses. Previous versions of libqcat only offered fixed width for each attribute. Look at the various children of this class for useful bin strategies.
 */
class QCATBinStrategy
{
public:
    QCATBinStrategy();

	/*!
	 * \brief Provides a SQL-compatible representation of this bin's width value
	 * \param field The field represented by this bin
	 * \return SQL-compatible string representing bin width
	 */
	virtual double width(QCATAttribute* field) const { return 1.0; };
};

/*!
 * \brief Divides the space into a number of pieces
 */
class QCATBinStrategyDivide: public QCATBinStrategy
{
public:
	QCATBinStrategyDivide(int by)
		:m_by(by) {}
	virtual double width(QCATAttribute* field) const;

protected:
	int m_by;
};

/*!
 * \brief Divides the space by a specified amount if the range is greater than a certain amount; else maintains the default or sets to 1
 */
class QCATBinStrategyDivideIfMore: public QCATBinStrategyDivide
{
public:
	QCATBinStrategyDivideIfMore(int by, double ifMoreThan, bool elseDefault = true) 
		: QCATBinStrategyDivide(by), m_moreThanWhat(ifMoreThan) {}
	virtual double width(QCATAttribute* field) const;

protected:
	int m_moreThanWhat;
};

/*!
 * \brief A sensible default bin strategy - sets the bin width to an exact number
 */
class QCATBinStrategyExact: public QCATBinStrategy
{
public:
	QCATBinStrategyExact(double width) 
		:m_width(width) {}
	virtual double width(QCATAttribute* field) const;

protected:
	int m_width;
};


#endif // QCATFIELDBINSTRATEGY_H

