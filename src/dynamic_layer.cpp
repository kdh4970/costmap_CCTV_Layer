#include "simple_layers/dynamic_layer.h"
#include <pluginlib/class_list_macros.h>
#include <ros/ros.h>
#include <ros/console.h>
#include "nav_cctv/Locations.h"
#include "point_subscriber.h"

PLUGINLIB_EXPORT_CLASS(simple_layer_namespace::DynamicLayer, costmap_2d::Layer)

using costmap_2d::LETHAL_OBSTACLE;
using costmap_2d::NO_INFORMATION;



namespace simple_layer_namespace
{

DynamicLayer::DynamicLayer() {}


void DynamicLayer::onInitialize()
{
  ros::NodeHandle nh("~/" + name_);
  current_ = true;
  default_value_ = NO_INFORMATION;
  matchSize();


  dsrv_ = new dynamic_reconfigure::Server<costmap_2d::GenericPluginConfig>(nh);
  dynamic_reconfigure::Server<costmap_2d::GenericPluginConfig>::CallbackType cb = boost::bind(
      &DynamicLayer::reconfigureCB, this, _1, _2);
  dsrv_->setCallback(cb);
}


void DynamicLayer::matchSize()
{
  Costmap2D* master = layered_costmap_->getCostmap();
  resizeMap(master->getSizeInCellsX(), master->getSizeInCellsY(), master->getResolution(),
            master->getOriginX(), master->getOriginY());
}


void DynamicLayer::reconfigureCB(costmap_2d::GenericPluginConfig &config, uint32_t level)
{
  enabled_ = config.enabled;
}

void DynamicLayer::updateBounds(double robot_x, double robot_y, double robot_yaw, double* min_x,
                                           double* min_y, double* max_x, double* max_y)
{
  if (!enabled_)
    return;

  //double mark_x = robot_x + 0.5*cos(robot_yaw), mark_y = robot_y + 0.5*sin(robot_yaw);
  // cos^2 + sin^2 = 1 
  
  //ROS_INFO("Received point is %d, msg_seq is %d",*person_x,*message_seq);
  double mark_x = 150, mark_y= 100;
  unsigned int mx;
  unsigned int my;
  if(worldToMap(mark_x, mark_y, mx, my)){
    setCost(mx, my, LETHAL_OBSTACLE);
  }

  


  *min_x = std::min(*min_x, mark_x);
  *min_y = std::min(*min_y, mark_y);
  *max_x = std::max(*max_x, mark_x);
  *max_y = std::max(*max_y, mark_y);
}

void DynamicLayer::updateCosts(costmap_2d::Costmap2D& master_grid, int min_i, int min_j, int max_i,
                                          int max_j)
{
  if (!enabled_)
    return;

  for (int j = min_j; j < max_j; j++)
  {
    for (int i = min_i; i < max_i; i++)
    {
      int index = getIndex(i, j);
      if (costmap_[index] == NO_INFORMATION)
        continue;
      master_grid.setCost(i, j, costmap_[index]); 
    }
  }
}

} // end namespace