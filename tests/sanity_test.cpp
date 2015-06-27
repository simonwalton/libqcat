
#include <iostream>
#include "../qcatdatasource.h"
#include "../qcat.h"
#include <boost/assign/list_of.hpp>
#include <time.h>
#include <algorithm>
#include <boost/timer/timer.hpp>

using namespace std;
using namespace boost::assign;
using namespace boost::timer;

#define CONNSTR "dbname=flight_database user=postgres password=duke3d"
#define TABLE "facas_simple_test"
#define TARGET_TOLERANCE 0.001
#define TARGET_MEAN_SURPRISE 3.66299367

auto db = make_shared<QCATDataSource>(CONNSTR, TABLE);

std::string result_str(bool result) 
{
	return result ? "SUCCESS" : "FAILURE"; 
}

void output_test_result(std::string name, bool result)
{
	std::cout << "Test: " << name << " : " << result_str(result) << std::endl;
}

bool float_near(float val, float target, float tolerance)
{
	return fabs(val-target) <= tolerance;
}

int main()
{
	cout << "----------------" << endl;
	cout << "QCAT Sanity Test" << endl;
	cout << "----------------" << endl;

	QCATSpec spec("Sanity QCAT");
	spec.add("c",ffr_cond);
	spec.add("a",ffr_von);
	spec.add("b",ffr_von);
	QCAT c(spec, db);
	c.fixConditional("c","y");

	boost::timer::auto_cpu_timer cpu;

	auto result = c();
	std::cout << result << endl;

	output_test_result("Mean surprise", float_near(result.surprise_mean, TARGET_MEAN_SURPRISE, TARGET_TOLERANCE));
}
