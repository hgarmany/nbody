#include "util.h"
#include <cstdlib>
#include <cctype>

size_t getIntsFromString(const char* string, int* out, size_t n, char dem) {
	size_t i = 0;

	while (i < n && *string) {
		if (isdigit(*string) || *string == '-') {
			out[i++] = atoi(string); // append integer to array
			while (*++string && *string != dem); // advance to next demarcator
		}
		string++;
	}

	return i;
}

size_t getFloatsFromString(const char* string, float* out, size_t n, char dem) {
	size_t i = 0;

	while (i < n && *string) {
		if (isdigit(*string) || *string == '-') {
			out[i++] = (float)atof(string); // append float to array
			while (*++string && *string != dem); // advance to next demarcator
		}
		string++;
	}

	return i;
}

double ellipsePerimeter(double semiMajorAxis, float eccentricity) {
	double semiMinorAxis = semiMajorAxis * sqrt(1 - eccentricity * eccentricity);

	// Ramanujan's first approximation
	return pi * (3 * (semiMajorAxis + semiMinorAxis) 
		- sqrt((3 * semiMajorAxis + semiMinorAxis) * (semiMajorAxis + 3 * semiMinorAxis)));
}