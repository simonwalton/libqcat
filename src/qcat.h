#ifndef FACAS_H
#define FACAS_H

#include <vector>
#include <list>
#include <set>
#include <unordered_map>
#include <boost/lexical_cast.hpp>

#include <math.h>
#include <iostream>
#include <memory>
#include <new>

using namespace std;

//#include "libpq-fe.h" 
#include "qcatcondition.h"
#include "qcatfield.h"

class QCAT;

enum QCATExecutionMethod {
	fem_client = 0,
	fem_server = 1
};

/*!
 * \brief A letter in the alphabet Z
 */
struct QCATLetter {
    QCATLetter() {
        prob = 0;
        surprise = 0;
        count = 0;
    }

    float prob;
    float surprise;
    int count;
};

struct QCATLetterBinStat
{
	QCATLetterBinStat() {
		count = 0;
		prob = 0;
		surprise = 0;
	}
	int count;
	double prob;
	double surprise;
	std::string valdesc;
};

struct QCATSummaryHeatmap
{
	QCATSummaryHeatmap() {}
	std::vector<std::string> fields;
	map<std::string,std::vector<double> > fieldbinvals;
	shared_ptr<double> heatmap;
	int heatmapSize;
};

struct QCATLetterWithMetadata {
	QCATLetterWithMetadata() {
		count = prob = surprise = 0;
	}
	int count;
	double prob, surprise;
	map<std::string,double> fieldToBinnedVal; // for each field, its last bin value
};

struct QCATRecordValue {
	double v_double;
	std::string v_string;
};

/*!
* An individual qcat record with attributes copied from DB results
*/
struct QCATRecord {
public:
	int _id;
    std::vector<QCATRecordValue> vals;
    float surprise;
	float surprise_factor;
	float count;
};

/*!
 * A summary of an executed QCAT
*/
class QCATSummary {
public:
    float entropy;
    float surprise_mean;
    float surprise_stddev;

	std::string qcatid;
    float uncertainty;
    unsigned int alphabet_size;
    unsigned int record_length;
    std::string sql_used;
    std::string message;
    float wall_time;

	bool success;

//	unordered_map<std::string,std::string> attrs;

    QCATSummary() {
        init();
    }

    QCATSummary(std::string message) {
        init();
        this->message = message;
    }

    void init() {
        wall_time = 0;
        entropy = 0;
        surprise_mean = 0;
        surprise_stddev = 0;
        uncertainty = 0;
        alphabet_size = 0;
        record_length = 0;
		success = false;
    }

    /*!
     * \brief Allows for QCATResult to be printed to output streams (calls toString())
     */
    friend std::ostream& operator<<(std::ostream &strm, const QCATSummary &f) {
        return strm << f.toString();
    }

    std::string toString() const {
        return std::string("Entropy ") + boost::lexical_cast<std::string>(entropy) +
            ", Surprise (mean: " + boost::lexical_cast<std::string>(surprise_mean) +
            " StdDev: " + boost::lexical_cast<std::string>(surprise_stddev) +
            " Variance: " + boost::lexical_cast<std::string>(surprise_stddev*surprise_stddev) +
            " Uncertainty: " + boost::lexical_cast<std::string>(uncertainty) +
            "), Z-size " + boost::lexical_cast<std::string>(alphabet_size) +
            " From " + boost::lexical_cast<std::string>(record_length) + " records.";
    }

    /*!
     * \brief Returns whether this QCAT result is valid or not based on whether a nonzero number of records created it
     */
    bool isValid() const {
        return record_length != 0;
    }

    QCATSummary operator+ (const QCATSummary &a){
        QCATSummary r;
        r.entropy = a.entropy + entropy;
        r.surprise_mean = a.surprise_mean + surprise_mean;
        r.surprise_stddev = a.surprise_stddev + surprise_stddev;
        r.uncertainty = a.uncertainty + uncertainty;
        return r;
    }

    QCATSummary operator/ (const double &v){
        QCATSummary r;
        r.entropy = entropy / v;
        r.surprise_mean = surprise_mean / v ;
        r.surprise_stddev = surprise_stddev / v;
        r.uncertainty = uncertainty / v;
        return r;
    }

    static const QCATSummary elementWiseMin(const QCATSummary &a, const QCATSummary &b) {
        QCATSummary r;
        r.entropy = std::min(a.entropy, b.entropy);
        r.surprise_mean =  std::min(a.surprise_mean, b.surprise_mean);
        r.surprise_stddev =  std::min(a.surprise_stddev, b.surprise_stddev);
        r.uncertainty =  std::min(a.uncertainty, b.uncertainty);
        return r;
    }

    static const QCATSummary elementWiseMax(const QCATSummary &a, const QCATSummary &b) {
        QCATSummary r;
        r.entropy = std::max(a.entropy, b.entropy);
        r.surprise_mean =  std::max(a.surprise_mean, b.surprise_mean);
        r.surprise_stddev =  std::max(a.surprise_stddev, b.surprise_stddev);
        r.uncertainty =  std::max(a.uncertainty, b.uncertainty);
        return r;
    }

    static QCATSummary infQCAT() {
        QCATSummary r;
        r.entropy = INFINITY;
        r.surprise_mean = INFINITY;
        r.surprise_stddev = INFINITY;
        r.uncertainty = INFINITY;
        return r;
    }
};

struct QCATExplanation
{
	QCATExplanation() {}
	QCATExplanation(std::string message) {
		summary = QCATSummary(message);
	}
	QCATExplanation(QCATSummary s)
		:summary(s) {}
	QCATExplanation(QCATSummary s, std::vector<QCATRecord> r)
		:summary(s), records(r) {}

	QCATSummary summary;
	std::vector<QCATRecord> records;
};

class QCATSummaryAndSurprisals
{
public:
	QCATSummaryAndSurprisals() {}
	QCATSummaryAndSurprisals(std::string message) {
		summary.message = message;
	}
	QCATSummary summary;
	std::vector<std::pair<std::string,double>> surprisals;
};

enum QCATHashType {
    fht_string_concat = 0,
    fht_int_1 = 1,				// 1 byte integer
    fht_int_2 = 2				// 2 byte integer
};

enum QCATFieldRole {
    ffr_none = 0,
    ffr_cond = 1,
    ffr_von = 2
};

/*!
 * \brief Represents a simple QCAT specification
 */
class QCATSpec
{
public:
    QCATSpec(std::string id = "0", std::string name = "Untitled QCAT") {
		m_id = id;
		m_name = name;
	}

    void addConditional(std::string name) { m_conditionals.insert(name); }
    void addVON(std::string name) { m_vons.insert(name); }
	void add(std::string name, QCATFieldRole role) {
		if(role == ffr_cond)
			addConditional(name);
		else if(role == ffr_von)
			addVON(name);
	}

    void removeConditional(std::string name) { m_conditionals.erase(name); }
    void removeVON(std::string name) { m_vons.erase(name); }

    void removeConditionals() { m_conditionals.clear(); }
    void removeVONs() { m_vons.clear(); }

    bool hasVON(std::string name) const { return m_vons.find(name) !=  m_vons.end(); }
    bool hasConditional(std::string name) const { return m_conditionals.find(name) !=  m_conditionals.end(); }

    void setName(std::string name) { m_name = name; }
    std::string name() const { return m_name; }

  	void setID(std::string id) { m_id = id; }
    std::string ID() const { return m_id; }

    std::vector<std::string> conditionals() const { return std::vector<std::string>(m_conditionals.begin(),m_conditionals.end()); }
    std::vector<std::string> vons() const { return std::vector<std::string>(m_vons.begin(),m_vons.end()); }

private:
    std::set<std::string> m_conditionals;
    std::set<std::string> m_vons;
    std::string m_name, m_id;
};


/*!
 * \brief Represents a QCAT instantiation with conditionals set to constant values
 */
class QCAT
{
public:
    QCAT(QCATSpec spec, shared_ptr<QCATDataSource> db);
    QCAT(shared_ptr<QCATDataSource> db);

    void setSpec(QCATSpec);
    QCATSpec spec() { return m_spec; }

	QCATSummary createFailureSummary(std::string message) const;

    /*!
    * For a conditional in the spec, fix its operator / value
    */
    void fixConditional(std::string,QCATOp,std::string);
    void fixConditional(std::string,QCATOp,std::string,std::string);

	/*!
	 * For a conditional in the spec, fix its value, assuming fop_equal
	 */
    void fixConditional(std::string, std::string);
	/*!
	 * For a conditional in the spec, fix its range, assuming fop_between
	 */
    void fixConditional(std::string, std::string, std::string);

    map<std::string, shared_ptr<QCATCondition> > conditionals() const { return m_conditionals;  }
    map<std::string, shared_ptr<QCATAttribute> > vons() const { return m_vons;  }
	map<std::string, shared_ptr<QCATAttribute> > attributes() const;
	
    void addVON(shared_ptr<QCATField>);
    void addConditional(shared_ptr<QCATField>);
    void addVON(std::string name);

    void removeConditional(shared_ptr<QCATCondition>);
    void removeVON(shared_ptr<QCATField>);

    void clearConditions();
    void clearVONs();

	void setID(std::string name) { m_spec.setID(name); }
    std::string id() const { return m_spec.ID(); }

    void setName(std::string name) { m_spec.setName(name); }
    std::string name() const { return m_spec.name(); }

    shared_ptr<QCATDataSource> db() const { return m_db; }

	/*!
	 * \brief Applies a 'global' bin strategy to all attributes
	 * \param bs Bin Strategy object
	 * \param override_existing If true, overrides all existing bin strategies; if false, applies bin strategy only to attributes that do not currently have a bin strategy
	 */
	void setBinStrategy(shared_ptr<QCATBinStrategy> bs, bool override_existing = false);

	/*!
	 * \return The current global bin strategy
	 */
	shared_ptr<QCATBinStrategy> binStrategy() const;

    /*!
     * \brief Run this QCAT in its current state
     * \return
     */
    QCATSummary operator()() const;

	/*!
	 * \brief Run this QCAT in its current state
	 */
	QCATSummary execute() const;

  	/*!
     * \brief Provides the top N most surprising results, sorted. Provides all columns
     */
    std::vector<QCATRecord> topNMostSurprising(int n = 100, bool includeColumns = true) const;
  	
	/*!
     * \brief Provides the resulting matching an SQL condition string 
     */
    QCATExplanation rowsMatchingCondition(std::string conditions) const;


	/*!
	 * \brief Explain the most surprising results
	 */
	QCATExplanation explain(int topn = 100, bool includeColumns = true);

	/*!
	 * \brief Provides a summary and a list of surprisal values corresponding exactly to the rows in the data
	 */
	QCATSummaryAndSurprisals summaryAndSurprisals() const;

    /*!
     * \brief Provides a nice readable description of this QCAT
     */
    std::string toString() const;

    /*!
     * \brief Allows for QCAT to be printed to output streams (calls toString())
     */
    friend std::ostream& operator<<(std::ostream &strm, const QCAT &f) {
        return strm << f.toString();
    }

    /*!
     * \brief The SQL that runs this QCAT.
     */
    std::string sql(std::vector<std::string> additionalSelects = std::vector<std::string>(), bool where = true) const;

    /*!
     * \brief isComplete
     * \param why Pointer to string that we put error/success message into 
     * \return true if the QCAT is complete enough to run
     */
    bool isComplete(std::string* why = NULL) const;

	/*!
	 * \brief userCanRun
	 * \param similar to isComplete but prints error
	 */
	bool userCanRun() const;
	std::string whyCantUserRun() const;

    bool hasAttrAsCondition(shared_ptr<QCATField> name) const;
    bool hasFieldAsVON(shared_ptr<QCATField> name) const;
    QCATFieldRole roleForField(shared_ptr<QCATField> name) const;
	shared_ptr<QCATAttribute> attributeForName(std::string name) const;

    /*!
     * \return Provides a string description for a given field's role
     */
    std::string roleStrForField(shared_ptr<QCATField> field) {
        switch(roleForField(field)) {
            case ffr_none: return "None";
            case ffr_cond: return "Conditional";
            case ffr_von: return "VON";
            default: return "?";
        }
    }

	/*!
	 * Set a LIMIT when selecting rows for evaluation (useful for testing)
	 * \param limit Number of rows to limit statements to
	 */
	void setLimit(int limit);

	/*!
	 * \return The current LIMIT (defaults to no limit)
	 */
	int limit() const;

	void setExecutionMethod(QCATExecutionMethod);
	QCATExecutionMethod executionMethod() const;

	void setServerSP(std::string name, std::string args);
	std::string serverSPName() const;
	std::string serverSPArgs() const;

    static std::string commaSepList(std::vector<std::string> lst, bool prefixWithComma = true);

    std::string sqlVONS() const;
    std::string sqlVONSHash(bool withAS = true) const;
    std::string sqlVONSHashSelect() const;
    std::string sqlVONSHashGroupBy() const;
    std::string sqlConditionals() const;
	std::string sqlLimit() const;
	std::string sqlServerTableName() const;

private:
    void syncWithSpec();
	void init();
	void initialiseBinsFromStrategy(bool override_existing);

    static std::string escapeQuotes(std::string);
    std::vector<std::string> ensureNoVONClash(std::vector<std::string>) const;

    QCATSummary clientRun() const;
    QCATSummary clientRunSSE() const;
    QCATSummary serverRun() const;
	QCATSummary run() const;

    std::map<std::string, shared_ptr<QCATCondition> > m_conditionals;
    std::map<std::string, shared_ptr<QCATAttribute> > m_vons;

	int m_limit;
	QCATExecutionMethod m_executionMethod;
	shared_ptr<QCATBinStrategy> m_binStrategy;
	std::string m_serverSPName, m_serverSPArgs;

    shared_ptr<QCATDataSource> m_db;
    QCATSpec m_spec;
    QCATHashType m_hashType;
};

#endif // FACAS_H
