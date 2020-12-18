#include "picoc/picoc_picoc.h"

#include <algorithm>
#include <fstream>
#include <gtest/gtest.h>
#include <string>

/* Override via STACKSIZE environment variable */
#define PICOC_STACK_SIZE (128000*4)
#define PICOC_TEST_RESULT "test.result"
#define PICOC_TEST_ERROR "test.err"
#define PICOC_TEST_INPUT "test.in"

// Include last so we don't contaminate picoc
#include "GeneratedTests.hpp"

//! Not const because we're passing this into the interpreter's main()
char* PICOC_TEST_ARGV[] = {
    "-",
    "arg1",
    "arg2",
    "arg3",
    "arg4",
    NULL
};

//! Not const because we're passing this into the interpreter's main()
char* PICOC_TEST_ARGV_NULL [] { NULL };

/**
 * Run all test cases and compare stdout to expected output
 */
TEST(Runner, RunAllTests) {
  // Setup
  int stackSize = getenv("STACKSIZE") ? atoi(getenv("STACKSIZE")) : PICOC_STACK_SIZE;

  // Broken tests: skip by id
  std::vector<int> skip {58 /* return 0; crash */, 65 /* typeless mode?? */};

  // For all generated tests
  for(const auto& pair : test_details::AllTestDetails){

    // pair: {int id, { script intput, expected output, use args?, call main? }}
    if(std::find(skip.begin(), skip.end(), pair.first) != skip.end()) {
      continue;
    }

    // Destructure the tuple into its parts
    std::string script;
    std::string expect;
    bool useArgs;
    bool callMain;
    std::tie(script, expect, useArgs, callMain) = pair.second;

    // IO capture
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

    // New picoc for each test
    Picoc pc;
    PicocInitialize(&pc, stackSize, &io);

    PicocIncludeAllSystemHeaders(&pc);

    if (PicocPlatformSetExitPoint(&pc)) {
      PicocCleanup(&pc);
      GTEST_FAIL() << " Failed to set exit point. Did you run generator? ID #"  << pair.first << std::endl;
    }

    // Execute script
    PicocPlatformScanFile(&pc, script.c_str());

    if(callMain) {
      int argc = useArgs ? 5 : 0;
      char** argv = useArgs ? PICOC_TEST_ARGV : PICOC_TEST_ARGV_NULL;
      PicocCallMain(&pc, argc, argv);
    }

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

    ASSERT_EQ(expectedResult, actualResult) << pair.first << " Failed " << script;
  }
}