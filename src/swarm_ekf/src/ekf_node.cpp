#include <rclcpp/rclcpp.hpp>
#include <px4_msgs/msg/sensor_combined.hpp>
#include <px4_msgs/msg/sensor_gps.hpp>
#include <Eigen/Dense>

using namespace Eigen;

class QuaternionEKF : public rclcpp::Node
{
public:
  QuaternionEKF() : Node("quaternion_ekf")
  {
    // STATE VECTOR (16 values total):
    // x_[0-2]  = position (x, y, z) in meters
    // x_[3-5]  = velocity (vx, vy, vz) in m/s
    // x_[6-9]  = quaternion (w, x, y, z) representing attitude
    // x_[10-12] = gyroscope bias (rad/s) - how much the gyro drifts
    // x_[13-15] = accelerometer bias (m/s^2) - how much the accel drifts
    x_ = VectorXd::Zero(16);
    x_(6) = 1.0; // quaternion starts as identity (no rotation)

    // COVARIANCE MATRIX (16x16):
    // Represents how uncertain we are about each state.
    // Starts as small identity - we're reasonably confident in initial state.
    P_ = MatrixXd::Identity(16, 16) * 0.1;

    RCLCPP_INFO(this->get_logger(), "Quaternion EKF initialized with 16-state vector");

    // SUBSCRIBE TO IMU:
    // SensorCombined gives us gyro + accelerometer at ~250Hz.
    // This will trigger imu_callback every time new data arrives.
    imu_sub_ = this->create_subscription<px4_msgs::msg::SensorCombined>(
      "/fmu/out/sensor_combined", rclcpp::SensorDataQoS(),
      std::bind(&QuaternionEKF::imu_callback, this, std::placeholders::_1));

    // SUBSCRIBE TO GPS:
    // SensorGps gives us lat/lon/alt at ~5Hz.
    // This will trigger gps_callback every time new data arrives.
    gps_sub_ = this->create_subscription<px4_msgs::msg::SensorGps>(
      "/fmu/out/vehicle_gps_position", rclcpp::SensorDataQoS(),
      std::bind(&QuaternionEKF::gps_callback, this, std::placeholders::_1));
  }

private:
  VectorXd x_; // 16-element state vector
  MatrixXd P_; // 16x16 covariance matrix

  rclcpp::Subscription<px4_msgs::msg::SensorCombined>::SharedPtr imu_sub_;
  rclcpp::Subscription<px4_msgs::msg::SensorGps>::SharedPtr gps_sub_;

  void imu_callback(const px4_msgs::msg::SensorCombined::SharedPtr msg)
  {
    // Extract raw gyro measurement from IMU
    Vector3d gyro(msg->gyro_rad[0], msg->gyro_rad[1], msg->gyro_rad[2]);

    // Subtract estimated gyro bias from state vector to get corrected reading.
    // This is why we track bias - raw gyro drifts over time, this corrects it.
    Vector3d gyro_corrected = gyro - x_.segment<3>(10);
    (void)gyro_corrected; // unused for now - prediction step goes here next

    RCLCPP_DEBUG(this->get_logger(), "IMU received");
  }

  void gps_callback(const px4_msgs::msg::SensorGps::SharedPtr msg)
  {
    // GPS gives us absolute position in lat/lon/alt.
    // This will be used in the EKF update step to correct our prediction.
    RCLCPP_DEBUG(this->get_logger(), "GPS received - lat: %.6f lon: %.6f alt: %.2f",
      msg->latitude_deg, msg->longitude_deg, msg->altitude_msl_m);
  }
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);  // start ROS2
  rclcpp::spin(std::make_shared<QuaternionEKF>()); // run node until killed
  rclcpp::shutdown();
  return 0;
}