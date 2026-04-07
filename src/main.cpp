#include "stereo_concat_node.hpp"

int main(int argc, char *argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<StereoSubscriber>());
  rclcpp::shutdown();
  return 0;
}
