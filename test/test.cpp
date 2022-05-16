#include <fstream>

#include <flv_stream_builder.hpp>

namespace test {
class AVFrame {
public:
  bool isVideo() const { return true; }

  bool isAudio() const { return true; }

  uint32_t timestamp() const { return 0; }

  const uint8_t *data() const { return nullptr; }

  const uint32_t length() const { return 0; }
};
typedef std::vector<AVFrame> AVFrameSource;

static void generate_flv_file(const AVFrameSource &source) {
  // Create the file stream and write the FLV data to the file
  std::ofstream ofs;
  ofs.open("test_flv_data.flv",
           std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);

  // The FLV builder
  flv::flv_stream_builder builder(ofs);

  // Create the meta data
  auto meta = flv::amf::amf_array::create()
                  ->with_item("duration", (double)0)
                  ->with_item("width", (double)1920)
                  ->with_item("height", (double)1080)
                  ->with_item("videodatarate", (double)520)
                  ->with_item("framerate", (double)25)
                  ->with_item("videocodecid", (double)7)
                  ->with_item("audiosamplerate", (double)44100)
                  ->with_item("audiosamplesize", (double)16)
                  ->with_item("stereo", true)
                  ->with_item("audiocodecid", (double)10)
                  ->with_item("filesize", (double)0);

  builder
      .init_stream_header(true, true) // Initialize the FLV stream header
      .append_meta_tag(meta);         // Append the meta tag

  for (auto &frame : source) {
    if (frame.isVideo()) {
      // Append a video tag
      builder.append_video_tag(frame.timestamp(), frame.data(), frame.length());
    } else if (frame.isAudio()) {
      // Append a audio tag
      builder.append_audio_tag(frame.timestamp(), frame.data(), frame.length());
    } else {
    }
  }

  ofs.close();
}
} // namespace test

int main() {

  test::AVFrameSource source;

  // generate flv file stream
  test::generate_flv_file(source);
}
