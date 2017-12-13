#include "util.h"
#include <gtest/gtest.h>

TEST(SplitTest, DefaultDelimiter) {
    std::vector<std::string> strs = split("abc def ijk");
    EXPECT_EQ(strs[0], "abc");
    EXPECT_EQ(strs[1], "def");
    EXPECT_EQ(strs[2], "ijk");
}

TEST(SplitTest, CustomDelimiter) {
    std::vector<std::string> strs = split("abc/def/ijk", '/');
    EXPECT_EQ(strs[0], "abc");
    EXPECT_EQ(strs[1], "def");
    EXPECT_EQ(strs[2], "ijk");
}

TEST(SplitTest, IgnoreEmptyString) {
    std::vector<std::string> strs = split("abc//def//ijk", '/');
    EXPECT_EQ(strs[0], "abc");
    EXPECT_EQ(strs[1], "def");
    EXPECT_EQ(strs[2], "ijk");
}

TEST(LowerTest, LowerString) {
    EXPECT_EQ("abc", lower("ABC"));
    EXPECT_EQ("abc", lower("abc"));
}

TEST(JoinTest, SingleString) {
    EXPECT_EQ("abc", join(' ', "abc"));
    EXPECT_EQ("abc", join('/', "abc"));
}

TEST(JoinTest, ManyStrings) {
    EXPECT_EQ("a/b/c", join('/', "a", "b", "c"));
}

TEST(TrimTest, AnyChar) {
    EXPECT_EQ("abc", trim(" abc "));
    EXPECT_EQ("abc", trim("abc"));
    EXPECT_EQ("abc", trim("//abc//", '/'));
    EXPECT_EQ("abc", trim("**abc**", '*'));
}

TEST(TrimLeftTest, AnyChar) {
    EXPECT_EQ("abc ", trim_left(" abc "));
    EXPECT_EQ("abc", trim_left("abc"));
    EXPECT_EQ("abc//", trim_left("//abc//", '/'));
    EXPECT_EQ("abc**", trim_left("**abc**", '*'));
}
TEST(TrimRightTest, AnyChar) {
    EXPECT_EQ(" abc", trim_right(" abc "));
    EXPECT_EQ("abc", trim_right("abc"));
    EXPECT_EQ("//abc", trim_right("//abc//", '/'));
    EXPECT_EQ("**abc", trim_right("**abc**", '*'));
}

TEST(PathJoinTest, ManyPaths) {
    EXPECT_EQ("aa/bb/cc", path::join("aa", "bb", "cc"));
    EXPECT_EQ("aa/bb/cc", path::join("aa//", "//bb//", "//cc//"));
    EXPECT_EQ("a/a/b/b/c/c", path::join("a/a/", "/b/b/", "/c/c/"));
    EXPECT_EQ("//aa/bb/cc", path::join("//aa//", "//bb//", "//cc//"));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
