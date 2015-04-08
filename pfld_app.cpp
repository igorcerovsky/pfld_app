// pfld_app.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "facet.hpp"
#include "pfld_compute.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;

const char* file_facets = "facets.txt";
const int max_facets_to_load = 10000;

int _tmain(int argc, _TCHAR* argv[])
{
	using point = pfld::Point3D < double >;
	using ptvec = pfld::ptvec;
	using facet_vec = pfld::facet_vec;
	using facet = pfld::Facet;

	facet_vec facets;
//#define CREATE_RANDOM_FACETS
#ifdef CREATE_RANDOM_FACETS
	std::cout << "creating random facets..." << "\n";
	std::srand(1);
	const int nf = 1000000;
	for (int i = 0; i < nf; ++i)
	{
		ptvec v{ point(rand() % 1001, rand() % 1001, rand() % 1001), 
			point(rand() % 1001, rand() % 1001, rand() % 1001), 
			point(rand() % 1001, rand() % 1001, rand() % 1001) };
		facets.emplace_back( facet(v) );
		if( (i%1000) == 0 )
			std::cout << "\r" << i << " of " << nf;
	}

	ofstream file(file_facets, ios::out);
	if (file.is_open())
	{
		for (auto it = facets.begin(); it != facets.end(); ++it)
		{
			ptvec& fpts = it->Data();
			for (auto itp = fpts.begin(); itp != fpts.end(); ++itp)
				file << itp->x << " " << itp->y << " " << itp->z << "\n";
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
		int i = 0;
		while (std::getline(file, stmp))
		{
			std::istringstream buffer(stmp);
			std::vector<double> vln{ std::istream_iterator<double>(buffer),
				std::istream_iterator<double>() };
			if (i % 3 == 0 && i!=0 )	{
				facets.emplace_back(facet(v));
				if (i / 3 >= max_facets_to_load)
					break;
			}
			v[i % 3] = point(vln[0], vln[1], vln[2]);
			++i;
		}
		file.close();
	}

#endif

	std::cout << "\n" << "initialize field points..." << "\n";
	const int n = 10000;
	pfld::ptvec field_points;
	pfld::valvec out_field(n, 0.0);
	for (int i = 0; i < n; ++i)
	{
		field_points.emplace_back(point(rand() % 100 - 50, rand() % 100 - 50, rand() % 100 - 50));
		if ((i % 1000) == 0)
			std::cout << "\r" << i << " of " << n;
	}

	std::cout << "\n" << "initializing facets... ";
	Field_Gz(facets, field_points, out_field);
	std::cout << "done." << "\n";

	return 0;
}

