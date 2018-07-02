#include <vector>
#include <string>

#include <Python.h>
#include "SalienceFiveAPI.h"

using namespace std;

typedef struct {
    PyObject_HEAD
	SalienceSession* pSalienceSession;
	int nLastError;
	int nCalculateConfidence;
	int nEntityThreshold;
	int nEntityTimeout;
	int nLastWarning;
	LexalyticsLicense pLicense;
	PyObject* oCallback;
} lxa_SalienceObject;







