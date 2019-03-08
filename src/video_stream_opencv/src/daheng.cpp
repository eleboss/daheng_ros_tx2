#include "daheng.h"









//-------------------------------------------------
/**
\brief Acquisition thread function
\param pParam       thread param, not used in this app
\return void*
*/
//-------------------------------------------------
void *ProcGetImage(void* pParam)
{
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    //Thread running flag setup
    g_bAcquisitionFlag = true;
    PGX_FRAME_BUFFER pFrameBuffer = NULL;


    uint32_t ui32FrameCount = 0;
    uint32_t ui32AcqFrameRate = 0;

    while(g_bAcquisitionFlag)
    {

        // Get a frame from Queue
        emStatus = GXDQBuf(g_hDevice, &pFrameBuffer, 1000);
        if(emStatus != GX_STATUS_SUCCESS)
        {
            if (emStatus == GX_STATUS_TIMEOUT)
            {
                continue;
            }
            else
            {
                GetErrorString(emStatus);
                break;
            }
        }

        if(pFrameBuffer->nStatus != GX_FRAME_STATUS_SUCCESS)
        {
            printf("<Abnormal Acquisition: Exception code: %d>\n", pFrameBuffer->nStatus);
        }
        //success received frame
        else
        {
            printf("<Successful acquisition: Width: %d Height: %d FrameID: %llu>\n", pFrameBuffer->nWidth, pFrameBuffer->nHeight, pFrameBuffer->nFrameID);

            int nRet = PixelFormatConvert(pFrameBuffer);
            Mat frame(pFrameBuffer->nHeight, pFrameBuffer->nWidth, CV_8UC3, g_pRGBImageBuf);

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

        emStatus = GXQBuf(g_hDevice, pFrameBuffer);
        if(emStatus != GX_STATUS_SUCCESS)
        {
            GetErrorString(emStatus);
            break;
        }  
    }
    printf("<Acquisition thread Exit!>\n");

    return 0;
}

int main(int argc, char** argv)
{


    ros::init(argc, argv, "image_publisher");
    ros::NodeHandle nh;
    ros::NodeHandle _nh("~"); // to get the private params
    image_transport::ImageTransport it(nh);
    image_transport::CameraPublisher pub = it.advertiseCamera("camera", 1);

    ROS_INFO_STREAM("Starting loading ros launch parameters....");

    std::string camera_name;
    _nh.param("camera_name", camera_name, std::string("camera"));
    ROS_INFO_STREAM("Camera name: " << camera_name);

    _nh.param("set_camera_fps", set_camera_fps, 30.0);
    ROS_INFO_STREAM("Setting camera FPS to: " << set_camera_fps);

    _nh.param("set_exposure_time", set_exposure_time, 30.0);
    ROS_INFO_STREAM("Setting camera exposure time(us) to: " << set_exposure_time);

    int buffer_queue_size;
    _nh.param("buffer_queue_size", buffer_queue_size, 100);
    ROS_INFO_STREAM("Setting buffer size for capturing frames to: " << buffer_queue_size);
    max_queue_size = buffer_queue_size;

    double fps;
    _nh.param("fps", fps, 240.0);
    ROS_INFO_STREAM("Throttling to fps: " << fps);

    if (fps > set_camera_fps)
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



    ROS_INFO_STREAM("Initializing industrial camera based on the ros parameter......"); 

     
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
        ROS_INFO_STREAM("<No device found>\n");
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
    ROS_INFO_STREAM("***********************************************\n");
    //Get libary version
    ROS_INFO_STREAM("<Libary Version : %s>\n", GXGetLibVersion());
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

    ROS_INFO_STREAM("<Model Name : %s>\n", pszModelName);
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
    emStatus = GXSetFloat(g_hDevice, GX_FLOAT_ACQUISITION_FRAME_RATE, set_camera_fps);
    GX_VERIFY_EXIT(emStatus);

    // setting auto exposure: off
    //emStatus = GXSetFloat(g_hDevice, GX_ENUM_EXPOSURE_AUTO, GX_EXPOSURE_AUTO_OFF);
    //GX_VERIFY_EXIT(emStatus);

    //GX_FLOAT_RANGE shutterRange;
    //emStatus = GXGetFloatRange(g_hDevice, GX_FLOAT_EXPOSURE_TIME, &shutterRange);
    //GX_VERIFY_EXIT(emStatus);       
    
    //printf("%f,%f",shutterRange.dMin,shutterRange.dMax);
    
    // setting exposure time (unit:us)
    emStatus = GXSetFloat(g_hDevice, GX_FLOAT_EXPOSURE_TIME, set_exposure_time);
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
    // Start acquisition thread, if thread create failed, exit this app
    int nRet = pthread_create(&g_nAcquisitonThreadID, NULL, ProcGetImage, NULL);
    if(nRet != 0)
    {
        //Release the memory allocated
        UnPreForAcquisition();

        GXCloseDevice(g_hDevice);
        g_hDevice = NULL;
        GXCloseLib();

        printf("<Failed to create the acquisition thread, App Exit!>\n");
        ROS_ERROR_STREAM("Could not open the stream.");
        exit(nRet);
    }



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
}


