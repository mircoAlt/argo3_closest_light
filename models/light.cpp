#include "light.h"

#include <utility>

double Light::calculateDiffThreshold(double distanceToFurther, int atThisLight, int atOtherLight,
                                     vector<double> distances) {
    double diff = diffToExpectedPercentOfLight(atThisLight, atOtherLight);
    pair<double, double> mAndS = meanAndStDev(std::move(distances));
    double vert = cdf(distanceToFurther, mAndS.first, mAndS.second);
    return (vert / diff) * distanceToFurther;
}