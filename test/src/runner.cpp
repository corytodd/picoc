#include <gtest/gtest.h>
#include <string>
#include <fstream>
#include <streambuf>

#include "picoc/picoc_picoc.h"

#include "GeneratedTests.hpp"

/* Override via STACKSIZE environment variable */
#define PICOC_STACK_SIZE (128000*4)
#define PICOC_TEST_RESULT "test.result"
#define PICOC_TEST_ERROR "test.err"
#define PICOC_TEST_INPUT "test.in"

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
    IOFILE* pStdout = fopen(PICOC_TEST_RESULT, "w");
    IOFILE* pStdin = fopen(PICOC_TEST_INPUT, "w");
    IOFILE* pStderr = fopen(PICOC_TEST_ERROR, "w");
    ASSERT_TRUE(pStdout != NULL);
    ASSERT_TRUE(pStdin != NULL);
    ASSERT_TRUE(pStderr != NULL);

    picoc_io_t io = {0};
    io.pStdout = pStdout;
    io.pStdin = pStdin;
    io.pStderr = pStderr;

    Picoc pc;
    PicocInitialize(&pc, stackSize, &io);
    PicocIncludeAllSystemHeaders(&pc);

    if (PicocPlatformSetExitPoint(&pc)) {
      PicocCleanup(&pc);
      GTEST_FAIL() << " Failed to set exit point" << std::endl;
    }

    PicocPlatformScanFile(&pc, script.c_str());

    // Cleanup interpreter
    PicocCleanup(&pc);

    // Read in expected string
    std::ifstream t(expect);
    std::string expectedResult((std::istreambuf_iterator<char>(t)),
                               std::istreambuf_iterator<char>());

    // Read in actual string
    fclose(pStdout);
    std::ifstream u(PICOC_TEST_RESULT);
    std::string actualResult((std::istreambuf_iterator<char>(u)),
                               std::istreambuf_iterator<char>());

    ASSERT_EQ(expectedResult, actualResult) << testData.first << " Failed " << script;
  }
}