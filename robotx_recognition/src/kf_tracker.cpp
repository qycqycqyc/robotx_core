#include <kf_tracker.h>

kf_tracker::kf_tracker() : tf_listener_(tf_buffer_)
{
    nh_.param<double>(ros::this_node::getName()+"/matching_distance_threashold", matching_distance_threashold_, 1.0);
    nh_.param<double>(ros::this_node::getName()+"/max_target_height", max_target_height_, 3.0);
    nh_.param<double>(ros::this_node::getName()+"/min_target_height", min_target_height_, 0.0);
    nh_.param<double>(ros::this_node::getName()+"/min_bbox_size", min_bbox_size_, 0.0);
    nh_.param<double>(ros::this_node::getName()+"/max_bbox_size", max_bbox_size_, 10.0);
    nh_.param<std::string>(ros::this_node::getName()+"/euclidean_cluster_topic", euclidean_cluster_topic_, ros::this_node::getName()+"/input_clusters");
    nh_.param<std::string>(ros::this_node::getName()+"/map_frame", map_frame_, "map");
    reset_();
    tracked_clusters_pub_ = nh_.advertise<robotx_msgs::TrackedClusterArray>(ros::this_node::getName()+"/tracked_clusters",1);
    marker_pub_ = nh_.advertise<visualization_msgs::MarkerArray>(ros::this_node::getName()+"/tracked_clusters/marker",1);
    euclidean_cluster_sub_ = nh_.subscribe(euclidean_cluster_topic_, 10, &kf_tracker::clusters_callback_, this);
    reset_sub_ = nh_.subscribe("/reset", 1, &kf_tracker::reset_callback_, this);
}

kf_tracker::~kf_tracker()
{
    
}

void kf_tracker::reset_callback_(const std_msgs::Empty::ConstPtr msg)
{
    reset_();
    return;
}

void kf_tracker::reset_()
{
    trackers_.clear();
    recieved_fast_time_ = true;
    return;
}

void kf_tracker::publish_marker_(ros::Time stamp)
{
    return;
}

void kf_tracker::publish_clusters_(ros::Time stamp)
{
    robotx_msgs::TrackedClusterArray msg;
    msg.header.frame_id = map_frame_;
    msg.header.stamp = stamp;
    for(int i=0; i<trackers_.size(); i++)
    {
        robotx_msgs::TrackedCluster cluster;
        cluster.bbox = trackers_[i]->get_predicted_bbox();
        cluster.tracking_id = i;
        msg.clusters.push_back(cluster);
    }
    tracked_clusters_pub_.publish(msg);
    return;
}

void kf_tracker::track_clusters_()
{
    if(recieved_fast_time_)
    {
        for(auto bbox_itr = bbox_data_.boxes.begin(); bbox_itr != bbox_data_.boxes.end(); bbox_itr++)
        {
            boost::shared_ptr<tracking_module> module_ptr = boost::make_shared<tracking_module>(map_frame_, trackers_.size());
            module_ptr->input_measurement(*bbox_itr, bbox_data_.header.stamp);
            trackers_.push_back(module_ptr);
        }
        recieved_fast_time_ = false;
    }
    else
    {
        int i = 0;
        std::vector<int> founded_index_list;
        std::vector<boost::shared_ptr<tracking_module> > new_trackers;
        for(auto bbox_itr = bbox_data_.boxes.begin(); bbox_itr != bbox_data_.boxes.end(); bbox_itr++)
        {
            boost::optional<int> index = get_tracker_index_(*bbox_itr);
            //found matched cluster
            if(index)
            {
                trackers_[index.get()]->input_measurement(*bbox_itr, bbox_data_.header.stamp);
                founded_index_list.push_back(index.get());
            }
            //found new cluster
            else
            {
                boost::shared_ptr<tracking_module> module_ptr = boost::make_shared<tracking_module>(map_frame_, trackers_.size()+i);
                module_ptr->input_measurement(*bbox_itr, bbox_data_.header.stamp);
                new_trackers.push_back(module_ptr);
                i = i + 1;
            }
        }
        std::copy(new_trackers.begin(),new_trackers.end(),std::back_inserter(trackers_));
    }
    return;
}

boost::optional<int> kf_tracker::get_tracker_index_(jsk_recognition_msgs::BoundingBox bbox)
{
    std::vector<double> ranges;
    for(auto bbox_itr = bbox_data_.boxes.begin(); bbox_itr != bbox_data_.boxes.end(); bbox_itr++)
    {
        double range = std::sqrt(std::pow(bbox_itr->pose.position.x - bbox.pose.position.x,2) + std::pow(bbox_itr->pose.position.y - bbox.pose.position.y,2));
        ranges.push_back(range);
    }
    std::vector<double>::iterator min_iter = std::min_element(ranges.begin(), ranges.end());
    if(ranges[ std::distance(ranges.begin(), min_iter)] < matching_distance_threashold_)
    {
        return std::distance(ranges.begin(), min_iter);
    }
    else
    {
        return boost::none;
    }
}

void kf_tracker::clusters_callback_(const jsk_recognition_msgs::BoundingBoxArray::ConstPtr msg)
{
    jsk_recognition_msgs::BoundingBoxArray transformed_clusters;
    transformed_clusters.header.stamp = msg->header.stamp;
    transformed_clusters.header.frame_id = map_frame_;
    for(auto cluster_itr = msg->boxes.begin(); cluster_itr != msg->boxes.end(); cluster_itr++)
    {
        jsk_recognition_msgs::BoundingBox bbox;
        geometry_msgs::PoseStamped pose;
        pose.pose = cluster_itr->pose;
        pose.header = cluster_itr->header;
        if(cluster_itr->header.frame_id == map_frame_)
        {
            geometry_msgs::TransformStamped transform_stamped;
            try
            {
                transform_stamped = tf_buffer_.lookupTransform(map_frame_, cluster_itr->header.frame_id, ros::Time(0));
                tf2::doTransform(pose, pose, transform_stamped);
            }
            catch (tf2::TransformException &ex)
            {
                ROS_WARN("%s",ex.what());
                continue;
            }
        }
        bbox.header.frame_id = map_frame_;
        bbox.header.stamp = cluster_itr->header.stamp;
        bbox.pose = pose.pose;
        bbox.dimensions = cluster_itr->dimensions;
        bbox.label = cluster_itr->label;
        bbox.value = cluster_itr->value;
        if(min_target_height_ < pose.pose.position.z && pose.pose.position.z < max_target_height_)
        {
            if(bbox.dimensions.x < max_bbox_size_ && min_bbox_size_ < bbox.dimensions.x)
            {
                if(bbox.dimensions.y < max_bbox_size_ && min_bbox_size_ < bbox.dimensions.y)
                {
                    transformed_clusters.boxes.push_back(bbox);
                }
            }
        }
    }
    bbox_data_ = transformed_clusters;
    track_clusters_();
    publish_clusters_(msg->header.stamp);
    publish_marker_(msg->header.stamp);
    return;
}