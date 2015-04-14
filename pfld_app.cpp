// pfld_app.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "facet.hpp"
#include "pfld_compute.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>


using namespace std;

const char* file_facets = "facets.txt";
const int max_facets_to_generate = 10000;
const int max_facets_to_load = max_facets_to_generate/1-5;
const int max_points = 10000;


using point = pfld::point;
using ptvec = pfld::ptvec;
using facet_vec = pfld::facet_vec;
using facet = pfld::Facet;

void GetFacets(pfld::facet_vec& facets);
void GetFieldPoints(ptvec& points, const int n);
void Compute(void(*FieldFn)(pfld::facet_vec&, pfld::ptvec&, pfld::valvec&),
	pfld::facet_vec& facets, pfld::ptvec& fldPoints, pfld::valvec& outFld,
	std::string message);

int _tmain(int argc, _TCHAR* argv[])
{
	facet_vec facets;
	GetFacets(facets);

	std::cout << "\n" << "initialize field points..." << "\n";
	ptvec fldPts;
	GetFieldPoints(fldPts, max_points);
	
	pfld::valvec outFld(max_points, 0.0);
	void(*FieldFn)(pfld::facet_vec&, pfld::ptvec&, pfld::valvec&);
	FieldFn = pfld::Field_Gz;
	Compute(FieldFn, facets, fldPts, outFld, "computing facets with parallel approach...");

	//pfld::valvec outFld2(max_points, 0.0);
	//FieldFn = pfld::Field_Gz__;
	//Compute(FieldFn, facets, fldPts, outFld2, "computing facets with naive approach...");

	//pfld::valvec outFld3(max_points, 0.0);
	//FieldFn = pfld::Field_Gz_fcs;
	//Compute(FieldFn, facets, fldPts, outFld3, "computing facets with facets division...");

	pfld::valvec outFld4; // uninitialized for this version
	FieldFn = pfld::Field_Gz_;
	Compute(FieldFn, facets, fldPts, outFld4, "computing facets parallel future approach...");

	return 0;
}


void GetFieldPoints(ptvec& points, const int n)
{
	std::srand(100);
	for (int i = 0; i < n; ++i)
	{
		points.emplace_back(point(rand() % 4000 - 2000, rand() % 40000 - 2000, rand() % 200));
	}
}

void GetFacets(pfld::facet_vec& facets)
{
//#define CREATE_RANDOM_FACETS
#ifdef CREATE_RANDOM_FACETS
	std::cout << "creating random facets..." << "\n";
	std::srand(1);
	for (int i = 0; i < max_facets_to_generate; ++i)
	{
		ptvec v{ point(rand() % 1001, rand() % 1001, -(rand() % 1001)),
			point(rand() % 1001, rand() % 1001, -rand() % 1001),
			point(rand() % 1001, rand() % 1001, -rand() % 1001) };
		facets.emplace_back(facet(v));
		if ((i % 1000) == 0)
			std::cout << "\r" << i << " of " << max_facets_to_generate;
	}

	ofstream file(file_facets, ios::out);
	if (file.is_open())
	{
		int i = 0;
		for (auto it = facets.begin(); it != facets.end(); ++it)
		{
			ptvec& fpts = it->Data();
			file << "Facet " << i << " " << fpts.size() << "\n";
			for (auto itp = fpts.begin(); itp != fpts.end(); ++itp)
				file << itp->x << " " << itp->y << " " << itp->z << " ";
			file << "\n";
			i++;
		}
		file.close();
	}
#else
	std::cout << "loading facets..." << "\n";
	ifstream file(file_facets, std::ios_base::in);
	if (file.is_open())
	{
		std::string stmp;
		ptvec v(3);
		int loadedFacets = 0;
		while (std::getline(file, stmp))
		{
			std::istringstream buffer(stmp);
			if (stmp.find("Facet") != string::npos)
			{
				// facet data
			}
			else
			{
				std::vector<double> vln{ std::istream_iterator<double>(buffer),
					std::istream_iterator<double>() };
				for (unsigned i = 0; i < vln.size() / 3; ++i)	{
					v[i] = point(vln[i * 3 + 0], vln[i * 3 + 1], vln[i * 3 + 2]);
				}
				facets.emplace_back(facet(v));
				loadedFacets++;
				if (loadedFacets >= max_facets_to_load)
					break;
			}
		}
		file.close();
	}
	std::cout << "loaded " << facets.size() << " facets." << "\n";
#endif
}

void Compute(void(*FieldFn)(pfld::facet_vec&, pfld::ptvec&, pfld::valvec&),
	pfld::facet_vec& facets, pfld::ptvec& fldPoints, pfld::valvec& outFld,
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
