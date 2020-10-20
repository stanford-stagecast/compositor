#include "mjpeg_input.hh"

using namespace std;

size_t find_jpeg_eof( const Chunk& chunk )
{
  const uint16_t* buffer = reinterpret_cast<const uint16_t*>( chunk.buffer() );
  const size_t size = chunk.size() / 2;

  for ( size_t i = 0; i < size; i++ ) {
    if ( buffer[i] == 0xd9ff ) {
      return i * 2;
    }
  }

  return size * 2;
}

optional<RGBRasterHandle> MJPEGInput::get_next_rgb_frame()
{
  auto chunk = input_file_.chunk();
  if ( current_offset >= chunk.size() ) {
    return {};
  }

  const size_t frame_length = find_jpeg_eof( chunk( current_offset ) ) + 2;

  RGBRasterHandle raster_handle { width_, height_ };
  auto& raster = raster_handle.get();

  if ( jpegdec_.has_value() ) {
    jpegdec_->set_output_rgb();
    jpegdec_->begin_decoding( input_file_( current_offset, frame_length ) );
    if ( jpegdec_->width() != width_ or jpegdec_->height() != height_ ) {
      throw runtime_error( "size mismatch" );
    }
    jpegdec_->decode( raster );
  } else {
    jpegdec_.emplace();
    /* ignore first frame as can contain invalid JPEG data */
  }

  current_offset += frame_length;

  return RGBRasterHandle { move( raster_handle ) };
}
