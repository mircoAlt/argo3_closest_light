#include "gtest/gtest.h"
#include <models/wall_at.h>
#include <models/bot_at.h>
#include <services/score_wheel.h>
namespace {
    double epsilon = 0.000000001;

    TEST(ScoreWheelTest, Init) {
        auto wheel = ScoreWheel(CRadians::ZERO, CRadians());
        EXPECT_EQ(wheel.areas.size(), 12);
        EXPECT_EQ(wheel.areas[0].getFrom(), -CRadians::PI_OVER_SIX / 2);
        EXPECT_EQ(wheel.areas[0].getTo(),  CRadians::PI_OVER_SIX / 2);
        EXPECT_EQ(wheel.areas[6].getFrom(), CRadians::PI - ( CRadians::PI_OVER_SIX / 2));
        EXPECT_EQ(wheel.areas[6].getTo(), (CRadians::PI + ( CRadians::PI_OVER_SIX / 2)).SignedNormalize());
        EXPECT_EQ(wheel.areas[11].getFrom(),  - 1.5 * CRadians::PI_OVER_SIX);
        ASSERT_NEAR(wheel.areas[11].getTo().GetValue(), -CRadians::PI_OVER_SIX.GetValue()/ 2, epsilon);
    }

    TEST(ScoreWheelTest, InitialScores) {
        auto wheel = ScoreWheel(CRadians::ZERO, CRadians());
        ASSERT_DOUBLE_EQ(wheel.areas[0].score.relevance, 1);
        ASSERT_DOUBLE_EQ(wheel.areas[1].score.relevance, 5/6.0);
        ASSERT_DOUBLE_EQ(wheel.areas[2].score.relevance, 4/6.0);
        ASSERT_DOUBLE_EQ(wheel.areas[3].score.relevance, 3/6.0);
        ASSERT_DOUBLE_EQ(wheel.areas[4].score.relevance, 2/6.0);
        ASSERT_DOUBLE_EQ(wheel.areas[5].score.relevance, 1/6.0);
        ASSERT_DOUBLE_EQ(wheel.areas[6].score.relevance, 0.0);
        ASSERT_NEAR(wheel.areas[7].score.relevance, 1/6.0, epsilon);
        ASSERT_DOUBLE_EQ(wheel.areas[8].score.relevance, 2/6.0);
        ASSERT_DOUBLE_EQ(wheel.areas[9].score.relevance, 3/6.0);
        ASSERT_DOUBLE_EQ(wheel.areas[10].score.relevance, 4/6.0);
        ASSERT_DOUBLE_EQ(wheel.areas[11].score.relevance, 5/6.0);
    }

    TEST(ScoreWheelTest, InitialScoresOtherDirection) {
        auto wheel = ScoreWheel(CRadians::ZERO, CRadians::PI_OVER_TWO);
        ASSERT_DOUBLE_EQ(wheel.areas[0].score.relevance, 0.9);
        ASSERT_DOUBLE_EQ(wheel.areas[6].score.relevance,  0.1);
    }

    TEST(ScoreWheelTest, WheelIndex) {
        auto wheel = ScoreWheel(CRadians::ZERO, CRadians());
        EXPECT_EQ(wheel.getWheelIndexFor(CRadians(14)), 3);
        EXPECT_EQ(wheel.getWheelIndexFor(ToRadians(CDegrees(-178))), 6);
        EXPECT_EQ(wheel.getWheelIndexFor(CRadians::PI_OVER_SIX / 2), 0);
        EXPECT_EQ(wheel.getWheelIndexFor(-CRadians::PI_OVER_SIX / 2), 11);
    }

    TEST(ScoreWheelTest, UpdateScoreDoesNothingIfBotIsFarAway) {
        auto wheel = ScoreWheel(CRadians::ZERO, CRadians());
        BotAt bot({ 1.1, 0 }, 10, 0);
        wheel.updateScores({bot}, {}, {}, 0);
        ASSERT_DOUBLE_EQ(wheel.areas[0].score.relevance, 1);
    }

    TEST(ScoreWheelTest, UpdateScoreIfBotIsCloseEnough) {
        auto wheel = ScoreWheel(CRadians::ZERO, CRadians());
        BotAt bot({ 0.9, 0 }, 10, 0);
        wheel.updateScores({bot}, {}, {}, 0);
        EXPECT_EQ(wheel.getWheelIndexFor(wheel.getWinningAngle().getMiddle()), 1);
    }

    TEST(ScoreWheelTest, UpdateScoreForCloserBot) {
        auto wheel = ScoreWheel(CRadians::ZERO, CRadians());
        BotAt bot({ 0.3, 0.3 }, 10, 0);
        wheel.updateScores({bot}, {}, {}, 0);
        EXPECT_EQ(wheel.getWheelIndexFor(wheel.getWinningAngle().getMiddle()), 11);
    }

    TEST(ScoreWheelTest, MovesBackIfTheRestIsOccuppied) {
        auto wheel = ScoreWheel(CRadians::ZERO, CRadians());
        BotAt bot({ 0.5, 0}, 10, 0);
        BotAt bot2({ 0, 0.5}, 10, 0);
        BotAt bot3({ 0, -0.5}, 10, 0);
        wheel.updateScores({bot, bot2, bot3}, {}, {}, 0);
        EXPECT_EQ(wheel.getWheelIndexFor(wheel.getWinningAngle().getMiddle()), 5);
    }

    TEST(ScoreWheelTest, MovesLeftIfTheRestIsOccuppied) {
        auto wheel = ScoreWheel(CRadians::ZERO, CRadians());
        BotAt bot({ 0.5, 0}, 10, 0);
        BotAt bot2({ 0, 0.5}, 10, 0);
        BotAt bot3({ -0.5, 0}, 10, 0);
        wheel.updateScores({bot, bot2, bot3}, {}, {}, 0);
        EXPECT_EQ(wheel.getWheelIndexFor(wheel.getWinningAngle().getMiddle()), 10);
    }

    TEST(ScoreWheelTest, ReturnsWinningAngleOriginalAngle) {
        CRadians angle = ToRadians(CDegrees(32.47));
        auto wheel = ScoreWheel(angle, CRadians());
        wheel.updateScores({}, {}, {}, 0);
        EXPECT_EQ(wheel.getWinningAngle().getMiddle(), angle);
    }

    TEST(ScoreWheelTest, ReturnsWinningAngleOriginalNegativeAngle) {
        CRadians angle = ToRadians(CDegrees(-78.96));
        auto wheel = ScoreWheel(angle, CRadians());
        wheel.updateScores({}, {}, {}, 0);
        ASSERT_DOUBLE_EQ(wheel.getWinningAngle().getMiddle().GetValue(), angle.GetValue());
    }

    TEST(ScoreWheelTest, UpdatesSpeedForRelevantAngles) {
        auto wheel = ScoreWheel(CRadians::ZERO, CRadians());
        BotAt bot({ 0.5, 0 }, 10, 0);
        bot.movesPerTs = { 0.1, 0.1 };
        wheel.updateScores({bot}, {}, {}, 0);
        EXPECT_EQ(wheel.getWinningAngle().getMiddle(), CRadians::ZERO);
        EXPECT_EQ(wheel.getWinningAngle().score.speed, 0.5);

    }

    TEST(ScoreWheelTest, ConsidersWalls) {
        auto wheel = ScoreWheel(CRadians::ZERO, CRadians::PI_OVER_TWO);
        WallAt wall({ 0, 0.35}, true);
        BotAt bot({ 0.75, 0}, 10, 0);
        wheel.updateScores({bot}, {wall}, {}, 0);

        EXPECT_EQ(wheel.getWheelIndexFor(wheel.getWinningAngle().getMiddle()), 10);
    }
}