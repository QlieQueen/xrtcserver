#ifndef __ICE_STUN_H_
#define __ICE_STUN_H_


#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <stdint.h>

#include <rtc_base/byte_buffer.h>
#include <rtc_base/socket_address.h>

namespace xrtc {

const size_t k_stun_header_size = 20;
const size_t k_stun_attribute_header_size = 4;   // 属性的type + length的长度（也是value在属性开始的偏移量）
const size_t k_stun_transaction_id_offset = 8;   // Transaction Id距离头部的偏移量
const size_t k_stun_transaction_id_length = 12;  // Transaction Id的长度 96位
const uint32_t k_stun_magic_cookie = 0x2112A442; // magic cookie的固定取值 0x2112A442
const size_t k_stun_magic_cookie_length = sizeof(k_stun_magic_cookie); // magic cookie的长度
const size_t k_stun_message_integrity_size = 20;

// stun协议的消息类型，如 Binding Request、Binding Response
enum StunMessageType {
    STUN_BINDING_REQUEST = 0x0001,
    STUN_BINDING_RESPONSE = 0x0101,

};

// stun协议中消息携带的属性的属性类型，如Binding Request消息携带了多个属性
// USERNAME          -- 0x0006
// PRIORITY          -- 0x0024
// MESSAGE_INTEGRIT  -- 0x0008
// FINGERPRINT       -- 0x8028
enum StunAttrbuteType {
    STUN_ATTR_USERNAME = 0x0006,
    STUN_ATTR_MESSAGE_INTEGRITY = 0x0008,
    STUN_ATTR_XOR_MAPPED_ADDRESS = 0x0020,
    STUN_ATTR_PRIORITY = 0x0024,
    STUN_ATTR_FINGERPRINT = 0x8028,
};

enum StunErrorCode {
    STUN_ERROR_BAD_REQUEST = 400,
    STUN_ERROR_UNATHORIZED = 401,
    STUN_ERROR_SERVER_ERROR = 500,
};

enum StunAddressFamily {
    STUN_ADDRESS_UNDEF = 0,
    STUN_ADDRESS_IPV4 = 1,
    STUN_ADDRESS_IPV6 = 2,
};

extern const char STUN_ERROR_REASON_BAD_REQUEST[];
extern const char STUN_ERROR_REASON_UNATHORIZED[];
extern const char STUN_ERROR_REASON_SERVER_ERROR[];


// 对属性中的value进行分类
// 比如：
//     FINGERPRIN: 存储的是uin32_t的值，所以定义其类型为：STUN_VALUE_UINT32
//     STUN_ATTR_USERNAME: 存储的是字符串类型，所以定义其类型为: STUN_VALUE_BYTE_STRING
enum StunAttributeValueType {
    STUN_VALUE_UNKNOWN = 0,
    STUN_VALUE_UINT32,
    STUN_VALUE_BYTE_STRING,
};

class StunAttribute;
class StunUInt32Attribute;
class StunByteStringAttribute;

std::string stun_method_to_string(int type);

class StunMessage {
public:
    enum class IntegrityStatus {
        k_not_set,
        k_no_integrity,
        k_integrity_ok,
        k_integrity_bad,
    };
    StunMessage();
    ~StunMessage();

    int type() const { return _type; }
    void set_type(uint16_t type) { _type = type; }

    size_t length() const { return _length; }
    void set_length(uint16_t length) { _length = length; }

    const std::string& transaction_id() const { return _transaction_id; }
    void set_transaction_id(const std::string& transaction_id) {
        _transaction_id = transaction_id; // 只支持新版本，不支持经典stun
    }

    static bool validate_fingerprint(const char* data, size_t len);
    bool add_fingerprint();

    IntegrityStatus validate_message_integrity(const std::string& password);
    bool add_message_integrity(const std::string& password);

    StunAttributeValueType get_attribute_value_type(int type);

    bool read(rtc::ByteBufferReader* buf); 
    bool write(rtc::ByteBufferWriter* buf) const;

    void add_attribute(std::unique_ptr<StunAttribute> attr);

    const StunByteStringAttribute* get_byte_string(uint16_t type);
    const StunUInt32Attribute* get_uint32_t(uint16_t type);

private:
    StunAttribute* _create_attribute(uint16_t type, uint16_t length);
    const StunAttribute* _get_attribute(uint16_t type);
    bool _validate_message_integrity_of_type(uint16_t mi_attr_type,
        size_t mi_attr_size, const char* data, size_t size,
        const std::string& password);
    bool _add_message_integrity_of_type(uint16_t attr_type,
        uint16_t attr_size, const char* key, size_t len);

private:
    uint16_t _type;
    uint16_t _length;  // 不包含头部的长度（属性的长度）
    std::string _transaction_id;
    std::vector<std::unique_ptr<StunAttribute>> _attrs;
    IntegrityStatus _integrity = IntegrityStatus::k_not_set;
    std::string _password;
    std::string _buffer;
};


class StunAttribute {
public:
    virtual ~StunAttribute(); // 声明为虚函数，实现父类析构函数调用子类的析构函数，回收子类资源

    int type() const { return _type; }
    void set_type(uint16_t type) {  _type = type; }

    size_t length() const { return _length; }
    void set_length(uint16_t length) { _length = length; }

    static StunAttribute* create(StunAttributeValueType value_type,
            uint16_t type, uint16_t length, void* owner);
    virtual bool read(rtc::ByteBufferReader* buf) = 0;
    virtual bool write(rtc::ByteBufferWriter* buf) = 0;

protected:
    StunAttribute(uint16_t type, uint16_t length);
    void consume_padding(rtc::ByteBufferReader* buf);
    void write_padding(rtc::ByteBufferWriter* buf);

private:
    uint16_t _type;
    uint16_t _length;
};

class StunAddressAttribute : public StunAttribute {
public:
    static const size_t SIZE_UNDEF = 0;
    static const size_t SIZE_IPV4 = 8;
    static const size_t SIZE_IPV6 = 20;
    
    StunAddressAttribute(uint16_t type, const rtc::SocketAddress& addr);
    ~StunAddressAttribute() {}

    void set_address(const rtc::SocketAddress& addr);
    StunAddressFamily family();

    bool read(rtc::ByteBufferReader* buf) override;
    bool write(rtc::ByteBufferWriter* buf) override;

protected:
    rtc::SocketAddress _address;
};

class StunXorAddressAttribute : public StunAddressAttribute {
public:
    StunXorAddressAttribute(uint16_t type, const rtc::SocketAddress& addr);
    ~StunXorAddressAttribute() {}

    bool write(rtc::ByteBufferWriter* buf) override;
private:
    rtc::IPAddress _get_xored_ip();
};

// 表示类型为uint32_t的属性
class StunUInt32Attribute : public StunAttribute {
public:
    static const size_t SIZE = 4;
    StunUInt32Attribute(uint16_t type);
    StunUInt32Attribute(uint16_t type, uint32_t value);
    ~StunUInt32Attribute() override {}

    uint32_t value() const { return _bits; }
    void set_value(uint32_t value) { _bits = value; }
    bool read(rtc::ByteBufferReader* buf) override;
    bool write(rtc::ByteBufferWriter* buf) override;

private:
    uint32_t _bits;
};

class StunByteStringAttribute : public StunAttribute {
public:
    StunByteStringAttribute(uint16_t type, uint16_t length);
    StunByteStringAttribute(uint16_t, const std::string& str);
    ~StunByteStringAttribute() override;

    void copy_bytes(const char* bytes, size_t len);

    bool read(rtc::ByteBufferReader* buf) override;
    bool write(rtc::ByteBufferWriter* buf) override;

    std::string get_string() const { return std::string(_bytes, length()); }

private:
    void _set_bytes(char* bytes);

private:
    char* _bytes = nullptr;
};


} // namespace xrtc


#endif