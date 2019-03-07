#include "daheng.h"



// Based on the ros tutorial on transforming opencv images to Image messages

sensor_msgs::CameraInfo get_default_camera_info_from_image(sensor_msgs::ImagePtr img){
    sensor_msgs::CameraInfo cam_info_msg;
    cam_info_msg.header.frame_id = img->header.frame_id;
    // Fill image size
    cam_info_msg.height = img->height;
    cam_info_msg.width = img->width;
    ROS_INFO_STREAM("The image width is: " << img->width);
    ROS_INFO_STREAM("The image height is: " << img->height);
    // Add the most common distortion model as sensor_msgs/CameraInfo says
    cam_info_msg.distortion_model = "plumb_bob";
    // Don't let distorsion matrix be empty
    cam_info_msg.D.resize(5, 0.0);
    // Give a reasonable default intrinsic camera matrix
    cam_info_msg.K = boost::assign::list_of(1.0) (0.0) (img->width/2.0)
            (0.0) (1.0) (img->height/2.0)
            (0.0) (0.0) (1.0);
    // Give a reasonable default rectification matrix
    cam_info_msg.R = boost::assign::list_of (1.0) (0.0) (0.0)
            (0.0) (1.0) (0.0)
            (0.0) (0.0) (1.0);
    // Give a reasonable default projection matrix
    cam_info_msg.P = boost::assign::list_of (1.0) (0.0) (img->width/2.0) (0.0)
            (0.0) (1.0) (img->height/2.0) (0.0)
            (0.0) (0.0) (1.0) (0.0);
    return cam_info_msg;
}


void do_capture(ros::NodeHandle &nh) {
    cv::Mat frame;
    ros::Rate camera_fps_rate(set_camera_fps);

    // Read frames as fast as possible
    while (nh.ok()) {
        cap >> frame;
        if (video_stream_provider_type == "videofile")
        {
            camera_fps_rate.sleep();
        }

        if(!frame.empty()) {
            std::lock_guard<std::mutex> g(q_mutex);
            // accumulate only until max_queue_size
            if (framesQueue.size() < max_queue_size) {
                framesQueue.push(frame.clone());
            }
            // once reached, drop the oldest frame
            else {
                framesQueue.pop();
                framesQueue.push(frame.clone());
            }
        }
    }
}


int main(int argc, char** argv)
{
printf("\n");
    printf("-------------------------------------------------------------\n");
    printf("Sample to show how to acquire color image continuously and save ppm image.\n");
    printf("version: 1.0.1808.9131\n");
    printf("-------------------------------------------------------------\n");
    printf("\n");
    printf("Initializing......"); 
    printf("\n\n");
     
    GX_STATUS emStatus = GX_STATUS_SUCCESS;

    uint32_t ui32DeviceNum = 0;

    //Initialize libary
    emStatus = GXInitLib(); 
    if(emStatus != GX_STATUS_SUCCESS)
    {
        GetErrorString(emStatus);
        return emStatus;
    }

    //Get device enumerated number
    emStatus = GXUpdateDeviceList(&ui32DeviceNum, 1000);
    if(emStatus != GX_STATUS_SUCCESS)
    { 
        GetErrorString(emStatus);
        GXCloseLib();
        return emStatus;
    }

    //If no device found, app exit
    if(ui32DeviceNum <= 0)
    {
        printf("<No device found>\n");
        GXCloseLib();
        return emStatus;
    }

    //Open first device enumerated
    emStatus = GXOpenDeviceByIndex(1, &g_hDevice);
    if(emStatus != GX_STATUS_SUCCESS)
    {
        GetErrorString(emStatus);
        GXCloseLib();
        return emStatus;           
    }

    //Get Device Info
    printf("***********************************************\n");
    //Get libary version
    printf("<Libary Version : %s>\n", GXGetLibVersion());
    size_t nSize = 0;
    //Get string length of Vendor name
    emStatus = GXGetStringLength(g_hDevice, GX_STRING_DEVICE_VENDOR_NAME, &nSize);
    GX_VERIFY_EXIT(emStatus);
    //Alloc memory for Vendor name
    char *pszVendorName = new char[nSize];
    //Get Vendor name
    emStatus = GXGetString(g_hDevice, GX_STRING_DEVICE_VENDOR_NAME, pszVendorName, &nSize);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        delete[] pszVendorName;
        pszVendorName = NULL;
        GX_VERIFY_EXIT(emStatus);
    }

    printf("<Vendor Name : %s>\n", pszVendorName);
    //Release memory for Vendor name
    delete[] pszVendorName;
    pszVendorName = NULL;

    //Get string length of Model name
    emStatus = GXGetStringLength(g_hDevice, GX_STRING_DEVICE_MODEL_NAME, &nSize);
    GX_VERIFY_EXIT(emStatus);
    //Alloc memory for Model name
    char *pszModelName = new char[nSize];
    //Get Model name
    emStatus = GXGetString(g_hDevice, GX_STRING_DEVICE_MODEL_NAME, pszModelName, &nSize);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        delete[] pszModelName;
        pszModelName = NULL;
        GX_VERIFY_EXIT(emStatus);
    }

    printf("<Model Name : %s>\n", pszModelName);
    //Release memory for Model name
    delete[] pszModelName;
    pszModelName = NULL;

    //Get string length of Serial number
    emStatus = GXGetStringLength(g_hDevice, GX_STRING_DEVICE_SERIAL_NUMBER, &nSize);
    GX_VERIFY_EXIT(emStatus);
    //Alloc memory for Serial number
    char *pszSerialNumber = new char[nSize];
    //Get Serial Number
    emStatus = GXGetString(g_hDevice, GX_STRING_DEVICE_SERIAL_NUMBER, pszSerialNumber, &nSize);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        delete[] pszSerialNumber;
        pszSerialNumber = NULL;
        GX_VERIFY_EXIT(emStatus);
    }

    printf("<Serial Number : %s>\n", pszSerialNumber);
    //Release memory for Serial number
    delete[] pszSerialNumber;
    pszSerialNumber = NULL;

    //Get string length of Device version
    emStatus = GXGetStringLength(g_hDevice, GX_STRING_DEVICE_VERSION, &nSize);
    GX_VERIFY_EXIT(emStatus);
    char *pszDeviceVersion = new char[nSize];
    //Get Device Version
    emStatus = GXGetString(g_hDevice, GX_STRING_DEVICE_VERSION, pszDeviceVersion, &nSize);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        delete[] pszDeviceVersion;
        pszDeviceVersion = NULL;
        GX_VERIFY_EXIT(emStatus);
    }

    printf("<Device Version : %s>\n", pszDeviceVersion);
    //Release memory for Device version
    delete[] pszDeviceVersion;
    pszDeviceVersion = NULL;
    printf("***********************************************\n");

    //Get the type of Bayer conversion. whether is a color camera.
    emStatus = GXIsImplemented(g_hDevice, GX_ENUM_PIXEL_COLOR_FILTER, &g_bColorFilter);
    GX_VERIFY_EXIT(emStatus);

    //This app only support color cameras
    if (!g_bColorFilter)
    {
        printf("<This app only support color cameras! App Exit!>\n");
        GXCloseDevice(g_hDevice);
        g_hDevice = NULL;
        GXCloseLib();
        return 0;
    }
    else
    {
        emStatus = GXGetEnum(g_hDevice, GX_ENUM_PIXEL_COLOR_FILTER, &g_i64ColorFilter);
        GX_VERIFY_EXIT(emStatus);
    }
    
    emStatus = GXGetInt(g_hDevice, GX_INT_PAYLOAD_SIZE, &g_nPayloadSize);
    GX_VERIFY(emStatus);

     //Set acquisition mode
    emStatus = GXSetEnum(g_hDevice, GX_ENUM_ACQUISITION_MODE, GX_ACQ_MODE_CONTINUOUS);
    GX_VERIFY_EXIT(emStatus);

    //Set trigger mode
    emStatus = GXSetEnum(g_hDevice, GX_ENUM_TRIGGER_MODE, GX_TRIGGER_MODE_OFF);
    GX_VERIFY_EXIT(emStatus);

    //Set buffer quantity of acquisition queue
    uint64_t nBufferNum = ACQ_BUFFER_NUM;
    emStatus = GXSetAcqusitionBufferNumber(g_hDevice, nBufferNum);
    GX_VERIFY_EXIT(emStatus);

    //Set size of data transfer block
    emStatus = GXSetInt(g_hDevice, GX_DS_INT_STREAM_TRANSFER_SIZE, ACQ_TRANSFER_SIZE);
    GX_VERIFY_EXIT(emStatus);

    //Set qty. of data transfer block
    emStatus = GXSetInt(g_hDevice, GX_DS_INT_STREAM_TRANSFER_NUMBER_URB, ACQ_TRANSFER_NUMBER_URB);
    GX_VERIFY_EXIT(emStatus);

    //Set Balance White Mode : Continuous
    emStatus = GXSetEnum(g_hDevice, GX_ENUM_BALANCE_WHITE_AUTO, GX_BALANCE_WHITE_AUTO_CONTINUOUS);
    GX_VERIFY_EXIT(emStatus);

    //Set Gain Mode : Continuous
    emStatus = GXSetEnum(g_hDevice, GX_ENUM_GAIN_AUTO, GX_GAIN_AUTO_CONTINUOUS);
    GX_VERIFY_EXIT(emStatus);

    // enable frame rate setting 
    emStatus = GXSetEnum(g_hDevice, GX_ENUM_ACQUISITION_FRAME_RATE_MODE, GX_ACQUISITION_FRAME_RATE_MODE_ON);
    GX_VERIFY_EXIT(emStatus);

    // setting the frame rate
    emStatus = GXSetFloat(g_hDevice, GX_FLOAT_ACQUISITION_FRAME_RATE, 1.0);
    GX_VERIFY_EXIT(emStatus);

    // setting auto exposure: off
    //emStatus = GXSetFloat(g_hDevice, GX_ENUM_EXPOSURE_AUTO, GX_EXPOSURE_AUTO_OFF);
    //GX_VERIFY_EXIT(emStatus);

    //GX_FLOAT_RANGE shutterRange;
    //emStatus = GXGetFloatRange(g_hDevice, GX_FLOAT_EXPOSURE_TIME, &shutterRange);
    //GX_VERIFY_EXIT(emStatus);       
    
    //printf("%f,%f",shutterRange.dMin,shutterRange.dMax);
    
    // setting exposure time (unit:us)
    emStatus = GXSetFloat(g_hDevice, GX_FLOAT_EXPOSURE_TIME, 20000.0);
    GX_VERIFY_EXIT(emStatus);    

   
    
    //Allocate the memory for pixel format transform 
    PreForAcquisition();

    //Device start acquisition
    emStatus = GXStreamOn(g_hDevice);
    if(emStatus != GX_STATUS_SUCCESS)
    {
        //Release the memory allocated
        UnPreForAcquisition();
        GX_VERIFY_EXIT(emStatus);
    }

    //Start acquisition thread, if thread create failed, exit this app
    int nRet = pthread_create(&g_nAcquisitonThreadID, NULL, ProcGetImage, NULL);
    if(nRet != 0)
    {
        //Release the memory allocated
        UnPreForAcquisition();

        GXCloseDevice(g_hDevice);
        g_hDevice = NULL;
        GXCloseLib();

        printf("<Failed to create the acquisition thread, App Exit!>\n");
        exit(nRet);
    }
















    ros::init(argc, argv, "image_publisher");
    ros::NodeHandle nh;
    ros::NodeHandle _nh("~"); // to get the private params
    image_transport::ImageTransport it(nh);
    image_transport::CameraPublisher pub = it.advertiseCamera("camera", 1);

    // provider can be an url (e.g.: rtsp://10.0.0.1:554) or a number of device, (e.g.: 0 would be /dev/video0)
    std::string video_stream_provider;
    if (_nh.getParam("video_stream_provider", video_stream_provider)){
        ROS_INFO_STREAM("Resource video_stream_provider: " << video_stream_provider);
        // If we are given a string of 4 chars or less (I don't think we'll have more than 100 video devices connected)
        // treat is as a number and act accordingly so we open up the videoNUMBER device
        if (video_stream_provider.size() < 4){
            ROS_INFO_STREAM("Getting video from provider: /dev/video" << video_stream_provider);
            video_stream_provider_type = "videodevice";
            cap.open(atoi(video_stream_provider.c_str()));
        }
        else{
            ROS_INFO_STREAM("Getting video from provider: " << video_stream_provider);
            if (video_stream_provider.find("http://") != std::string::npos ||
                    video_stream_provider.find("https://") != std::string::npos){
                video_stream_provider_type = "http_stream";
            }
            else if(video_stream_provider.find("rtsp://") != std::string::npos){
                video_stream_provider_type = "rtsp_stream";
            }
            else {
                // Check if file exists to know if it's a videofile
                std::ifstream ifs;
                ifs.open(video_stream_provider.c_str(), std::ifstream::in);
                if (ifs.good()){
                    video_stream_provider_type = "videofile";
                }
                else
                    video_stream_provider_type = "unknown";
            }
            cap.open(video_stream_provider);
        }
    }
    else{
        ROS_ERROR("Failed to get param 'video_stream_provider'");
        return -1;
    }

    ROS_INFO_STREAM("Video stream provider type detected: " << video_stream_provider_type);

    std::string camera_name;
    _nh.param("camera_name", camera_name, std::string("camera"));
    ROS_INFO_STREAM("Camera name: " << camera_name);

    _nh.param("set_camera_fps", set_camera_fps, 30.0);
    ROS_INFO_STREAM("Setting camera FPS to: " << set_camera_fps);
    cap.set(CV_CAP_PROP_FPS, set_camera_fps);

    double reported_camera_fps;
    // OpenCV 2.4 returns -1 (instead of a 0 as the spec says) and prompts an error
    // HIGHGUI ERROR: V4L2: Unable to get property <unknown property string>(5) - Invalid argument
    reported_camera_fps = cap.get(CV_CAP_PROP_FPS);
    if (reported_camera_fps > 0.0)
        ROS_INFO_STREAM("Camera reports FPS: " << reported_camera_fps);
    else
        ROS_INFO_STREAM("Backend can't provide camera FPS information");

    int buffer_queue_size;
    _nh.param("buffer_queue_size", buffer_queue_size, 100);
    ROS_INFO_STREAM("Setting buffer size for capturing frames to: " << buffer_queue_size);
    max_queue_size = buffer_queue_size;

    double fps;
    _nh.param("fps", fps, 240.0);
    ROS_INFO_STREAM("Throttling to fps: " << fps);

    if (video_stream_provider.size() < 4 && fps > set_camera_fps)
        ROS_WARN_STREAM("Asked to publish at 'fps' (" << fps
                        << ") which is higher than the 'set_camera_fps' (" << set_camera_fps <<
                        "), we can't publish faster than the camera provides images.");

    std::string frame_id;
    _nh.param("frame_id", frame_id, std::string("camera"));
    ROS_INFO_STREAM("Publishing with frame_id: " << frame_id);

    std::string camera_info_url;
    _nh.param("camera_info_url", camera_info_url, std::string(""));
    ROS_INFO_STREAM("Provided camera_info_url: '" << camera_info_url << "'");

    bool flip_horizontal;
    _nh.param("flip_horizontal", flip_horizontal, false);
    ROS_INFO_STREAM("Flip horizontal image is: " << ((flip_horizontal)?"true":"false"));

    bool flip_vertical;
    _nh.param("flip_vertical", flip_vertical, false);
    ROS_INFO_STREAM("Flip vertical image is: " << ((flip_vertical)?"true":"false"));

    int width_target;
    int height_target;
    _nh.param("width", width_target, 0);
    _nh.param("height", height_target, 0);
    if (width_target != 0 && height_target != 0){
        ROS_INFO_STREAM("Forced image width is: " << width_target);
        ROS_INFO_STREAM("Forced image height is: " << height_target);
    }

    // From http://docs.opencv.org/modules/core/doc/operations_on_arrays.html#void flip(InputArray src, OutputArray dst, int flipCode)
    // FLIP_HORIZONTAL == 1, FLIP_VERTICAL == 0 or FLIP_BOTH == -1
    bool flip_image = true;
    int flip_value;
    if (flip_horizontal && flip_vertical)
        flip_value = -1; // flip both, horizontal and vertical
    else if (flip_horizontal)
        flip_value = 1;
    else if (flip_vertical)
        flip_value = 0;
    else
        flip_image = false;

    if(!cap.isOpened()){
        ROS_ERROR_STREAM("Could not open the stream.");
        return -1;
    }
    if (width_target != 0 && height_target != 0){
        cap.set(CV_CAP_PROP_FRAME_WIDTH, width_target);
        cap.set(CV_CAP_PROP_FRAME_HEIGHT, height_target);
    }

    cv::Mat frame;
    sensor_msgs::ImagePtr msg;
    sensor_msgs::CameraInfo cam_info_msg;
    std_msgs::Header header;
    header.frame_id = frame_id;
    camera_info_manager::CameraInfoManager cam_info_manager(nh, camera_name, camera_info_url);
    // Get the saved camera info if any
    cam_info_msg = cam_info_manager.getCameraInfo();
    cam_info_msg.header = header;

    ROS_INFO_STREAM("Opened the stream, starting to publish.");
    boost::thread cap_thread(do_capture, nh);

    ros::Rate r(fps);
    while (nh.ok()) {

        {
            std::lock_guard<std::mutex> g(q_mutex);
            if (!framesQueue.empty()){
                frame = framesQueue.front();
                framesQueue.pop();
            }
        }

        if (pub.getNumSubscribers() > 0){
            // Check if grabbed frame is actually filled with some content
            if(!frame.empty()) {
                // Flip the image if necessary
                if (flip_image)
                    cv::flip(frame, frame, flip_value);
                msg = cv_bridge::CvImage(header, "bgr8", frame).toImageMsg();
                // Create a default camera info if we didn't get a stored one on initialization
                if (cam_info_msg.distortion_model == ""){
                    ROS_WARN_STREAM("No calibration file given, publishing a reasonable default camera info.");
                    cam_info_msg = get_default_camera_info_from_image(msg);
                    cam_info_manager.setCameraInfo(cam_info_msg);
                }
                // The timestamps are in sync thanks to this publisher
                pub.publish(*msg, cam_info_msg, ros::Time::now());
            }

            ros::spinOnce();
        }
        r.sleep();
    }
    cap_thread.join();
}


