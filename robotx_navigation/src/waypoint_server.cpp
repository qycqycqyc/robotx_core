#include <waypoint_server.h>

waypoint_server::waypoint_server() : tf_listener_(tf_buffer_)
{
    std::string waypoint_bag_filename;
    nh_.param<std::string>(ros::this_node::getName()+"/waypoint_bag_filename", waypoint_bag_filename, "waypoints.bag");
    nh_.param<std::string>(ros::this_node::getName()+"/map_frame", map_frame_, "map");
    waypoint_bag_file_path_ = ros::package::getPath("robotx_navigation") + "/data/" + waypoint_bag_filename;
    rosbag::Bag bag;
    bag.open(waypoint_bag_file_path_);
    for(rosbag::MessageInstance const m: rosbag::View(bag))
    {
        robotx_msgs::WayPointArray::ConstPtr i = m.instantiate<robotx_msgs::WayPointArray>();
        if(i != NULL)
        {
            waypoints_ = *i;
            break;
        }
    }
    bag.close();
    marker_pub_ = nh_.advertise<visualization_msgs::MarkerArray>(ros::this_node::getName()+"/marker",1);
    boost::thread marker_thread(boost::bind(&waypoint_server::publish_marker_, this));
}

waypoint_server::~waypoint_server()
{

}

void waypoint_server::publish_marker_()
{
    while(ros::ok())
    {
        visualization_msgs::MarkerArray marker_msg;
        ros::Rate rate(10);
        mutex_.lock();
        for(int i=0; i<waypoints_.waypoints.size(); i++)
        {
            visualization_msgs::Marker marker;
            marker.header.frame_id = map_frame_;
            marker.header.stamp = ros::Time::now();
            marker.ns = "waypoint_marker";
            marker.id = i;
            marker.type = marker.ARROW;
            marker.action = marker.MODIFY;
            marker.pose = waypoints_.waypoints[i].pose.pose;
            marker.color.r = 0;
            marker.color.g = 1;
            marker.color.b = 0;
            marker.color.a = 1;
            marker.scale.x = 1;
            marker.scale.y = 0.3;
            marker.scale.z = 0.3;
            marker.frame_locked = true;
            marker.lifetime = ros::Duration(0.3);
            marker_msg.markers.push_back(marker);
            visualization_msgs::Marker text_marker;
            text_marker.header.frame_id = map_frame_;
            text_marker.header.stamp = ros::Time::now();
            text_marker.ns = "waypoint_marker";
            text_marker.id = i + waypoints_.waypoints.size();
            text_marker.type = text_marker.TEXT_VIEW_FACING;
            text_marker.action = text_marker.MODIFY;
            text_marker.pose = waypoints_.waypoints[i].pose.pose;
            text_marker.pose.position.z = text_marker.pose.position.z + 0.3;
            text_marker.pose.orientation.x = 0;
            text_marker.pose.orientation.y = 0;
            text_marker.pose.orientation.z = 0;
            text_marker.pose.orientation.w = 1;
            text_marker.color.r = 1;
            text_marker.color.g = 1;
            text_marker.color.b = 1;
            text_marker.color.a = 1;
            text_marker.scale.x = 1;
            text_marker.scale.y = 1;
            text_marker.scale.z = 1;
            text_marker.frame_locked = true;
            text_marker.text = std::to_string(i);
            text_marker.lifetime = ros::Duration(0.3);
            marker_msg.markers.push_back(text_marker);
            marker_pub_.publish(marker_msg);
            rate.sleep();
        }
        mutex_.unlock();
    }
    return;
}

void waypoint_server::robot_pose_callback_(const geometry_msgs::PoseStamped::ConstPtr msg)
{
    
    return;
}