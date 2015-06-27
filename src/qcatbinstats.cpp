#include "qcatbinstats.h"
#include "qcatfield.h"
#include "qcatattribute.h"
#include "qcatbin.h"
#include "qcatdatasource.h"

QCATBinStats::QCATBinStats(const QCATAttribute* attr)
{
    compile(attr);
}

void QCATBinStats::compile(const QCATAttribute* attr)
{
	auto db = attr->field()->db();
	auto bin = attr->bin();
	auto field = attr->field();

    m_bins.clear();

    std::string sql = "SELECT cnt, x1, x2 FROM ( SELECT MIN(" + attr->name() + ") as x1, MAX(" + attr->name() + ") as x2, COUNT(id) AS cnt, " + bin->sqlAttrToBin(field->name()) +
            "AS bin FROM " + db->table() + " GROUP BY "+ bin->sqlAttrToBin(field->name()) +
            ") a ORDER BY bin";

    auto result = db->executeSQL(sql);
    bool numeric = field->type() == fft_integer || field->type() == fft_double;

    double total = 0;
    int nonNumericCounter = 0;
    for(int i=0;i<result->nrows();i++) {
        double cnt = result->getDouble(i,"cnt");
        double x1,x2;
        if(numeric) {
			x1 = result->getDouble(i,"x1");
			x2 = result->getDouble(i,"x2");
        }
        else {
            x1 = nonNumericCounter++;
            x2 = nonNumericCounter++;
        }

        auto b = shared_ptr<QCATBinSummary>(new QCATBinSummary());
        b->prob = cnt;
        b->x1 = x1;
        b->x2 = x2;
        b->cnt = cnt;
        total += cnt;
        m_bins.push_back(b);
    }

    for(auto item: m_bins) {
        item->prob /= total;
    }
}
