/*  GlobeMotion.h
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

#import <Eigen/Eigen>
#import <Eigen/Geometry>

namespace WhirlyKit
{

/// Clamps for PredictGlobeMotion
static constexpr double GlobeMotionMaxAngle = 0.15;          // radians ahead
static constexpr double GlobeMotionMaxHeightFactor = 2.0;    // height change ahead
static constexpr double GlobeMotionMinSampleGap = 0.005;     // seconds
static constexpr double GlobeMotionMaxSampleGap = 0.3;       // seconds

/**
    Where the globe camera will be `lookAhead` seconds after the latest of two samples, if it
    keeps moving the same way: rotation extrapolated as an axis-angle, height on a log scale (so a
    zoom keeps its rate), both clamped. Returns false when there is nothing to predict: not
    moving, look-ahead off, or samples too close or too far apart in time.
 */
bool PredictGlobeMotion(double t0,const Eigen::Quaterniond &q0,double h0,
                        double t1,const Eigen::Quaterniond &q1,double h1,
                        double lookAhead,
                        Eigen::Quaterniond &predictedRot,double &predictedHeight);

}
