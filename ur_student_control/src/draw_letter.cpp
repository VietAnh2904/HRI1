#include <rclcpp/rclcpp.hpp>
#include <moveit/move_group_interface/move_group_interface.h>
#include <geometry_msgs/msg/pose.hpp>
#include <visualization_msgs/msg/marker.hpp>
#include <thread>

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    rclcpp::NodeOptions node_options;
    node_options.automatically_declare_parameters_from_overrides(true);
    auto node = rclcpp::Node::make_shared("draw_letter_node", node_options);

    rclcpp::executors::SingleThreadedExecutor executor;
    executor.add_node(node);
    std::thread([&executor]() { executor.spin(); }).detach();

    // 1. DÙNG QoS TRANSIENT LOCAL ĐỂ GHIM CHỮ A NGAY LẬP TỨC
    rclcpp::QoS qos(1);
    qos.transient_local();
    auto marker_pub = node->create_publisher<visualization_msgs::msg::Marker>("visualization_marker", qos);

    static const std::string PLANNING_GROUP = "ur_manipulator";
    moveit::planning_interface::MoveGroupInterface move_group(node, PLANNING_GROUP);

    // 2. TĂNG TỐC ĐỘ LÊN 50% (Sửa từ 0.1 thành 0.5)
    move_group.setMaxVelocityScalingFactor(0.5);
    move_group.setMaxAccelerationScalingFactor(0.5);

    // 3. TẠO VÀ PHÁT CHỮ A MẪU TRƯỚC KHI ĐẾM NGƯỢC THỜI GIAN
    visualization_msgs::msg::Marker marker;
    marker.header.frame_id = "base_link"; 
    marker.header.stamp = node->now();
    marker.ns = "letter_A_model";
    marker.id = 0;
    marker.type = visualization_msgs::msg::Marker::LINE_STRIP;
    marker.action = visualization_msgs::msg::Marker::ADD;
    marker.scale.x = 0.005; // Nét mảnh
    marker.color.a = 1.0;   
    marker.color.r = 1.0;   // Màu đỏ
    marker.color.g = 0.0;
    marker.color.b = 0.0;

    geometry_msgs::msg::Pose start_pose = move_group.getCurrentPose().pose;
    start_pose.position.y -= 0.10; start_pose.position.z -= 0.25; marker.points.push_back(start_pose.position);
    start_pose.position.y += 0.10; start_pose.position.z += 0.20; marker.points.push_back(start_pose.position);
    start_pose.position.y += 0.10; start_pose.position.z -= 0.20; marker.points.push_back(start_pose.position);
    start_pose.position.y -= 0.05; start_pose.position.z += 0.10; marker.points.push_back(start_pose.position);
    start_pose.position.y -= 0.10; marker.points.push_back(start_pose.position);

    // Phát chữ A lên hệ thống
    marker_pub->publish(marker);
    RCLCPP_INFO(node->get_logger(), "Da ghim chu A do len RViz!");

    // ĐẾM NGƯỢC 10 GIÂY (GIỮ NGUYÊN THEO THIẾT LẬP CỦA BẠN)
    RCLCPP_INFO(node->get_logger(), "Dang cho 10 giay de dong bo Gazebo...");
    rclcpp::sleep_for(std::chrono::seconds(10));

    RCLCPP_INFO(node->get_logger(), "Di chuyen den tu the chuan bi...");
    std::vector<double> ready_joints = {0.0, -1.57, 1.57, -1.57, -1.57, 0.0};
    move_group.setJointValueTarget(ready_joints);
    
    if (move_group.move() == moveit::core::MoveItErrorCode::SUCCESS) {
        RCLCPP_INFO(node->get_logger(), "Da den tu the chuan bi!");
    } else {
        RCLCPP_ERROR(node->get_logger(), "Loi khi di chuyen den tu the chuan bi!");
        rclcpp::shutdown();
        return 1;
    }

    // TẠO TỌA ĐỘ CHO ROBOT VẼ THEO
    std::vector<geometry_msgs::msg::Pose> waypoints;
    geometry_msgs::msg::Pose target_pose = move_group.getCurrentPose().pose;

    target_pose.position.y -= 0.10; target_pose.position.z -= 0.25; waypoints.push_back(target_pose);
    target_pose.position.y += 0.10; target_pose.position.z += 0.20; waypoints.push_back(target_pose);
    target_pose.position.y += 0.10; target_pose.position.z -= 0.20; waypoints.push_back(target_pose);
    target_pose.position.y -= 0.05; target_pose.position.z += 0.10; waypoints.push_back(target_pose);
    target_pose.position.y -= 0.10; waypoints.push_back(target_pose);

    moveit_msgs::msg::RobotTrajectory trajectory;
    const double jump_threshold = 0.0;
    const double eef_step = 0.01;
    
    RCLCPP_INFO(node->get_logger(), "Dang lap ke hoach do theo chu A...");
    double fraction = move_group.computeCartesianPath(waypoints, eef_step, jump_threshold, trajectory);

    if (fraction > 0.9) {
        move_group.execute(trajectory);
    } else {
        RCLCPP_ERROR(node->get_logger(), "Khong the lap ke hoach.");
    }

    rclcpp::shutdown();
    return 0;
}
