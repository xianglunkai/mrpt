/* +------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)            |
   |                          https://www.mrpt.org/                         |
   |                                                                        |
   | Copyright (c) 2005-2022, Individual contributors, see AUTHORS file     |
   | See: https://www.mrpt.org/Authors - All rights reserved.               |
   | Released under BSD License. See: https://www.mrpt.org/License          |
   +------------------------------------------------------------------------+ */

#include <gtest/gtest.h>
#include <mrpt/config/CConfigFile.h>
#include <mrpt/io/CFileGZInputStream.h>
#include <mrpt/maps/CMultiMetricMap.h>
#include <mrpt/maps/COccupancyGridMap2D.h>
#include <mrpt/maps/CSimpleMap.h>
#include <mrpt/obs/CRawlog.h>
#include <mrpt/random.h>
#include <mrpt/serialization/CArchive.h>
#include <mrpt/slam/CMonteCarloLocalization2D.h>
#include <mrpt/system/filesystem.h>
#include <mrpt/system/os.h>
#include <test_mrpt_common.h>

using namespace mrpt;
using namespace mrpt::bayes;
using namespace mrpt::slam;
using namespace mrpt::maps;
using namespace mrpt::io;
using namespace mrpt::poses;
using namespace mrpt::config;
using namespace mrpt::math;
using namespace mrpt::random;
using namespace mrpt::system;
using namespace mrpt::obs;
using namespace std;

void run_test_pf_localization(CPose2D& meanPose, CMatrixDouble33& cov)
{
	// ------------------------------------------------------
	// The code below is a simplification of the program "pf-localization"
	// ------------------------------------------------------
	const string ini_fil =
		UNITTEST_BASEDIR + string("/tests/montecarlo_test1.ini");
	if (!mrpt::system::fileExists(ini_fil))
	{
		cerr << "WARNING: Skipping test due to missing file: " << ini_fil
			 << "\n";
		return;
	}

	CConfigFile iniFile(ini_fil);
	std::vector<int>
		particles_count;  // Number of initial particles (if size>1, run
	// the experiments N times)

	// Load configuration:
	// -----------------------------------------
	string iniSectionName("LocalizationExperiment");

	// Mandatory entries:
	iniFile.read_vector(
		iniSectionName, "particles_count", std::vector<int>(1, 0),
		particles_count,
		/*Fail if not found*/ true);
	string RAWLOG_FILE = iniFile.read_string(
		iniSectionName, "rawlog_file", "", /*Fail if not found*/ true);

	RAWLOG_FILE = UNITTEST_BASEDIR + string("/") + RAWLOG_FILE;

	// Non-mandatory entries:
	string MAP_FILE = iniFile.read_string(iniSectionName, "map_file", "");

	MAP_FILE = UNITTEST_BASEDIR + string("/") + MAP_FILE;

	size_t rawlog_offset = iniFile.read_int(iniSectionName, "rawlog_offset", 0);
	int NUM_REPS = iniFile.read_int(iniSectionName, "experimentRepetitions", 1);

	// PF-algorithm Options:
	// ---------------------------
	CParticleFilter::TParticleFilterOptions pfOptions;
	pfOptions.loadFromConfigFile(iniFile, "PF_options");

	// PDF Options:
	// ------------------
	TMonteCarloLocalizationParams pdfPredictionOptions;
	pdfPredictionOptions.KLD_params.loadFromConfigFile(iniFile, "KLD_options");

	// Metric map options:
	// -----------------------------
	TSetOfMetricMapInitializers mapList;
	mapList.loadFromConfigFile(iniFile, "MetricMap");

	// --------------------------------------------------------------------
	//						EXPERIMENT PREPARATION
	// --------------------------------------------------------------------
	CTicTac tictac, tictacGlobal;
	CSimpleMap simpleMap;
	CRawlog rawlog;
	size_t rawlogEntry, rawlogEntries;
	CParticleFilter::TParticleFilterStats PF_stats;

	// Load the set of metric maps to consider in the experiments:
	auto metricMap = CMultiMetricMap::Create();
	metricMap->setListOfMaps(mapList);

	getRandomGenerator().randomize();

	// Load the map (if any):
	// -------------------------
	if (MAP_FILE.size())
	{
		ASSERT_(fileExists(MAP_FILE));

		// Detect file extension:
		// -----------------------------
		string mapExt = lowerCase(extractFileExtension(
			MAP_FILE, true));  // Ignore possible .gz extensions

		if (!mapExt.compare("simplemap"))
		{
			// It's a ".simplemap":
			// -------------------------
			CFileGZInputStream f(MAP_FILE);
			mrpt::serialization::archiveFrom(f) >> simpleMap;
			ASSERT_(simpleMap.size() > 0);
			// Build metric map:
			metricMap->loadFromProbabilisticPosesAndObservations(simpleMap);
		}
		else
		{
			THROW_EXCEPTION_FMT(
				"Map file has unknown extension: '%s'", mapExt.c_str());
		}
	}

	// --------------------------
	// Load the rawlog:
	// --------------------------
	rawlog.loadFromRawLogFile(RAWLOG_FILE);
	rawlogEntries = rawlog.size();

	for (int PARTICLE_COUNT : particles_count)
	{
		// Global stats for all the experiment loops:
		vector<double> covergenceErrors;
		covergenceErrors.reserve(NUM_REPS);
		// --------------------------------------------------------------------
		//					EXPERIMENT REPETITIONS LOOP
		// --------------------------------------------------------------------
		tictacGlobal.Tic();
		for (int repetition = 0; repetition < NUM_REPS; repetition++)
		{
			CMonteCarloLocalization2D pdf(PARTICLE_COUNT);

			// PDF Options:
			pdf.options = pdfPredictionOptions;

			pdf.options.metricMap = metricMap;

			// Create the PF object:
			CParticleFilter PF;
			PF.m_options = pfOptions;

			size_t step = 0;
			rawlogEntry = 0;

			// Initialize the PDF:
			// -----------------------------
			tictac.Tic();
			if (!iniFile.read_bool(
					iniSectionName, "init_PDF_mode", false,
					/*Fail if not found*/ true))
			{
				auto grid = metricMap->mapByClass<COccupancyGridMap2D>();
				pdf.resetUniformFreeSpace(
					grid.get(), 0.7f, PARTICLE_COUNT,
					iniFile.read_float(
						iniSectionName, "init_PDF_min_x", 0, true),
					iniFile.read_float(
						iniSectionName, "init_PDF_max_x", 0, true),
					iniFile.read_float(
						iniSectionName, "init_PDF_min_y", 0, true),
					iniFile.read_float(
						iniSectionName, "init_PDF_max_y", 0, true),
					DEG2RAD(iniFile.read_float(
						iniSectionName, "init_PDF_min_phi_deg", -180)),
					DEG2RAD(iniFile.read_float(
						iniSectionName, "init_PDF_max_phi_deg", 180)));
			}
			else
				pdf.resetUniform(
					iniFile.read_float(
						iniSectionName, "init_PDF_min_x", 0, true),
					iniFile.read_float(
						iniSectionName, "init_PDF_max_x", 0, true),
					iniFile.read_float(
						iniSectionName, "init_PDF_min_y", 0, true),
					iniFile.read_float(
						iniSectionName, "init_PDF_max_y", 0, true),
					DEG2RAD(iniFile.read_float(
						iniSectionName, "init_PDF_min_phi_deg", -180)),
					DEG2RAD(iniFile.read_float(
						iniSectionName, "init_PDF_max_phi_deg", 180)),
					PARTICLE_COUNT);

			// -----------------------------
			//		Particle filter
			// -----------------------------
			CActionCollection::Ptr action;
			CSensoryFrame::Ptr observations;
			bool end = false;

			// TTimeStamp cur_obs_timestamp;

			while (rawlogEntry < (rawlogEntries - 1) && !end)
			{
				// Load pose change from the rawlog:
				// ----------------------------------------
				if (!rawlog.getActionObservationPair(
						action, observations, rawlogEntry))
					THROW_EXCEPTION("End of rawlog");

				CPose2D expectedPose;  // Ground truth

				//				if (observations->size()>0)
				//					cur_obs_timestamp =
				// observations->getObservationByIndex(0)->timestamp;

				if (step >= rawlog_offset)
				{
					// Do not execute the PF at "step=0", to let the initial PDF
					// to be
					//   reflected in the logs.
					if (step > rawlog_offset)
					{
						// ----------------------------------------
						// RUN ONE STEP OF THE PARTICLE FILTER:
						// ----------------------------------------
						tictac.Tic();

						PF.executeOn(
							pdf,
							action.get(),  // Action
							observations.get(),	 // Obs.
							&PF_stats  // Output statistics
						);
					}

					const auto [C, M] = pdf.getCovarianceAndMean();
					cov = C;
					meanPose = M;

				}  // end if rawlog_offset

				step++;

			};	// while rawlogEntries
		}  // for repetitions
	}  // end of loop for different # of particles
}

// TEST =================
TEST(MonteCarlo2D, RunSampleDataset)
{
	try
	{
		// Actual ending point:
		const CPose2D GT_endpose(15.904, -10.010, 4.93_deg);

		// Placeholder for results:
		CPose2D meanPose;
		CMatrixDouble33 cov;

		// Invoke test:
		// Give it 3 opportunities, since it might fail once for bad luck, or
		// even twice in an extreme bad luck:
		for (int op = 0; op < 3; op++)
		{
			run_test_pf_localization(meanPose, cov);

			const double final_pf_cov_trace = cov.trace();
			const CPose2D final_pf_pose = meanPose;

			bool pass1 = (final_pf_pose - GT_endpose).norm() < 0.10;
			bool pass2 = final_pf_cov_trace < 0.01;

			if (pass1 && pass2) return;	 // OK!

			// else: give it another try...
			cout << "\n*Warning: Test failed. Will give it another chance, "
					"since "
					"after all it's nondeterministic!\n";
		}

		FAIL() << "Failed to converge after 3 opportunities!!" << endl;
	}
	catch (const std::exception& e)
	{
		FAIL() << mrpt::exception_to_str(e);
	}
}
