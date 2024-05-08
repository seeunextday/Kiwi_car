#include <opencv2/imgproc/imgproc.hpp>

using flag_t = uint32_t;

constexpr flag_t INVALID_READ = 0xFF;
constexpr flag_t VALID_READ = 0;


class KiwiDetector
{
public:
    KiwiDetector(std::map<std::string, std::string> cmd) : m_name{cmd["name"]},                                      
                                                            m_property{cmd["property"]}, m_width{static_cast<uint32_t>(std::stoi(cmd["width"]))},
                                                            m_height{static_cast<uint32_t>(std::stoi(cmd["height"]))},
                                                            m_format{cmd.count("bgr") != 0 ? CV_8UC3 : CV_8UC4}, m_verbose{cmd.count("verbose") != 0},
                                                            m_dp{std::stod(cmd["dp"])}, m_minDist{std::stod(cmd["minDist"])},
                                                            m_cannyThreshold{std::stod(cmd["cannyThreshold"])},
                                                            m_accumulatorThreshold{std::stod(cmd["accumulatorThreshold"])},
                                                            m_minRadius{static_cast<uint32_t>(std::stoi(cmd["minRadius"]))}, m_maxRadius{static_cast<uint32_t>(std::stoi(cmd["maxRadius"]))}
    {
    }

    // void cannyEdge(cv::Mat &image, cv::Mat &cannyImage) const;
    // void findContours(cv::Mat &image, std::vector<std::vector<cv::Point>> &contours) const;
    void houghCircles(cv::Mat &image, std::vector<cv::Vec3f> &circles) const;

    // Getters
    inline int32_t getFormat() const { return m_format; };
    inline bool getVerbose() const { return m_verbose; };
    inline std::string getName() const { return m_name; };
    inline uint32_t getWidth() const { return m_width; };
    inline uint32_t getHeight() const { return m_height; };

private:
    std::string m_name;
    std::string m_property;

    uint32_t const m_width;
    uint32_t const m_height;
    int32_t const m_format;
    bool const m_verbose;

    double m_dp; //Inverse ratio of the accumulator resolution to the image resolution
    double m_minDist; //Minimum distance between the centers of the detected circles
    double m_cannyThreshold; //High threshold passed to Cannyedge in HoughCircles, low=0.5*high
    double m_accumulatorThreshold; //Detection stage accumulator threshold for the circle centers
    uint32_t m_minRadius; //min radius in pixels
    uint32_t m_maxRadius; //max radius in pixels
    // Methods
};