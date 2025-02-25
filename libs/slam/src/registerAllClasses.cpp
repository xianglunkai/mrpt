/* +------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)            |
   |                          https://www.mrpt.org/                         |
   |                                                                        |
   | Copyright (c) 2005-2022, Individual contributors, see AUTHORS file     |
   | See: https://www.mrpt.org/Authors - All rights reserved.               |
   | Released under BSD License. See: https://www.mrpt.org/License          |
   +------------------------------------------------------------------------+ */

#include "slam-precomp.h"  // Precompiled headers
//
#define MRPT_NO_WARN_BIG_HDR
#include <mrpt/core/initializer.h>
#include <mrpt/slam.h>

using namespace mrpt::slam;
using namespace mrpt::maps;
using namespace mrpt::opengl;

MRPT_INITIALIZER(registerAllClasses_mrpt_core)
{
#if !defined(DISABLE_MRPT_AUTO_CLASS_REGISTRATION)
	registerClass(CLASS_ID(CIncrementalMapPartitioner));
	registerClass(CLASS_ID(CMultiMetricMapPDF));
#endif
}
