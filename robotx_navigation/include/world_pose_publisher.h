#ifndef WORLD_POSE_PUBLISHER_H_INCLUDED
#define WORLD_POSE_PUBLISHER_H_INCLUDED

//headers in ROS
#include <ros/ros.h>
#include <message_filters/subscriber.h>
#include <message_filters/time_synchronizer.h>
#include <message_filters/sync_policies/approximate_time.h>
#include <sensor_msgs/NavSatFix.h>
#include <geometry_msgs/TwistStamped.h>
#include <geometry_msgs/QuaternionStamped.h>

//headers in this package
#include <UTM.h>

//headers in Boost
#include <boost/shared_ptr.hpp>

typedef message_filters::sync_policies::ApproximateTime
    <sensor_msgs::NavSatFix, geometry_msgs::TwistStamped, geometry_msgs::QuaternionStamped> sync_policy;

class world_pose_publisher
{
public:
    world_pose_publisher(ros::NodeHandle nh,ros::NodeHandle pnh);
    ~world_pose_publisher();
private:
    ros::NodeHandle nh_;
    ros::NodeHandle pnh_;
    boost::shared_ptr<message_filters::Synchronizer<sync_policy> > sync_ptr_;
    boost::shared_ptr<message_filters::Subscriber<sensor_msgs::NavSatFix> > fix_sub_ptr_;
    boost::shared_ptr<message_filters::Subscriber<geometry_msgs::TwistStamped> > twist_sub_ptr_;
    boost::shared_ptr<message_filters::Subscriber<geometry_msgs::QuaternionStamped> > true_course_sub_ptr_;
    std::string fix_topic_;
    std::string twist_topic_;
    std::string true_course_topic_;
    void gnss_callback_(const sensor_msgs::NavSatFixConstPtr& fix,
        const geometry_msgs::TwistStampedConstPtr& twist,
        const geometry_msgs::QuaternionStampedConstPtr true_course);
};
#endif  //WORLD_POSE_PUBLISHER_H_INCLUDED