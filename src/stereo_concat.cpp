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

#include "message_filters/subscriber.h"
#include "message_filters/time_synchronizer.h"
#include "message_filters/sync_policies/exact_time.h"

#include "cv_bridge/cv_bridge.hpp"
#include <opencv2/opencv.hpp>

#include <chrono>
#include <string>

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

    publisher_ = this->create_publisher<sensor_msgs::msg::Image>("stereo/concatenated", 10);

    // auto topic_callback =
    //     [this](sensor_msgs::msg::CompressedImage::SharedPtr msg) -> void
    // {
    //   RCLCPP_INFO(
    //       this->get_logger(), "Received image! Format: %s | Size: %zu bytes",
    //       msg->format.c_str(), msg->data.size());
    // };

    // leftSubscription_ = this->create_subscription<sensor_msgs::msg::CompressedImage>(left_image_topic, rclcpp::SensorDataQoS(), topic_callback);
    // rightSubscription_ = this->create_subscription<sensor_msgs::msg::CompressedImage>(right_image_topic, rclcpp::SensorDataQoS(), topic_callback);
    left_sub_.subscribe(this, left_image_topic, rclcpp::SensorDataQoS().get_rmw_qos_profile());
    right_sub_.subscribe(this, right_image_topic, rclcpp::SensorDataQoS().get_rmw_qos_profile());

    sync_ = std::make_shared<message_filters::TimeSynchronizer<sensor_msgs::msg::CompressedImage,
                                                               sensor_msgs::msg::CompressedImage>>(left_sub_, right_sub_, 10);

    sync_->registerCallback(std::bind(&StereoSubscriber::sync_callback, this, std::placeholders::_1, std::placeholders::_2));
    sync_->registerCallback(std::bind(&StereoSubscriber::sync_image_callback, this, std::placeholders::_1, std::placeholders::_2));

    RCLCPP_INFO(this->get_logger(),
                "Synchronized subscriber started for: %s and %s",
                left_image_topic.c_str(), right_image_topic.c_str());

    timer_ = this->create_wall_timer(std::chrono::milliseconds(20), std::bind(&StereoSubscriber::timer_callback, this));

    RCLCPP_INFO(this->get_logger(), "Node started. Publishing to /stereo/concatenated at 50Hz");

    //publisher_ = this->create_publisher<sensor_msgs::msg::Image>("stereo/concatenated", 10);
  }

private:
  void sync_callback(
      const sensor_msgs::msg::CompressedImage::ConstSharedPtr &left_msg,
      const sensor_msgs::msg::CompressedImage::ConstSharedPtr &right_msg)
  {
    RCLCPP_INFO(this->get_logger(), "Synced Pair Received! Timestamps: %d.%d | %d.%d",
                left_msg->header.stamp.sec, left_msg->header.stamp.nanosec,
                right_msg->header.stamp.sec, right_msg->header.stamp.nanosec);
  }
  void sync_image_callback(
      const sensor_msgs::msg::CompressedImage::ConstSharedPtr &left_msg,
      const sensor_msgs::msg::CompressedImage::ConstSharedPtr &right_msg)
  {
    try
    {
      cv::Mat left_img = cv_bridge::toCvCopy(left_msg, "bgr8")->image;
      cv::Mat right_img = cv_bridge::toCvCopy(right_msg, "bgr8")->image;
      if (left_img.rows == right_img.rows)
      {
        cv::hconcat(left_img, right_img, concatenated_img);

        // 3. Display the result (for debugging)
        //cv::imshow("Stereo Concat", concatenated_img);
        //cv::waitKey(1);
        RCLCPP_INFO(this->get_logger(), "Successfully processed stereo image.");
      }
      else
      {
        RCLCPP_ERROR(this->get_logger(), "Image row counts do not match: left %d, right %d", left_img.rows, right_img.rows);
      }
    }
    catch (cv_bridge::Exception &e)
    {
      RCLCPP_ERROR(this->get_logger(), "Could not convert image: %s", e.what());
    }
  }

  void timer_callback()
  {
    if (!this->concatenated_img.empty())
    {
      RCLCPP_INFO(this->get_logger(), "Publishing concatenated image...");
      auto msg = cv_bridge::CvImage(std_msgs::msg::Header(), "bgr8", this->concatenated_img).toImageMsg();
      msg->header.stamp = this->now();
      publisher_->publish(*msg);
    }
    else
    {
      RCLCPP_WARN(this->get_logger(), "No concatenated image to publish yet.");
    }
  }
  message_filters::Subscriber<sensor_msgs::msg::CompressedImage> left_sub_;
  message_filters::Subscriber<sensor_msgs::msg::CompressedImage> right_sub_;
  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr publisher_;

  std::shared_ptr<message_filters::TimeSynchronizer<
      sensor_msgs::msg::CompressedImage,
      sensor_msgs::msg::CompressedImage>>
      sync_;

  // rclcpp::Subscription<sensor_msgs::msg::CompressedImage>::SharedPtr leftSubscription_;
  // rclcpp::Subscription<sensor_msgs::msg::CompressedImage>::SharedPtr rightSubscription_;
  cv::Mat concatenated_img;
};

int main(int argc, char *argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<StereoSubscriber>());
  rclcpp::shutdown();
  return 0;
}
