#include <charconv>
#include <memory>
#include <sstream>

#include "pc/session_description.h"
#include "pc/codec_info.h"

#include <rtc_base/logging.h>

namespace xrtc {

const char k_media_protocol_dtls_savpf[] = "UDP/TLS/RTP/SAVPF"; // 支持dtls
const char k_meida_protocol_savpf[] = "RTP/SAVPF";


AudioContentDescription::AudioContentDescription() {
    auto codec = std::make_shared<AudioCodecInfo>();
    codec->id = 111;
    codec->name = "opus";
    codec->clockrate = 48000;
    codec->channels = 2;

    // add feedback param
    codec->feedback_param.push_back(FeedBackParam("transport-cc"));

    // add codec param
    codec->codec_param["minptime"] = "10";
    codec->codec_param["useinbandfec"] = "1";

    _codecs.push_back(codec);
}

VideoContentDescription::VideoContentDescription() {
    auto codec = std::make_shared<VideoCodecInfo>();
    codec->id = 107;
    codec->name = "H264";
    codec->clockrate = 90000;

    // add feedback param
    codec->feedback_param.push_back(FeedBackParam("goog-remb"));
    codec->feedback_param.push_back(FeedBackParam("transport-cc"));
    codec->feedback_param.push_back(FeedBackParam("ccm", "fir"));
    codec->feedback_param.push_back(FeedBackParam("nack"));
    codec->feedback_param.push_back(FeedBackParam("nack", "pli"));

    // add codec param
    codec->codec_param["level-asymmetry-allowed"] = "1";
    codec->codec_param["packetization-mode"] = "1";
    codec->codec_param["profile-level-id"] = "42e01";


    _codecs.push_back(codec);

    // 重传使用
    auto rtx_codec = std::make_shared<VideoCodecInfo>();
    rtx_codec->id = 99;
    rtx_codec->name = "rtx";
    rtx_codec->clockrate = 90000;
    
    // add codec param
    rtx_codec->codec_param["apt"] = std::to_string(codec->id);
    _codecs.push_back(rtx_codec);
}

bool ContentGroup::has_content_name(const std::string& content_name) {
    for (auto name : _content_names) {
        if (name == content_name) {
            return true;
        }
    }
    return false;
}

void ContentGroup::add_content_name(const std::string& content_name) {
    if (!has_content_name(content_name)) {
        _content_names.push_back(content_name);
    }
}


SessionDescription::SessionDescription(SdpType type) :
    _sdp_type(type)
{

}

SessionDescription::~SessionDescription() {

}


void SessionDescription::add_content(std::shared_ptr<MediaContentDescription> content) {
    _contents.push_back(content);
}

void SessionDescription::add_group(const ContentGroup& group) {
    _content_groups.push_back(group);
}

std::vector<const ContentGroup*> SessionDescription::get_group_by_name(
        const std::string& name) const
{
    std::vector<const ContentGroup*> content_groups;
    for (const ContentGroup& group : _content_groups) {
        if (group.semantics() == name) {
            content_groups.push_back(&group);
        }
    }

    return content_groups;
}

static void add_rtcp_fb_line(std::shared_ptr<CodecInfo> codec,
        std::stringstream& ss)
{
    for (auto param : codec->feedback_param) {
        ss << "a=rtcp-fb:" << codec->id << " " << param.id();
        if (!param.param().empty()) {
            ss << " " << param.param();
        }
        ss << "\r\n";
    }
}

static void add_fmtp_line(std::shared_ptr<CodecInfo> codec,
        std::stringstream& ss) 
{
    if (!codec->codec_param.empty()) {
        ss << "a=fmtp:" << codec->id << " ";
        std::string data;
        for (auto param : codec->codec_param) {
            data += (";" + param.first + "=" + param.second);
        }
        // data = ";key1=value1;key2=value2"
        data = data.substr(1);
        ss << data << "\r\n";
    }

}


static void build_rtp_map(std::shared_ptr<MediaContentDescription> content,
        std::stringstream& ss)
{
    for (auto codec : content->get_codecs()) {
        ss << "a=rtpmap:" << codec->id << " " << codec->name << "/" << codec->clockrate;
        if (MediaType::MEDIA_TYPE_AUDIO == content->type()) {
            auto audio_codec = codec->as_audio();   
            ss << "/" << audio_codec->channels;
        }
        ss << "\r\n";
        add_rtcp_fb_line(codec, ss);
        add_fmtp_line(codec, ss);
    }



}

std::string SessionDescription::to_string() {
    std::stringstream ss;

    // version
    ss << "v=0\r\n";
    // session orign
    // RFC 4566
    // o=<username> <sess-id> <sess-version> <nettype> <addrtype> <unicast-address>
    ss << "o=- 0 2 IN IP4 127.0.0.1\r\n";
    // session name
    ss << "s=-\r\n";
    // time description
    ss << "t=0 0\r\n";

    // BUNDLE
    std::vector<const ContentGroup*> content_group = get_group_by_name("BUNDLE"); 
    if (!content_group.empty()) {
        ss <<"a=group:BUNDLE";
        for (auto group : content_group) {
            for (auto content_name : group->content_names()) {
                ss << " " << content_name;
            }
        }
        ss << "\r\n";
    }

    ss << "a=msid-semantic: WMS\r\n";

    for (auto content : _contents) {
        // RFC 4566
        // 媒体源描述起始
        // m=<media> <port> <proto> <fmt>
        std::string fmt;
        for (auto codec : content->get_codecs()) {
            fmt.append(" ");
            fmt.append(std::to_string(codec->id));
        }

        ss << "m=" << content->mid() << " 9 " << k_media_protocol_dtls_savpf
           << fmt << "\r\n";

        ss << "c=IN IP4 0.0.0.0\r\n";
        ss << "a=rtcp:9 IN IP4 0.0.0.0\r\n";

        build_rtp_map(content, ss);
    }


    return ss.str();
}

}