#ifndef FACASFIELDSTATS_H
#define FACASFIELDSTATS_H

#include <string>

class QCATField;
class QCATDataSource;

class QCATFieldStatResult
{
public:
    double numeric;
    std::string text;
    bool reliable_numeric;

    /*!
    * Constructor takes a string representation of the statistic and performs conversion to double type if possible
    */
    QCATFieldStatResult(std::string);
    QCATFieldStatResult() { init(); }
    void init() {
        numeric = 0;
        reliable_numeric = false;
    }

    /*!
     * \brief Allows for QCATFieldStatResult to be printed to output streams (calls toString())
     */
    friend std::ostream& operator<<(std::ostream &strm, const QCATFieldStatResult &f) {
        return strm << f.toString();
    }

    std::string toString() const {
        return text;
    }
};

class QCATFieldStats
{
public:
    QCATFieldStats();
    QCATFieldStats(const QCATField*, const QCATDataSource* db);

    QCATFieldStatResult min() const { return m_min; }
    QCATFieldStatResult max() const { return m_max; }
    QCATFieldStatResult stddev() const { return m_stddev; }
    QCATFieldStatResult avg() const { return m_avg; }

    double range() const { return m_max.numeric-m_min.numeric; }
    long unique() const { return m_unique; }
	QCATField* field() const { return m_field; }

    std::string special() const { return m_special; }

private:
    bool cacheAcceptable(const QCATField*, const QCATDataSource* db) const;
    void compileFromCache(const QCATField*, const QCATDataSource* db);
    void saveToCache(const QCATField*, const QCATDataSource* db);
    void ensureCache(const QCATField*, const QCATDataSource* db);

    void compileStats(const QCATField*, const QCATDataSource* db);
    QCATFieldStatResult runStat(std::string sql, const QCATDataSource *db) const;

    template<class T>T executeSingleShot(std::string str, const QCATDataSource *db, bool* success) const;
    std::string executeToCommaSepList(std::string str, const QCATDataSource* db) const;

    QCATFieldStatResult m_min, m_max, m_avg, m_stddev;
    long m_unique;
    std::string m_special;
	QCATField* m_field;
};

template<>
std::string QCATFieldStats::executeSingleShot<std::string>(std::string str, const QCATDataSource *db, bool *success) const;

#endif // FACASFIELDSTATS_H
