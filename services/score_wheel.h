#ifndef ARGOS3_EXAMPLES_SCORE_WHEEL_H
#define ARGOS3_EXAMPLES_SCORE_WHEEL_H

#include <vector>
#include <models/bot_at.h>
#include <models/wall_at.h>

using namespace std;
using namespace argos;
#define NO_OF_AREAS 12

class ScoreWheel {
    struct Score {
        double relevance;
        double speed = 1.0;
        explicit Score(double relevance): relevance(relevance) {}
        explicit Score(CRadians toGoalDirection, CRadians toCurrentDirection, double importanceOfDirection) :
            relevance(importanceOfDirection * relevanceForAngle(toGoalDirection) + 0.2 * relevanceForAngle(toCurrentDirection)) {}
        inline friend std::ostream &operator<<(std::ostream &c_os,
                                               const Score &s) {
            c_os << "relevance " << s.relevance << ", speed: " << s.speed;
            return c_os;
        }

        inline double increaseRightSide(CRadians middle) {
            return (middle.SignedNormalize() > CRadians::ZERO) ? 1.0/NO_OF_AREAS : 0.0;
        }

        inline double relevanceForAngle(CRadians angle) {
            return Abs(1 - angle / CRadians::PI );
        }
    };
    struct WheelArea {
        Score score;
        void setFrom(CRadians f) {from = f.SignedNormalize(); }
        void setTo(CRadians t) {to = t.SignedNormalize(); }
        CRadians getFrom() const { return from; }
        CRadians getTo() const { return to; }
        CRadians getMiddle() const { return getFrom() + (CRadians::PI / NO_OF_AREAS ); }
        CRadians getClosestFor(CRadians angle) const {
            if (includes(angle) || score.relevance > 0.3) return getMiddle();
            CRadians diffToEnd = Abs(NormalizedDifference(getTo(), angle));
            CRadians diffFromStart = Abs(NormalizedDifference(getFrom(), angle));
            return (diffToEnd < diffFromStart) ? getTo() : getFrom();
        }
        explicit WheelArea(CRadians middle, CRadians mainDirection, CRadians currentDirection, double importanceOfDirection) : score(
                (middle - mainDirection).UnsignedNormalize(), (middle - currentDirection).UnsignedNormalize(), importanceOfDirection) {
            setFrom(middle - (CRadians::PI / NO_OF_AREAS ));
            setTo(middle + (CRadians::PI / NO_OF_AREAS ));
        }
        inline friend std::ostream &operator<<(std::ostream &c_os,
                                               const WheelArea &a) {
            c_os << "area from " << a.getFrom() << " to " << a.getTo() << " has score " << a.score;
            return c_os;
        }
        inline bool includes(CRadians angle) const {
            CRadians diffToEnd = NormalizedDifference(getTo() + CRadians(epsilon), angle);
            if (Abs(diffToEnd) >= (CRadians::TWO_PI / NO_OF_AREAS )) return false;
            CRadians diffFromStart = NormalizedDifference(getFrom(), angle);
            return diffFromStart < CRadians::ZERO && diffToEnd >= CRadians::ZERO;
        }
        bool operator<(const WheelArea &otherArea) const {
            return score.relevance < otherArea.score.relevance;
        }
    private:
        CRadians from;
        CRadians to;
        Real epsilon = 0.000000001;

    };
public:

    constexpr static const double distThreshold = 1.0;
    constexpr static const double wallDistThreshold = 0.5;
    explicit ScoreWheel(CRadians direction, CRadians currentDirection, string id = "", double importanceOfDirection = 0.8);
    void updateScores(const vector<BotAt> &bots, const set<WallAt> &walls, const CVector2 &botPosition,
                      unsigned long ts);
    inline string GetId() const {
        return id;
    }
    vector<WheelArea> areas;
    int getWheelIndexFor(CRadians angle);

    WheelArea getWinningAngle();

private:
    string id;

    void updateRelevances(const Real &length, const CRadians &angle);
    void updateSpeeds(const CRadians &angle, const int &wheelIndex);
};


#endif //ARGOS3_EXAMPLES_SCORE_WHEEL_H
