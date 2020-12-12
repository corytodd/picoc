#include <gtest/gtest.h>
#include <string>
#include <fstream>
#include <streambuf>

#include "picoc/picoc_picoc.h"

#include "GeneratedTests.hpp"

/* Override via STACKSIZE environment variable */
#define PICOC_STACK_SIZE (128000*4)
/**
 * Run all test cases and compare stdout to expected output
 */
TEST(Runner, RunAllTests) {
  // Setup

  // Broken tests
  std::vector<int> skip {58};

  // Execute
  for(const auto& testData : test_details::test_paths){

    if(std::find(skip.begin(), skip.end(), testData.first) != skip.end()) {
      continue;
    }

    std::string script = testData.second.first;
    std::string expect = testData.second.second;

    int stackSize = getenv("STACKSIZE") ? atoi(getenv("STACKSIZE")) : PICOC_STACK_SIZE;
    Picoc pc;
    PicocInitialize(&pc, stackSize);
    PicocIncludeAllSystemHeaders(&pc);

    if (PicocPlatformSetExitPoint(&pc)) {
      PicocCleanup(&pc);
      GTEST_FAIL() << " Failed to set exit point" << std::endl;
    }

    freopen("test.txt", "w", stdout);
    PicocPlatformScanFile(&pc, script.c_str());

    // Read in expected string
    std::ifstream t(expect);
    std::string expectedResult((std::istreambuf_iterator<char>(t)),
                               std::istreambuf_iterator<char>());

    // Read in actual string
    std::ifstream u("test.txt");
    std::string actualResult((std::istreambuf_iterator<char>(u)),
                               std::istreambuf_iterator<char>());

    // Cleanup interpreter
    PicocCleanup(&pc);
    remove("test.txt");

    ASSERT_EQ(expectedResult, actualResult) << testData.first << " Failed " << script;
  }
}