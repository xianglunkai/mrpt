/* +------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)            |
   |                          https://www.mrpt.org/                         |
   |                                                                        |
   | Copyright (c) 2005-2022, Individual contributors, see AUTHORS file     |
   | See: https://www.mrpt.org/Authors - All rights reserved.               |
   | Released under BSD License. See: https://www.mrpt.org/License          |
   +------------------------------------------------------------------------+ */

#include <mrpt/gui/CDisplayWindow.h>
#include <mrpt/math/CMatrixF.h>
#include <mrpt/system/CTicTac.h>
#include <mrpt/system/CTimeLogger.h>

#include <iostream>

using namespace mrpt;
using namespace mrpt::gui;
using namespace mrpt::img;
using namespace mrpt::math;
using namespace mrpt::system;
using namespace std;

#include <mrpt/examples_config.h>
string myDataDir(MRPT_EXAMPLES_BASE_DIRECTORY + string("img_basic_example/"));

// ------------------------------------------------------
//				TestImageCap
// ------------------------------------------------------
void TestImageConversion()
{
	// BMP -> JPEG conversion tester:
	// --------------------------------
	CImage img;
	CTicTac tictac;
	CTimeLogger timlog;

	tictac.Tic();
	if (!img.loadFromFile(myDataDir + string("frame_color.jpg")))
	{
		cerr << "Cannot load " << myDataDir + string("frame_color.jpg") << endl;
		return;
	}
	printf("Image loaded in %.03fms\n", 1000 * tictac.Tac());

	if (false)	// A very simple test:
	{
		CDisplayWindow win1("JPEG file, color");
		win1.setPos(10, 10);

		win1.showImage(img);

		cout << "Push a key in the console or in the window to continue...";
		win1.waitForKey();
		cout << "Done" << endl;

		timlog.enter("grayscale1");
		img = img.grayscale();
		timlog.leave("grayscale1");

		CDisplayWindow win2("JPEG file, gray");
		win2.showImage(img);
		win1.setPos(300, 10);

		cout << "Push a key in the console or in the window to continue...";
		win2.waitForKey();
		cout << "Done" << endl;

		mrpt::system::pause();
		return;
	}

	CDisplayWindow win1("win1"), win2("win2"), win3("win3"), win4("win4");

	CImage imgSmall(img);
	CImage imgGray;

	for (int i = 0; i < 50; i++)
	{
		timlog.enter("grayscale2");
		imgSmall.grayscale(imgGray);
		timlog.leave("grayscale2");
	}

	CImage imgSmall2 = imgGray.scaleHalf(IMG_INTERP_LINEAR);
	CImage imgSmallRGB = img.scaleHalf(IMG_INTERP_LINEAR);

	// Test some draw capabilities:
	// ---------------------------------
	imgSmall.rectangle(85, 35, 170, 170, TColor(255, 0, 0), 10);

	imgSmall.line(550, 75, 650, 25, TColor(0, 0, 255));
	imgSmall.line(-10, -20, 20, 30, TColor(0, 0, 255));

	CMatrixDouble22 COV;
	COV(0, 0) = 100;
	COV(1, 1) = 50;
	COV(0, 1) = COV(1, 0) = -30;
	imgSmall.ellipseGaussian(COV, 600.0, 50.0, 2, TColor(255, 255, 0), 4);
	imgGray.ellipseGaussian(COV, 100.0, 100.0, 2, TColor(0, 0, 255), 4);

	imgGray.drawImage(50, 40, imgSmall2);

	// Show the windows now:
	// ------------------------------------------------------
	win1.showImage(imgSmall);
	win1.setPos(0, 0);
	win2.showImage(imgSmall2);
	win2.setPos(810, 0);
	win3.showImage(imgGray);
	win3.setPos(810, 300);
	win4.showImage(imgSmallRGB);
	win4.setPos(300, 400);

	cout << "Press any key on 'win4' to exit" << endl;
	win4.waitForKey();

	tictac.Tic();
	imgGray.saveToFile("frame_out.jpg");
	printf("jpeg file saved in %.03fms\n", 1000.0 * tictac.Tac());

	imgSmall2.saveToFile("frame_out_small.png");

	return;
}

// ------------------------------------------------------
//						MAIN
// ------------------------------------------------------
int main()
{
	try
	{
		TestImageConversion();
		return 0;
	}
	catch (const std::exception& e)
	{
		std::cerr << "MRPT error: " << mrpt::exception_to_str(e) << std::endl;
		return -1;
	}
	catch (...)
	{
		printf("Untyped exception!!");
		return -1;
	}
}
