#include "videorecorder.h"

int file_counter = 0;
bool is_recording = false;
bool start = false;
bool stop = false;
int buffer_size;
cv::VideoWriter video_writer;
std::vector<cv::Mat> buffer;

void add_timestamp(cv::Mat &frame)
{
    // Get the current time
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm tm_now = *std::localtime(&time_t_now);

    // Format the time as a string
    std::ostringstream time_stream;
    time_stream << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S");
    std::string timestamp = time_stream.str();

    // Define the position and font properties
    cv::Point text_position(10, 30); // Top-left corner
    int font_face = cv::FONT_HERSHEY_SIMPLEX;
    double font_scale = 0.7;
    int thickness = 2;

    // Add the timestamp to the frame
    cv::putText(frame, timestamp, text_position, font_face, font_scale, cv::Scalar(0, 0, 255), thickness);
}

std::string createDailyFolder() 
{
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::localtime(&now_time_t);
 
    std::ostringstream folder_name_stream;
    folder_name_stream << "Videos_" << std::put_time(&now_tm, "%Y-%m-%d");
    std::string folder_name = folder_name_stream.str();
 
    if (!std::filesystem::exists(folder_name)) 
    {
        std::filesystem::create_directory(folder_name);
    }
 
    return folder_name;
}

void PrepareRingBuffer() 
{
    const int fps = 20;
    const double frame_duration_ms = 1000.0 / fps;
    const int max_frames = static_cast<int>(30 * fps); // Buffer for last 30 seconds
    const int recording_duration_frames = 30 * fps;    // Record additional 30 seconds

    const int output_width = 1920;
    const int output_height = 1080;

    std::deque<cv::Mat> circle_buffer;
    std::mutex buffer_mutex;
    std::condition_variable buffer_cv;
    bool is_recording = false;
    
    std::string folder_path = createDailyFolder();

    // Thread to capture and store frames in the circular buffer
    std::thread capture_thread([&]() 
    {
        while (runThread) 
        {
            cv::Mat frame /*= record_frame*/; // Capture for Rear Camera

            if (frame.empty()) 
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(frame_duration_ms)));
                continue;
            }

            cv::Mat resized_frame, bgr_frame;
            cv::resize(frame, bgr_frame, cv::Size(output_width, output_height));

            add_timestamp(bgr_frame);

            {
                std::lock_guard<std::mutex> lock(buffer_mutex);
                if (circle_buffer.size() >= max_frames) 
                {
                    circle_buffer.pop_front();
                }
                circle_buffer.push_back(bgr_frame.clone());
            }

            buffer_cv.notify_all();
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(frame_duration_ms)));
        }
    });

    while (runThread) 
    {
        if (start && !is_recording) 
        {
            is_recording = true;
            // std::thread recording_thread([&, this]() 
            // {
                auto now = std::chrono::system_clock::now();

                auto now_time_t = std::chrono::system_clock::to_time_t(now);

                std::tm now_tm = *std::localtime(&now_time_t);
 
                // Generate timestamped video filename

                std::ostringstream file_name_stream;

                file_name_stream << folder_path << "/video_" << std::put_time(&now_tm, "%Y_%m_%d_%H_%M_%S") << ".mp4";

                std::string output_file = file_name_stream.str();
                // std::string output_file = "videos" + std::to_string(file_counter) + ".mp4";
                cv::VideoWriter out(output_file, cv::VideoWriter::fourcc('m', 'p', '4', 'v'), fps, cv::Size(output_width, output_height));
                if (!out.isOpened()) 
                {
                    std::cerr << "Error opening " << output_file << " for writing." << std::endl;
                    is_recording = false;
                    return;
                }

                int frames_written = 0;

                while (frames_written < max_frames + recording_duration_frames) 
                {
                    cv::Mat frame;
                    {
                        std::unique_lock<std::mutex> lock(buffer_mutex);
                        if (!circle_buffer.empty()) 
                        {
                            frame = circle_buffer.front();
                            circle_buffer.pop_front();
                        } else 
                        {
                            buffer_cv.wait(lock);
                        }
                    }

                    if (!frame.empty()) 
                    {
                        out.write(frame);
                        frames_written++;
                    }
                }

                out.release();
                file_counter++;
                is_recording = false;
                start = false;
            });

            // recording_thread.detach();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    capture_thread.join();
}

// Set the recording state (start or stop)
void setRecordState(bool startRecord, bool stopRecord) {
    start = startRecord;
    stop = stopRecord;
}

/**
 * @brief Starts the video RingBuffer process by launching a separate thread.
 *
 * The method initializes processing and starts the `PrepareRingBuffer` method in a new thread.
 */
void Start()
{
    runThread = true;
    // this_thread::sleep_for(std::chrono::milliseconds(1000));
    // thread t(&VideoRingBuffer::PrepareRingBuffer, this);
    std::thread t([this]()
             { PrepareRingBuffer(); });
    int result = pthread_setname_np(t.native_handle(), "PC_RingBuffer_FRONT");
    if (result != 0)
    {
        // Handle failure, perhaps print an error message
        std::cerr << "\nFailed to set thread name, error: " << result << std::endl;
    }
    t.detach();

}