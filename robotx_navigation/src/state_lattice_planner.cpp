#include <state_lattice_planner.h>

state_lattice_planner::state_lattice_planner()
{

}

state_lattice_planner::~state_lattice_planner()
{

}

planner_result state_lattice_planner::plan(geometry_msgs::Twist raw_twist_cmd, geometry_msgs::PoseStamped target_pose, 
    geometry_msgs::Twist current_twist, boost::shared_ptr<nav_msgs::OccupancyGrid> map_ptr, geometry_msgs::Twist& twist_cmd)
{
    boost::optional<flann::Matrix<float> > mat = map_to_matrix_(map_ptr);
    if(!mat)
    {
        twist_cmd = raw_twist_cmd;
        return NO_OBSTACLE;
    }
    twist_cmd = raw_twist_cmd;
    return FAILED_TO_FIND_PATH;
}

boost::optional<flann::Matrix<float> > state_lattice_planner::map_to_matrix_(boost::shared_ptr<nav_msgs::OccupancyGrid> map_ptr)
{
    int matrix_size = 0;
    for(auto data_itr = map_ptr->data.begin(); data_itr != map_ptr->data.end(); data_itr++)
    {
        if(*data_itr > 0)
        {
            matrix_size++;
        }
    }
    if(matrix_size==0)
    {
        return boost::none;
    }
    flann::Matrix<float> mat(new float[matrix_size*2], matrix_size, 2);
    int index = 0;
    for(int i = 0; i < map_ptr->info.height; i++)
    {
        for (int m = 0; m < map_ptr->info.width; m++)
        {
            if(map_ptr->data[i * map_ptr->info.height + m] > 0)
            {
                mat[index][0] = (float)m * map_ptr->info.resolution - 0.5 * map_ptr->info.resolution * map_ptr->info.width;
                mat[index][1] = (float)i * map_ptr->info.resolution - 0.5 * map_ptr->info.resolution * map_ptr->info.height;
                index++;
            }
        }
    }
    return mat;
}