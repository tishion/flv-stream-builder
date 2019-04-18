/*
 * This CPP header-only file implements the FLV stream packer according to the
 * FLV file format specification. Please refer to
 * https://wwwimages2.adobe.com/content/dam/acom/en/devnet/flv/video_file_format_spec_v10.pdf
 * for all types and values.
 *
 * Sheen Tian @ 2019/01/26
 * https://github.com/tishion
 *
 */

#pragma once
#include <assert.h>
#include <atomic>
#include <map>
#include <memory>
#include <vector>
#include <string>

// @cond PRIVATE_ENTITY
/// <summary>
/// Disallows the copy constructor and operator= functions.
/// </summary>
// @endcond
#define DISALLOW_COPY_AND_ASSIGN(cls)                                          \
  cls(const cls &) = delete;                                                   \
  cls &operator=(const cls &) = delete

namespace flv {
namespace amf {
/// <summary>
/// The AFM object types.
/// </summary>
enum amf_value_type_e {
  NumberType = 0,  // 8 bytes
  BooleanType = 1, // 1 bytes
  StringType = 2,
  ObjectType = 3,
  MovieClipType = 4,
  NullType = 5,
  UndefinedType = 6,
  ReferenceType = 7,
  ECMAArrayType = 8,
  StrictArrayType = 10,
  DateType = 11,
  LongStringType = 12,
};
typedef amf_value_type_e amf_value_type_t;

template <class T> class amf_ref : public std::shared_ptr<T> {
public:
  using std::shared_ptr<T>::shared_ptr;
  amf_ref(const std::shared_ptr<T>& p) : std::shared_ptr<T>(p){}
protected:
  T* get() { return std::shared_ptr<T>::get(); }
};

/// <summary>
/// Represents the AMF object root.
/// </summary>
class amf_root {
public:
  virtual ~amf_root(){};

  /// <summary>
  /// Gets the AMF value type.
  /// </summary>
  /// <returns></returns>
  virtual amf_value_type_t value_type() const = 0;

  /// <summary>
  /// Serializes the AMF object to bytes array.
  /// </summary>
  /// <param name="buf">The buffer to receive the serialized bytes array
  /// data.</param>
  virtual void serialize(std::vector<uint8_t> &buf) = 0;

  /// <summary>
  /// Deserializes the bytes array to AMF object.
  /// </summary>
  /// <param name="data">The bytes array data to be parsed.</param>
  /// <returns>True if successful; otherwise fale.</returns>
  virtual bool deserialize(const std::vector<uint8_t> &data) = 0;
};
typedef amf_ref<amf_root> amf_root_ref;

/// <summary>
/// Represents the AMF value object base.
/// </summary>
class amf_value : public amf_root {
protected:
  // <summary>
  // The AMF value type.
  // </summary>
  amf_value_type_t type;

protected:
  explicit amf_value(amf_value_type_t t) : type(t){};
  ~amf_value() {}

  /// <summary>
  /// Gets the AMF value type.
  /// </summary>
  /// <returns></returns>
  virtual amf_value_type_t value_type() const override { return type; }
};
typedef amf_ref<amf_value> amf_value_ref;

/// <summary>
/// Represents the AMF Number object.
/// </summary>
class amf_number : public amf_value {
public:
  /// <summary>
  /// Creates an instance of the AMF Number object.
  /// </summary>
  /// <param name="value">The value of the Number object.</param>
  /// <returns>The instance of the AMF Number object.</returns>
  static amf_ref<amf_number> create(double value) {
    return amf_ref<amf_number>(new amf_number(value));
  }

  /// <summary>
  /// Serializes the AMF object to bytes array.
  /// </summary>
  /// <param name="buf">The buffer to receive the serialized bytes array
  /// data.</param>
  virtual void serialize(std::vector<uint8_t> &buf) override {
    buf.reserve(buf.size() + 1 + 8);
    buf.emplace_back(type);
    uint8_t *p = (uint8_t *)&v;
    int i = 8;
    do {
      buf.emplace_back(p[--i]);
    } while (i);
  }

  /// <summary>
  /// Deserializes the bytes array to AMF object.
  /// </summary>
  /// <param name="data">The bytes array data to be parsed.</param>
  /// <returns>True if successful; otherwise fale.</returns>
  virtual bool deserialize(const std::vector<uint8_t> &data) override {
    throw std::logic_error("The method or operation is not implemented.");
  }

protected:
  explicit amf_number(double value) : amf_value(NumberType), v(value){};

private:
  DISALLOW_COPY_AND_ASSIGN(amf_number);

private:
  /// <summary>
  /// The number value.
  /// </summary>
  double v;
};
typedef amf_ref<amf_number> amf_number_ref;

/// <summary>
/// Represents the AMF Boolean object.
/// </summary>
class amf_boolean : public amf_value {
public:
  /// <summary>
  /// Creates an instance of the AMF Boolean object.
  /// </summary>
  /// <param name="value">The value of the Boolean object.</param>
  /// <returns>The instance of the AMF Boolean object.</returns>
  static amf_ref<amf_boolean> create(bool value) {
    return amf_ref<amf_boolean>(new amf_boolean(value));
  }

  /// <summary>
  /// Serializes the AMF object to bytes array.
  /// </summary>
  /// <param name="buf">The buffer to receive the serialized bytes array
  /// data.</param>
  virtual void serialize(std::vector<uint8_t> &buf) override {
    buf.reserve(buf.size() + 1 + 1);
    buf.emplace_back(type);
    buf.emplace_back(v);
  }

  /// <summary>
  /// Deserializes the bytes array to AMF object.
  /// </summary>
  /// <param name="data">The bytes array data to be parsed.</param>
  /// <returns>True if successful; otherwise fale.</returns>
  virtual bool deserialize(const std::vector<uint8_t> &data) override {
    throw std::logic_error("The method or operation is not implemented.");
  }

protected:
  explicit amf_boolean(bool value) : amf_value(BooleanType), v(value){};

private:
  DISALLOW_COPY_AND_ASSIGN(amf_boolean);

private:
  /// <summary>
  /// The boolean value.
  /// </summary>
  bool v;
};
typedef amf_ref<amf_boolean> amf_boolean_ref;

/// <summary>
/// Represents the AMF String object.
/// </summary>
class amf_string : public amf_value {
public:
  /// <summary>
  /// Creates an instance of the AMF String object.
  /// </summary>
  /// <param name="value">The value of the String object.</param>
  /// <returns>The instance of the AMF String object.</returns>
  static amf_ref<amf_string> create(const char *value) {
    assert(strlen(value) < (size_t)0xffff);
    return amf_ref<amf_string>(new amf_string(value));
  }

  /// <summary>
  /// Serializes the AMF object to bytes array.
  /// </summary>
  /// <param name="buf">The buffer to receive the serialized bytes array
  /// data.</param>
  virtual void serialize(std::vector<uint8_t> &buf) override {
    assert(v.length() < (size_t)0xffff);
    uint16_t length = static_cast<uint16_t>(v.length());
    buf.reserve(buf.size() + 1 + 2 + length);
    buf.emplace_back(type);
    buf.emplace_back(length >> 8);
    buf.emplace_back(length & 0x00ff);
    std::copy(v.begin(), v.begin() + length, std::back_inserter(buf));
  }

  /// <summary>
  /// Deserializes the bytes array to AMF object.
  /// </summary>
  /// <param name="data">The bytes array data to be parsed.</param>
  /// <returns>True if successful; otherwise fale.</returns>
  virtual bool deserialize(const std::vector<uint8_t> &data) override {
    throw std::logic_error("The method or operation is not implemented.");
  }

protected:
  explicit amf_string(const char *value) : amf_value(StringType), v(value){};

private:
  DISALLOW_COPY_AND_ASSIGN(amf_string);

private:
  /// <summary>
  /// The string value.
  /// </summary>
  std::string v;
};
typedef amf_ref<amf_string> amf_string_ref;

/// <summary>
/// Represents the AMF Object object.
/// </summary>
class amf_object : public amf_value,
                   public std::enable_shared_from_this<amf_object> {
public:
  /// <summary>
  /// Enables the std::make_shared to be able to access
  /// the protected constructor and destructor.
  /// </summary>
  // friend class std::_Ref_count_obj<amf_object>;

  /// <summary>
  /// Creates an instance of the AMF Object object.
  /// </summary>
  /// <param name="value">The value of the Object object.</param>
  /// <returns>The instance of the AMF Object object.</returns>
  static amf_ref<amf_object> create() {
    return amf_ref<amf_object>(new amf_object());
  }

  /// <summary>
  /// Adds an AMF Number property for the AMF Object instance.
  /// </summary>
  /// <param name="key">The property name,</param>
  /// <param name="v">The property value.</param>
  /// <returns>The self-reference.</returns>
  amf_ref<amf_object> with_property(const char *key, double v) {
    assert(strlen(key) < (size_t)0xffff);
    auto pv = amf_number::create(v);
    return this->with_property(key, pv);
  }

  /// <summary>
  /// Adds an AMF Boolean property for the AMF Object instance.
  /// </summary>
  /// <param name="key">The property name,</param>
  /// <param name="v">The property value.</param>
  /// <returns>The self-reference.</returns>
  amf_ref<amf_object> with_property(const char *key, bool v) {
    assert(strlen(key) < (size_t)0xffff);
    auto pv = amf_boolean::create(v);
    return this->with_property(key, pv);
  }

  /// <summary>
  /// Adds an AMF String property for the AMF Object instance.
  /// </summary>
  /// <param name="key">The property name,</param>
  /// <param name="v">The property value.</param>
  /// <returns>The self-reference.</returns>
  amf_ref<amf_object> with_property(const char *key, const char *v) {
    assert(strlen(key) < (size_t)0xffff);
    auto pv = amf_string::create(v);
    return this->with_property(key, pv);
  }

  /// <summary>
  /// Adds an AMF value property for the AMF Object instance.
  /// </summary>
  /// <param name="key">The property name,</param>
  /// <param name="v">The property value.</param>
  /// <returns>The self-reference.</returns>
  amf_ref<amf_object> with_property(const char *key, amf_value_ref v) {
    this->v[key] = v;
    return shared_from_this();
  }

  /// <summary>
  /// Serializes the AMF object to bytes array.
  /// </summary>
  /// <param name="buf">The buffer to receive the serialized bytes array
  /// data.</param>
  virtual void serialize(std::vector<uint8_t> &buf) override {
    buf.reserve(buf.size() + 1);
    buf.emplace_back(type);

    uint16_t key_length = 0;
    for (auto &kv : v) {
      key_length = static_cast<uint16_t>(kv.first.length());
      buf.emplace_back(key_length >> 8);
      buf.emplace_back(key_length & 0x0ff);
      std::copy(kv.first.begin(), kv.first.begin() + key_length,
                std::back_inserter(buf));

      kv.second->serialize(buf);
    }

    buf.emplace_back(0);
    buf.emplace_back(0);
    buf.emplace_back(9);
  }

  /// <summary>
  /// Deserializes the bytes array to AMF object.
  /// </summary>
  /// <param name="data">The bytes array data to be parsed.</param>
  /// <returns>True if successful; otherwise fale.</returns>
  virtual bool deserialize(const std::vector<uint8_t> &data) override {
    throw std::logic_error("The method or operation is not implemented.");
  }

protected:
  explicit amf_object() : amf_value(ObjectType){};

private:
  DISALLOW_COPY_AND_ASSIGN(amf_object);

private:
  /// <summary>
  /// The properties collection.
  /// </summary>
  std::map<std::string, amf_value_ref> v;
};
typedef amf_ref<amf_object> amf_object_ref;

/// <summary>
/// Represents the AMF Array object.
/// </summary>
class amf_array : public amf_value,
                  public std::enable_shared_from_this<amf_array> {
public:
  /// <summary>
  /// Creates an instance of the AMF Array object.
  /// </summary>
  /// <param name="value">The value of the Array object.</param>
  /// <returns>The instance of the AMF Array object.</returns>
  static amf_ref<amf_array> create() {
    return amf_ref<amf_array>(new amf_array());
  }

  /// <summary>
  /// Adds an AMF Number item to the AMF Array instance.
  /// </summary>
  /// <param name="key">The property name,</param>
  /// <param name="v">The property value.</param>
  /// <returns>The self-reference.</returns>
  amf_ref<amf_array> with_item(const char *key, double v) {
    auto pv = amf_number::create(v);
    assert(strlen(key) < (size_t)0xffff);
    return this->with_item(key, pv);
  }

  /// <summary>
  /// Adds an AMF Boolean item to the AMF Array instance.
  /// </summary>
  /// <param name="key">The property name,</param>
  /// <param name="v">The property value.</param>
  /// <returns>The self-reference.</returns>
  amf_ref<amf_array> with_item(const char *key, bool v) {
    assert(strlen(key) < (size_t)0xffff);
    auto pv = amf_boolean::create(v);
    return this->with_item(key, pv);
  }

  /// <summary>
  /// Adds an AMF String item to the AMF Array instance.
  /// </summary>
  /// <param name="key">The property name,</param>
  /// <param name="v">The property value.</param>
  /// <returns>The self-reference.</returns>
  amf_ref<amf_array> with_item(const char *key, const char *v) {
    assert(strlen(key) < (size_t)0xffff);
    auto pv = amf_string::create(v);
    return this->with_item(key, pv);
  }

  /// <summary>
  /// Adds an AMF value item to the AMF Array instance.
  /// </summary>
  /// <param name="key">The property name,</param>
  /// <param name="v">The property value.</param>
  /// <returns>The self-reference.</returns>
  amf_ref<amf_array> with_item(const char *key, amf_value_ref v) {
    this->v[key] = v;
    return shared_from_this();
  }

  /// <summary>
  /// Serializes the AMF object to bytes array.
  /// </summary>
  /// <param name="buf">The buffer to receive the serialized bytes array
  /// data.</param>
  virtual void serialize(std::vector<uint8_t> &buf) override {
    buf.reserve(buf.size() + 1);
    buf.emplace_back(type);
    uint32_t count = v.size();
    buf.emplace_back((count & 0xff000000) >> 24);
    buf.emplace_back((count & 0x00ff0000) >> 16);
    buf.emplace_back((count & 0x0000ff00) >> 8);
    buf.emplace_back((count & 0x000000ff));

    uint16_t key_length = 0;
    for (auto &kv : v) {
      key_length = static_cast<uint16_t>(kv.first.length());
      buf.emplace_back(key_length >> 8);
      buf.emplace_back(key_length & 0x0ff);
      std::copy(kv.first.begin(), kv.first.begin() + key_length,
                std::back_inserter(buf));

      kv.second->serialize(buf);
    }

    buf.emplace_back(0);
    buf.emplace_back(0);
    buf.emplace_back(9);
  }

  /// <summary>
  /// Deserializes the bytes array to AMF object.
  /// </summary>
  /// <param name="data">The bytes array data to be parsed.</param>
  /// <returns>True if successful; otherwise fale.</returns>
  virtual bool deserialize(const std::vector<uint8_t> &data) override {
    throw std::logic_error("The method or operation is not implemented.");
  }

protected:
  explicit amf_array() : amf_value(ECMAArrayType){};

private:
  DISALLOW_COPY_AND_ASSIGN(amf_array);

private:
  /// <summary>
  /// The items collection.
  /// </summary>
  std::map<std::string, amf_value_ref> v;
};
typedef amf_ref<amf_array> amf_array_ref;
} // namespace amf

static const uint8_t FLV_HEADER_SIZE = 9;
static const uint8_t FLV_TAG_HEADER_SIZE = 11;
static const uint8_t VIDEO_HEADER_SIZE = 5;
static const uint8_t VIDEO_SPECIFIC_CONFIG_EXTENDED_SIZE = 11;
static const uint8_t AUDIO_HEADER_SIZE = 2;
static const uint8_t AUDIO_SPECIFIC_CONFIG_SIZE = 2;
static const char *ON_META_DATA = "onMetaData";
static const uint8_t ON_META_DATA_LENGTH = 0x0a;

/// <summary>
/// The FLV tag types.
/// </summary>
enum tag_type_e {
  Unknown = 0,
  Audio = 0x08,
  Video = 0x09,
  Script = 0x12,

  FORCE_UINT8 = (uint8_t)0xff
};
typedef tag_type_e tag_type_t;

/// <summary>
/// The audio data sound formats.
/// </summary>
enum audio_data_sound_format {
  LPCM_PE = 0,
  ADPCM = 1,
  MP3 = 2,
  LPCM_LE = 3,
  NELLYMOSER_16K_MONO = 4,
  NELLYMOSER_8K_MONO = 5,
  NELLYMOSER = 6,
  G711_A = 7,
  G711_MU = 8,
  RESERVERD = 9,
  AAC = 10,
  SPEEX = 11,
  MP3_8K = 14,
  DEVICE_SPECIFIC = 15,
};

/// <summary>
/// The AAC audio data packet types.
/// </summary>
enum aac_audio_data_packet_type {
  AacSequenceHeader = 0,
  AacRaw = 1,
};

/// <summary>
/// The audio data sound sample rates.
/// </summary>
enum audio_data_sound_rate_e {
  R5K5HZ = 0,
  R11KHZ = 1,
  R22KHZ = 2,
  R44KHZ = 3,
};
typedef audio_data_sound_rate_e audio_data_sound_rate_t;

/// <summary>
/// The audio data sound bit depths.
/// </summary>
enum audio_data_sound_size_e {
  S8BIT = 0,
  S16BIT = 1,
};
typedef audio_data_sound_size_e audio_data_sound_size_t;

/// <summary>
/// The audio data sound channel counts.
/// </summary>
enum audio_data_sound_type_e {
  MONO = 0,
  STEREO = 1,
};
typedef audio_data_sound_type_e audio_data_sound_type_t;

/// <summary>
/// The video data frame types.
/// </summary>
enum video_data_frame_type {
  KEY_FRAME = 1,
  INTER_FRAME = 2,
  DISPOSABLE_INTER_FRAME = 3,
  GENERATED_KEY_FRAME = 4,
  VIDEO_INFO_COMMAND_FRAME = 5,
};

/// <summary>
/// The video data codec ids.
/// </summary>
enum video_data_codec_id {
  JPEG = 1,
  H263 = 2,
  SCREEN_VIDEO = 3,
  ON2VP6 = 4,
  ON2VP6_WITH_ALPHA = 5,
  SCREEN_VIDEO_V2 = 6,
  AVC = 7,
};

/// <summary>
/// The AVC video packet types.
/// </summary>
enum avc_video_packet_type {
  AvcSequenceHeader = 0,
  AvcNALU = 1,
  AvcSequenceHeaderEOF = 2,
};

/// <summary>
/// Represents the FLV stream builder.
/// </summary>
class flv_stream_builder {
private:
  /// <summary>
  /// The tag count already proceed.
  /// </summary>
  std::atomic<uint64_t> tag_count_;

  /// <summary>
  /// Indicates whether there is audio data in the stream.
  /// </summary>
  bool has_audio_;

  /// <summary>
  /// Indicates whether there is video data in the stream.
  /// </summary>
  bool has_video_;

public:
  /// <summary>
  /// Constructs an instance of the FLV builder stream.
  /// </summary>
  flv_stream_builder() {}

  /// <summary>
  /// Destructs the instance.
  /// </summary>
  ~flv_stream_builder() {}

  /// <summary>
  /// Resets the FLV stream builder instance.
  /// </summary>
  void reset() {
    tag_count_ = 0;
    has_audio_ = false;
    has_video_ = false;
  }

  /// <summary>
  /// Initializes the FLV stream/file header and append it to the end of the
  /// specified buffer.
  /// </summary>
  /// <param name="buf">The buffer to receive the header data.</param>
  /// <param name="has_audio">Whether there is audio data or not.</param>
  /// <param name="has_video">Whether there is video data or not.</param>
  /// <returns>The self-reference.</returns>
  flv_stream_builder &init_stream_header(std::vector<uint8_t> &buf,
                                         bool has_audio, bool has_video) {
    buf.clear();
    buf.resize(9 + 4, 0);

    uint8_t flags = 0;
    has_audio_ = has_audio;
    if (has_audio_) {
      flags |= 0x04;
    }
    has_video_ = has_video;
    if (has_video_) {
      flags |= 0x01;
    }

    // Signature
    buf[0] = 'F';
    buf[1] = 'L';
    buf[2] = 'V';

    // Version
    buf[3] = 0x01;

    // Flags
    buf[4] = flags;

    // Header size
    buf[5] = 0;
    buf[6] = 0;
    buf[7] = 0;
    buf[8] = FLV_HEADER_SIZE;

    buf[9] = 0;
    buf[10] = 0;
    buf[11] = 0;
    buf[12] = 0;
    return *this;
  }

  /// <summary>
  /// Appends a meta tag data to the end of the specified buffer. This method
  /// uses the meta passed in as AFM type 2 and appends it to a constructed AMF
  /// type 1. Then it constructs a meta tag (head + body) with the AMF type 1
  /// and type 2 and append it to the end of the buffer.
  /// </summary>
  /// <param name="buf">The buffer to receive the meta tag data.</param>
  /// <param name="meta">
  /// An AMF value which represents AMF type 2 of the meta tag data.
  /// </param>
  /// <returns>The self-reference.</returns>
  flv_stream_builder &append_meta_tag(std::vector<uint8_t> &buf,
                                      amf::amf_value_ref meta) {
    std::vector<uint8_t> meta_data;
    amf::amf_string::create(ON_META_DATA)->serialize(meta_data);
    meta->serialize(meta_data);
    append_tag(buf, Script, 0, 0, meta_data.data(), meta_data.size());
    return *this;
  }

  /// <summary>
  /// Appends a new video tag to the end of the specified buffer. This method
  /// uses the data passed in as an VIDEODATA to construct a video tag
  /// with the VIDEODATA and appends it to the end of the buffer.
  /// </summary>
  /// <param name="buf">The buffer to receive the video tag data.</param>
  /// <param name="timestamp">The timetamp of the tag.</param>
  /// <param name="data">The video tag body data.</param>
  /// <param name="length">The lenght of the tag body data.</param>
  /// <returns>The self-reference.</returns>
  flv_stream_builder &append_video_tag(std::vector<uint8_t> &buf,
                                       uint32_t timestamp, const uint8_t *data,
                                       uint32_t length) {
    append_tag(buf, Video, timestamp, 0, data, length);
    return *this;
  }

  /// <summary>
  /// Appends a new video tag to the end of the specified buffer. This method
  /// first uses the data passed in as an AVCDecoderConfigRecord to construct an
  /// AVCVideoPacket, then constructs a VIDEODATA with the AVCVideoPacket.
  /// Finally it constructs a video tag with the VIDEODATA and appends it to the
  /// end of the buffer.
  /// </summary>
  /// <param name="buf">The buffer to receive the video tag data.</param>
  /// <param name="timestamp">The timetamp of the tag.</param>
  /// <param name="data">The AVCDecoderConfigRecord data.</param>
  /// <param name="length">The data length.</param>
  /// <returns>The self-reference.</returns>
  flv_stream_builder &append_video_tag_with_avc_decoder_config(
      std::vector<uint8_t> &buf, uint32_t timestamp, const uint8_t *data,
      uint32_t length) {
    std::vector<uint8_t> avc_packet;
    avc_packet.reserve(512);
    avc_packet.emplace_back(INTER_FRAME << 4 | AVC);
    avc_packet.emplace_back(AvcSequenceHeader);
    avc_packet.emplace_back(0);
    avc_packet.emplace_back(0);
    avc_packet.emplace_back(0);
    std::copy(data, data + length, std::back_inserter(avc_packet));
    append_video_tag(buf, timestamp, avc_packet.data(), avc_packet.size());
    return *this;
  }

  /// <summary>
  /// Appends a new video tag to the end of the specified buffer. This method
  /// first uses the data passed in as an NALU to construct an AVCVideoPacket,
  /// then constructs a VIDEODATA with the AVCVideoPacket. Finally it constructs
  /// a video tag with the VIDEODATA and appends it to the end of the buffer.
  /// </summary>
  /// <param name="buf">The buffer to receive the video tag data.</param>
  /// <param name="timestamp">The timetamp of the tag.</param>
  /// <param name="data">
  /// The NALU data (AVC format, first 4 bytes represents the length).
  /// </param> <param name="length">The data length.</param>
  /// <returns>The self-reference.</returns>
  flv_stream_builder &
  append_video_tag_with_avc_nalu_data(std::vector<uint8_t> &buf,
                                      uint32_t timestamp, const uint8_t *data,
                                      uint32_t length) {
    std::vector<uint8_t> avc_packet;
    avc_packet.reserve(512);
    avc_packet.emplace_back(INTER_FRAME << 4 | AVC);
    avc_packet.emplace_back(AvcNALU);
    uint32_t composition_time = timestamp;
    avc_packet.emplace_back((composition_time & 0x00ff0000) >> 16);
    avc_packet.emplace_back((composition_time & 0x0000ff00) >> 8);
    avc_packet.emplace_back((composition_time & 0x000000ff));
    std::copy(data, data + length, std::back_inserter(avc_packet));
    append_video_tag(buf, timestamp, avc_packet.data(), avc_packet.size());
    return *this;
  }

  /// <summary>
  /// Appends a new audio tag to the end of the specified buffer. This method
  /// first uses the data passed in as an AUDIODATA to construct a video tag
  /// with the AUDIODATA, then appends it to the end of the buffer.
  /// </summary>
  /// <param name="buf">The buffer to receive the audio tag data.</param>
  /// <param name="timestamp">The timetamp of the tag.</param>
  /// <param name="data">The audio tag body data.</param>
  /// <param name="length">The lenght of the tag body data.</param>
  /// <returns>The self-reference.</returns>
  flv_stream_builder &append_audio_tag(std::vector<uint8_t> &buf,
                                       uint32_t timestamp, const uint8_t *data,
                                       uint32_t length) {
    append_tag(buf, Audio, timestamp, 0, data, length);
    return *this;
  }

  /// <summary>
  /// Appends a new audio tag to the end of the specified buffer. This method
  /// first uses the data passed in as an AudioSpecificConfig to construct an
  /// AACAudioPacket, then constructs an AUDIODTA with the AACAudioPacket.
  /// Finally it constructs a video tag with the AUDIODTA and appends it to the
  /// end of the buffer.
  /// </summary>
  /// <param name="buf">The buffer to receive the audio tag data.</param>
  /// <param name="timestamp">The timetamp of the tag.</param>
  /// <param name="rate">The sound sample rate.</param>
  /// <param name="size">The sound bit depth.</param>
  /// <param name="type">The sound channel count.</param>
  /// <param name="data">The AudioSpecificConfig data.</param>
  /// <param name="length">The lenght of the AudioSpecificConfig data.</param>
  /// <returns>The self-reference.</returns>
  flv_stream_builder &append_audio_tag_with_aac_specific_config(
      std::vector<uint8_t> &buf, uint32_t timestamp,
      audio_data_sound_rate_t rate, audio_data_sound_size_t size,
      audio_data_sound_type_t type, const uint8_t *data, uint32_t length) {
    std::vector<uint8_t> aac_packet;
    aac_packet.reserve(32);
    uint8_t fb = AAC << 4;
    fb |= ((rate << 2) & 0x06);
    fb |= ((size << 1) & 0x02);
    fb |= ((type)&0x01);
    aac_packet.emplace_back(fb);
    aac_packet.emplace_back(AacSequenceHeader);
    std::copy(data, data + length, std::back_inserter(aac_packet));
    append_audio_tag(buf, timestamp, aac_packet.data(), aac_packet.size());
    return *this;
  }

  /// <summary>
  /// Appends a new audio tag to the end of the specified buffer. This method
  /// first uses the data passed in as an raw AAC frame data to construct an
  /// AACAudioPacket, then constructs an AUDIODTA with the AACAudioPacket.
  /// Finally it constructs a video tag with the AUDIODTA and appends it to the
  /// end of the buffer.
  /// </summary>
  /// <param name="buf">The buffer to receive the audio tag data.</param>
  /// <param name="timestamp">The timetamp of the tag.</param>
  /// <param name="rate">The sound sample rate.</param>
  /// <param name="size">The sound bit depth.</param>
  /// <param name="type">The sound channel count.</param>
  /// <param name="data">The raw AAC frame data.</param>
  /// <param name="length">The length of the raw AAC frame data.</param>
  /// <returns>The self-reference.</returns>
  flv_stream_builder &append_audio_tag_with_aac_frame_data(
      std::vector<uint8_t> &buf, uint32_t timestamp,
      audio_data_sound_rate_t rate, audio_data_sound_size_t size,
      audio_data_sound_type_t type, const uint8_t *data, uint32_t length) {
    std::vector<uint8_t> aac_packet;
    aac_packet.reserve(32);
    uint8_t fb = AAC << 4;
    fb |= ((rate << 2) & 0x06);
    fb |= ((size << 1) & 0x02);
    fb |= ((type)&0x01);
    aac_packet.emplace_back(fb);
    aac_packet.emplace_back(AacRaw);
    std::copy(data, data + length, std::back_inserter(aac_packet));
    append_audio_tag(buf, timestamp, aac_packet.data(), aac_packet.size());
    return *this;
  }

protected:
  /// <summary>
  /// Appends a new flv tag to the end of the specified buffer. This method
  /// uses the data passed in as an tag body to constructs a FLV tag then
  /// appends it to the end of the buffer.
  /// </summary>
  /// <param name="buf">The buffer to receive the audio tag data.</param>
  /// <param name="type"></param>
  /// <param name="timestamp">The timetamp of the tag.</param>
  /// <param name="strem_id">The strem id (always 0).</param>
  /// <param name="data">The tag body data.</param>
  /// <param name="length">The lenght of the tag body data.</param>
  void append_tag(std::vector<uint8_t> &buf, tag_type_t type,
                  uint32_t timestamp, uint32_t strem_id, const uint8_t *data,
                  uint32_t length) {
    int32_t original_size = buf.size();
    buf.reserve(buf.size() + FLV_TAG_HEADER_SIZE + length);

    // Header.Type
    buf.emplace_back(type);

    // Header.DataSize
    buf.emplace_back((length & 0x00ff0000) >> 16);
    buf.emplace_back((length & 0x0000ff00) >> 8);
    buf.emplace_back((length & 0x000000ff));

    // Header.Timestamp
    buf.emplace_back((timestamp & 0x00ff0000) >> 16);
    buf.emplace_back((timestamp & 0x0000ff00) >> 8);
    buf.emplace_back((timestamp & 0x000000ff));

    // Header.TimestampExtended
    buf.emplace_back((timestamp & 0xff000000) >> 24);

    // Header.StreamID (actually this is always 0 according to the
    // specification)
    buf.emplace_back((strem_id & 0x00ff0000) >> 16);
    buf.emplace_back((strem_id & 0x0000ff00) >> 8);
    buf.emplace_back((strem_id & 0x000000ff));

    // Data
    std::copy(data, data + length, std::back_inserter(buf));

    uint32_t tag_size = buf.size() - original_size;
    append_tag_size(buf, tag_size);

    tag_count_++;
  }

  /// <summary>
  /// Appends the tag size to the specified buffer.
  /// </summary>
  /// <param name="buf">The buffer.</param>
  /// <param name="length">The tag size.</param>
  void append_tag_size(std::vector<uint8_t> &buf, uint32_t length) {
    buf.reserve(buf.size() + 4);
    buf.emplace_back((length & 0xff000000) >> 24);
    buf.emplace_back((length & 0x00ff0000) >> 16);
    buf.emplace_back((length & 0x0000ff00) >> 8);
    buf.emplace_back((length & 0x000000ff));
  }
};
} // namespace flv
