#include "gtest/gtest.h"
#include "util/utility.h"
#include "common/config.h"

INITIALIZE_EASYLOGGINGPP

QuicConfig default_quic_config;

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    START_EASYLOGGINGPP(argc, argv);
    el::Configurations defaultConf;
    defaultConf.setToDefault();
    defaultConf.setGlobally(
            el::ConfigurationType::Format, 
            "%datetime [%levshort] %fbase:%line [%func] %msg");
    el::Loggers::reconfigureLogger("default", defaultConf);
    return RUN_ALL_TESTS();
}
