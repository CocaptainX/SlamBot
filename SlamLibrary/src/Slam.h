#pragma once

#include <iostream>
#include <vector>

using namespace std;

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <fstream> // logging to file

#include <BreezySLAM/Position.hpp>
#include <BreezySLAM/Laser.hpp>
#include <BreezySLAM/WheeledRobot.hpp>
#include <BreezySLAM/Velocities.hpp>
#include <BreezySLAM/algorithms.hpp>

#define LOG_FILENAME "slam.log.txt"

bool isFirstLogMessage = true;

void dbg(string message)
{
	/*
	std::ofstream outfile;

	outfile.open("slam.log.txt", std::ios_base::app);
	outfile << message.c_str();
	*/

	FILE *file = fopen(LOG_FILENAME, isFirstLogMessage ? "w" : "a");

	isFirstLogMessage = false;

	fprintf(file, (message + "\n").c_str());

	fclose(file);
}

// SinglePositionSLAM params: gives us a nice-size map
static const int MAP_SIZE_PIXELS = 800;
static const double MAP_SIZE_METERS = 32;
static const int SCAN_SIZE = 682;

#define MAXLINE 10000 // Arbitrary maximum length of line in input logfile

// Methods to load all data from file ------------------------------------------
// Each line in the file has the format:
//
//  TIMESTAMP  ... Q1  Q1 ... Distances
//  (usec)                    (mm)
//  0          ... 2   3  ... 24 ... 
//  
//where Q1, Q2 are odometry values

static void skiptok(char ** cpp)
{
	*cpp = strtok(NULL, " ");
}

static int nextint(char ** cpp)
{
	skiptok(cpp);

	return atoi(*cpp);
}

static bool load_data(
	const char * dataset,
	vector<int *> & scans,
	vector<long *> & odometries)
{
	char filename[256];

	sprintf(filename, "%s.dat", dataset);
	printf("Loading data from %s ... \n", filename);

	FILE * fp = fopen(filename, "rt");

	if (!fp)
	{
		fprintf(stderr, "Failed to open file\n");
		
		return false;
	}

	char s[MAXLINE];

	while (fgets(s, MAXLINE, fp))
	{
		char * cp = strtok(s, " ");

		long * odometry = new long[3];
		odometry[0] = atol(cp);
		skiptok(&cp);
		odometry[1] = nextint(&cp);
		odometry[2] = nextint(&cp);

		odometries.push_back(odometry);

		// Skip unused fields
		for (int k = 0; k<20; ++k)
		{
			skiptok(&cp);
		}

		int * scanvals = new int[SCAN_SIZE];

		for (int k = 0; k<SCAN_SIZE; ++k)
		{
			scanvals[k] = nextint(&cp);
		}

		scans.push_back(scanvals);
	}

	fclose(fp);

	return true;
}

// Class for Mines verison of URG-04LX Lidar -----------------------------------

class MinesURG04LX : public URG04LX
{

public:

	MinesURG04LX(void) : URG04LX(
		70,          // detectionMargin
		145)         // offsetMillimeters
	{}
};

// Class for MinesRover custom robot -------------------------------------------

class Rover : WheeledRobot
{

public:

	Rover() : WheeledRobot(
		77,     // wheelRadiusMillimeters
		165)     // halfAxleLengthMillimeters
	{}

	Velocities computeVelocities(long * odometry, Velocities & velocities)
	{
		return WheeledRobot::computeVelocities(
			odometry[0],
			odometry[1],
			odometry[2]);
	}

protected:

	void extractOdometry(
		double timestamp,
		double leftWheelOdometry,
		double rightWheelOdometry,
		double & timestampSeconds,
		double & leftWheelDegrees,
		double & rightWheelDegrees)
	{
		// Convert microseconds to seconds, ticks to angles        
		timestampSeconds = timestamp / 1e6;
		leftWheelDegrees = ticksToDegrees(leftWheelOdometry);
		rightWheelDegrees = ticksToDegrees(rightWheelOdometry);
	}

	void descriptorString(char * str)
	{
		sprintf(str, "ticks_per_cycle=%d", this->TICKS_PER_CYCLE);
	}

private:

	double ticksToDegrees(double ticks)
	{
		return ticks * (180. / this->TICKS_PER_CYCLE);
	}

	static const int TICKS_PER_CYCLE = 2000;
};


// Progress-bar class
// Adapted from http://code.activestate.com/recipes/168639-progress-bar-class/
// Downloaded 12 January 2014

class ProgressBar
{
public:

	ProgressBar(int minValue, int maxValue, int totalWidth)
	{
		strcpy(this->progBar, "[]");   // This holds the progress bar string
		this->min = minValue;
		this->max = maxValue;
		this->span = maxValue - minValue;
		this->width = totalWidth;
		this->amount = 0;       // When amount == max, we are 100% done 
		this->updateAmount(0);  // Build progress bar string
	}

	void updateAmount(int newAmount)
	{
		if (newAmount < this->min)
		{
			newAmount = this->min;
		}
		if (newAmount > this->max)
		{
			newAmount = this->max;
		}

		this->amount = newAmount;

		// Figure out the new percent done, round to an integer
		float diffFromMin = float(this->amount - this->min);
		int percentDone = (int)round((diffFromMin / float(this->span)) * 100.0);

		// Figure out how many hash bars the percentage should be
		int allFull = this->width - 2;
		int numHashes = (int)round((percentDone / 100.0) * allFull);


		// Build a progress bar with hashes and spaces
		strcpy(this->progBar, "[");
		this->addToProgBar("#", numHashes);
		this->addToProgBar(" ", allFull - numHashes);
		strcat(this->progBar, "]");

		// Figure out where to put the percentage, roughly centered
		int percentPlace = (strlen(this->progBar) / 2) - ((int)(log10(percentDone + 1)) + 1);
		char percentString[5];
		sprintf(percentString, "%d%%", percentDone);

		// Put it there
		for (int k = 0; k<strlen(percentString); ++k)
		{
			this->progBar[percentPlace + k] = percentString[k];
		}

	}

	char * str()
	{
		return this->progBar;
	}

private:

	char progBar[1000]; // more than we should ever need
	int min;
	int max;
	int span;
	int width;
	int amount;

	void addToProgBar(const char * s, int n)
	{
		for (int k = 0; k<n; ++k)
		{
			strcat(this->progBar, s);
		}
	}
};

// Helpers ----------------------------------------------------------------

int coords2index(double x, double y)
{
	return y * MAP_SIZE_PIXELS + x;
}


int mm2pix(double mm)
{
	return (int)(mm / (MAP_SIZE_METERS * 1000. / MAP_SIZE_PIXELS));
}


#define DLL_EXPORT __declspec(dllexport)

#ifdef __cplusplus
extern "C"
{
#endif

	bool DLL_EXPORT invertBool(bool value);
	void DLL_EXPORT testSlam();

#ifdef __cplusplus
}
#endif