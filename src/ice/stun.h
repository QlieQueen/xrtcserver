#ifndef __ICE_STUN_H_
#define __ICE_STUN_H_

#include <vector>
#include <string>
#include <memory>
#include <stdint.h>

namespace xrtc {

const size_t k_stun_head_size = 20;
const size_t k_stun_attribute_header_size = 4;   // 属性的type + length的长度（也是value在属性开始的偏移量）
const size_t k_stun_transaction_id_offset = 8;   // Transaction Id距离头部的偏移量
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

private:
    uint16_t _type;
    uint16_t _length;
    std::string _transaction_id;
    std::vector<std::unique_ptr<StunAttribute>> _attrs;
};


class StunAttribute {


};

// 表示类型为uint32_t的属性
class StunUInt32Attribute : public StunAttribute {
public:
    static const size_t SIZE = 4;
};


} // namespace xrtc


#endif