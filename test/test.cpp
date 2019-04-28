#include "../src/flv_stream_builder.hpp"
#include <fstream>

namespace test {
static void generate_flv_file() {
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

  // Initialize the FLV stream header
  builder
      .init_stream_header(true, true)

      // Append the meta tag
      .append_meta_tag(meta)

      // Append a video tag
      .append_video_tag(0, 0, 0)

      // Append a audio tag
      .append_audio_tag(0, 0, 0);

  ofs.close();
}
} // namespace test

int main() { test::generate_flv_file(); }
