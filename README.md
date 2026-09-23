# UR3 vẽ chữ A bằng MoveIt 2

ROS 2 package cho Bài thực hành 01. Robot mô phỏng trong Gazebo, MoveIt 2 lập kế hoạch và gửi `FollowJointTrajectory` đến `joint_trajectory_controller`. Đường vẽ được hiển thị trong RViz qua topic `/visualization_marker` (Marker). Mặc định dùng UR3, chữ **A**.

## Đã thử trên máy này

- Ubuntu 22.04 LTS; ROS 2 Humble; MoveIt 2; `ur_simulation_gz`.
- UR3, chữ A: `SUCCESS`, MoveIt lập kế hoạch thành công, không gặp điểm kỳ dị.
- Mọi đoạn Cartesian đều đạt tỉ lệ `fraction > 0.9`; `joint_state_broadcaster` và `joint_trajectory_controller` ở trạng thái `active`.

## Cấu trúc

```text
ur_student_control/
├── CMakeLists.txt
├── include/
│   └── ur_student_control/
├── package.xml
├── launch/main.launch.py       # Gazebo + MoveIt + RViz + node vẽ
└── src/draw_letter.cpp         # Waypoint, lập kế hoạch, thực thi
```
Launch file `main.launch.py` dùng lại cấu hình của `ur_simulation_gz` và `ur_moveit_config`. Node dùng MoveIt `MoveGroupInterface` với group `ur_manipulator` và frame `base_link`.

## Build

Các package `ur_simulation_gz`, `ur_moveit_config`, MoveIt 2 phải có sẵn trong ROS workspace. Chép thư mục này vào `~/workspaces/ur_gz/src/`, rồi:

```bash
source /opt/ros/humble/setup.bash
cd ~/workspaces/ur_gz
colcon build --packages-select ur_student_control
source install/setup.bash
```

## Chạy

Khởi chạy hệ thống bằng lệnh:

```bash
ros2 launch ur_student_control main.launch.py
```

**Lưu ý:** Ngay khi Gazebo mở lên, cần bấm **Play** (mũi tên màu cam) để server Gazebo và controller hoạt động. Node sẽ chờ khoảng 10 giây để người dùng cấu hình RViz: add Marker `/visualization_marker`, tắt `Show Robot Visual`, bật `Show Trail`. 

## Thiết kế waypoint

Node đưa robot về cấu hình `[0.0, -1.57, 1.57, -1.57, -1.57, 0.0]` để tránh điểm kỳ dị, lấy pose hiện tại làm chuẩn. Chữ A được thiết kế trong mặt phẳng Y-Z dựng đứng:

- Điểm bắt đầu (hạ thấp trọng tâm): `y - 0.10`, `z - 0.25` m.
- Kéo chéo lên đỉnh: `y + 0.10`, `z + 0.20` m.
- Kéo chéo xuống góc phải: `y + 0.10`, `z - 0.20` m.
- Lùi về giữa nét phải: `y - 0.05`, `z + 0.10` m.
- Kéo ngang sang trái: `y - 0.10` m.

MoveIt kiểm tra va chạm khi `computeCartesianPath`, với bước nội suy `0.01` m. Nếu tỉ lệ hoàn thành nhỏ hơn `0.9`, node **không thực thi** đoạn đó. Quỹ đạo được giới hạn vận tốc và gia tốc ở mức 50% giới hạn tối đa để controller Gazebo bám theo. 

## Kiểm tra

Mở terminal khác sau khi launch:

```bash
source /opt/ros/humble/setup.bash
source ~/workspaces/ur_gz/install/setup.bash
ros2 control list_controllers
ros2 topic echo /joint_states --once
ros2 topic info /visualization_marker
```

Trong RViz, đường kẻ màu đỏ hiển thị khung mẫu tĩnh do Node publish. Đường nét đứt màu tím (Trail) là đường đi thực tế của `tool0` trong không gian do robot thực thi.
