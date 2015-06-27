
#include "qcatngram.h"
#include "qcatdatasource.h"
#include "qcatfield.h"
#include "qcatcondition.h"
#include "qcatbin.h"
#include <numeric>

#define BOOST_LOG_DYN_LINK 1
#include <boost/log/trivial.hpp>

using namespace std::placeholders;

std::string QCATNGramHashAccumulator(std::string res, QCATNGramBin bin)
{
	return res += boost::lexical_cast<std::string>(bin) + ",";
}

QCATNGram::QCATNGram(shared_ptr<QCATDataSource> ds)
	:m_db(ds),m_qcat(new QCAT(ds))
{
	setN(QCATNGRAM_DEFAULT_N);
	setLetterType(qlt_absolute_value);
    
	QCATNGramLetterFunc fn = std::bind(&QCATNGram::buildAbsoluteZ, this, _1);
}

void QCATNGram::setDependentVariable(std::string fieldname)
{
	// dependent variables are just VONs
	m_dependent = make_shared<QCATAttribute>(m_db->fieldForName(fieldname));
	initialiseBins();
}

void QCATNGram::setIndependentVariable(std::string fieldname)
{
	// independent variable usually time or something
	m_independent = make_shared<QCATAttribute>(m_db->fieldForName(fieldname));
}

void QCATNGram::setQCAT(shared_ptr<QCAT> qcat)
{
	m_qcat = qcat;
}

shared_ptr<QCAT> QCATNGram::qcat() const
{
	return m_qcat;
}

void QCATNGram::setN(int n)
{
	m_N = n;
	initialiseBins();
}

void QCATNGram::initialiseBins()
{
	discoverBinWidth();
	setDependentBin();
}

void QCATNGram::discoverBinWidth()
{
	if(m_dependent.get() == NULL)
		return;

	std::string sql = "SELECT MIN(" + m_dependent->sqlUnbinned() + ") AS _min,"
		+ " MAX(" + m_dependent->sqlUnbinned() + ") AS _max "
		+ " FROM " + m_db->table()
		+ " WHERE " + m_qcat->sqlConditionals();
	
	QCATDBResult rows = m_db->executeSQL(sql);
	
	m_depStatsMin = rows->getDouble(0,"_min");
	m_depStatsMax = rows->getDouble(0,"_max");
	m_binWidth = (m_depStatsMax - m_depStatsMin) / (double)m_N;

	BOOST_LOG_TRIVIAL(info) << "QCATNGram::discoverBinWidth got bin width of " << m_binWidth << std::endl;
}

void QCATNGram::setDependentBin()
{
	if(m_dependent.get() == NULL)
		return;

	m_dependent->bin()->setBinWidth(m_binWidth);
}
	
void QCATNGram::setLetterType(QCATNGramLetterType type)
{
	m_letterType = type;
}

/*
 * grab all rows
 */
std::string QCATNGram::sqlNGram() const
{
	std::string sql = "SELECT " + m_dependent->sqlSelect()		// dependent variable
		+ ", " + m_independent->sqlUnbinned()
	   	+ ", " + m_qcat->sqlVONSHashSelect()					// hash of VONs
		+ " FROM " + m_db->table()
		+ " WHERE " + m_qcat->sqlConditionals()					// any additional conditions (not ngram related)
		+ " ORDER BY " + m_dependent->name() + ", hash ";	// ensures dependent variables in correct order

	return sql;
}

QCATNGramZ QCATNGram::buildAbsoluteZ(QCATNGramTimeToDep& timeToDeps)
{
	std::unordered_map<QCATNGramHash,QCATNGramLetter> Z;

	// compile alphabet Z using the dependent list as the letter 
	for(auto item: timeToDeps) {
		// maintain list of timestamps for this letter for display and also entropy calculation
		std::string hash = std::accumulate(item.second.begin(),item.second.end(),
			std::string(),QCATNGramHashAccumulator);
		Z[hash].independents.push_back(item.first);
	}

	return Z;
}

int QCATNGramIndexSortCompare(int a, int b, QCATNGramDepList data)
{
	return data[a] < data[b];
}

QCATNGramZ QCATNGram::buildRelativeZ(QCATNGramTimeToDep& timeToDeps)
{
    std::unordered_map<QCATNGramHash,QCATNGramLetter> Z;

	// compile alphabet Z using the dependent list as the letter 
	for(auto item: timeToDeps) {
		// build list of numbers to be used as sort indices
		auto indices = QCATNGramDepList(item.second.size(),0);
		std::iota(indices.begin(),indices.end(),0);
		// now sort on it but referencing the actual vector to be sorted
		std::sort(indices.begin(),indices.end(),std::bind(QCATNGramIndexSortCompare,_1,_2,item.second));
		// build hash from resulting indices
		std::string hash = std::accumulate(indices.begin(),indices.end(),
			std::string(),QCATNGramHashAccumulator);
		Z[hash].independents.push_back(item.first);
	}

	return Z;
}

QCATNGramZ QCATNGram::buildDeltaZ(QCATNGramTimeToDep& timeToDeps, bool clamp)
{
    std::unordered_map<QCATNGramHash,QCATNGramLetter> Z;

	auto prev = timeToDeps.begin()->second;

	// compile alphabet Z using the dependent list as the letter 
	for(auto item: timeToDeps) {
		auto indices = QCATNGramDepList(item.second.size(),0);
		QCATNGramDepList curr = item.second;
		for(int j=0;j<indices.size();j++) {
			indices[j] = curr[j] == prev[j] ? 0 : (curr[j] > prev[j] ? 1 : -1);
		}
		// build hash from resulting indices
		std::string hash = std::accumulate(indices.begin(),indices.end(),
			std::string(),QCATNGramHashAccumulator);
		Z[hash].independents.push_back(item.first);
		
		prev = item.second;
	}

	return Z;
}

QCATNGramLetterFunc QCATNGram::letterFunc() 
{
	switch(m_letterType) {
		case qlt_absolute_value:
			return std::bind(&QCATNGram::buildAbsoluteZ, this, _1);
		case qlt_relative_order:
			return std::bind(&QCATNGram::buildRelativeZ, this, _1);
		case qlt_delta:
			return std::bind(&QCATNGram::buildDeltaZ, this, _1, false);
		case qlt_direction:
			return std::bind(&QCATNGram::buildDeltaZ, this, _1, true);
		default:
			std::cerr << "QCATNGram::letterFunc(): invalid letter function type!" << std::endl;
			return std::bind(&QCATNGram::buildAbsoluteZ, this, _1);
	}
}

QCATNGramResult QCATNGram::executeNGram() 
{
	// set up hashtable of T => list of dependents (which is actually the letter hash)
	QCATNGramTimeToDep timeToDeps;

    // get all rows from DB matching conditionals
	initialiseBins();
    const std::string sql = this->sqlNGram();
    QCATDBResult rows = m_db->executeSQL(sql);
    int totalRows = 0;

	const int depCol = rows->colForName(m_dependent->name());
	const int indepCol = rows->colForName(m_independent->name());

    // compile hashtable of time => list of dependent variables
	for(int i=0;i<rows->nrows();i++) {
        std::string hash(rows->get(i,"hash"));
		const QCATNGramTime t = rows->getInt(i,indepCol);
		const QCATNGramBin b = rows->getInt(i,depCol);
		timeToDeps[t].push_back(b);
        totalRows++;
    }

	auto Z = letterFunc()(timeToDeps);
	
	// calculate overall entropy of Z
    double HZ = 0;
    double totalSurprise = 0;
	std::vector<QCATNGramLetter> letters;
	const float oneOverTotalRows = 1.0/(float)totalRows;

    for(auto item: Z) {
        const float prob = item.second.independents.size() * oneOverTotalRows; 
        const float log2prob = log2(prob);
		item.second.letter = item.first;
        item.second.prob = prob;
        item.second.surprise = -log2prob;
        HZ += prob * log2prob;
        totalSurprise += -log2prob;
    }

    HZ = -HZ;

    // compile results struct
    QCATSummary summary;
    summary.message = "Successfully run QCAT.";
    summary.entropy = HZ;
    summary.surprise_mean = totalSurprise / (float)Z.size();
    summary.surprise_stddev = 0; // TODO
    summary.alphabet_size = Z.size();
    summary.record_length = totalRows;
    summary.uncertainty = HZ / log2(summary.alphabet_size);
    summary.sql_used = sql;
    
	QCATNGramResult result;
	result.qcatsummary = summary;

    return result;
}

