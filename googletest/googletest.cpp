#include "gtest/gtest.h"
#include "../src/c_api/utils.cpp"

int mult(int a, int b) {
    return a * b;
}

TEST(MultTest, Positive) {
    EXPECT_EQ(6, mult(2, 3));
    EXPECT_EQ(0, mult(0, 0));
    EXPECT_EQ(-6, mult(-2, 3));
    EXPECT_EQ(-6, mult(2, -3));
}

int main (int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
