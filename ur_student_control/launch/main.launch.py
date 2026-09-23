import os
import yaml
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, TimerAction
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node

def load_yaml(package_name, file_path):
    package_path = get_package_share_directory(package_name)
    absolute_file_path = os.path.join(package_path, file_path)
    with open(absolute_file_path, 'r') as file:
        return yaml.safe_load(file)

def generate_launch_description():
    # 1. Gọi file launch có sẵn của UR (Bật Gazebo + MoveIt)
    ur_sim_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(get_package_share_directory('ur_simulation_gz'), 'launch', 'ur_sim_moveit.launch.py')
        ),
        launch_arguments={'ur_type': 'ur3e'}.items()
    )

    # 2. Tải thông số bộ giải toán động học (kinematics)
    kinematics_yaml = load_yaml('ur_moveit_config', 'config/kinematics.yaml')

    # 3. Khai báo Node C++ vẽ chữ của bạn
    student_node = Node(
        package='ur_student_control',
        executable='draw_letter',
        output='screen',
        parameters=[
            {'use_sim_time': True},
            {'robot_description_kinematics': kinematics_yaml}
        ]
    )

    # 4. Trì hoãn chạy Node 15 giây để chờ Gazebo khởi động xong
    delayed_student_node = TimerAction(
        period=15.0,
        actions=[student_node]
    )

    return LaunchDescription([
        ur_sim_launch,
        delayed_student_node
    ])
