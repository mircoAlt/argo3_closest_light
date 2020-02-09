#ifndef ARGOS3_EXAMPLES_LIGHT_H
#define ARGOS3_EXAMPLES_LIGHT_H
#include <argos3/plugins/simulator/entities/light_entity.h>
#include <list>
#include <numeric>
#include <unordered_set>
#include <algorithm>
#include <set>

using namespace std;
using namespace argos;

class Light {

    double percentOfLightForOtherLight(int atThisLight, int atOtherLight) {
        double relevant_bots = (double) atThisLight + atOtherLight;
        return atOtherLight / relevant_bots;
    }

    double diffToExpectedPercentOfLight(int atThisLight, int atOtherLight) {
        return 0.5 - percentOfLightForOtherLight(atThisLight, atOtherLight);
    }

    double cdf(double x, double mean, double standardDev) {
        return 0.5 * erfc(-(x - mean) / (standardDev * sqrt(2)));
    }

protected:
    int id;
    CVector3 position;
public:
    static pair<double, double> meanAndStDev(vector<double> distances) {
        double sum = accumulate(distances.begin(), distances.end(), 0.0);
        double mean = sum / distances.size();

        vector<double> diff(distances.size());
        transform(distances.begin(), distances.end(), diff.begin(), [mean](double x) { return x - mean; });

        double sq_sum = inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
        double stdev = sqrt(sq_sum / distances.size());
        return {mean, stdev};
    }

    int getId() const {
        return id;
    }
    inline string GetId() {
        return to_string(getId());
    }

    CVector3 getPosition() const {
        return position;
    }

    CVector2 get2DPosition() const {
        return { position.GetX(), position.GetY() };
    }

    double calculateDiffThreshold(double distanceToFurther, int atThisLight, int atOtherLight,
                                  vector<double> distances);
    inline friend std::ostream &operator<<(std::ostream &c_os,
                                           const Light &l) {

        c_os << "[L" << l.id << "] Position: " << l.position;
        return c_os;
    }
    Light(CVector3 pos, int id): position(pos), id(id) {};
};


#endif //ARGOS3_EXAMPLES_LIGHT_H
