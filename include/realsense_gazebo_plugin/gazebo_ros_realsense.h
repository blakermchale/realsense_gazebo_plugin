#ifndef _GAZEBO_ROS_REALSENSE_PLUGIN_
#define _GAZEBO_ROS_REALSENSE_PLUGIN_

#include "realsense_gazebo_plugin/RealSensePlugin.h"

#include "rclcpp/rclcpp.hpp"
#include <sensor_msgs/msg/camera_info.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <sensor_msgs/image_encodings.hpp>

#include <image_transport/image_transport.h>
#include <camera_info_manager/camera_info_manager.h>

#include <gazebo_ros/node.hpp>

#include <string>
#include <memory>

namespace gazebo
{
  /// \brief A plugin that simulates Real Sense camera streams.
  class GazeboRosRealsense : public RealSensePlugin
  {
    /// \brief Constructor.
    public: GazeboRosRealsense();

    /// \brief Destructor.
    public: ~GazeboRosRealsense();

    // Documentation Inherited.
    public: virtual void Load(physics::ModelPtr _model, sdf::ElementPtr _sdf);

    /// \brief Callback that publishes a received Depth Camera Frame as an
    /// ImageStamped message.
    public: virtual void OnNewDepthFrame();

    /// \brief Callback that publishes a received Camera Frame as an
    /// ImageStamped message.
    public: virtual void OnNewFrame(const rendering::CameraPtr cam,
                                    const transport::PublisherPtr pub);

    protected: boost::shared_ptr<camera_info_manager::CameraInfoManager> camera_info_manager_;

    /// \brief A pointer to the ROS node.
    ///  A node will be instantiated if it does not exist.
    protected: gazebo_ros::Node::SharedPtr rosnode_{nullptr};
    private: image_transport::ImageTransport* itnode_;
    protected: image_transport::CameraPublisher color_pub_, ir1_pub_, ir2_pub_, depth_pub_;

    /// \brief ROS image messages
    protected: sensor_msgs::msg::Image image_msg_, depth_msg_;
  };
}
#endif /* _GAZEBO_ROS_REALSENSE_PLUGIN_ */
