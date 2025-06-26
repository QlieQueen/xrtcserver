#ifndef __CODEC_INFO_H_
#define __CODEC_INFO_H_

#include <map>
#include <vector>
#include <string>

namespace xrtc {

class AudioCodecInfo;
class VideoCodecInfo;

// rtcp 控制传输质量的参数类
class FeedBackParam {
public:
    FeedBackParam(const std::string& id, const std::string& param) :
        _id(id), _param(param) {}
    FeedBackParam(const std::string& id) : _id(id), _param("") {}

    std::string id() { return _id; }
    std::string param() { return _param; }

private:
    std::string _id;
    std::string _param;
};

// 控制编解码器质量的参数类
typedef std::map<std::string, std::string> CodecParam;

class CodecInfo {
public:
    virtual AudioCodecInfo* as_audio() { return nullptr; }
    virtual VideoCodecInfo* as_video() { return nullptr; }
public:
    int id;
    std::string name;
    int clockrate;

    std::vector<FeedBackParam> feedback_param;
    CodecParam codec_param;
};

class AudioCodecInfo : public CodecInfo {
public:
    AudioCodecInfo* as_audio() override { return this; }

public:
    int channels;

};

class VideoCodecInfo : public CodecInfo {
public:
    VideoCodecInfo* as_video() override { return this; }
};

};

#endif