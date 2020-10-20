#include "mjpeg_input.hh"

using namespace std;

optional<RGBRasterHandle> MJPEGInput::get_next_rgb_frame()
{
  RGBRasterHandle raster_handle { width_, height_ };
  auto& raster = raster_handle.get();

  if ( jpegdec_.has_value() ) {
    jpegdec_->set_output_rgb();
    jpegdec_->begin_decoding( input_file_.chunk() );
    if ( jpegdec_->width() != width_ or jpegdec_->height() != height_ ) {
      throw runtime_error( "size mismatch" );
    }
    jpegdec_->decode( raster );
  } else {
    jpegdec_.emplace();
    /* ignore first frame as can contain invalid JPEG data */
  }

  return RGBRasterHandle{ move( raster_handle ) };
}
