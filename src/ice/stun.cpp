#include <cstdint>
#include <cstring>
#include <memory>
#include <rtc_base/byte_order.h>
#include <rtc_base/crc32.h>

#include "ice/stun.h"
#include "rtc_base/byte_buffer.h"

namespace xrtc {

const char EMPTY_TRANSACTION_ID[] = "000000000000";
const size_t STUN_FINGERPRINT_XOR_VALUE = 0x5354554e;

StunMessage::StunMessage() :
        _type(0),
        _length(0),
        _transaction_id(EMPTY_TRANSACTION_ID)    
{

}

StunMessage::~StunMessage() = default;


bool StunMessage::validate_fingerprint(const char* data, size_t len) {
    // 检查长度
    size_t fingerprint_attr_size = k_stun_attribute_header_size + 
        StunUInt32Attribute::SIZE;
    if (len % 4 != 0 || len < k_stun_head_size + fingerprint_attr_size) {
        return false;
    }

    // 检查magic_cookie
    const char* magic_cookie = data + k_stun_transaction_id_offset - 
        k_stun_magic_cookie_length; //定位到magic cookie的位置
    if (rtc::GetBE32(magic_cookie) != k_stun_magic_cookie) {
        return false;
    }

    // 检查attr type和length
    const char* fingerprint_attr_data = data + len - fingerprint_attr_size; // 定位到fingerprint属性在整个报文中的偏移量
    if (rtc::GetBE16(fingerprint_attr_data) != STUN_ATTR_FINGERPRINT || // 检查属性中的type是否为fingerprint的类型(0x8028)
            rtc::GetBE16(fingerprint_attr_data + sizeof(uint16_t)) !=   // 检查属性中的length是否符合fingerprint类型属性的长度（fingerprint属性的value长度为4）
            StunUInt32Attribute::SIZE)
    {
        return false;
    }

    // 检查fingerprint的值
    uint32_t fingerprint = rtc::GetBE32(fingerprint_attr_data +
            k_stun_attribute_header_size); // 获取报文中fingerprint属性的value值
    
    // 通过STUN 的头部+所有属性的内容（不包含 FINGERPRINT的value）计算CRC32的值 并进行校验
    return (fingerprint ^ STUN_FINGERPRINT_XOR_VALUE) ==
        rtc::ComputeCrc32(data, len - fingerprint_attr_size); 
}

bool StunMessage::read(rtc::ByteBufferReader* buf) {
    if (!buf) {
        return false;
    }

    // 读取stun message的type
    if (!buf->ReadUInt16(&_type)) {
        return false;
    }

    // rtp/rtcp 10(2)
    if (_type & 0x0800) {
        return false;
    }

    // 读取stun message的length
    if (!buf->ReadUInt16(&_length)) {
        return false;
    }

    std::string magic_cookie;
    if (!buf->ReadString(&magic_cookie, k_stun_magic_cookie_length)) {
        return false;
    }

    std::string transaction_id;
    if (!buf->ReadString(&transaction_id, k_stun_transaction_id_length)) {
        return false;
    }

    uint32_t magic_cookie_int;
    memcpy(&magic_cookie_int, magic_cookie.data(), sizeof(magic_cookie_int));
    if (rtc::NetworkToHost32(magic_cookie_int) != k_stun_magic_cookie) {
        transaction_id.insert(0, magic_cookie);
    }

    _transaction_id = transaction_id;

    // 读取完头部后剩余的长度(body len)
    if (buf->Length() != _length) {
        return false;
    } 

    _attrs.resize(0);

    // 属性有多个 
    while (buf->Length() > 0) {
        uint16_t attr_type;
        uint16_t attr_length;
        if (!buf->ReadUInt16(&attr_type)) {
            return false;
        }

        if (!buf->ReadUInt16(&attr_length)) {
            return false;
        }

        // 属性有多种，使用工厂方法创建
        std::unique_ptr<StunAttribute> attr = 
                        _create_attribute(attr_type, attr_length);
        if (!attr) {
            // attr_length保存的是真实长度，但是value必须为4字节对齐，所以可能会有字节填充
            if (attr_length % 4 != 0) {
                attr_length += (4 - (attr_length % 4));
            }

            if (!buf->Consume(attr_length)) {
                return false;
            }
        } else {
            if (!attr->read(buf)) {
                return false;
            }

            _attrs.push_back(std::move(attr));

        }
    }


    return true;
}

std::unique_ptr<StunAttribute> StunMessage::_create_attribute(uint16_t type, uint16_t length) {

    return nullptr;
}


} // namespace xrtc
