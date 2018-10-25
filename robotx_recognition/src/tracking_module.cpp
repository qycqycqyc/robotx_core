#include <tracking_module.h>

tracking_module::tracking_module(std::string map_frame,int id) : target_id(id)
{
    kf_ptr_ = std::make_shared<cv::KalmanFilter>(2,2,0,CV_32F);
    state_ = cv::Mat(2, 1, CV_32F);
    process_noise_ = cv::Mat(2, 1, CV_32F);
    cv::setIdentity(kf_ptr_->transitionMatrix);
    cv::setIdentity(kf_ptr_->measurementMatrix);
    cv::setIdentity(kf_ptr_->processNoiseCov, cv::Scalar::all(1e-5));
    cv::setIdentity(kf_ptr_->measurementNoiseCov, cv::Scalar::all(1e-1));
    cv::setIdentity(kf_ptr_->errorCovPost, cv::Scalar::all(1));
    map_frame_ = map_frame;
}

tracking_module::~tracking_module()
{

}

jsk_recognition_msgs::BoundingBox tracking_module::input_measurement(boost::optional<jsk_recognition_msgs::BoundingBox> measurement, ros::Time stamp)
{
    if(predicted_bbox_.header.stamp == stamp)
    {
        return predicted_bbox_;
    }
    if(measurement)
    {
        latest_measurement_ = *measurement;
        cv::Mat measure = (cv::Mat_<float>(2, 1) << measurement->pose.position.x, measurement->pose.position.y);
        kf_ptr_->correct(measure);
    }
    cv::randn(process_noise_, cv::Scalar(0), cv::Scalar::all(std::sqrt(kf_ptr_->processNoiseCov.at<float>(0, 0))));
    state_ = kf_ptr_->transitionMatrix*state_ + process_noise_;
    predicted_bbox_.header.stamp = stamp;
    predicted_bbox_.header.frame_id = map_frame_;
    predicted_bbox_.dimensions = latest_measurement_.dimensions;
    predicted_bbox_.pose.orientation = latest_measurement_.pose.orientation;
    predicted_bbox_.pose.position.x = state_.data[0];
    predicted_bbox_.pose.position.y = state_.data[1];
    predicted_bbox_.pose.position.z = latest_measurement_.pose.position.z;
    return predicted_bbox_;
}

jsk_recognition_msgs::BoundingBox tracking_module::get_predicted_bbox()
{
    return predicted_bbox_;
}