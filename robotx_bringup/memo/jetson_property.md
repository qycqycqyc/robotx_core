# jetson_0
- IP : 192.168.1.100
- ROS MASTER

### Description
- 制御（操舵・スラスタ制御）
- 自己位置推定（localization.launch）

##### Motor Driver IP
- motor_driver_left : 192.168.1.20
- motor_driver_right : 192.168.1.21

---

# jetson_1
- IP : 192.168.1.101
- ROS CLIENT

### Desctiption
- velodyne (AI_Pilot), velodyne（front）
- euclidean_cluster

###### Velodyne IP
- AI Pilot velodyne : 192.168.1.10
- Front velodyne : 192.168.1.11

---

# jetson_2
- IP : 192.168.1.102
- ROS CLIENT

### Description
- usb_cam
- 画像処理・DLによる認識

