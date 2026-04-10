
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
    x_ = VectorXd::Zero(16);
    x_(6) = 1.0;
    P_ = MatrixXd::Identity(16, 16) * 0.1;

    RCLCPP_INFO(this->get_logger(), "Quaternion EKF initialized with 16-state vector");

    imu_sub_ = this->create_subscription<px4_msgs::msg::SensorCombined>(
      "/fmu/out/sensor_combined", rclcpp::SensorDataQoS(),
      std::bind(&QuaternionEKF::imu_callback, this, std::placeholders::_1));

    gps_sub_ = this->create_subscription<px4_msgs::msg::SensorGps>(
      "/fmu/out/vehicle_gps_position", rclcpp::SensorDataQoS(),
      std::bind(&QuaternionEKF::gps_callback, this, std::placeholders::_1));
  }

private:
  VectorXd x_;
  MatrixXd P_;

  rclcpp::Subscription<px4_msgs::msg::SensorCombined>::SharedPtr imu_sub_;
  rclcpp::Subscription<px4_msgs::msg::SensorGps>::SharedPtr gps_sub_;

  void imu_callback(const px4_msgs::msg::SensorCombined::SharedPtr msg)
  {
    Vector3d gyro(msg->gyro_rad[0], msg->gyro_rad[1], msg->gyro_rad[2]);
    Vector3d gyro_corrected = gyro - x_.segment<3>(10);
    (void)gyro_corrected;
    RCLCPP_DEBUG(this->get_logger(), "IMU received");
  }

  void gps_callback(const px4_msgs::msg::SensorGps::SharedPtr msg)
  {
    RCLCPP_DEBUG(this->get_logger(), "GPS received - lat: %.6f lon: %.6f alt: %.2f",
      msg->latitude_deg, msg->longitude_deg, msg->altitude_msl_m);
  }
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<QuaternionEKF>());
  rclcpp::shutdown();
  return 0;
}
