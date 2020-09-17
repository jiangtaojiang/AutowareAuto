# Copyright 2020, The Autoware Foundation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Launch Modules for Milestone 3 of the AVP 2020 Demo."""

from launch import LaunchContext
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.conditions import IfCondition
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackage
from pathlib import Path

import os


context = LaunchContext()


def get_package_share_directory(package_name):
    """Return the absolute path to the share directory of the given package."""
    return os.path.join(Path(FindPackage(package_name).perform(context)), 'share', package_name)


def generate_launch_description():
    """
    Launch all nodes defined in the architecture for Milestone 3 of the AVP 2020 Demo.

    More details about what is included can
    be found at https://gitlab.com/autowarefoundation/autoware.auto/AutowareAuto/-/milestones/25.
    """
    avp_demo_pkg_prefix = get_package_share_directory('autoware_auto_avp_demo')
    euclidean_cluster_param_file = os.path.join(
        avp_demo_pkg_prefix, 'param/euclidean_cluster.param.yaml')
    ray_ground_classifier_param_file = os.path.join(
        avp_demo_pkg_prefix, 'param/ray_ground_classifier.param.yaml')
    rviz_cfg_path = os.path.join(avp_demo_pkg_prefix, 'config/ms3.rviz')
    scan_downsampler_param_file = os.path.join(
        avp_demo_pkg_prefix, 'param/scan_downsampler_ms3.param.yaml')
    recordreplay_planner_param_file = os.path.join(
        avp_demo_pkg_prefix, 'param/recordreplay_planner.param.yaml')

    pc_filter_transform_pkg_prefix = get_package_share_directory(
        'point_cloud_filter_transform_nodes')
    pc_filter_transform_param_file = os.path.join(
        pc_filter_transform_pkg_prefix, 'param/vlp16_sim_lexus_filter_transform.param.yaml')

    point_cloud_fusion_pkg_prefix = get_package_share_directory(
        'point_cloud_fusion')
    point_cloud_fusion_param_file = os.path.join(
        point_cloud_fusion_pkg_prefix, 'param/vlp16_sim_lexus_pc_fusion.param.yaml')

    # Arguments

    euclidean_cluster_param = DeclareLaunchArgument(
        'euclidean_cluster_param_file',
        default_value=euclidean_cluster_param_file,
        description='Path to config file for Euclidean Clustering'
    )
    pc_filter_transform_param = DeclareLaunchArgument(
        'pc_filter_transform_param_file',
        default_value=pc_filter_transform_param_file,
        description='Path to config file for Point Cloud Filter/Transform Nodes'
    )
    ray_ground_classifier_param = DeclareLaunchArgument(
        'ray_ground_classifier_param_file',
        default_value=ray_ground_classifier_param_file,
        description='Path to config file for Ray Ground Classifier'
    )
    with_rviz_param = DeclareLaunchArgument(
        'with_rviz',
        default_value='True',
        description='Launch RVIZ2 in addition to other nodes'
    )
    scan_downsampler_param = DeclareLaunchArgument(
        'scan_downsampler_param_file',
        default_value=scan_downsampler_param_file,
        description='Path to config file for lidar scan downsampler'
    )
    recordreplay_planner_param = DeclareLaunchArgument(
        'recordreplay_planner_param_file',
        default_value=recordreplay_planner_param_file,
        description='Path to config file for record/replay planner'
    )
    point_cloud_fusion_param = DeclareLaunchArgument(
        'point_cloud_fusion_param_file',
        default_value=point_cloud_fusion_param_file,
        description='Path to config file for point cloud fusion'
    )

    # Nodes

    euclidean_clustering = Node(
        package='euclidean_cluster_nodes',
        node_executable='euclidean_cluster_node_exe',
        node_namespace='perception',
        parameters=[LaunchConfiguration('euclidean_cluster_param_file')],
        remappings=[
            ("points_in", "points_nonground")
        ]
    )
    filter_transform_vlp16_front = Node(
        package='point_cloud_filter_transform_nodes',
        node_executable='point_cloud_filter_transform_node_exe',
        node_name='filter_transform_vlp16_front',
        node_namespace='lidar_front',
        parameters=[LaunchConfiguration('pc_filter_transform_param_file')],
        remappings=[("points_in", "points_raw")]
    )
    filter_transform_vlp16_rear = Node(
        package='point_cloud_filter_transform_nodes',
        node_executable='point_cloud_filter_transform_node_exe',
        node_name='filter_transform_vlp16_rear',
        node_namespace='lidar_rear',
        parameters=[LaunchConfiguration('pc_filter_transform_param_file')],
        remappings=[("points_in", "points_raw")]
    )
    # point cloud fusion runner to fuse front and rear lidar
    point_cloud_fusion = Node(
        package='point_cloud_fusion',
        node_executable='pointcloud_fusion_node_exe',
        node_namespace='lidars',
        parameters=[LaunchConfiguration('point_cloud_fusion_param_file')],
        remappings=[
            ("output_topic", "points_fused"),
            ("input_topic1", "/lidar_front/points_filtered"),
            ("input_topic2", "/lidar_rear/points_filtered")
        ]
    )
    ray_ground_classifier = Node(
        package='ray_ground_classifier_nodes',
        node_executable='ray_ground_classifier_cloud_node_exe',
        node_namespace='perception',
        parameters=[LaunchConfiguration('ray_ground_classifier_param_file')],
        remappings=[("points_in", "/lidars/points_fused")]
    )
    rviz2 = Node(
        package='rviz2',
        node_executable='rviz2',
        node_name='rviz2',
        arguments=['-d', str(rviz_cfg_path)],
        condition=IfCondition(LaunchConfiguration('with_rviz')),
        remappings=[("initialpose", "/localization/initialpose")]
    )
    scan_downsampler = Node(
        package='voxel_grid_nodes',
        node_executable='voxel_grid_node_exe',
        node_namespace='lidars',
        node_name='voxel_grid_cloud_node',
        parameters=[LaunchConfiguration('scan_downsampler_param_file')],
        remappings=[
            ("points_in", "points_fused"),
            ("points_downsampled", "points_fused_downsampled")
        ]
    )
    recordreplay_planner = Node(
        package='recordreplay_planner_node',
        node_executable='recordreplay_planner_node_exe',
        node_name='recordreplay_planner',
        node_namespace='planning',
        parameters=[LaunchConfiguration('recordreplay_planner_param_file')],
        remappings=[
            ('vehicle_state', '/vehicle/vehicle_kinematic_state'),
            ('planned_trajectory', '/planning/trajectory'),
            ('obstacle_bounding_boxes', '/perception/lidar_bounding_boxes'),
        ]
    )

    return LaunchDescription([
        euclidean_cluster_param,
        pc_filter_transform_param,
        ray_ground_classifier_param,
        scan_downsampler_param,
        with_rviz_param,
        recordreplay_planner_param,
        point_cloud_fusion_param,
        euclidean_clustering,
        filter_transform_vlp16_front,
        filter_transform_vlp16_rear,
        ray_ground_classifier,
        scan_downsampler,
        recordreplay_planner,
        point_cloud_fusion,
        rviz2
    ])