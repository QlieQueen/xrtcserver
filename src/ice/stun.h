#ifndef __ICE_STUN_H_
#define __ICE_STUN_H_

#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <stdint.h>

#include <rtc_base/byte_buffer.h>

namespace xrtc {

const size_t k_stun_head_size = 20;
const size_t k_stun_attribute_header_size = 4;   // 属性的type + length的长度（也是value在属性开始的偏移量）
const size_t k_stun_transaction_id_offset = 8;   // Transaction Id距离头部的偏移量
const size_t k_stun_transaction_id_length = 12;  // Transaction Id的长度 96位
const uint32_t k_stun_magic_cookie = 0x2112A442; // magic cookie的固定取值 0x2112A442
const size_t k_stun_magic_cookie_length = sizeof(k_stun_magic_cookie); // magic cookie的长度

enum StunAttrbuteValue {
    STUN_ATTR_FINGERPRINT = 0x8028,
};

class StunAttribute;

class StunMessage {
public:
    StunMessage();
    ~StunMessage();

    static bool validate_fingerprint(const char* data, size_t len);
    bool read(rtc::ByteBufferReader* buf);

private:
    std::unique_ptr<StunAttribute> _create_attribute(uint16_t type, uint16_t length);

private:
    uint16_t _type;
    uint16_t _length;  // 不包含头部的长度（属性的长度）
    std::string _transaction_id;
    std::vector<std::unique_ptr<StunAttribute>> _attrs;
};


class StunAttribute {
public:
    StunAttribute(uint16_t type, uint16_t length);
    virtual ~StunAttribute(); // 声明为虚函数，实现父类析构函数调用子类的析构函数，回收子类资源

    virtual bool read(rtc::ByteBufferReader* buf) = 0;

private:
    uint16_t _type;
    uint16_t _length;
};

// 表示类型为uint32_t的属性
class StunUInt32Attribute : public StunAttribute {
public:
    static const size_t SIZE = 4;
};


} // namespace xrtc


#endif