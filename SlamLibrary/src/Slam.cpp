#include "Slam.h"


bool DLL_EXPORT invertBool(bool value)
{
	return !value;
}

void DLL_EXPORT testSlam()
{
	dbg("running Slam test");

	// Grab input args
	const char * dataset = "slam-scans";
	bool use_odometry = true;
	int random_seed = 0;

	// Load the Lidar and odometry data from the file   
	vector<int *> scans;
	vector<long *> odometries;
	
	if (!load_data(dataset, scans, odometries))
	{
		dbg("failed to load data, giving up");

		return;
	}

	dbg("data was loaded successfully");

	// Build a robot model in case we want odometry
	Rover robot = Rover();

	// Create a byte array to receive the computed maps
	unsigned char * mapbytes = new unsigned char[MAP_SIZE_PIXELS * MAP_SIZE_PIXELS];

	// Create SLAM object
	MinesURG04LX laser;
	SinglePositionSLAM * slam = random_seed ?
		(SinglePositionSLAM*)new RMHC_SLAM(laser, MAP_SIZE_PIXELS, MAP_SIZE_METERS, random_seed) :
		(SinglePositionSLAM*)new Deterministic_SLAM(laser, MAP_SIZE_PIXELS, MAP_SIZE_METERS);

	// Report what we're doing
	int nscans = scans.size();
	printf("Processing %d scans with%s odometry / with%s particle filter...\n",
		nscans, use_odometry ? "" : "out", random_seed ? "" : "out");
	ProgressBar * progbar = new ProgressBar(0, nscans, 80);

	// Start with an empty trajectory of positions
	vector<double *> trajectory;

	// Start timing
	time_t start_sec = time(NULL);

	// Loop over scans
	for (int scanno = 0; scanno<nscans; ++scanno)
	{
		int * lidar = scans[scanno];

		// Update with/out odometry
		if (use_odometry)
		{
			Velocities velocities = robot.computeVelocities(odometries[scanno], velocities);
			slam->update(lidar, velocities);
		}
		else
		{
			slam->update(lidar);
		}

		Position position = slam->getpos();

		// Add new coordinates to trajectory
		double * v = new double[2];
		v[0] = position.x_mm;
		v[1] = position.y_mm;
		trajectory.push_back(v);

		// Tame impatience
		progbar->updateAmount(scanno);
		printf("\r%s", progbar->str());
		fflush(stdout);
	}

	// Report speed
	time_t elapsed_sec = time(NULL) - start_sec;
	printf("\n%d scans in %ld seconds = %f scans / sec\n",
		nscans, elapsed_sec, (float)nscans / elapsed_sec);

	// Get final map
	slam->getmap(mapbytes);

	// Put trajectory into map as black pixels
	for (int k = 0; k<(int)trajectory.size(); ++k)
	{
		double * v = trajectory[k];

		int x = mm2pix(v[0]);
		int y = mm2pix(v[1]);

		delete v;

		mapbytes[coords2index(x, y)] = 0;
	}

	dbg("saving map");

	// Save map and trajectory as PGM file    

	char filename[100];
	sprintf(filename, "%s.pgm", dataset);
	printf("\nSaving map to file %s\n", filename);

	FILE * output = fopen(filename, "wt");

	fprintf(output, "P2\n%d %d 255\n", MAP_SIZE_PIXELS, MAP_SIZE_PIXELS);

	for (int y = 0; y<MAP_SIZE_PIXELS; y++)
	{
		for (int x = 0; x<MAP_SIZE_PIXELS; x++)
		{
			fprintf(output, "%d ", mapbytes[coords2index(x, y)]);
		}
		fprintf(output, "\n");
	}

	printf("\n");

	dbg("map saved, cleaning up");

	// Clean up
	for (int scanno = 0; scanno<(int)scans.size(); ++scanno)
	{
		delete scans[scanno];
		delete odometries[scanno];
	}

	if (random_seed)
	{
		delete ((RMHC_SLAM *)slam);
	}
	else
	{
		delete ((Deterministic_SLAM *)slam);
	}

	delete progbar;
	delete mapbytes;
	fclose(output);

	dbg("slam test complete");
}