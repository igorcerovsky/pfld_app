// pfld_app.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "facet.h"
#include "pfld_compute.hpp"
#include "pfld_compute.cpp"
#include "pfld_test_io.h"
#include "pfld_test_io.cpp"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>


using namespace std;


const char* file_facets = "pfld_facets.txt";
const char* file_points = "pfld_points.txt";
const char* file_results = "pfld_results.txt";
const int max_facets_to_generate = 10000;
const int max_facets_to_load = max_facets_to_generate/10;
const int max_points_to_generate = 10000;
const int max_points_to_load = max_points_to_generate;

using facet = pfld::facet;
using point = facet::point;
using ptVec = facet::ptvec;
using valVec = facet::valvec;
using facetVec = facet::facetvec;

#define GEN_FACETS false
#define GEN_POINTS false
#define SAVE_RESULTS


void Compute(void(*FieldFn)(facetVec&, ptVec&, valVec&),
	facetVec& facets, ptVec& fldPoints, valVec& outFld,
	std::string message);

int _tmain(int argc, _TCHAR* argv[])
{
	facetVec facets;
	GetFacets(facets, file_facets, max_facets_to_load, GEN_FACETS);

	ptVec fldPts;
	GetFieldPoints(fldPts, file_points, max_points_to_load, GEN_POINTS);
	
	valVec outFld(fldPts.size(), 0.0);
	void(*FieldFn)(facetVec&, ptVec&, valVec&);
	FieldFn = pfld::Field_Gz;
	Compute(FieldFn, facets, fldPts, outFld, "computing facets with parallel approach...");

	//pfld::valvec outFld2(fldPts.size(), 0.0);
	//FieldFn = pfld::Field_Gz__;
	//Compute(FieldFn, facets, fldPts, outFld2, "computing facets with naive approach...");

	valVec outFld4; // uninitialized for this version
	FieldFn = pfld::Field_Gz_;
	Compute(FieldFn, facets, fldPts, outFld4, "computing facets parallel future approach...");

#ifdef SAVE_RESULTS
	pfld::SaveResults(outFld, file_results);
#endif

	return 0;
}

void Compute(void(*FieldFn)(facetVec&, ptVec&, valVec&),
	facetVec& facets, ptVec& fldPoints, valVec& outFld,
	std::string message)
{
	std::cout << "\n" << message << "\n";
	using namespace std::chrono;
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	FieldFn(facets, fldPoints, outFld);
	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
	std::cout << "computation took me: " << time_span.count() << " seconds." << "\n";
	std::cout << "done." << "\n";
}
