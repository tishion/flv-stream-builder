#include "../src/flv_stream_builder.hpp"
#include <fstream>


namespace test {
static void generate_flv_file() {
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

  std::vector<uint8_t> buf;
  flv::flv_stream_builder builder;
  builder.init_stream_header(buf, true, true)
      .append_meta_tag(buf, meta)
      .append_video_tag(buf, 0, 0, 0)
      .append_audio_tag(buf, 0, 0, 0);

  std::ofstream ofs;
  ofs.open("test_flv_data.flv",
           std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);
  ofs.write((char *)buf.data(), buf.size());
  ofs.close();
}
} // namespace test

int main() { test::generate_flv_file(); }
