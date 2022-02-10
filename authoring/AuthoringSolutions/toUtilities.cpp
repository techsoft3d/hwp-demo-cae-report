#include "toUtilities.h"

void SubVector(const double a[3], const double b[3], double c[3])
{
	for (size_t cI = 0; cI < 3; cI++) {
		c[cI] = a[cI] - b[cI];
	}
}

void CalcCrossProduct(const double a[3], const double b[3], double c[3])
{
	c[0] = (a[1] * b[2]) - (a[2] * b[1]);
	c[1] = (a[2] * b[0]) - (a[0] * b[2]);
	c[2] = (a[0] * b[1]) - (a[1] * b[0]);
}

double CalcVctLeng(const double dVec[3])
{
	return sqrt(dVec[0] * dVec[0] + dVec[1] * dVec[1] + dVec[2] * dVec[2]);
}

bool IsZero(double d, const double dTol = 1e-6)
{
	return fabs(d) < dTol ? true : false;
}

vector<string> split_str(char* szText)
{
	vector<string> sbuf;
	char *ctx;
	char* token = strtok_s(szText, ",", &ctx);
	if (token == NULL) {
		sbuf.push_back(szText);
	}
	else {
		while (token != NULL) {
			sbuf.push_back(token);
			token = strtok_s(NULL, ",", &ctx);
		}
	}
	return sbuf;
}

void CalcColor(double dVal, double dMin, double dMax, double dCol[3])
{
	const double dPi2 = PI * 0.5;
	double dMid = (dMax + dMin) / 2.0;
	double dLen = (dMax - dMin) / 2.0;

	if (dVal > dMid) {
		double d = dPi2 * (dVal - dMid) / dLen;
		dCol[0] = sin(d);
		dCol[1] = cos(d);
		dCol[2] = 0.0;
	}
	else if (dVal < dMid) {
		double d = dPi2 * (dVal - dMin) / dLen;
		dCol[0] = 0.0;
		dCol[1] = sin(d);
		dCol[2] = cos(d);
	}
	else {
		dCol[0] = 0.0;
		dCol[1] = 1.0;
		dCol[2] = 1.0;
	}
}

bool CalcNormal(const double dP1[3], const double dP2[3], const double dP3[3], double dNorm[3])
{
	double dU[3] = { 0.0 };
	double dV[3] = { 0.0 };
	SubVector(dP2, dP1, dU);
	SubVector(dP3, dP1, dV);

	CalcCrossProduct(dU, dV, dNorm);

	double dLen = CalcVctLeng(dNorm);
	if (IsZero(dLen)) {
		return false;
	}

	double d = 1.0 / dLen;
	dNorm[0] *= d;
	dNorm[1] *= d;
	dNorm[2] *= d;

	return true;
}