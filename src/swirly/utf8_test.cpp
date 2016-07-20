#define CATCH_CONFIG_MAIN

#include <swirly/utf8.h>
#include <catch/catch.hpp>

namespace swirly {
namespace utf8 {

#if 0
TEST_CASE("UTF8", "Basics") {
    for (auto& i: {"foo", "bar", "", " ", "\n", "\0"})
        REQUIRE(i == utf8ToJson(i));
}

TEST_CASE("UTF8", "Extended") {
    REQUIRE(utf8ToJson("\xc3\x87") == "\\u00c7");
    REQUIRE(utf8ToJson("Ç") == "\\u00c7");
}

TEST_CASE("UTF8", "Failure") {
    // TODO: EXPECT_THROW({utf8ToJson("\x80");}, Exception);
}
#endif

void testRoundTrip(CodePoint cp) {
    if (isValid(cp)) {
        auto s = toUTF8(cp);
        auto bytes = Bytes(s);
        auto cp2 = consumeCodePoint(bytes);
        REQUIRE(bytes.size() == 0);
        REQUIRE(cp == cp2);
    }
}

TEST_CASE("UTF8 ASCII") {
    for (CodePoint cp = 0; cp < codePointRange()[0]; ++cp)
        testRoundTrip(cp);
}

TEST_CASE("UTF8 SimpleExtended") {
    for (CodePoint cp = codePointRange()[0]; cp < codePointRange()[1]; ++cp)
        testRoundTrip(cp);
}

TEST_CASE("UTF8 ThreeBytes") {
    for (CodePoint cp = codePointRange()[1]; cp < codePointRange()[2]; ++cp)
        testRoundTrip(cp);
}

TEST_CASE("UTF8 MoreBytes") {
    auto delta = 1000;
    for (auto i = 3; i < MAX_CODEPOINT_BYTES - 1; ++i) {
        for (CodePoint cp = codePointRange()[i] - delta;
             cp < codePointRange()[i] + delta; ++cp) {
            testRoundTrip(cp);
        }
    }
}

#if 0
TEST_CASE("UTF8", "EdgeBytes") {
    auto max = codePointRange().back();
    EXPECT_NO_THROW({ toUTF8(0); });
    EXPECT_NO_THROW({ toUTF8(max - 1); });
    EXPECT_THROW({ toUTF8(max); }, Exception);
    EXPECT_THROW({ toUTF8(max + 1); }, Exception);
}

TEST_CASE("UTF8", "TooShort") {
    auto run = [](std::string const& s) {
        Bytes bytes(s);
        consumeCodePoint(bytes);
    };
    EXPECT_THROW( { run(""); }, Exception);

    EXPECT_THROW( { run("\xc2"); }, Exception);
    EXPECT_NO_THROW( { run("\xc2\xa9"); });

    EXPECT_THROW( { run("\xe2"); }, Exception);
    EXPECT_THROW( { run("\xe2\x89"); }, Exception);
    EXPECT_NO_THROW( { run("\xe2\x89\xa0"); });
}

TEST_CASE("UTF8", "Overlong") {
    auto run = [](std::string const& s, char const* message = "Overly long") {
        try {
            Bytes bytes(s);
            consumeCodePoint(bytes);
        } catch (Exception const& e) {
            std::string w = e.what();
            auto isOK = w.find(message) != std::string::npos;
            EXPECT_TRUE(isOK) << e.what();
            return isOK;
        }
        return false;
    };
    EXPECT_TRUE(run("\xc0\x80"));
    EXPECT_TRUE(run("\xc1\xBF"));

    EXPECT_TRUE(run("\xe0\x80\x80"));
    EXPECT_TRUE(run("\xe0\x9F\xBF"));

    EXPECT_TRUE(run("\xf0\x80\x80\x80"));
    EXPECT_TRUE(run("\xf0\x8F\xBF\xBD"));
    EXPECT_TRUE(run("\xf0\x8F\xBF\xBE", "Invalid"));
    EXPECT_TRUE(run("\xf0\x8F\xBF\xBF", "Invalid"));

    EXPECT_TRUE(run("\xf8\x80\x80\x80\x80"));
    EXPECT_TRUE(run("\xf8\x87\xBF\xBF\xBF"));

    EXPECT_TRUE(run("\xfc\x80\x80\x80\x80\x80"));
    EXPECT_TRUE(run("\xfc\x83\xBF\xBF\xBF\xBF"));
}
#endif

} // utf8
} // swirly
