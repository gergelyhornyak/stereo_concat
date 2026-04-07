#pragma once

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

class StereoSubscriber : public rclcpp::Node {
public:
    StereoSubscriber();
private:
    void sync_callback(
        const sensor_msgs::msg::CompressedImage::ConstSharedPtr &left_msg,
        const sensor_msgs::msg::CompressedImage::ConstSharedPtr &right_msg);
    void sync_image_callback(
        const sensor_msgs::msg::CompressedImage::ConstSharedPtr &left_msg,
        const sensor_msgs::msg::CompressedImage::ConstSharedPtr &right_msg);
    void timer_callback();
    message_filters::Subscriber<sensor_msgs::msg::CompressedImage> left_sub_;
    message_filters::Subscriber<sensor_msgs::msg::CompressedImage> right_sub_;
    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr publisher_;

    std::shared_ptr<message_filters::TimeSynchronizer<
        sensor_msgs::msg::CompressedImage,
        sensor_msgs::msg::CompressedImage>>
        sync_;

    cv::Mat concatenated_img;
};