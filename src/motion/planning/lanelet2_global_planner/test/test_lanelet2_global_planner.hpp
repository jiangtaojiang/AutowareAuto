// Copyright 2019 Apex.AI, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Co-developed by Tier IV, Inc. and Apex.AI, Inc.

#ifndef TEST_LANELET2_GLOBAL_PLANNER_HPP_
#define TEST_LANELET2_GLOBAL_PLANNER_HPP_

#include <gtest/gtest.h>
#include <lanelet2_global_planner/lanelet2_global_planner.hpp>
#include <common/types.hpp>
#include <experimental/filesystem>
#include <string>
#include <memory>
#include <vector>

using autoware::common::types::float64_t;
using autoware::common::types::bool8_t;
using autoware::motion::planning::lanelet2_global_planner::Lanelet2GlobalPlanner;

class TestGlobalPlannerBasicMap : public ::testing::Test
{
public:
  TestGlobalPlannerBasicMap()
  {
    node_ptr = std::make_shared<Lanelet2GlobalPlanner>();
    std::string root_folder = std::string(std::experimental::filesystem::current_path());
    std::string file_path = std::string("/test/map_data/mapping_example_pk.osm");
    std::string file = root_folder + file_path;
    float64_t lat = 51.502091;
    float64_t lon = -0.08719;
    float64_t alt = 39.0144;
    node_ptr->load_osm_map(file, lat, lon, alt);
    node_ptr->parse_lanelet_element();
  }
  std::shared_ptr<Lanelet2GlobalPlanner> node_ptr;
};

class TestGlobalPlannerFullMap : public ::testing::Test
{
public:
  TestGlobalPlannerFullMap()
  {
    node_ptr = std::make_shared<Lanelet2GlobalPlanner>();
    std::string root_folder = std::string(std::experimental::filesystem::current_path());
    std::string file_path =
      std::string("/test/map_data/AStuff_test_map.osm");
    std::string file = root_folder + file_path;
    float64_t lat = 37.380811523812845;
    float64_t lon = -121.90840595108715;
    float64_t alt = 11.0;
    node_ptr->load_osm_map(file, lat, lon, alt);
    node_ptr->parse_lanelet_element();
  }
  std::shared_ptr<Lanelet2GlobalPlanner> node_ptr;
};

TEST(TestFunction, Point3d_copy)
{
  auto assign_point3d = [](lanelet::Point3d & pcopy)
    {
      lanelet::Point3d p(lanelet::utils::getId(), 2.0, 4.0, 6.0);
      pcopy = p;
    };

  lanelet::Point3d point3d;
  assign_point3d(point3d);
  ASSERT_DOUBLE_EQ(2.0, point3d.x());
  ASSERT_DOUBLE_EQ(4.0, point3d.y());
  ASSERT_DOUBLE_EQ(6.0, point3d.z());
}

TEST(TestFunction, Point3d_eigen)
{
  lanelet::Point3d p1(lanelet::utils::getId(), 1.0, 2.0, 3.0);
  lanelet::Point3d p2(lanelet::utils::getId(), 4.0, 5.0, 6.0);
  Eigen::Vector3d pd = p1.basicPoint() - p2.basicPoint();
  Eigen::Vector3d pd2 = pd.array().square();
  float64_t dist = std::sqrt(pd2.x() + pd2.y() + pd2.z());
  float64_t actual = std::sqrt(std::pow(p2.x() - p1.x(), 2) +
      std::pow(p2.y() - p1.y(), 2) +
      std::pow(p2.z() - p1.z(), 2));
  ASSERT_DOUBLE_EQ(actual, dist);
}

TEST(TestFunction, Point3d_mean)
{
  lanelet::Point3d p1(lanelet::utils::getId(), 1, 4, 7);
  lanelet::Point3d p2(lanelet::utils::getId(), 2, 5, 8);
  lanelet::Point3d p3(lanelet::utils::getId(), 3, 6, 9);
  lanelet::LineString3d ls(lanelet::utils::getId(), {p1, p2, p3});
  size_t num_points = ls.size();
  // sum x,y,z points
  float64_t mean_x = 0.0;
  float64_t mean_y = 0.0;
  float64_t mean_z = 0.0;
  std::for_each(ls.begin(), ls.end(), [&](lanelet::Point3d p)
    {
      mean_x += p.x() / static_cast<float64_t>(num_points);
      mean_y += p.y() / static_cast<float64_t>(num_points);
      mean_z += p.z() / static_cast<float64_t>(num_points);
    });
  ASSERT_DOUBLE_EQ(2.0, mean_x);
  ASSERT_DOUBLE_EQ(5.0, mean_y);
  ASSERT_DOUBLE_EQ(8.0, mean_z);
}

TEST_F(TestGlobalPlannerBasicMap, test_find_parking)
{
  // Find parking id from point
  // parking id: 5830 location: 20.7607, -10.2697, 15.6 near_roads: 479054
  lanelet::Point3d position(lanelet::utils::getId(), 20.7607, -10.2697, 15.6);
  lanelet::Id parking_id = node_ptr->find_nearparking_from_point(position);
  EXPECT_EQ(parking_id, 5830);

  // Compute parking centre
  lanelet::Point3d p1;
  bool8_t ret = false;
  ret = node_ptr->compute_parking_center(parking_id, p1);
  EXPECT_TRUE(ret);

  // near road id
  lanelet::Id near_road = node_ptr->find_nearroute_from_parking(parking_id);
  EXPECT_EQ(near_road, 479054);
}

TEST_F(TestGlobalPlannerBasicMap, test_p2p_distance)
{
  lanelet::Point3d p1(lanelet::utils::getId(), 1.0, 2.0, 3.0);
  lanelet::Point3d p2(lanelet::utils::getId(), 4.0, 5.0, 6.0);
  float64_t dist = node_ptr->p2p_euclidean(p1, p2);
  float64_t actual = std::sqrt(std::pow(p2.x() - p1.x(), 2) +
      std::pow(p2.y() - p1.y(), 2) +
      std::pow(p2.z() - p1.z(), 2));
  ASSERT_DOUBLE_EQ(actual, dist);
}

TEST_F(TestGlobalPlannerBasicMap, test_lanes_chr2num)
{
  std::string str = "[u'429933', u'430462']";
  std::vector<lanelet::Id> num = node_ptr->lanelet_chr2num(str);
  ASSERT_EQ(num[0], 429933);
  ASSERT_EQ(num[1], 430462);
}

TEST_F(TestGlobalPlannerBasicMap, test_lanes_str2num)
{
  std::string str = "1258,4258,3798";
  std::vector<lanelet::Id> num = node_ptr->lanelet_str2num(str);
  ASSERT_EQ(num.size(), 3U);
  ASSERT_EQ(num[0], 1258);
  ASSERT_EQ(num[1], 4258);
  ASSERT_EQ(num[2], 3798);
}


TEST_F(TestGlobalPlannerFullMap, test_find_parkingaccess)
{
  lanelet::Id parking_id = 8008;
  lanelet::Id expect_parking_access_id = 9538;
  lanelet::Id parking_access_id = node_ptr->find_parkingaccess_from_parking(parking_id);
  EXPECT_EQ(parking_access_id, expect_parking_access_id);
  parking_id = 8663;
  expect_parking_access_id = 10140;
  parking_access_id = node_ptr->find_parkingaccess_from_parking(parking_id);
  EXPECT_EQ(parking_access_id, expect_parking_access_id);
  parking_id = 8664;
  expect_parking_access_id = 10150;
  parking_access_id = node_ptr->find_parkingaccess_from_parking(parking_id);
  EXPECT_EQ(parking_access_id, expect_parking_access_id);
  parking_id = 101930;
  expect_parking_access_id = 9140;
  parking_access_id = node_ptr->find_parkingaccess_from_parking(parking_id);
  EXPECT_EQ(parking_access_id, expect_parking_access_id);
}

TEST_F(TestGlobalPlannerFullMap, test_find_no_parkingaccess)
{
  lanelet::Id parking_id = 8220;
  lanelet::Id expect_parking_access_id = -1;
  lanelet::Id parking_access_id = node_ptr->find_parkingaccess_from_parking(parking_id);
  EXPECT_EQ(parking_access_id, expect_parking_access_id);
}

TEST_F(TestGlobalPlannerFullMap, test_find_lane)
{
  lanelet::Id parking_access_id = 7849;
  std::vector<lanelet::Id> lane_id = node_ptr->find_lane_from_parkingaccess(parking_access_id);
  EXPECT_EQ(lane_id, (std::vector<lanelet::Id>{6518, 6525}));

  parking_access_id = 10140;
  lane_id = node_ptr->find_lane_from_parkingaccess(parking_access_id);
  EXPECT_EQ(lane_id, (std::vector<lanelet::Id>{6399}));

  parking_access_id = 10150;
  lane_id = node_ptr->find_lane_from_parkingaccess(parking_access_id);
  EXPECT_EQ(lane_id, (std::vector<lanelet::Id>{6392}));
}

// test find route giving a single lane option
TEST_F(TestGlobalPlannerFullMap, test_find_route_single_lane)
{
  std::vector<lanelet::Id> start_lane_id;
  std::vector<lanelet::Id> end_lane_id;
  std::vector<lanelet::Id> route_id;
  // entry route
  start_lane_id = std::vector<lanelet::Id>{6714};
  end_lane_id = std::vector<lanelet::Id>{6546};
  route_id = node_ptr->get_lane_route(start_lane_id, end_lane_id);
  EXPECT_GT(route_id.size(), 0u);

  // designate drop-off 1 to parking route
  start_lane_id = std::vector<lanelet::Id>{6392};
  end_lane_id = std::vector<lanelet::Id>{6518};
  // result: 6392, 7319, 6686, 6672, 6504, 6560, 6742, 7387, 6518
  route_id = node_ptr->get_lane_route(start_lane_id, end_lane_id);
  EXPECT_GT(route_id.size(), 0u);

  // designate drop-off 2 to parking route
  start_lane_id = start_lane_id = std::vector<lanelet::Id>{6399};
  end_lane_id = start_lane_id = std::vector<lanelet::Id>{6490};
  // result: 6399, 6826, 6616, 6618, 6546, 6378, 6280, 6490
  route_id = node_ptr->get_lane_route(start_lane_id, end_lane_id);

  // parking to designate pick_up 1 route
  start_lane_id = start_lane_id = std::vector<lanelet::Id>{6525};
  end_lane_id = start_lane_id = std::vector<lanelet::Id>{6399};
  // 6525, 7251, 6749, 6567, 6511, 6679, 6693, 7013, 6399
  route_id = node_ptr->get_lane_route(start_lane_id, end_lane_id);
  EXPECT_GT(route_id.size(), 0u);
}

// test find route giving a multiple lane option (so can find route any direction)
TEST_F(TestGlobalPlannerFullMap, test_find_route_mutiple_lanes)
{
  std::vector<lanelet::Id> start_lane_id;
  std::vector<lanelet::Id> end_lane_id;
  std::vector<lanelet::Id> route_id;
  start_lane_id = std::vector<lanelet::Id>{6546, 6553};
  end_lane_id = std::vector<lanelet::Id>{6518, 6525};
  route_id = node_ptr->get_lane_route(start_lane_id, end_lane_id);
  EXPECT_GT(route_id.size(), 0u);
}

TEST_F(TestGlobalPlannerFullMap, test_find_parking_from_point)
{
  // Vehicle location in the map frame: -25.9749 102.129 -1.74268
  // parking id: 101930
  lanelet::Point3d position_1(lanelet::utils::getId(), -25.97, 102.12, -1.74);
  lanelet::Id parking_id_1 = node_ptr->find_nearparking_from_point(position_1);
  EXPECT_EQ(parking_id_1, 101930);

  // Vehicle location in the map frame: -12.5381 67.6976 -1.34077
  // parking id: 8113
  lanelet::Point3d position_2(lanelet::utils::getId(), -12.53, 67.69, -1.34);
  lanelet::Id parking_id_2 = node_ptr->find_nearparking_from_point(position_2);
  EXPECT_EQ(parking_id_2, 8113);
}

TEST_F(TestGlobalPlannerFullMap, test_plan_full_route)
{
  // take the parking spot from previous test
  autoware_auto_msgs::msg::TrajectoryPoint from_point;
  from_point.x = -25.97;
  from_point.y = 102.12;
  autoware_auto_msgs::msg::TrajectoryPoint to_point;
  to_point.x = -12.53;
  to_point.y = 67.69;
  std::vector<lanelet::Id> route;
  bool8_t result = node_ptr->plan_route(from_point, to_point, route);
  EXPECT_TRUE(result);
}

#endif  // TEST_LANELET2_GLOBAL_PLANNER_HPP_
