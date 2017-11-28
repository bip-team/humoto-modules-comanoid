/**
    @file
    @author  Stanislas Brossette

    @copyright 2014-2017 INRIA. Licensed under the Apache License, Version 2.0.
    (see @ref LICENSE or http://www.apache.org/licenses/LICENSE-2.0)

    @brief
*/

#pragma once

#include "task_kinematic.h.in"
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>

namespace humoto
{
namespace wpg05
{
/// @brief Class used to log and display the CoM and CoP history
class HUMOTO_LOCAL Logger
{
  public:
    /// @brief Constructor
    Logger(double timeStep, const StepPlan &stepPlan, const FootTraj &rightFootTraj,
           const FootTraj &leftFootTraj, const ProblemParameters &pbParams, const ModelState& model_state)
        : stepPlan_(stepPlan),
          rightFootTraj_(rightFootTraj),
          leftFootTraj_(leftFootTraj),
          timeStep_(timeStep),
          pbParams_(pbParams)
    {
        xMin_ = stepPlan_.xMin();
        xMax_ = stepPlan_.xMax();
        yMin_ = stepPlan_.yMin();
        yMax_ = stepPlan_.yMax();
        zMin_ = stepPlan_.z();
        zMax_ = stepPlan_.z();
        gravity_ = pbParams_.g_;
        for (long i = 0; i < stepPlan_.z().size(); ++i)
        {
            zMin_(i) += (pbParams_.zetaZero_ - pbParams_.zetaSpan_ / 2) * gravity_;
            zMax_(i) += (pbParams_.zetaZero_ + pbParams_.zetaSpan_ / 2) * gravity_;
        }
        highestFeasibleZ_.resize(xMin_.rows());
        highestFeasibleZ_.setZero();
        addState(model_state.position(), model_state.velocity(), model_state.acceleration(),
                 pbParams_.zetaZero_);
    }

    /// @brief Adds a single state and control to the history
    ///
    /// @param state state
    /// @param control control
    void addStateAndControl(const etools::Vector9 &state, const etools::Vector3 &control)
    {
        addStateAndControl(state, control, pbParams_.zetaZero_);
    }

    void addControl(const etools::Vector3 &control) { jerksCoM_.push_back(control); }

    void addState(const etools::Vector3 &position, const etools::Vector3 &velocity,
                  const etools::Vector3 &acceleration, const double &zeta)
    {
        positionsCoM_.push_back(position);
        velocitiesCoM_.push_back(velocity);
        accelerationsCoM_.push_back(acceleration);

        double zetaMin = zeta - pbParams_.zetaSpan_ / 2;
        double zetaMax = zeta + pbParams_.zetaSpan_ / 2;

        Eigen::Vector3d cop, copMin, copMax;
        copMin(0) = position(0) - zetaMin * acceleration(0);
        copMin(1) = position(1) - zetaMin * acceleration(1);
        copMin(2) = position(2) - zetaMin * (acceleration(2) + gravity_);
        cop(0) = position(0) - zeta * acceleration(0);
        cop(1) = position(1) - zeta * acceleration(1);
        cop(2) = position(2) - zeta * (acceleration(2) + gravity_);
        copMax(0) = position(0) - zetaMax * acceleration(0);
        copMax(1) = position(1) - zetaMax * acceleration(1);
        copMax(2) = position(2) - zetaMax * (acceleration(2) + gravity_);
        positionsCoPMin_.push_back(copMin);
        positionsCoP_.push_back(cop);
        positionsCoPMax_.push_back(copMax);
    }

    void addStateAndControl(const ModelState& state, const etools::Vector3 &control,
                            const double &zeta)
    {
        addState(state.position(), state.velocity(), state.acceleration(), zeta);
        addControl(control);
    }

    void addStateAndControl(const etools::Vector9 &state, const etools::Vector3 &control,
                            const double &zeta)
    {
        Eigen::Vector3d position, velocity, acceleration, cop, copMin, copMax;
        position << state(0), state(3), state(6);
        velocity << state(1), state(4), state(7);
        acceleration << state(2), state(5), state(8);
        addState(position, velocity, acceleration, zeta);
        addControl(control);
    }

    void addHighestFeasibleZ(double zMax, long index) const { highestFeasibleZ_[index] = zMax; }

    Eigen::MatrixXd getPositionsAsMatrix() const { return toMatrix(positionsCoM_); }
    Eigen::MatrixXd getVelocitiesAsMatrix() const { return toMatrix(velocitiesCoM_); }
    Eigen::MatrixXd getAccelerationsAsMatrix() const { return toMatrix(accelerationsCoM_); }
    Eigen::MatrixXd getJerksAsMatrix() const { return toMatrix(jerksCoM_); }
    Eigen::MatrixXd getCoPMinsAsMatrix() const { return toMatrix(positionsCoPMin_); }
    Eigen::MatrixXd getCoPsAsMatrix() const { return toMatrix(positionsCoP_); }
    Eigen::MatrixXd getCoPMaxsAsMatrix() const { return toMatrix(positionsCoPMax_); }

    /// @brief Transforms a list of vector3 into a matrix
    ///
    /// @param vec list of vector3
    ///
    /// @return Matrix concatenation of the vectors
    Eigen::MatrixXd toMatrix(const std::vector<Eigen::Vector3d> &vec) const
    {
        Eigen::MatrixXd mat;
        mat.resize(vec.size(), 3);
        for (size_t i = 0; i < vec.size(); ++i) mat.row(i) << vec[i].transpose();
        return mat;
    }

    /// @brief Getter for size
    size_t size() const { return positionsCoM_.size(); }

    /// @brief Prints the history of the CoM
    void print() const
    {
        std::cout << "Position History:" << std::endl;
        std::cout << toMatrix(positionsCoM_) << std::endl;
        std::cout << "Velocity History:" << std::endl;
        std::cout << toMatrix(velocitiesCoM_) << std::endl;
        std::cout << "Acceleration History:" << std::endl;
        std::cout << toMatrix(accelerationsCoM_) << std::endl;
        std::cout << "Jerk History:" << std::endl;
        std::cout << toMatrix(jerksCoM_) << std::endl;
    }

    void logEverything(std::ofstream &logFile) const
    {
        Eigen::IOFormat cleanFmt(4, 0, ", ", "\n", "[", "]");
        Eigen::IOFormat stepFmt(4, 0, ", ", ", ", "[", "]");
        logFile << "t = np.arange(0.," << (double)size() * (double)timeStep_ << ", " << timeStep_
                << ")\n\n";

        Eigen::MatrixXd positions(toMatrix(positionsCoM_));
        Eigen::MatrixXd positionsCoPMin(toMatrix(positionsCoPMin_));
        Eigen::MatrixXd positionsCoPMax(toMatrix(positionsCoPMax_));
        Eigen::MatrixXd positionsCoP(toMatrix(positionsCoP_));
        Eigen::MatrixXd velocities(toMatrix(velocitiesCoM_));
        Eigen::MatrixXd accelerations(toMatrix(accelerationsCoM_));
        Eigen::MatrixXd jerks(toMatrix(jerksCoM_));

        logFile << "x = np.array(" << positions.col(0).transpose().format(cleanFmt) << ")\n";
        logFile << "y = np.array(" << positions.col(1).transpose().format(cleanFmt) << ")\n";
        logFile << "z = np.array(" << positions.col(2).transpose().format(cleanFmt) << ")\n";
        logFile << "highestFeasibleZ = np.array("
                << highestFeasibleZ_.segment(0, positions.rows()).transpose().format(cleanFmt)
                << ")\n";
        logFile << "xCoPMin = np.array(" << positionsCoPMin.col(0).transpose().format(cleanFmt)
                << ")\n";
        logFile << "yCoPMin = np.array(" << positionsCoPMin.col(1).transpose().format(cleanFmt)
                << ")\n";
        logFile << "zCoPMin = np.array(" << positionsCoPMin.col(2).transpose().format(cleanFmt)
                << ")\n";
        logFile << "xCoP = np.array(" << positionsCoP.col(0).transpose().format(cleanFmt) << ")\n";
        logFile << "yCoP = np.array(" << positionsCoP.col(1).transpose().format(cleanFmt) << ")\n";
        logFile << "zCoP = np.array(" << positionsCoP.col(2).transpose().format(cleanFmt) << ")\n";
        logFile << "xCoPMax = np.array(" << positionsCoPMax.col(0).transpose().format(cleanFmt)
                << ")\n";
        logFile << "yCoPMax = np.array(" << positionsCoPMax.col(1).transpose().format(cleanFmt)
                << ")\n";
        logFile << "zCoPMax = np.array(" << positionsCoPMax.col(2).transpose().format(cleanFmt)
                << ")\n";
        logFile << "xMin = np.array(" << xMin_.transpose().format(cleanFmt) << ")\n";
        logFile << "yMin = np.array(" << yMin_.transpose().format(cleanFmt) << ")\n";
        logFile << "zMin = np.array(" << zMin_.transpose().format(cleanFmt) << ")\n";
        logFile << "xMax = np.array(" << xMax_.transpose().format(cleanFmt) << ")\n";
        logFile << "yMax = np.array(" << yMax_.transpose().format(cleanFmt) << ")\n";
        logFile << "zMax = np.array(" << zMax_.transpose().format(cleanFmt) << ")\n";
        logFile << "dx = np.array(" << velocities.col(0).transpose().format(cleanFmt) << ")\n";
        logFile << "dy = np.array(" << velocities.col(1).transpose().format(cleanFmt) << ")\n";
        logFile << "dz = np.array(" << velocities.col(2).transpose().format(cleanFmt) << ")\n";
        logFile << "ddx = np.array(" << accelerations.col(0).transpose().format(cleanFmt) << ")\n";
        logFile << "ddy = np.array(" << accelerations.col(1).transpose().format(cleanFmt) << ")\n";
        logFile << "ddz = np.array(" << accelerations.col(2).transpose().format(cleanFmt) << ")\n";
        logFile << "dddx = np.array(" << jerks.col(0).transpose().format(cleanFmt) << ")\n";
        logFile << "dddy = np.array(" << jerks.col(1).transpose().format(cleanFmt) << ")\n";
        logFile << "dddz = np.array(" << jerks.col(2).transpose().format(cleanFmt) << ")\n";

        logFile << "time = np.array(" << rightFootTraj_.t_.head(size()).transpose().format(cleanFmt)
                << ")\n";
        logFile << "trajRFootX = np.array("
                << rightFootTraj_.x_.head(size()).transpose().format(cleanFmt) << ")\n";
        logFile << "trajRFootY = np.array("
                << rightFootTraj_.y_.head(size()).transpose().format(cleanFmt) << ")\n";
        logFile << "trajRFootZ = np.array("
                << rightFootTraj_.z_.head(size()).transpose().format(cleanFmt) << ")\n";
        logFile << "trajLFootX = np.array("
                << leftFootTraj_.x_.head(size()).transpose().format(cleanFmt) << ")\n";
        logFile << "trajLFootY = np.array("
                << leftFootTraj_.y_.head(size()).transpose().format(cleanFmt) << ")\n";
        logFile << "trajLFootZ = np.array("
                << leftFootTraj_.z_.head(size()).transpose().format(cleanFmt) << ")\n";

        logFile << "leftFootSteps = [";
        for (size_t i = 0; i < stepPlan_.leftSteps().size(); ++i)
            logFile << stepPlan_.leftSteps().at(i).pos().transpose().format(stepFmt) << ", ";
        logFile << "]\n";

        logFile << "rightFootSteps = [";
        for (size_t i = 0; i < stepPlan_.rightSteps().size(); ++i)
            logFile << stepPlan_.rightSteps().at(i).pos().transpose().format(stepFmt) << ", ";
        logFile << "]\n";
    }

    void plotTrajectoryFoot(std::ofstream &logFile) const
    {
        /**************************
         *  PLOT FOOT TRAJECTORY  *
         **************************/
        logFile << "plt.plot(time, trajRFootZ, 'r', linewidth=0.5, label='RFootZ')\n";
        logFile << "plt.plot(time, trajLFootZ, 'b', linewidth=0.5, label='LFootZ')\n";
        logFile << "plt.legend()\n";
        logFile << "plt.savefig('latest_test/trajFoot.pdf', format='pdf', dpi=1000)\n";
        logFile << "plt.show()\n";
    }
    void plotXYZ(std::ofstream &logFile) const
    {
        /***********************************
         *  PLOT COM AND COP TRAJECTORIES  *
         ***********************************/
        logFile << "f, (ax1, ax2, ax3) = plt.subplots(3, sharex=False, sharey=False)\n";
        logFile << "ax1.plot(t, x[0:len(t)],       'g--' , linewidth=0.5, label='xCoM')\n";
        logFile << "ax1.plot(t, xMin[0:len(t)],    'b--' , linewidth=0.5, label='xCoMMin')\n";
        logFile << "ax1.plot(t, xMax[0:len(t)],    'r--' , linewidth=0.5, label='xCoMMax')\n";
        logFile << "ax1.plot(t, xCoPMin[0:len(t)], 'b'   , linewidth=0.5, label='xCoPMin')\n";
        logFile << "ax1.plot(t, xCoP[0:len(t)],    'g'   , linewidth=0.5, label='xCoP')\n";
        logFile << "ax1.plot(t, xCoPMax[0:len(t)], 'r'   , linewidth=0.5, label='xCoPMax')\n";

        logFile << "ax2.plot(t, y[0:len(t)],       'g--' , linewidth=0.5 , label='yCoM')\n";
        logFile << "ax2.plot(t, yMin[0:len(t)],    'b--' , linewidth=0.5 , label='yCoMMin')\n";
        logFile << "ax2.plot(t, yMax[0:len(t)],    'r--' , linewidth=0.5 , label='yCoMMax')\n";
        logFile << "ax2.plot(t, yCoPMin[0:len(t)], 'b'   , linewidth=0.5 , label='yCoPMin')\n";
        logFile << "ax2.plot(t, yCoP[0:len(t)],    'g'   , linewidth=0.5 , label='yCoP')\n";
        logFile << "ax2.plot(t, yCoPMax[0:len(t)], 'r'   , linewidth=0.5 , label='yCoPMax')\n";

        logFile << "ax3.plot(t, z[0:len(t)],       'g--' , linewidth=0.5 , label='zCoM')\n";
        logFile << "ax3.plot(t, zMin[0:len(t)],    'b--' , linewidth=0.5 , label='zCoMMin')\n";
        logFile << "ax3.plot(t, zMax[0:len(t)],    'r--' , linewidth=0.5 , label='zCoMMax')\n";
        logFile << "ax3.plot(t, zCoPMin[0:len(t)], 'b'   , linewidth=0.5 , label='zCoPMin')\n";
        logFile << "ax3.plot(t, zCoP[0:len(t)],    'g'   , linewidth=0.5 , label='zCoP')\n";
        logFile << "ax3.plot(t, zCoPMax[0:len(t)], 'r'   , linewidth=0.5 , label='zCoPMax')\n";

        logFile << "ax1.set_xlabel('Time (s)')\n";
        logFile << "ax2.set_xlabel('Time (s)')\n";
        logFile << "ax3.set_xlabel('Time (s)')\n";
        logFile << "ax1.set_ylabel('x (m)')\n";
        logFile << "ax2.set_ylabel('y (m)')\n";
        logFile << "ax3.set_ylabel('z (m)')\n";
        logFile << "ax1.legend(loc='upper right', shadow=True)\n";
        logFile << "ax2.legend(loc='upper right', shadow=True)\n";
        logFile << "ax3.legend(loc='upper right', shadow=True)\n";
        logFile << "plt.axis('auto')\n";
        logFile << "plt.savefig('latest_test/xyz.pdf', format='pdf', dpi=1000)\n";
        logFile << "plt.show()\n";
    }
    void plot3Dtraj(std::ofstream &logFile) const
    {
        logFile << "def plotStep(ax, pos, wX, wY, color):\n";
        logFile << "    x = pos[0]\n";
        logFile << "    y = pos[1]\n";
        logFile << "    z = pos[2]\n";
        logFile << "    pointsX = np.array([x-wX*0.5, x+wX*0.5, x+wX*0.5, x-wX*0.5, x-wX*0.5])\n";
        logFile << "    pointsY = np.array([y+wY*0.5, y+wY*0.5, y-wY*0.5, y-wY*0.5, y+wY*0.5])\n";
        logFile << "    pointsZ = np.array([z,z,z,z,z])\n";
        logFile << "    ax.plot(pointsX, pointsY, pointsZ, color, linewidth=0.5)\n";
        /************************************
         *  PLOT ALL TRAJECTORIES TOGETHER  *
         ************************************/
        logFile << "fig = plt.figure()\n";
        logFile << "ax = fig.gca(projection='3d')\n";
        logFile << "for step in leftFootSteps:\n";
        logFile << "    plotStep(ax, step, 0.2, 0.1, 'r')\n";
        logFile << "for step in rightFootSteps:\n";
        logFile << "    plotStep(ax, step, 0.2, 0.1, 'b')\n";
        logFile << "ax.plot(trajLFootX, trajLFootY, trajLFootZ, 'r', linewidth=0.5, label='Left foot')\n";
        logFile << "ax.plot(trajRFootX, trajRFootY, trajRFootZ, 'b', linewidth=0.5, label='Right foot')\n";
        logFile << "ax.plot(x, y, z, 'g', linewidth=0.5, label='CoM')\n";
        logFile << "ax.plot(x, y, highestFeasibleZ, '--g', linewidth=0.5, label='CoM')\n";
        logFile << "ax.plot(xCoP, yCoP, zCoP, 'y', linewidth=0.5, label='CoP')\n";
        logFile << "ax.legend()\n";
        logFile << "plt.savefig('latest_test/3D.pdf', format='pdf', dpi=1000)\n";
        logFile << "plt.show()\n";
    }

    void plotPlaneProjectionTraj(std::ofstream &logFile) const
    {
        /****************************************
         *  PLOT SAGITAL AND TRANSVERSE MOTION  *
         ****************************************/
        logFile << "f, (ax1, ax2, ax3) = plt.subplots(3, sharex=False, sharey=False)\n";
        logFile << "ax1.plot(y, z, 'r', linewidth=0.5, label='z_CoM = f(y_CoM)')\n";
        logFile << "ax1.plot(y, highestFeasibleZ, '--g', label='z_Max = f(y_CoM)')\n";
        logFile << "ax2.plot(x, z, 'b', linewidth=0.5, label='z_CoM = f(x_CoM)')\n";
        logFile << "ax2.plot(x, highestFeasibleZ, '--g', label='z_Max = f(x_CoM)')\n";
        logFile << "ax3.plot(x, y, 'g', linewidth=0.5, label='x_CoM = f(y_CoM)')\n";
        logFile << "ax1.legend(loc='lower left', shadow=False)\n";
        logFile << "ax2.legend(loc='lower left', shadow=False)\n";
        logFile << "ax3.legend(loc='lower left', shadow=False)\n";
        logFile << "plt.savefig('latest_test/projections.pdf', format='pdf', dpi=1000)\n";
        logFile << "plt.show()\n";
    }

    /// @brief Writes a python file that can later be executed to display the history of the system
    void plot() const
    {
        std::ofstream logFile("plotFile.py");

        logFile << "import matplotlib as mpl\n";
        logFile << "from mpl_toolkits.mplot3d import Axes3D\n";
        logFile << "import numpy as np\n";
        logFile << "import matplotlib.pyplot as plt\n";
        logFile << "import os\n";
        logFile << "if not os.path.isdir('latest_test'):\n";
        logFile << "    os.mkdir('latest_test')\n";

        logEverything(logFile);
        plotTrajectoryFoot(logFile);
        plotXYZ(logFile);
        plot3Dtraj(logFile);
        plotPlaneProjectionTraj(logFile);

        logFile.close();
    }

  private:
    std::vector<Eigen::Vector3d> positionsCoM_;
    std::vector<Eigen::Vector3d> positionsCoPMin_;
    std::vector<Eigen::Vector3d> positionsCoP_;
    std::vector<Eigen::Vector3d> positionsCoPMax_;
    std::vector<Eigen::Vector3d> velocitiesCoM_;
    std::vector<Eigen::Vector3d> accelerationsCoM_;
    std::vector<Eigen::Vector3d> jerksCoM_;
    mutable Eigen::VectorXd highestFeasibleZ_;
    Eigen::VectorXd xMin_;
    Eigen::VectorXd xMax_;
    Eigen::VectorXd yMin_;
    Eigen::VectorXd yMax_;
    Eigen::VectorXd zMin_;
    Eigen::VectorXd zMax_;
    const StepPlan &stepPlan_;
    FootTraj rightFootTraj_, leftFootTraj_;
    double timeStep_;
    const ProblemParameters &pbParams_;
    double gravity_;
};
}  // namespace wpg05
}  // namespace humoto
