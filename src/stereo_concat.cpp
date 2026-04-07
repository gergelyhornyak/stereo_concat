// Copyright 2016 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/compressed_image.hpp"

class StereoSubscriber : public rclcpp::Node
{
public:
  StereoSubscriber() : Node("stereo_concat_subscriber")
  {
    this->declare_parameter<std::string>("left_image_topic", "/left_image");
    this->declare_parameter<std::string>("right_image_topic", "/right_image");

    std::string left_image_topic = this->get_parameter("left_image_topic").as_string();
    std::string right_image_topic = this->get_parameter("right_image_topic").as_string();

    RCLCPP_INFO(this->get_logger(), "Left image topic: %s", left_image_topic.c_str());
    RCLCPP_INFO(this->get_logger(), "Right image topic: %s", right_image_topic.c_str());

    auto topic_callback =
        [this](sensor_msgs::msg::CompressedImage::SharedPtr msg) -> void
    {
      RCLCPP_INFO(
          this->get_logger(), "Received image! Format: %s | Size: %zu bytes",
          msg->format.c_str(), msg->data.size());
    };

    leftSubscription_ = this->create_subscription<sensor_msgs::msg::CompressedImage>(left_image_topic, rclcpp::SensorDataQoS(), topic_callback);
    rightSubscription_ = this->create_subscription<sensor_msgs::msg::CompressedImage>(right_image_topic, rclcpp::SensorDataQoS(), topic_callback);

  }

private:

  rclcpp::Subscription<sensor_msgs::msg::CompressedImage>::SharedPtr leftSubscription_;
  rclcpp::Subscription<sensor_msgs::msg::CompressedImage>::SharedPtr rightSubscription_;
};

int main(int argc, char *argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<StereoSubscriber>());
  rclcpp::shutdown();
  return 0;
}
