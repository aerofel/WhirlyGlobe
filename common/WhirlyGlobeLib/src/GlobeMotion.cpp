/*  GlobeMotion.cpp
 *  WhirlyGlobeLib (offto fork)
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#import "GlobeMotion.h"
#import <cmath>
#import <algorithm>

namespace WhirlyKit
{

bool PredictGlobeMotion(double t0,const Eigen::Quaterniond &q0,double h0,
                        double t1,const Eigen::Quaterniond &q1,double h1,
                        double lookAhead,
                        Eigen::Quaterniond &predictedRot,double &predictedHeight)
{
    const double dt = t1 - t0;
    if (lookAhead <= 0.0 || dt < GlobeMotionMinSampleGap || dt > GlobeMotionMaxSampleGap || h0 <= 0.0 || h1 <= 0.0)
        return false;
    const double steps = lookAhead / dt;

    // Rotation: the step from q0 to q1, repeated `steps` times
    const Eigen::AngleAxisd delta(q1 * q0.inverse());
    double angle = delta.angle();
    Eigen::Vector3d axis = delta.axis();
    if (angle > M_PI) {       // take the short way round
        angle = 2.0 * M_PI - angle;
        axis = -axis;
    }
    const double aheadAngle = std::min(angle * steps,GlobeMotionMaxAngle);

    // Height: same ratio per step (log scale), clamped
    const double logRatio = std::log(h1 / h0) * steps;
    const double maxLog = std::log(GlobeMotionMaxHeightFactor);
    const double aheadLog = std::max(-maxLog,std::min(maxLog,logRatio));

    if (aheadAngle < 1e-6 && std::abs(aheadLog) < 1e-6)
        return false;

    predictedRot = (aheadAngle > 0.0) ? Eigen::Quaterniond(Eigen::AngleAxisd(aheadAngle,axis)) * q1 : q1;
    predictedRot.normalize();
    predictedHeight = h1 * std::exp(aheadLog);
    return true;
}

}
