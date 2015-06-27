#ifndef QCATNGRAM_H
#define QCATNGRAM_H

#include <string>
#include <memory>
using namespace std;

#include "qcat.h"
#include <functional>

class QCATCondition;

typedef int QCATNGramBin;
typedef int QCATNGramTime;
typedef std::string QCATNGramHash;
typedef std::vector<QCATNGramTime> QCATNGramIndepList;
typedef std::vector<QCATNGramBin> QCATNGramDepList;


enum QCATNGramLetterType
{
	qlt_absolute_value = 0,
	qlt_relative_order = 1,
	qlt_delta = 2,
	qlt_direction = 3
};

struct QCATNGramLetter {
    QCATNGramLetter() {
        prob = 0;
        surprise = 0;
    }

    float prob;
    float surprise;
	std::string letter;
	std::vector<QCATNGramTime> independents;		
};


class QCATNGramResult {
public:
	QCATSummary qcatsummary;	
};

#define QCATNGRAM_DEFAULT_N 4

class QCATNGram;
typedef std::unordered_map<QCATNGramHash,QCATNGramLetter> QCATNGramZ;
typedef std::map<QCATNGramTime,QCATNGramDepList> QCATNGramTimeToDep;
typedef std::function<QCATNGramZ(QCATNGramTimeToDep&)> QCATNGramLetterFunc;

/*!
 * \brief QCAT N-Grams
 */
class QCATNGram 
{
public:
    QCATNGram(shared_ptr<QCATDataSource>);

	void setIndependentVariable(std::string);
	void setDependentVariable(std::string);
	void setN(int);
	void setLetterType(QCATNGramLetterType);

	void setQCAT(shared_ptr<QCAT> qcat);
	shared_ptr<QCAT> qcat() const;

	QCATNGramResult executeNGram();

private:

	void setDependentBin();
	void discoverBinWidth();

	void initialiseBins();

	QCATNGramZ buildAbsoluteZ(QCATNGramTimeToDep&);
	QCATNGramZ buildRelativeZ(QCATNGramTimeToDep&);
	QCATNGramZ buildDeltaZ(QCATNGramTimeToDep&, bool);

	std::string sqlNGram() const;

    QCATNGramLetterType m_letterType;
	shared_ptr<QCATAttribute> m_independent;
	shared_ptr<QCATAttribute> m_dependent;
	int m_N;
	
	double m_depStatsMin, m_depStatsMax;
	double m_binWidth;

	QCATNGramLetterFunc letterFunc();

	shared_ptr<QCAT> m_qcat;
	shared_ptr<QCATDataSource> m_db;
};


#endif // QCATNGRAM_H

