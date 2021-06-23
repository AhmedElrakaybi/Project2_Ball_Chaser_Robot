#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
        // Request a service and pass the velocities to it to drive the robot
        ball_chaser::DriveToTarget srv;
        srv.request.linear_x=lin_x;
        srv.request.angular_z=ang_z;

        if (!client.call(srv))
                ROS_ERROR("Failed to call service ball chaser");
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{
        int white_pixel = 255;
        int count_left=0;
        int count_center=0;
        int count_right=0;

        //Count the white pixels in each zone of the image
        for (int i = 0; i < img.height*img.step; i+=3) {
                if (img.data[i, i+1, i+2] == white_pixel) {
                        if(((i/3)%img.width) <= (img.width/3)) {
                                count_left++;
                        }
                        else if((img.width/3)< ((i/3)%img.width) && ((i/3)%img.width) <= ((2*img.width)/3)) {
                                count_center++;
                        }
                        else if(((2*img.width)/3)< ((i/3)%img.width)) {
                                count_right++;
                        }
                        else{
                                continue;
                        }
                }
        }

//Command the robot to drive towards the zone with the most white pixel count
        if (count_left > count_center && count_left > count_right) {
                drive_robot(0.000,0.500);
        }
        else if (count_center >= count_left && count_center >= count_right && count_center !=0) {
                drive_robot(0.5,0.0);
        }
        else if (count_right > count_center && count_right > count_left) {
                drive_robot(0.000,-0.500);
        }
        else {
                drive_robot(0.000, 0.000);
        }
}


int main(int argc, char** argv)
{
        // Initialize the process_image node and create a handle to it
        ros::init(argc, argv, "process_image");
        ros::NodeHandle n;

        // Define a client service capable of requesting services from command_robot
        client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

        // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
        ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

        // Handle ROS communication events
        ros::spin();

        return 0;
}
