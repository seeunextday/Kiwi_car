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
                                                            m_format{cmd.count("bgr") != 0 ? CV_8UC3 : CV_8UC4}, m_verbose{cmd.count("verbose") != 0}
    {
    }

    // void cannyEdge(cv::Mat &image, cv::Mat &cannyImage) const;
    // void findContours(cv::Mat &image, std::vector<std::vector<cv::Point>> &contours) const;

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

    // Methods
};