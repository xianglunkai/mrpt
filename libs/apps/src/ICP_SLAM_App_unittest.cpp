/* +------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)            |
   |                          https://www.mrpt.org/                         |
   |                                                                        |
   | Copyright (c) 2005-2019, Individual contributors, see AUTHORS file     |
   | See: https://www.mrpt.org/Authors - All rights reserved.               |
   | Released under BSD License. See: https://www.mrpt.org/License          |
   +------------------------------------------------------------------------+ */

#include <gtest/gtest.h>
#include <mrpt/apps/ICP_SLAM_App.h>
#include <mrpt/system/filesystem.h>
#include <test_mrpt_common.h>
#include <iostream>

using config_changer_t = std::function<void(mrpt::config::CConfigFileBase&)>;
using post_tester_t = std::function<void(mrpt::apps::ICP_SLAM_App_Base&)>;

template <class SLAM_CLASS>
void generic_icp_slam_test(
    const std::string& ini_filename, const std::string& rawlog_filename,
    config_changer_t cfg_changer, post_tester_t post_tester)
{
	using namespace std::string_literals;

	const auto ini_fil = mrpt::UNITTEST_BASEDIR +
	                     "/share/mrpt/config_files/icp-slam/"s + ini_filename;
	EXPECT_TRUE(mrpt::system::fileExists(ini_fil));

	const auto rawlog_fil =
	    mrpt::UNITTEST_BASEDIR + "/share/mrpt/datasets/"s + rawlog_filename;
	EXPECT_TRUE(mrpt::system::fileExists(rawlog_fil));

	try
	{
		SLAM_CLASS app;
		app.setMinLoggingLevel(mrpt::system::LVL_ERROR);

		const char* argv[] = {"icp-slam", ini_fil.c_str(), rawlog_fil.c_str()};
		const int argc = sizeof(argv) / sizeof(argv[0]);

		app.initialize(argc, argv);

		app.params.write(
		    "MappingApplication", "logOutput_dir",
		    mrpt::system::getTempFileName() + "_dir"s);
		app.params.write(
		    "MappingApplication", "SHOW_PROGRESS_3D_REAL_TIME", false);

		cfg_changer(app.params);
		app.run();

		// Check results:
		post_tester(app);
	}
	catch (const std::exception& e)
	{
		std::cerr << mrpt::exception_to_str(e);
		GTEST_FAIL();
	}
}

static auto tester_for_2006_01_21 = [](mrpt::apps::ICP_SLAM_App_Base& o) {
	EXPECT_EQ(o.out_estimated_path.size(), 224U);
	const mrpt::math::TPose3D p = *o.out_estimated_path.rbegin();
	const auto p_gt = mrpt::math::TPose3D::FromString(
	    "[4.308892 -18.489119 0.000000 -87.687641 0.000000 0.000000]");

	for (int i = 0; i < 6; i++) EXPECT_NEAR(p[i], p_gt[i], 0.5);
};

TEST(ICP_SLAM_App, MapFromRawlog_PointMap)
{
	using namespace std::string_literals;

	generic_icp_slam_test<mrpt::apps::ICP_SLAM_App_Rawlog>(
	    "icp-slam_demo_classic.ini",
	    "2006-01ENE-21-SENA_Telecom Faculty_one_loop_only.rawlog",
	    [](mrpt::config::CConfigFileBase&) {}, tester_for_2006_01_21);
}

TEST(ICP_SLAM_App, MapFromRawlog_Grid)
{
	using namespace std::string_literals;

	generic_icp_slam_test<mrpt::apps::ICP_SLAM_App_Rawlog>(
	    "icp-slam_demo_classic_gridmatch.ini",
	    "2006-01ENE-21-SENA_Telecom Faculty_one_loop_only.rawlog",
	    [](mrpt::config::CConfigFileBase&) {}, tester_for_2006_01_21);
}

TEST(ICP_SLAM_App, MapFromRawlog_LM)
{
	using namespace std::string_literals;

	generic_icp_slam_test<mrpt::apps::ICP_SLAM_App_Rawlog>(
	    "icp-slam_demo_LM.ini",
	    "2006-01ENE-21-SENA_Telecom Faculty_one_loop_only.rawlog",
	    [](mrpt::config::CConfigFileBase&) {}, tester_for_2006_01_21);
}
