#pragma once
#include <vector>
#include <iostream>

using namespace std;

#define PI 3.14159265358979323846

vector<string> split_str(char* szText);
void CalcColor(double dVal, double dMin, double dMax, double dCol[3]);
bool CalcNormal(const double dP1[3], const double dP2[3], const double dP3[3], double dNorm[3]);