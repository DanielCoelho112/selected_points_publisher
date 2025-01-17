/*
 * Copyright 2019-2020 Autoware Foundation. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ********************
 *  v0.1.0: drwnz (david.wong@tier4.jp) *
 *
 * selected_points_publisher.cpp
 *
 *  Created on: December 5th 2019
 */

#include "rviz/selection/selection_manager.h"
#include "rviz/viewport_mouse_event.h"
#include "rviz/display_context.h"
#include "rviz/selection/forwards.h"
#include "rviz/properties/property_tree_model.h"
#include "rviz/properties/property.h"
#include "rviz/properties/color_property.h"
#include "rviz/properties/vector_property.h"
#include "rviz/properties/float_property.h"
#include "rviz/view_manager.h"
#include "rviz/view_controller.h"
#include "OGRE/OgreCamera.h"

#include "selected_points_publisher/selected_points_publisher.hpp"

#include <ros/ros.h>
#include <ros/time.h>
#include <sensor_msgs/PointCloud2.h>
#include <std_msgs/Int32MultiArray.h>
#include <QVariant>
#include <visualization_msgs/Marker.h>

namespace rviz_plugin_selected_points_publisher
{
SelectedPointsPublisher::SelectedPointsPublisher()
{
  updateTopic();
}

SelectedPointsPublisher::~SelectedPointsPublisher()
{
}

void SelectedPointsPublisher::updateTopic()
{
  node_handle_.param("frame_id", tf_frame_, std::string("/base_link"));
  rviz_cloud_topic_ = std::string("/rviz_selected_points");
  rviz_cloud_border_topic_ = std::string("/rviz_selected_border_points");
  rviz_cloud_remove_topic_ = std::string("/rviz_selected_remove_points");
  rviz_cloud_clear_all_topic_ = std::string("/rviz_selected_clear_all_points");

  rviz_selected_publisher_ = node_handle_.advertise<sensor_msgs::PointCloud2>(rviz_cloud_topic_.c_str(), 1);
  rviz_selected_border_publisher_ = node_handle_.advertise<sensor_msgs::PointCloud2>(rviz_cloud_border_topic_.c_str(), 1);
  rviz_selected_remove_publisher_ = node_handle_.advertise<sensor_msgs::PointCloud2>(rviz_cloud_remove_topic_.c_str(), 1);
  rviz_selected_clear_all_publisher_ = node_handle_.advertise<sensor_msgs::PointCloud2>(rviz_cloud_clear_all_topic_.c_str(), 1);
  num_selected_points_ = 0;
}

int SelectedPointsPublisher::processKeyEvent(QKeyEvent* event, rviz::RenderPanel* panel)
{
  if (event->type() == QKeyEvent::KeyPress)

    // Todo : add letter 'c' to unselect one point


    if (event->key() == 'p' || event->key() == 'P')
    {
      ROS_INFO_STREAM_NAMED("SelectedPointsPublisher.updateTopic",
                            "Publishing " << num_selected_points_ << " selected points to topic "
                                          << node_handle_.resolveName(rviz_cloud_topic_));
      rviz_selected_publisher_.publish(selected_points_);
    }

    if (event->key() == 'b' || event->key() == 'B')
    {
      ROS_INFO_STREAM_NAMED("SelectedPointsPublisher.updateTopic",
                            "Publishing " << num_selected_points_ << " selected points to topic "
                                          << node_handle_.resolveName(rviz_cloud_border_topic_));
      rviz_selected_border_publisher_.publish(selected_points_);
    }

    if (event->key() == 'r' || event->key() == 'R')
    {
      ROS_INFO_STREAM_NAMED("SelectedPointsPublisher.updateTopic",
                            "Publishing " << num_selected_points_ << " selected points to topic "
                                          << node_handle_.resolveName(rviz_cloud_remove_topic_));
      rviz_selected_remove_publisher_.publish(selected_points_);
    }

    if (event->key() == 'c' || event->key() == 'C')
    {
      ROS_INFO_STREAM_NAMED("SelectedPointsPublisher.updateTopic",
                            "Publishing " << num_selected_points_ << " selected points to topic "
                                          << node_handle_.resolveName(rviz_cloud_clear_all_topic_));
      rviz_selected_clear_all_publisher_.publish(selected_points_);
    }


  return 0;
  }


int SelectedPointsPublisher::processMouseEvent(rviz::ViewportMouseEvent& event)
{
  int flags = rviz::SelectionTool::processMouseEvent(event);
  if (event.alt())
  {
    selecting_ = false;
  }
  else
  {
    if (event.leftDown())
    {
      selecting_ = true;
    }
  }

  if (selecting_)
  {
    if (event.leftUp())
    {
      this->processSelectedArea();
    }
  }
  return flags;
}

int SelectedPointsPublisher::processSelectedArea()
{
  rviz::SelectionManager* selection_manager = context_->getSelectionManager();
  rviz::M_Picked selection = selection_manager->getSelection();
  rviz::PropertyTreeModel* model = selection_manager->getPropertyModel();

  selected_points_.header.frame_id = context_->getFixedFrame().toStdString();
  selected_points_.height = 1;
  selected_points_.point_step = 5 * 5;
  selected_points_.is_dense = false;
  selected_points_.is_bigendian = false;
  selected_points_.fields.resize(5);

  selected_points_.fields[0].name = "x";
  selected_points_.fields[0].offset = 0;
  selected_points_.fields[0].datatype = sensor_msgs::PointField::FLOAT32;
  selected_points_.fields[0].count = 1;

  selected_points_.fields[1].name = "y";
  selected_points_.fields[1].offset = 4;
  selected_points_.fields[1].datatype = sensor_msgs::PointField::FLOAT32;
  selected_points_.fields[1].count = 1;

  selected_points_.fields[2].name = "z";
  selected_points_.fields[2].offset = 8;
  selected_points_.fields[2].datatype = sensor_msgs::PointField::FLOAT32;
  selected_points_.fields[2].count = 1;

  selected_points_.fields[3].name = "idx";
  selected_points_.fields[3].offset = 12;
  selected_points_.fields[3].datatype = sensor_msgs::PointField::FLOAT32;
  selected_points_.fields[3].count = 1;

  selected_points_.fields[4].name = "sensor_idx";
  selected_points_.fields[4].offset = 16;
  selected_points_.fields[4].datatype = sensor_msgs::PointField::FLOAT32;
  selected_points_.fields[4].count = 1;

  int i = 0;
  while (model->hasIndex(i, 0))
  {
    selected_points_.row_step = (i + 1) * selected_points_.point_step;
    selected_points_.data.resize(selected_points_.row_step);

    QModelIndex child_index = model->index(i, 0);

    rviz::Property* child = model->getProp(child_index);
    rviz::VectorProperty* subchild = (rviz::VectorProperty*)child->childAt(0);
    Ogre::Vector3 point_data = subchild->getVector();

    uint8_t* data_pointer = &selected_points_.data[0] + i * selected_points_.point_step;
    *(float*)data_pointer = point_data.x;
    data_pointer += 4;
    *(float*)data_pointer = point_data.y;
    data_pointer += 4;
    *(float*)data_pointer = point_data.z;
    data_pointer += 4;

    // Search for the idx value
    for (int j = 1; j < child->numChildren(); j++)
    {
      rviz::Property* grandchild = child->childAt(j);
      QString nameOfChild = grandchild->getName();
      QString idxValue("idx");

      if (nameOfChild.contains(idxValue))
      {
        rviz::FloatProperty* floatchild = (rviz::FloatProperty*)grandchild;
        float idx = floatchild->getValue().toFloat();
        *(float*)data_pointer = idx;
        break;
      }
    }
    data_pointer += 4;

    // Search for the sensor_idx value
    for (int j = 1; j < child->numChildren(); j++)
    {
      rviz::Property* grandchild = child->childAt(j);
      QString nameOfChild = grandchild->getName();
      QString sensorIdxValue("sensor_idx");

      if (nameOfChild.contains(sensorIdxValue))
      {
        rviz::FloatProperty* floatchild = (rviz::FloatProperty*)grandchild;
        float sensor_idx = floatchild->getValue().toFloat();
        *(float*)data_pointer = sensor_idx;
        break;
      }
    }
    data_pointer += 4;
    i++;
  }
  num_selected_points_ = i;
  ROS_INFO_STREAM_NAMED("SelectedPointsPublisher._processSelectedAreaAndFindPoints",
                        "Number of points in the selected area: " << num_selected_points_);

  selected_points_.width = i;
  selected_points_.header.stamp = ros::Time::now();
  return 0;
}
}  // namespace rviz_plugin_selected_points_publisher
#include <pluginlib/class_list_macros.h>
PLUGINLIB_EXPORT_CLASS(rviz_plugin_selected_points_publisher::SelectedPointsPublisher, rviz::Tool)
