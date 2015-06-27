#ifndef FACASBINSTATS_H
#define FACASBINSTATS_H

#include <string>
#include <vector>
#include <string>
#include <memory>
using namespace std;
class QCATAttribute;
class QCATDataSource;

class QCATBinSummary
{
public:
    double x1,x2;
    double prob;
    double cnt;
};

class QCATBinStats
{
public:
    QCATBinStats() {}
    QCATBinStats(const QCATAttribute*);

    std::vector<shared_ptr<QCATBinSummary> > bins() { return m_bins; }

private:
    void compile(const QCATAttribute*);
    std::vector<shared_ptr<QCATBinSummary> > m_bins;
};

#endif // FACASBINSTATS_H
