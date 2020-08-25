/*
 * ------------------------------------------------------------------------------------------------------------
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 * Copyright (c) 2018-2020 Lawrence Livermore National Security LLC
 * Copyright (c) 2018-2020 The Board of Trustees of the Leland Stanford Junior University
 * Copyright (c) 2018-2020 Total, S.A
 * Copyright (c) 2020-     GEOSX Contributors
 * All rights reserved
 *
 * See top level LICENSE, COPYRIGHT, CONTRIBUTORS, NOTICE, and ACKNOWLEDGEMENTS files for details.
 * ------------------------------------------------------------------------------------------------------------
 */

#include <iostream>
#include <fstream>

#include "Parallel/Communicator.hpp"
#include "test_io.h"
#include "gtest/gtest.h"

#ifdef WITH_VTK
#include<vtkSmartPointer.h>
#include<vtkMPIController.h>
#endif

using namespace PAMELA;

int main(int argc, char **argv) {
    Communicator::initialize();
    ::testing::InitGoogleTest(&argc, argv);
#ifdef WITH_VTK
    vtkSmartPointer<vtkMPIController> controler = vtkMPIController::New();
    controler->Initialize(&argc,&argv,true);
    vtkMultiProcessController::SetGlobalController(controler.Get());
#endif
  int const result = RUN_ALL_TESTS();
  Communicator::finalize();
  return result;
}

TEST(testMEDIUM,testMEDIUM)
{
    std::ifstream file_list(std::string(PAMELA_PATH) + "/tests/medium.txt");
    load_and_save(file_list);
}
