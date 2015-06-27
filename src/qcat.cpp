#include "qcat.h"
#include "qcatdatasource.h"
#include "qcatbin.h"
#include <math.h>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/functional/hash.hpp>
#include <boost/timer/timer.hpp>
using namespace boost::timer;

#define SERVER_SP_FUNC "qcat_server"
#define SERVER_SP_EXPLAIN_FUNC "qcat_server_morestats"
#define SERVER_SP_ARGS "totalrowcount bigint, zcount bigint, hz numeric, sum_surprise numeric"
#define HASH_TYPE fht_string_concat

#define BOOST_LOG_DYN_LINK 1
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/file.hpp>
namespace logging = boost::log;

struct QCATLetterAndRecord : public QCATLetter
{
public:
    QCATLetterAndRecord() {}
	QCATRecord record;

    bool operator< (const QCATLetterAndRecord &rhs) const {
        return this->surprise < rhs.surprise;
    }
};

QCAT::QCAT(QCATSpec spec, shared_ptr<QCATDataSource> db)
    :m_db(db),m_hashType(HASH_TYPE) {
    setSpec(spec);
	init();
}

QCAT::QCAT(shared_ptr<QCATDataSource> db)
    :m_db(db),m_hashType(HASH_TYPE) {
    setSpec(QCATSpec());
	init();
}

void QCAT::init()
{
	m_limit = -1;
	m_executionMethod = fem_client;
	m_serverSPName = SERVER_SP_FUNC;
	m_serverSPArgs = SERVER_SP_ARGS;
}

void QCAT::setSpec(QCATSpec spec)
{
    m_spec = spec;
    syncWithSpec();
}

QCATSummary QCAT::execute() const
{
	return this->operator()();
}

QCATSummary QCAT::operator()() const
{
    std::string error;
    if(!isComplete(&error))
        return QCATSummary(error);

	return run();
}

QCATSummary QCAT::run() const
{
	switch(m_executionMethod) {
		case fem_server:
			return serverRun();
		case fem_client:
			return clientRun();
	}
}

bool QCAT::isComplete(std::string* why) const
{
	cout << "Checking..." << endl;

    for(auto cond: m_conditionals) {
        if(!cond.second->isComplete()) {
            if(why) *why = "A conditional is not complete; QCAT not run.";
            return false;
        }
		if(!cond.second->LHS()) {
            if(why) *why = "A conditional field is invalid.";
			return false;
		}
		if(!cond.second->LHS()->OK()) {
            if(why) *why = "A conditional field doesn't exist.";
			return false;	
		}
	}
    if(m_vons.size() == 0) {
        if(why) *why = "No VONs yet; QCAT not run.";
        return false;
    }
	for(auto von: m_vons) {
		if(!von.second) {
            if(why) *why = "A VON field is invalid.";
			return false;
		}
		if(!von.second->OK()) {
            if(why) *why = "A VON field doesn't exist.";
			return false;	
		}
	}

	cout << "Complete!";

    return true;
}

bool QCAT::userCanRun() const
{
	std::string err;
	bool success = isComplete(&err);
	if(!success)
		std::cerr << err << std::endl;
	return success;
}

std::string QCAT::whyCantUserRun() const
{
	std::string err = "Unknown error.";
	isComplete(&err);
	return err;
}

void QCAT::setBinStrategy(shared_ptr<QCATBinStrategy> bs, bool override_existing)
{
	m_binStrategy = bs;
	initialiseBinsFromStrategy(override_existing);
}

shared_ptr<QCATBinStrategy> QCAT::binStrategy() const 
{
	return m_binStrategy;
}

void QCAT::initialiseBinsFromStrategy(bool override_existing)
{
	for(auto attr: attributes()) {
		if(override_existing || (!attr.second->hasBinStrategy()))
			attr.second->setBinStrategy(m_binStrategy);
	}
}

QCATSummary QCAT::serverRun() const
{
	if(!this->userCanRun()) return createFailureSummary(whyCantUserRun());

    std::string sql = "SELECT * FROM " + m_serverSPName + "('" + sqlServerTableName() +
        "','" + QCAT::escapeQuotes(sqlConditionals()) +
        "','" + sqlVONS() +"') AS f(" + m_serverSPArgs + ");";
    bool success;
    boost::timer::cpu_timer cpu;
    QCATDBResult rows = m_db->executeSQL(sql,&success);
    boost::timer::cpu_times times = cpu.elapsed();

    if(!success) {
        std::cerr << "***Problem executing server QCAT" << std::endl;
        std::cerr.flush();
		QCATSummary sum;
		sum.message = "There was a problem executing the QCAT. Check datatypes?";
		sum.success = false;
		return sum;
    }

    // fill summary structure based on server function result
    QCATSummary result;
//	for(int i=0;i<rows->ncols();i++)
//		result.attrs[rows->colName(i)] = std::string(rows->get(0,i));
	result.success = true;
	result.qcatid = m_spec.ID();
    result.message = "Successfully run QCAT.";
    result.alphabet_size = rows->getDouble(0,"zcount");
    result.entropy = rows->getDouble(0,"hz");
    result.surprise_mean = rows->getDouble(0,"sum_surprise") / result.alphabet_size;

	result.surprise_stddev = 0;
	if(rows->hasCol("stddev_surprise"))
	    result.surprise_stddev = rows->getDouble(0, "stddev_surprise");
	
    result.record_length = rows->getInt(0,"totalrowcount");
    result.sql_used = sql;
    result.wall_time = times.wall / (float)1000000000LL;
    result.uncertainty = result.entropy / log2(result.alphabet_size);
	result.success = true;

    return result;
}

QCATSummary QCAT::clientRunSSE() const
{
	if(!this->userCanRun()) return createFailureSummary(whyCantUserRun());
    
	// set up our alphabet hashtable
    std::unordered_map<std::string,QCATLetter> Z;

    // execute QCAT
    const std::string sql = this->sql();
    QCATDBResult rows = m_db->executeSQL(sql);
    int totalRows = 0;

    // add QCAT results to hashtable
    for(int i=0;i<rows->nrows();i++) {
        std::string hash(rows->get(i,"hash"));
        Z[hash].count += 1;
        totalRows++;
    }
	
	const float oneOverTotalRows = 1.0/(float)totalRows;

    // calculate overall entropy of Z
    double HZ = 0;
    double totalSurprise = 0;
    for(auto item: Z) {
        const float prob = item.second.count * oneOverTotalRows; 
        const float log2prob = log2(prob);
        item.second.prob = prob;
        item.second.surprise = -log2prob;

        HZ += prob * log2prob;
        totalSurprise += -log2prob;
    }

    HZ = -HZ;

    // compile results struct
    QCATSummary result;
	result.success = true;
    result.qcatid = m_spec.ID();
	result.message = "Successfully run QCAT.";
    result.entropy = HZ;
    result.surprise_mean = totalSurprise / (float)Z.size();
    result.surprise_stddev = 0; // TODO
    result.alphabet_size = Z.size();
    result.record_length = totalRows;
    result.uncertainty = HZ / log2(result.alphabet_size);
    result.sql_used = sql;
	result.success = true;

    return result;
}

QCATSummary QCAT::clientRun() const
{
	if(!this->userCanRun()) return createFailureSummary(whyCantUserRun());

	// set up our alphabet hashtable
    std::unordered_map<std::string,QCATLetter> Z;

    // execute QCAT
    const std::string sql = this->sql();
    QCATDBResult rows = m_db->executeSQL(sql);
    int totalRows = 0;

    // add QCAT results to hashtable
    for(int i=0;i<rows->nrows();i++) {
        std::string hash(rows->get(i,"hash"));
        Z[hash].count += 1;
        totalRows++;
    }
	
	const float oneOverTotalRows = 1.0/(float)totalRows;

    // calculate overall entropy of Z
    double HZ = 0;
    double totalSurprise = 0;
    for(auto item: Z) {
        const float prob = item.second.count * oneOverTotalRows; 
        const float log2prob = log2(prob);
        item.second.prob = prob;
        item.second.surprise = -log2prob;

        HZ += prob * log2prob;
        totalSurprise += -log2prob;
    }

    HZ = -HZ;

    // compile results struct
    QCATSummary result;
	result.success = true;
	result.qcatid = m_spec.ID();
    result.message = "Successfully run QCAT.";
    result.entropy = HZ;
    result.surprise_mean = totalSurprise / (float)Z.size();
    result.surprise_stddev = 0; // TODO
    result.alphabet_size = Z.size();
    result.record_length = totalRows;
    result.uncertainty = HZ / log2(result.alphabet_size);
    result.sql_used = sql;

    return result;
}

QCATExplanation QCAT::explain(int topn, bool includeColumns) 
{
	if(!this->userCanRun()) {
		auto ex = QCATExplanation();
		ex.summary = createFailureSummary(whyCantUserRun());
		return ex;
	}
	
	QCATExplanation exp;
	exp.records = this->topNMostSurprising(topn, includeColumns);
	exp.summary = run(); 

	return exp;
}

bool surprisal_sorter(QCATRecord i, QCATRecord j) { return (i.surprise < j.surprise); }

/*
std::vector<QCATRecord> QCAT::topNMostSurprising(int n, std::vector<std::string> additionalSelects) const
{
    std::vector<QCATRecord> results;

    // set up our alphabet hashtable
    std::unordered_map<std::string,QCATLetterAndRecord> Z;
    // map from count to the tuple (will sort in-place)
    std::map<int,std::map<std::string,std::string> > lar;

    // execute QCAT
    const std::string sql = this->sql(additionalSelects);

    QCATDBResult rows = m_db->executeSQL(sql);
    int totalRows = 0;

    // add QCAT results to hashtable
    for(int i=0;i<rows->nrows();i++) {
        std::string hash(rows->get(i,"hash"));
        Z[hash].count += 1;
        for(auto it = additionalSelects.begin(); it!=additionalSelects.end();it++)
            Z[hash].attrs[*it] = rows->get(i,it->c_str());
        totalRows++;
    }

    // we can now iterate Z to get most surprising results
    for(auto letter : Z) {
		letter.second.attrs["count"] = boost::lexical_cast<string>(letter.second.count);
        
		QCATRecord rec;
        const float prob = letter.second.count / (float)totalRows;
        const float log2prob = log2(prob);
        rec.surprise = -log2prob;
        rec.attrs = letter.second.attrs;

        results.push_back(rec);
    }

	std::sort(results.begin(), results.end(), surprisal_sorter);

    return std::vector<QCATRecord>(results.rbegin(),results.rbegin()+std::min(n,(int)results.size()));
}*/

QCATSummary QCAT::createFailureSummary(std::string msg) const {
	QCATSummary sum;
	sum.success = false;
	sum.message = msg;
	sum.qcatid = m_spec.ID();
	return sum;
}

QCATSummaryAndSurprisals QCAT::summaryAndSurprisals() const
{
  	if(!userCanRun()) {
		auto sum =  QCATSummaryAndSurprisals("");
		sum.summary = createFailureSummary(whyCantUserRun());
		return sum;
	}

	// set up our alphabet hashtable
    std::unordered_map<std::string,QCATLetter> Z;

    // execute QCAT
    const std::string sql = this->sql();
    QCATDBResult rows = m_db->executeSQL(sql);
    int totalRows = 0;

	std::vector<std::string> rowToHash(rows->nrows());
	std::vector<std::string> rowToID(rows->nrows());

    // add QCAT results to hashtable
    for(int i=0;i<rows->nrows();i++) {
        std::string hash(rows->get(i,"hash"));
        Z[hash].count += 1;
		rowToHash[i] = hash;
		rowToID[i] = rows->get(i,"id");
        totalRows++;
    }
	const float oneOverTotalRows = 1.0/(float)totalRows;

    // calculate overall entropy of Z
    double HZ = 0;
    double totalSurprise = 0;
    for(auto &item: Z) {
        const float prob = item.second.count * oneOverTotalRows; 
        const float log2prob = log2(prob);

		auto& second = item.second;
        second.prob = prob;
        second.surprise = -log2prob;
		
        HZ += prob * log2prob;
        totalSurprise += -log2prob;
    }

    HZ = -HZ;

    // compile results struct
    QCATSummary sum;
	sum.success = true;
	sum.qcatid = m_spec.ID();
    sum.message = "Successfully run QCAT.";
    sum.entropy = HZ;
    sum.surprise_mean = totalSurprise / (float)Z.size();
    sum.surprise_stddev = 0; // TODO
    sum.alphabet_size = Z.size();
    sum.record_length = totalRows;
    sum.uncertainty = HZ / log2(sum.alphabet_size);
    sum.sql_used = sql;

	QCATSummaryAndSurprisals result;
	result.summary = sum;
	result.surprisals = std::vector<std::pair<std::string,double>>(totalRows);

	auto sit = rowToHash.cbegin();
	auto dit = result.surprisals.begin();
	auto idit = rowToID.cbegin();

	for( ; sit != rowToHash.cend(); ++sit, ++dit, ++idit) {
		*dit = std::make_pair(*idit, Z[*sit].surprise);
	}

    return result;
}

std::vector<QCATRecord> QCAT::topNMostSurprising(int n, bool includeColumns) const
{
	if(!userCanRun()) {
		return std::vector<QCATRecord>();
	}

    std::vector<QCATRecord> results;

    // set up our alphabet hashtable
    std::unordered_map<std::string,QCATLetterAndRecord> Z;
    // map from count to the tuple (will sort in-place)
    std::map<int,std::map<std::string,std::string> > lar;

    // execute QCAT
    const std::string sql = this->sql();

    QCATDBResult rows = m_db->executeSQL(sql);
    int totalRows;

    // add QCAT results to hashtable
    for(totalRows = 0; totalRows < rows->nrows(); ++totalRows) {
        std::string h(rows->get(totalRows,"hash"));
        Z[h].count += 1;
		Z[h].record._id = totalRows;
    }

    // we can now iterate Z to get most surprising results
    for(auto letter : Z) {
		const int count = letter.second.count;
        
		QCATRecord rec;
        const float prob = count / (float)totalRows;
        const float log2prob = log2(prob);
        rec.surprise = -log2prob;
		rec.count = count;

		if(includeColumns) {
			// copy columns over
			for(int j=0;j<rows->ncols();j++) {
				rec.vals.push_back(rows->getRecordValue(letter.second.record._id,j,this->m_db.get()));
			}
		}

        results.push_back(rec);
    }

	std::sort(results.begin(), results.end(), surprisal_sorter);

    return std::vector<QCATRecord>(results.rbegin(),results.rbegin()+std::min(n,(int)results.size()));
}

// TODO: inefficient 
QCATExplanation QCAT::rowsMatchingCondition(std::string conditions) const
{
    if(!this->userCanRun()) return QCATExplanation(this->whyCantUserRun()); 

	QCATSummary summary = serverRun();

	/*
	 * run query again on conditionals only but aggregate PMF/surprisal based on full run above
	 */
	std::vector<QCATRecord> results;

    // set up our alphabet hashtable
    std::unordered_map<std::string,QCATLetterAndRecord> Z;
    // map from count to the tuple (will sort in-place)
    std::map<int,std::map<std::string,std::string> > lar;

    // execute QCAT
    const std::string sql = "SELECT * FROM ( " + this->sql() + ") a WHERE " + conditions;

    QCATDBResult rows = m_db->executeSQL(sql);

    // add QCAT results to hashtable
    for(int i = 0; i < rows->nrows(); ++i) {
        std::string h(rows->get(i,"hash"));
        Z[h].count += 1;
		Z[h].record._id = i;
    }

    // we can now iterate Z to get most surprising results
    for(auto letter : Z) {
		const int count = letter.second.count;
        
		QCATRecord rec;
        const float prob = count / (float)summary.record_length;
        const float log2prob = log2(prob);
        rec.surprise = -log2prob;
		rec.count = count;

		results.push_back(rec);
    }

    return QCATExplanation(summary, results);
}

std::string QCAT::escapeQuotes(std::string str)
{
    boost::replace_all(str,"'","''");
    return str;
}

std::string QCAT::commaSepList(std::vector<std::string> lst, bool prefixWithComma)
{
    std::string result = prefixWithComma ? " ," : " ";
    for(string str: lst) {
        result += str + ",";
    }
    return result.substr(0, result.size()-1);
}

std::vector<std::string> QCAT::ensureNoVONClash(std::vector<std::string> lst) const
{
    std::vector<std::string> newlist;
    for(auto item: lst) {
        if(m_vons.find(item) == m_vons.end())
            newlist.push_back(item);
    }
    return newlist;
}

shared_ptr<QCATAttribute> QCAT::attributeForName(std::string name) const
{
	return attributes()[name];
}	

std::string QCAT::sql(std::vector<std::string> additionalSelects, bool where) const
{
	// TODO, ugly hack selecting id explicitly
	std::string sql = "SELECT id, " + sqlVONS() + " , " + sqlVONSHashSelect() + commaSepList(additionalSelects,true) +
		" FROM " + m_db->tableSafe();

	if(where)
		sql += " WHERE " + sqlConditionals();
	
	sql+= sqlLimit();
	return sql;
}

std::string QCAT::sqlVONS() const
{
    std::string str = "";
    for(auto f: m_vons) {
        str += f.second->sqlWhere() + ",";
    }
    return str.substr(0, str.size()-1);
}

std::string QCAT::sqlVONSHashSelect() const
{
    return sqlVONSHash(true);
}

std::string QCAT::sqlVONSHashGroupBy() const
{
    return sqlVONSHash(false);
}

std::string QCAT::sqlVONSHash(bool withAS) const
{
    std::string str;
    std::string as = withAS ? " as hash" : "";
    switch(m_hashType) {
        case fht_string_concat:
            str = "md5(CAST((";
            for(auto f: m_vons) {
                str += f.second->sqlHash() + ",";
            }
            str = str.substr(0,str.size()-1);
            str += ") as text)) " + as;
            return str;
        case fht_int_1:
        case fht_int_2:
            str = "(";
            unsigned long p = 1;
            const int mult = m_hashType == fht_int_1 ? 256 : 256*256;
            for(auto f: m_vons) {
                str += f.second->sqlHash() + " * " + boost::lexical_cast<std::string>(p) + " + ";
                p *= mult;
            }
            str = str.substr(0,str.size()-2);
            str += ") " + as;
            return str;
    }
}

std::string QCAT::sqlConditionals() const
{
    std::string str = "";

    if(m_conditionals.size() == 0)
        return " TRUE ";

    for(auto c: m_conditionals) {
        str += c.second->sql() + " AND ";
    }

    return str.empty() ? "" : str.substr(0, str.size() - 5);
}

std::string QCAT::sqlLimit() const
{
	std::string str = "";
	if(m_limit != -1)
		str = " LIMIT " + boost::lexical_cast<std::string>(m_limit);
	return str;
}

std::string QCAT::sqlServerTableName() const
{
	if(m_limit == -1)
		return m_db->tableSafe();
	else
		return "(SELECT * FROM " + m_db->tableSafe() + " LIMIT " + boost::lexical_cast<std::string>(m_limit) + ") _sstn";
}

void QCAT::clearConditions()
{
    m_spec.removeConditionals();
    syncWithSpec();
}

void QCAT::clearVONs()
{
    m_spec.removeVONs();
    syncWithSpec();
}

std::string QCAT::toString() const
{
    std::string str = "COND ";
    for(auto c: m_conditionals) {
        str += c.second->toString() + ", ";
    }
    str += "VON ";
    for(auto f: m_vons) {
        str += f.first + ", ";
    }
    str = str.substr(0,str.size()-2);
    return str;
}

void QCAT::syncWithSpec()
{
    // vons
    for(auto von: m_spec.vons()) {
        if(m_vons.find(von) == m_vons.end())
            m_vons[von] = make_shared<QCATAttribute>(m_db->fieldForName(von));
    }

    for(auto von: m_vons) {
        if(!m_spec.hasVON(von.first))
            m_vons.erase(von.first);
    }

    // conditionals
    for(auto con: m_spec.conditionals()) {
        if(m_conditionals.find(con) == m_conditionals.end())
            m_conditionals[con].reset(
				new QCATCondition(make_shared<QCATAttribute>(m_db->fieldForName(con)),fop_equal,""));
    }

    for(auto con: m_conditionals) {
        if(!m_spec.hasConditional(con.first))
            m_conditionals.erase(con.first);
    }
}

void QCAT::fixConditional(std::string conditional, QCATOp op, std::string constant)
{
    m_conditionals[conditional].reset(
		new QCATCondition(make_shared<QCATAttribute>(m_db->fieldForName(conditional)),
			op,
			boost::algorithm::trim_copy(constant)));
}

void QCAT::fixConditional(std::string conditional, QCATOp op, std::string constanta, std::string constantb)
{
    m_conditionals[conditional].reset(
		new QCATCondition(make_shared<QCATAttribute>(m_db->fieldForName(conditional)),
			op,
			boost::algorithm::trim_copy(constanta), boost::algorithm::trim_copy(constantb)));
}

void QCAT::fixConditional(std::string conditional, std::string constant)
{
	fixConditional(conditional, fop_equal, constant);
}

void QCAT::fixConditional(std::string conditional, std::string constanta, std::string constantb)
{
	fixConditional(conditional, fop_between, constanta, constantb);
}

void QCAT::removeConditional(shared_ptr<QCATCondition> condition)
{
    m_spec.removeConditional(condition->LHS()->field()->name());
    syncWithSpec();
}

void QCAT::removeVON(shared_ptr<QCATField> field)
{
    m_spec.removeVON(field->name());
    syncWithSpec();
}

void QCAT::addVON(shared_ptr<QCATField> field)
{
    m_spec.addVON(field->name());
    syncWithSpec();
}

void QCAT::addVON(std::string field)
{
    m_spec.addVON(field);
    syncWithSpec();
}

void QCAT::addConditional(shared_ptr<QCATField> field)
{
    m_spec.addConditional(field->name());
    syncWithSpec();
}

map<std::string, shared_ptr<QCATAttribute> > QCAT::attributes() const
{
	auto attrs = vons();
	for(auto c: conditionals())
		attrs[c.first] = c.second->LHS();

	return attrs;
}

/*shared_ptr<QCATCondition> QCAT::conditionalForField(shared_ptr<QCATField> field) const
{
    return m_conditionals.at(field->name());
}

bool QCAT::hasFieldAsCondition(shared_ptr<QCATField> field) const
{
    return m_conditionals.find(field->name()) != m_conditionals.end();
}

bool QCAT::hasFieldAsVON(shared_ptr<QCATField> field) const
{
    for(auto f: m_vons) {
        if(f.second->name() == field->name())
            return true;
    }
    return false;
}

QCATFieldRole QCAT::roleForField(shared_ptr<QCATField> field) const
{
    if(hasFieldAsCondition(field))
        return ffr_cond;
    else if(hasFieldAsVON(field))
        return ffr_von;
    return ffr_none;
}
*/

void QCAT::setLimit(int limit)
{
	m_limit = limit;
}

int QCAT::limit() const
{
	return m_limit;
}

void QCAT::setExecutionMethod(QCATExecutionMethod method)
{
	m_executionMethod = method;
}

QCATExecutionMethod QCAT::executionMethod() const
{
	return m_executionMethod;
}

void QCAT::setServerSP(std::string name, std::string args)
{
	m_serverSPName = name;
	m_serverSPArgs = args;
}

std::string QCAT::serverSPName() const
{
	return m_serverSPName;
}

std::string QCAT::serverSPArgs() const
{
	return m_serverSPArgs;
}
