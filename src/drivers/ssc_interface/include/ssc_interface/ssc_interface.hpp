// Copyright 2020 The Autoware Foundation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/// \copyright Copyright 2020 The Autoware Foundation
/// \file
/// \brief This file defines the ssc_interface class.

#ifndef SSC_INTERFACE__SSC_INTERFACE_HPP_
#define SSC_INTERFACE__SSC_INTERFACE_HPP_

#include <ssc_interface/visibility_control.hpp>

#include <common/types.hpp>
#include <vehicle_interface/platform_interface.hpp>

#include <automotive_platform_msgs/msg/gear_command.hpp>
#include <automotive_platform_msgs/msg/gear_feedback.hpp>
#include <automotive_platform_msgs/msg/speed_mode.hpp>
#include <automotive_platform_msgs/msg/steering_feedback.hpp>
#include <automotive_platform_msgs/msg/steer_mode.hpp>
#include <automotive_platform_msgs/msg/turn_signal_command.hpp>
#include <automotive_platform_msgs/msg/velocity_accel_cov.hpp>
#include <autoware_auto_msgs/msg/high_level_control_command.hpp>
#include <autoware_auto_msgs/msg/raw_control_command.hpp>
#include <autoware_auto_msgs/msg/trajectory_point.hpp>
#include <autoware_auto_msgs/msg/vehicle_control_command.hpp>
#include <autoware_auto_msgs/msg/vehicle_kinematic_state.hpp>
#include <autoware_auto_msgs/msg/vehicle_state_command.hpp>
#include <autoware_auto_msgs/msg/vehicle_state_report.hpp>
#include <std_msgs/msg/bool.hpp>

#include <rclcpp/rclcpp.hpp>

#include <chrono>
#include <iostream>

using autoware::common::types::bool8_t;
using autoware::common::types::float32_t;

using automotive_platform_msgs::msg::GearCommand;
using automotive_platform_msgs::msg::GearFeedback;
using automotive_platform_msgs::msg::SpeedMode;
using automotive_platform_msgs::msg::SteeringFeedback;
using automotive_platform_msgs::msg::SteerMode;
using automotive_platform_msgs::msg::TurnSignalCommand;
using automotive_platform_msgs::msg::VelocityAccelCov;
using autoware_auto_msgs::msg::HighLevelControlCommand;
using autoware_auto_msgs::msg::RawControlCommand;
using autoware_auto_msgs::msg::TrajectoryPoint;
using autoware_auto_msgs::msg::VehicleControlCommand;
using autoware_auto_msgs::msg::VehicleKinematicState;
using autoware_auto_msgs::msg::VehicleStateCommand;

namespace ssc_interface
{

static constexpr float32_t STEERING_TO_TIRE_RATIO = 8.6F / 0.533F;

/// \brief Class for interfacing with AS SSC
class SSC_INTERFACE_PUBLIC SscInterface
  : public ::autoware::drivers::vehicle_interface::PlatformInterface
{
public:
  /// \brief Default constructor.
  /// \param[in] node Reference to node
  /// \param[in] wheelbase_m Vehicle wheelbase in meters
  /// \param[in] max_accel_mps2 Maximum acceleration in m/s^2
  /// \param[in] max_decel_mps2 Maximum deceleration in m/s^2
  /// \param[in] max_yaw_rate_rad Maximum rate of change of heading in radians/sec
  explicit SscInterface(
    rclcpp::Node & node,
    float32_t wheelbase_m,
    float32_t max_accel_mps2,
    float32_t max_decel_mps2,
    float32_t max_yaw_rate_rad
  );
  /// \brief Default destructor
  ~SscInterface() noexcept override = default;

  /// \brief Try to receive data from the vehicle platform, and update StateReport and Odometry.
  ///   Exceptions may be thrown on errors
  /// \param[in] timeout The maximum amount of time to check/receive data
  /// \return True if data was received before the timeout, false otherwise
  bool8_t update(std::chrono::nanoseconds timeout) override;
  /// \brief Send the state command to the vehicle platform.
  ///   Exceptions may be thrown on errors
  /// \param[in] msg The state command to send to the vehicle
  /// \return false if sending failed in some way, true otherwise
  bool8_t send_state_command(const VehicleStateCommand & msg) override;
  /// \brief Send the control command to the vehicle platform.
  ///   Exceptions may be thrown on errors
  /// \param[in] msg The control command to send to the vehicle
  /// \return false if sending failed in some way, true otherwise
  bool8_t send_control_command(const HighLevelControlCommand & msg);
  /// \brief Send the control command to the vehicle platform.
  ///   Exceptions may be thrown on errors
  /// \param[in] msg The control command to send to the vehicle
  /// \return false if sending failed in some way, true otherwise
  bool8_t send_control_command(const RawControlCommand & msg) override;
  /// \brief Send the control command to the vehicle platform.
  ///   Exceptions may be thrown on errors
  /// \param[in] msg The control command to send to the vehicle
  /// \return false if sending failed in some way, true otherwise
  bool8_t send_control_command(const VehicleControlCommand & msg) override;

private:
  // Publishers (to SSC)
  rclcpp::Publisher<GearCommand>::SharedPtr m_gear_cmd_pub;
  rclcpp::Publisher<SpeedMode>::SharedPtr m_speed_cmd_pub;
  rclcpp::Publisher<SteerMode>::SharedPtr m_steer_cmd_pub;
  rclcpp::Publisher<TurnSignalCommand>::SharedPtr m_turn_signal_cmd_pub;

  // Publishers (to Autoware)
  rclcpp::Publisher<VehicleKinematicState>::SharedPtr m_kinematic_state_pub;

  // Subscribers (from SSC)
  rclcpp::SubscriptionBase::SharedPtr m_dbw_state_sub, m_gear_feedback_sub, m_vel_accel_sub;

  rclcpp::Logger m_logger;
  float32_t m_veh_wheelbase;
  float32_t m_accel_limit;
  float32_t m_decel_limit;
  float32_t m_max_yaw_rate;

  void on_dbw_state_report(const std_msgs::msg::Bool::SharedPtr & msg);
  void on_gear_report(const GearFeedback::SharedPtr & msg);
  void on_steer_report(const SteeringFeedback::SharedPtr & msg);
  void on_vel_accel_report(const VelocityAccelCov::SharedPtr & msg);

  inline float32_t yaw_rate_to_curvature_rate(float32_t yaw_rate, float32_t velocity)
  {
    return 1.0F / (velocity / yaw_rate);
  }
};

}  // namespace ssc_interface

#endif  // SSC_INTERFACE__SSC_INTERFACE_HPP_