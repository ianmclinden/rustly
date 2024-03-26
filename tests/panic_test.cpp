#include <gtest/gtest.h>
#include <rustly/panic.h>

TEST(Panic, Simple)
{
    static const std::string FileName(__FILE__);

    EXPECT_EXIT(panic(), ::testing::KilledBySignal(SIGABRT), "panicked at .*" + FileName + ":" + "8\nexplicit panic");
    EXPECT_EXIT(panic("a message"), ::testing::KilledBySignal(SIGABRT), "panicked at .*" + FileName + ":" + "9\na message");
    EXPECT_EXIT(panic("a message {}/{}", 16, 32), ::testing::KilledBySignal(SIGABRT), "panicked at .*" + FileName + ":" + "10\na message 16/32");
}