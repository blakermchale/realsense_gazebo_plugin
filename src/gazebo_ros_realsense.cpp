#include "realsense_gazebo_plugin/gazebo_ros_realsense.h"
#include <sensor_msgs/fill_image.hpp>

namespace
{
  std::string extractCameraName(const std::string& name);
  sensor_msgs::msg::CameraInfo cameraInfo(const sensor_msgs::msg::Image& image, float horizontal_fov);
}

namespace gazebo
{
// Register the plugin
GZ_REGISTER_MODEL_PLUGIN(GazeboRosRealsense)

GazeboRosRealsense::GazeboRosRealsense()
{
}

GazeboRosRealsense::~GazeboRosRealsense()
{
  RCLCPP_DEBUG(this->rosnode_->get_logger(), "Unloaded");
}

void GazeboRosRealsense::Load(physics::ModelPtr _model, sdf::ElementPtr _sdf)
{
  // Make sure the ROS node for Gazebo has already been initialized
  if (!rclcpp::is_initialized ()) {
    int argc = 0;
    char **argv = NULL;

    rclcpp::init (argc, argv);
  }
  // if (!rclcpp::is_initialized())
  // {
  //   RCLCPP_FATAL(this->rosnode_->get_logger(), "A ROS node for Gazebo has not been initialized, unable to load plugin. Load the Gazebo system plugin 'libgazebo_ros_api_plugin.so' in the gazebo_ros package)");
  //   return;
  // }
  // RCLCPP_INFO(this->rosnode_->get_logger(), "Realsense Gazebo ROS plugin loading.");

  // gzerr << _sdf->GetName();
  // gzerr << _sdf->ToString("");
  // gzerr << _sdf->Get<std::string>("name");
  RealSensePlugin::Load(_model, _sdf);
  this->rosnode_ = gazebo_ros::Node::Get(_sdf);//rclcpp::Node::make_shared(this->GetHandle());

  // initialize camera_info_manager
  this->camera_info_manager_.reset(
    new camera_info_manager::CameraInfoManager(this->rosnode_.get()));

  this->itnode_ = new image_transport::ImageTransport(this->rosnode_);

  this->color_pub_ = this->itnode_->advertiseCamera("~/color/image_raw", 2);
  this->ir1_pub_ = this->itnode_->advertiseCamera("~/ir/image_raw", 2);
  this->ir2_pub_ = this->itnode_->advertiseCamera("~/ir2/image_raw", 2);
  this->depth_pub_ = this->itnode_->advertiseCamera("~/depth/image_raw", 2);

  // if (pointCloud_)
  // {
  //   this->pointcloud_pub_ =
  //       this->rosnode_->advertise<sensor_msgs::msg::PointCloud2>(pointCloudTopic_, 2, false);
  // }
}

void GazeboRosRealsense::OnNewFrame(const rendering::CameraPtr cam,
                                    const transport::PublisherPtr pub)
{
#if GAZEBO_MAJOR_VERSION >= 9
  common::Time current_time = this->world->SimTime();
#else
  common::Time current_time = this->world->GetSimTime();
#endif

  // identify camera
  std::string camera_id = extractCameraName(cam->Name());
  const std::map<std::string, image_transport::CameraPublisher*> camera_publishers = {
    {COLOR_CAMERA_NAME, &(this->color_pub_)},
    {IRED1_CAMERA_NAME, &(this->ir1_pub_)},
    {IRED2_CAMERA_NAME, &(this->ir2_pub_)},
  };
  const auto image_pub = camera_publishers.at(camera_id);

  // copy data into image
  this->image_msg_.header.frame_id = prefix+camera_id;
  this->image_msg_.header.stamp.sec = current_time.sec;
  this->image_msg_.header.stamp.nanosec = current_time.nsec;

  // set image encoding
  const std::map<std::string, std::string> supported_image_encodings = {
    {"L_INT8", sensor_msgs::image_encodings::MONO8},
    {"RGB_INT8", sensor_msgs::image_encodings::RGB8},
  };
  const auto pixel_format = supported_image_encodings.at(cam->ImageFormat());

  // copy from simulation image to ROS msg
  sensor_msgs::fillImage(this->image_msg_,
    pixel_format,
    cam->ImageHeight(), cam->ImageWidth(),
    cam->ImageDepth() * cam->ImageWidth(),
    reinterpret_cast<const void*>(cam->ImageData()));

  // identify camera rendering
  const std::map<std::string, rendering::CameraPtr> cameras = {
    {COLOR_CAMERA_NAME, this->colorCam},
    {IRED1_CAMERA_NAME, this->ired1Cam},
    {IRED2_CAMERA_NAME, this->ired2Cam},
  };

  // publish to ROS
  auto camera_info_msg = cameraInfo(this->image_msg_, cameras.at(camera_id)->HFOV().Radian());
  image_pub->publish(this->image_msg_, camera_info_msg);
}

void GazeboRosRealsense::OnNewDepthFrame()
{
  // get current time
#if GAZEBO_MAJOR_VERSION >= 9
  common::Time current_time = this->world->SimTime();
#else
  common::Time current_time = this->world->GetSimTime();
#endif

  RealSensePlugin::OnNewDepthFrame();

  // copy data into image
  this->depth_msg_.header.frame_id = prefix+COLOR_CAMERA_NAME;
  this->depth_msg_.header.stamp.sec = current_time.sec;
  this->depth_msg_.header.stamp.nanosec = current_time.nsec;

  // set image encoding
  std::string pixel_format = sensor_msgs::image_encodings::TYPE_16UC1;

  // copy from simulation image to ROS msg
  sensor_msgs::fillImage(this->depth_msg_,
    pixel_format,
    this->depthCam->ImageHeight(), this->depthCam->ImageWidth(),
    2 * this->depthCam->ImageWidth(),
    reinterpret_cast<const void*>(this->depthMap.data()));

  // publish to ROS
  auto depth_info_msg = cameraInfo(this->depth_msg_, this->depthCam->HFOV().Radian());
  this->depth_pub_.publish(this->depth_msg_, depth_info_msg);
}

}

namespace
{
  std::string extractCameraName(const std::string& name)
  {
    if (name.find(COLOR_CAMERA_NAME) != std::string::npos) return COLOR_CAMERA_NAME;
    if (name.find(IRED1_CAMERA_NAME) != std::string::npos) return IRED1_CAMERA_NAME;
    if (name.find(IRED2_CAMERA_NAME) != std::string::npos) return IRED2_CAMERA_NAME;

    //RCLCPP_ERROR(this->rosnode_->get_logger(), "Unknown camera name");
    return COLOR_CAMERA_NAME;
  }

  sensor_msgs::msg::CameraInfo cameraInfo(const sensor_msgs::msg::Image& image, float horizontal_fov)
  {
    sensor_msgs::msg::CameraInfo info_msg;

    info_msg.header = image.header;
    info_msg.height = image.height;
    info_msg.width = image.width;

    float focal = 0.5 * image.width / tan(0.5 * horizontal_fov);

    info_msg.k[0] = focal;
    info_msg.k[4] = focal;
    info_msg.k[2] = info_msg.width * 0.5;
    info_msg.k[5] = info_msg.height * 0.5;
    info_msg.k[8] = 1.;

    info_msg.p[0] = info_msg.k[0];
    info_msg.p[5] = info_msg.k[4];
    info_msg.p[2] = info_msg.k[2];
    info_msg.p[6] = info_msg.k[5];
    info_msg.p[10] = info_msg.k[8];

    return info_msg;
  }
}

