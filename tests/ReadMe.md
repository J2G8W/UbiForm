# Testing
UbiForm provides a testing suite using google-test to 
demonstrate its usage. These are built when 
`BUILD_TESTS` is specified in the CMake and at that 
point will automatically fetch and setup google-test for 
usage.

When built the tests are run via the google-test 
interface, so you can specify the options on the 
executable `run_tests` just as would be expected with 
google-test