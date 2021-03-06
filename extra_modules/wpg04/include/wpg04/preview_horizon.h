/**
    @file
    @author  Alexander Sherikov
    @author  Don Joven Agravante
    @author  Jan Michalczyk
    @copyright 2014-2017 INRIA, 2014-2015 CNRS. Licensed under the Apache
    License, Version 2.0. (see @ref LICENSE or http://www.apache.org/licenses/LICENSE-2.0)
    @brief
*/

#pragma once


namespace humoto
{
    namespace wpg04
    {
        /**
         * @brief A class representing a "state" of the walk
         */
        class HUMOTO_LOCAL WalkState : public humoto::walking::Stance
        {
            public:
                etools::Vector2 cvel_ref_;
                etools::Vector2 cpos_ref_;

                double           theta_;

                etools::Matrix2 rotation_;
                etools::Vector2 R_cvel_ref_;
                etools::Vector2 R_cpos_ref_;

                double           step_height_;

                etools::Matrix2 fd_bounds_;
                etools::Matrix2 cop_bounds_;

                etools::Matrix2 cpos_bounds_;

            public:
                /**
                 * @brief Default constructor
                 */
                WalkState()
                {
                    theta_ = 0.0;
                    step_height_ = 0.0;

                    etools::unsetMatrix(cvel_ref_);
                    etools::unsetMatrix(cpos_ref_);

                    etools::unsetMatrix(rotation_);
                    etools::unsetMatrix(R_cvel_ref_);
                    etools::unsetMatrix(R_cpos_ref_);

                    etools::unsetMatrix(fd_bounds_);
                    etools::unsetMatrix(cop_bounds_);

                    etools::unsetMatrix(cpos_bounds_);
                }


                /**
                 * @brief Copy stance info
                 *
                 * @param[in] stance
                 *
                 * @return this
                 */
                WalkState& operator=(const humoto::walking::Stance & stance)
                {
                    humoto::walking::Stance::operator=(stance);
                    return (*this);
                }


                /**
                 * @brief Log
                 *
                 * @param[in,out] logger logger
                 * @param[in] parent parent
                 * @param[in] name name
                 */
                void log(   humoto::Logger    &logger HUMOTO_GLOBAL_LOGGER_IF_DEFINED,
                            const LogEntryName &parent = LogEntryName(),
                            const std::string &name = "walk_state") const
                {
                    LogEntryName subname = parent;
                    subname.add(name);

                    logger.log(LogEntryName(subname).add("theta")      , theta_      );
                    logger.log(LogEntryName(subname).add("step_height"), step_height_);
                    logger.log(LogEntryName(subname).add("cvel_ref")  , cvel_ref_);
                    logger.log(LogEntryName(subname).add("cpos_ref")  , cpos_ref_);
                    logger.log(LogEntryName(subname).add("rotation")  , rotation_);
                    logger.log(LogEntryName(subname).add("R_cvel_ref"), R_cvel_ref_);
                    logger.log(LogEntryName(subname).add("R_cpos_ref"), R_cpos_ref_);
                    logger.log(LogEntryName(subname).add("fd_bounds") , fd_bounds_);
                    logger.log(LogEntryName(subname).add("cop_bounds"), cop_bounds_);
                    logger.log(LogEntryName(subname).add("cpos_bounds"), cpos_bounds_);

                    humoto::walking::Stance::log(logger, subname, "stance");
                }
        };



        /**
         * @brief A helper class defining one interval of a preview horizon
         */
        class HUMOTO_LOCAL PreviewHorizonInterval
        {
            public:
                std::size_t state_index_;       /// in TDS points to the next SS

                etools::Vector2 cvel_ref_;
                etools::Vector2 cpos_ref_;

                etools::Matrix2 cvel_bounds_;
                //etools::Matrix2 cpos_bounds_;

                double com_height_;
                double omega_;
                double T_;
                std::size_t T_ms_;


            public:
                /**
                 * @brief Log
                 *
                 * @param[in,out] logger logger
                 * @param[in] parent parent
                 * @param[in] name name
                 */
                void log(   humoto::Logger & logger HUMOTO_GLOBAL_LOGGER_IF_DEFINED,
                            const LogEntryName &parent = LogEntryName(),
                            const std::string &name = "preview_interval") const
                {
                    LogEntryName subname = parent;
                    subname.add(name);

                    logger.log(LogEntryName(subname).add("state_index") , state_index_);
                    logger.log(LogEntryName(subname).add("omega")       , omega_      );
                    logger.log(LogEntryName(subname).add("T")           , T_          );
                    logger.log(LogEntryName(subname).add("cvel_ref")    , cvel_ref_   );
                    logger.log(LogEntryName(subname).add("cpos_ref")    , cpos_ref_   );
                    logger.log(LogEntryName(subname).add("cvel_bounds") , cvel_bounds_);
                    //logger.log(LogEntryName(subname).add("cpos_bounds"), cpos_bounds_);
                }
        };



        /**
         * @brief Preview horizon of an MPC [form_preview_horizon.m]
         */
        class HUMOTO_LOCAL PreviewHorizon
        {
            public:
                std::vector<std::size_t> variable_steps_indices_;

                std::vector<PreviewHorizonInterval> intervals_;
                std::vector<WalkState>              walk_states_;


            public:
                bool form(const MPCParameters                             &mpc_params,
                          const Model                                     &model,
                          const humoto::walking::StanceFiniteStateMachine &stance_fsm,
                          const WalkParameters                               &walk_parameters);

                std::size_t getNumberOfVariableSteps() const;

                void log(humoto::Logger &, const LogEntryName &, const std::string &) const;


                /**
                 * @brief Get CoP bounds
                 *
                 * @param[in] interval_index
                 *
                 * @return 2d matrix [lb, ub]
                 */
                etools::Matrix2    getCoPBounds(const std::size_t interval_index) const
                {
                    std::size_t state_index = intervals_[interval_index].state_index_;
                    return (walk_states_[state_index].cop_bounds_);
                }

                /**
                 * @brief Get CoM position bounds
                 *
                 * @param[in] interval_index
                 *
                 * @return 2d matrix [lb, ub]
                 */
                etools::Matrix2    getCoMBounds(const std::size_t interval_index) const
                {
                    std::size_t state_index = intervals_[interval_index].state_index_;
                    return (walk_states_[state_index].cpos_bounds_);
                }


                /**
                 * @brief Get bounds on foot position
                 *
                 * @param[in] var_support_index index of a variable step in the preview horizon
                 *
                 * @return 2d matrix [lb, ub]
                 */
                etools::Matrix2    getFootPositionBounds(const std::size_t var_support_index) const
                {
                    std::size_t interval_index = variable_steps_indices_[var_support_index];
                    std::size_t state_index = intervals_[interval_index].state_index_;
                    return (walk_states_[state_index].fd_bounds_);
                }

                /**
                 * @brief Get orientation of the support corresponding to the given interval
                 *
                 * @param[in] interval_index
                 *
                 * @return 2d rotation matrix
                 */
                etools::Matrix2    getRotationMatrix(const std::size_t interval_index) const
                {
                    std::size_t state_index = intervals_[interval_index].state_index_;
                    return (walk_states_[state_index].rotation_);
                }


                /**
                 * @brief Get walk state corresponding to the given interval
                 *
                 * @param[in] interval_index
                 *
                 * @return walk state
                 */
                const WalkState & getWalkState(const std::size_t interval_index) const
                {
                    std::size_t state_index = intervals_[interval_index].state_index_;
                    return (walk_states_[state_index]);
                }


            private:
                void getConstraints(humoto::wpg04::WalkState        &state,
                                    const Model                     &model,
                                    const WalkParameters            &walk_parameters);

                void previewStates( const Model                                     &model,
                                    const humoto::walking::StanceFiniteStateMachine &stance_fsm,
                                    const WalkParameters                            &walk_parameters,
                                    const std::size_t                               preview_duration);
        };



        /**
         * @brief Preview a sequence of walk states with velocities, rotations etc.
         *
         * @param[in] model
         * @param[in] stance_fsm
         * @param[in] walk_parameters
         * @param[in] preview_duration
         */
        void PreviewHorizon::previewStates(const Model                                     &model,
                                           const humoto::walking::StanceFiniteStateMachine &stance_fsm,
                                           const WalkParameters                            &walk_parameters,
                                           const std::size_t                               preview_duration)
        {
            std::vector<humoto::walking::Stance> stances;

            stances = stance_fsm.previewStances(preview_duration);

            double lss_theta = model.getFootState(humoto::LeftOrRight::LEFT).rpy_(humoto::AngleIndex::YAW);
            double rss_theta = model.getFootState(humoto::LeftOrRight::RIGHT).rpy_(humoto::AngleIndex::YAW);
            double ds_theta  = (lss_theta + rss_theta) / 2.;

            walk_states_.resize(stances.size());

            std::vector<std::vector<double> > steps;

            if (!walk_parameters.step_sequence_.empty())
            {
                const std::vector<double> zero = {0, 0, 0};
                steps.push_back(zero);

                for(std::size_t i = 1; i <= walk_parameters.step_sequence_.size()-1; i++)
                {
                    std::vector<double> o = walk_parameters.step_sequence_[i];
                    std::vector<double> p = walk_parameters.step_sequence_[i-1];
                    std::vector<double> stp = {o[0] - p[0], o[1] - p[1], o[2] - p[2]};
                    steps.push_back(stp);
                }
            }

            std::size_t step = 0;

            for(std::size_t i = 0; i < stances.size(); ++i)
            {
                WalkState state;
                state = stances.at(i);

                switch(state.type_)
                {
                    case humoto::walking::StanceType::LSS:
                        state.theta_        = lss_theta;
                        state.cvel_ref_     = walk_parameters.com_velocity_;
                        state.cpos_ref_     = walk_parameters.com_position_;
                        //state.cvel_bounds_  <<  walk_parameters.com_velocity_bound_x_(0), walk_parameters.com_velocity_bound_x_(1),
                        //                        walk_parameters.com_velocity_bound_y_(0), walk_parameters.com_velocity_bound_y_(1);
                        state.cpos_bounds_  <<  walk_parameters.com_position_bound_x_(0), walk_parameters.com_position_bound_x_(1),
                                                walk_parameters.com_position_bound_y_(0), 0;
                        break;
                    case humoto::walking::StanceType::RSS:
                        state.theta_        = rss_theta;
                        state.cvel_ref_     = walk_parameters.com_velocity_;
                        state.cpos_ref_     = walk_parameters.com_position_;
                        //state.cvel_bounds_  <<  walk_parameters.com_velocity_bound_x_(0), walk_parameters.com_velocity_bound_x_(1),
                        //                        walk_parameters.com_velocity_bound_y_(0), walk_parameters.com_velocity_bound_y_(1);
                        state.cpos_bounds_  <<  walk_parameters.com_position_bound_x_(0), walk_parameters.com_position_bound_x_(1),
                                                                                       0, walk_parameters.com_position_bound_y_(1);
                        break;
                    case humoto::walking::StanceType::DS:
                        if (state.subtype_ == humoto::walking::StanceSubType::FIRST)
                        {
                            state.cvel_ref_ = walk_parameters.first_stance_com_velocity_;
                        }
                        else if(state.subtype_ == humoto::walking::StanceSubType::LAST)
                        {
                            state.cvel_ref_ = walk_parameters.last_stance_com_velocity_;
                        }
                        else
                        {
                            HUMOTO_THROW_MSG("Unsupported stance subtype.");
                        }

                        state.theta_        = ds_theta;
                        state.cpos_ref_     = walk_parameters.com_position_;
                        //state.cvel_bounds_  <<  walk_parameters.com_velocity_bound_x_(0), walk_parameters.com_velocity_bound_x_(1),
                        //                        walk_parameters.com_velocity_bound_y_(0), walk_parameters.com_velocity_bound_y_(1);
                        // TODO This boundaries on the CoM are not correct
                        state.cpos_bounds_  <<  walk_parameters.com_position_bound_x_(0), walk_parameters.com_position_bound_x_(1),
                                                walk_parameters.com_position_bound_y_(0), walk_parameters.com_position_bound_y_(1);
                        break;
                    case humoto::walking::StanceType::TDS:
                        state.cvel_ref_ = walk_parameters.com_velocity_;
                        state.cpos_ref_ = walk_parameters.com_position_;
                        //state.cvel_bounds_ << walk_parameters.com_velocity_bound_x_(0), walk_parameters.com_velocity_bound_x_(1),
                        //                      walk_parameters.com_velocity_bound_y_(0), walk_parameters.com_velocity_bound_y_(1);
                        // TODO This boundaries on the CoM are not correct
                        state.cpos_bounds_  <<  walk_parameters.com_position_bound_x_(0), walk_parameters.com_position_bound_x_(1),
                                                walk_parameters.com_position_bound_y_(0), walk_parameters.com_position_bound_y_(1);

                        state.theta_ = ds_theta;

                        if((stances.at(i + 1).type_ == humoto::walking::StanceType::RSS) &&
                           (state.previous_nontds_stance_type_ != humoto::walking::StanceType::DS))
                        {
                            rss_theta = lss_theta + walk_parameters.theta_increment_;
                            state.theta_ = rss_theta;
                        }
                        if((stances.at(i + 1).type_ == humoto::walking::StanceType::LSS) &&
                           (state.previous_nontds_stance_type_ != humoto::walking::StanceType::DS))
                        {
                            lss_theta = rss_theta + walk_parameters.theta_increment_;
                            state.theta_ = lss_theta;
                        }
                        if((stances.at(i + 1).type_ == humoto::walking::StanceType::DS) &&
                           (state.previous_nontds_stance_type_ != humoto::walking::StanceType::LSS))
                        {
                            state.cvel_ref_ = walk_parameters.last_stance_com_velocity_;
                            state.theta_    = rss_theta;
                            ds_theta        = rss_theta;
                        }
                        if((stances.at(i + 1).type_ == humoto::walking::StanceType::DS) &&
                           (state.previous_nontds_stance_type_ != humoto::walking::StanceType::RSS))
                        {
                            state.cvel_ref_ = walk_parameters.last_stance_com_velocity_;
                            state.theta_    = lss_theta;
                            ds_theta        = lss_theta;
                        }
                        break;

                    default :
                        HUMOTO_THROW_MSG("Unsupported stance type.");
                        break;
                }

                state.rotation_    = Eigen::Rotation2Dd(state.theta_);
                state.R_cvel_ref_.noalias()  = state.rotation_ * state.cvel_ref_;
                state.R_cpos_ref_.noalias()  = state.rotation_ * state.cpos_ref_;
                state.step_height_ = 0.;

                state.cpos_bounds_ = state.rotation_ * state.cpos_bounds_;

                if (humoto::walking::StanceType::TDS != state.type_)
                {
                    humoto::wpg04::WalkParameters wp;

                    if ( !walk_parameters.step_sequence_.empty() )
                    {
                        std::vector<std::vector<double> > previewed_steps;
                        previewed_steps.push_back(steps[step]);
                        wp.step_sequence_.assign(previewed_steps.begin(), previewed_steps.end());
                    }

                    getConstraints(state, model, wp);
                    step++;
                }

                walk_states_[i] = state;
            }
        }



        /**
         * @brief Returns number of variable steps in the preview.
         *
         * @return number of variable steps in the preview
         */
        std::size_t PreviewHorizon::getNumberOfVariableSteps() const
        {
            return (variable_steps_indices_.size());
        }



        /**
         * @brief Form the preview horizon object
         *
         * @param[in] mpc_params
         * @param[in] model
         * @param[in] stance_fsm
         * @param[in] walk_parameters
         *
         * @return
         */
        bool PreviewHorizon::form(const MPCParameters                             &mpc_params,
                                  const Model                                     &model,
                                  const humoto::walking::StanceFiniteStateMachine &stance_fsm,
                                  const WalkParameters                            &walk_parameters)
        {
            bool preview_horizon_formed = true;

            // Length of the preview horizon (N)
            std::size_t preview_horizon_length_ = mpc_params.preview_horizon_length_;
            std::size_t sampling_time_ms_       = mpc_params.sampling_time_ms_;
            std::size_t walk_index              = 0;
            std::size_t num_intervals_left      = 0;

            //is_new_sample_ = stance_fsm.is_new_sample_;
            previewStates(model, stance_fsm, walk_parameters, preview_horizon_length_*sampling_time_ms_);

            if(walk_states_.empty())
            {
                preview_horizon_formed = false;
                return (preview_horizon_formed);
            }

            variable_steps_indices_.clear();

            std::vector<std::size_t> support_indices;

            intervals_.resize(preview_horizon_length_);

            std::size_t interval_index = 0;
            while(interval_index < preview_horizon_length_)
            {
                WalkState state = walk_states_.at(walk_index);

                PreviewHorizonInterval  interval;
                interval.cvel_ref_    = state.R_cvel_ref_;
                interval.cpos_ref_    = state.R_cpos_ref_;
                interval.cvel_bounds_ << walk_parameters.com_velocity_bound_x_(0), walk_parameters.com_velocity_bound_x_(1),
                                         walk_parameters.com_velocity_bound_y_(0), walk_parameters.com_velocity_bound_y_(1);
                interval.com_height_  = model.getCoMHeight();
                interval.omega_       = model.getOmega(interval.com_height_);

                support_indices.push_back(interval_index);

                if (walk_index == 0)
                {
                    // current sampling interval (the 1st in the preview window)
                    if (state.type_ == humoto::walking::StanceType::TDS)
                    {
                        interval.T_ms_ = state.duration_ms_;
                        interval.T_ = convertMillisecondToSecond(interval.T_ms_);

                        ++walk_index;
                        state = walk_states_.at(walk_index);
                        num_intervals_left = floor(state.duration_ms_ / mpc_params.sampling_time_ms_);
                    }
                    else
                    {
                        interval.T_ms_      = state.duration_ms_ % mpc_params.sampling_time_ms_;
                        interval.T_         = convertMillisecondToSecond(interval.T_ms_);
                        num_intervals_left  = floor(state.duration_ms_ / mpc_params.sampling_time_ms_);

                        if ((state.duration_ms_ % mpc_params.sampling_time_ms_) == 0)
                        {
                            interval.T_ms_ = mpc_params.sampling_time_ms_;
                            interval.T_ = convertMillisecondToSecond(interval.T_ms_);

                            --num_intervals_left;
                        }
                    }

                    interval.state_index_ = walk_index;
                    intervals_[interval_index] = interval;
                    ++interval_index;
                }
                else
                {
                    HUMOTO_ASSERT(humoto::walking::StanceType::TDS == state.type_, "Unexpected state type!");

                    // new variable footstep
                    variable_steps_indices_.push_back(interval_index);

                    // transitional DS
                    interval.T_ms_ = mpc_params.tds_sampling_time_ms_;
                    interval.T_ = convertMillisecondToSecond(interval.T_ms_);

                    // next SS
                    ++walk_index;
                    state = walk_states_.at(walk_index);

                    interval.state_index_ = walk_index;
                    intervals_[interval_index] = interval;
                    ++interval_index;

                    num_intervals_left = floor(state.duration_ms_ / mpc_params.sampling_time_ms_);
                }

                interval.T_ms_ = mpc_params.sampling_time_ms_;
                interval.T_ = convertMillisecondToSecond(interval.T_ms_);

                // fill the other intervals
                std::size_t max_index = std::min(preview_horizon_length_, interval_index + num_intervals_left);
                for(std::size_t i = interval_index; i < max_index; ++i)
                {
                    intervals_[i] = interval;
                }

                interval_index += num_intervals_left;
                ++walk_index;
            }

            return (preview_horizon_formed);
        }


        /**
         * @brief Get constraints for each state
         *
         * @param[in,out] state
         * @param[in] model
         * @param[in] walk_parameters
         */
        void PreviewHorizon::getConstraints(WalkState             &state,
                                            const Model           &model,
                                            const WalkParameters  &walk_parameters)
        {
            if(state.subtype_ == humoto::walking::StanceSubType::FIRST)
            {
                state.fd_bounds_.setZero();
            }
            else
            {
                if (!walk_parameters.step_sequence_.empty())
                {
                    state.fd_bounds_ << walk_parameters.step_sequence_[0][0], walk_parameters.step_sequence_[0][0],
                                        walk_parameters.step_sequence_[0][1], walk_parameters.step_sequence_[0][1];
                }
                else
                {
                    switch(state.type_)
                    {
                        case humoto::walking::StanceType::LSS:
                            switch(state.previous_nontds_stance_type_)
                            {
                                case humoto::walking::StanceType::DS:
                                    state.fd_bounds_ = model.getFootBounds(humoto::LeftOrRight::LEFT, humoto::walking::FootBoundsType::WITH_RESPECT_TO_ADS);
                                    break;
                                case humoto::walking::StanceType::RSS:
                                    state.fd_bounds_ = model.getFootBounds(humoto::LeftOrRight::LEFT, humoto::walking::FootBoundsType::WITH_RESPECT_TO_SS);
                                    break;
                                default:
                                    HUMOTO_THROW_MSG("unexpected support type");
                                    break;
                            }
                            break;

                        case humoto::walking::StanceType::RSS:
                            switch(state.previous_nontds_stance_type_)
                            {
                                case humoto::walking::StanceType::DS:
                                    state.fd_bounds_ = model.getFootBounds(humoto::LeftOrRight::RIGHT, humoto::walking::FootBoundsType::WITH_RESPECT_TO_ADS);
                                    break;
                                case humoto::walking::StanceType::LSS:
                                    state.fd_bounds_ = model.getFootBounds(humoto::LeftOrRight::RIGHT, humoto::walking::FootBoundsType::WITH_RESPECT_TO_SS);
                                    break;
                                default:
                                    HUMOTO_THROW_MSG("unexpected support type");
                                    break;
                            }
                            break;

                        case humoto::walking::StanceType::DS:
                            switch(state.previous_nontds_stance_type_)
                            {
                                case humoto::walking::StanceType::LSS:
                                    state.fd_bounds_ = model.getFootBounds(humoto::LeftOrRight::RIGHT, humoto::walking::FootBoundsType::ALIGNED_TO_SS);
                                    break;
                                case humoto::walking::StanceType::RSS:
                                    state.fd_bounds_ = model.getFootBounds(humoto::LeftOrRight::LEFT, humoto::walking::FootBoundsType::ALIGNED_TO_SS);
                                    break;
                                default:
                                    HUMOTO_THROW_MSG("unexpected support type");
                                    break;
                            }
                            break;

                        default:
                            HUMOTO_THROW_MSG("unexpected support type");
                            break;
                    }
                }
            }

            if (humoto::walking::StanceType::DS == state.type_)
            {
                state.cop_bounds_ = model.getADSCoPBounds();
            }
            else
            {
                state.cop_bounds_ = model.getSSCoPBounds();
            }
        }


        /**
         * @brief Log
         *
         * @param[in,out] logger logger
         * @param[in] parent parent
         * @param[in] name name
         */
        void PreviewHorizon::log(   humoto::Logger & logger HUMOTO_GLOBAL_LOGGER_IF_DEFINED,
                                    const LogEntryName &parent = LogEntryName(),
                                    const std::string &name = "preview_horizon") const
        {
            LogEntryName subname = parent;
            LogEntryName subname_loop;
            subname.add(name);


            // variable_steps_indices
            logger.log(LogEntryName(subname).add("variable_steps_indices"), variable_steps_indices_);


            // intervals
            subname_loop = subname;
            subname_loop.add("interval");
            for (std::size_t i = 0; i < intervals_.size(); ++i)
            {
                intervals_[i].log(logger, LogEntryName(subname_loop).add(i), "");
            }


            // states
            subname_loop = subname;
            subname_loop.add("state");
            for (std::size_t i = 0; i < walk_states_.size(); ++i)
            {
                walk_states_[i].log(logger, LogEntryName(subname_loop).add(i), "");
            }
        }
    } // end namespace wpg04
}// end namespace humoto
