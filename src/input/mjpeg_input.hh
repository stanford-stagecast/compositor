#include "frame_input.hh"

#include <optional>
#include <string>

#include "jpeg.hh"
#include "util/file.hh"

class MJPEGInput : public FrameInput
{
private:
  File input_file_;
  const uint16_t width_;
  const uint16_t height_;

  std::optional<JPEGDecompresser> jpegdec_ {};

public:
  MJPEGInput( const std::string& filename,
              const uint16_t width,
              const uint16_t height )
    : input_file_( filename )
    , width_( width )
    , height_( height )
  {}

  std::optional<RGBRasterHandle> get_next_rgb_frame() override;
  uint16_t display_width() { return width_; }
  uint16_t display_height() { return height_; }
  ~MJPEGInput() {}
};
