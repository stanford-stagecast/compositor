/* -*-mode:c++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/* Copyright 2013-2019 the Alfalfa authors
                       and the Massachusetts Institute of Technology

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:

      1. Redistributions of source code must retain the above copyright
         notice, this list of conditions and the following disclaimer.

      2. Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in the
         documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

#include <array>
#include <iostream>

#include "jpeg.hh"

using namespace std;

JPEGDecompresser::JPEGDecompresser()
{
  jpeg_std_error( &error_manager_ );
  error_manager_.error_exit = JPEGDecompresser::error;
  decompresser_.err = &error_manager_;
  jpeg_create_decompress( &decompresser_ );
}

JPEGDecompresser::~JPEGDecompresser()
{
  jpeg_destroy_decompress( &decompresser_ );
}

void JPEGDecompresser::error( const j_common_ptr cinfo )
{
  array<char, JMSG_LENGTH_MAX> error_message;
  ( *cinfo->err->format_message )( cinfo, error_message.data() );

  throw runtime_error( error_message.data() );
}

void JPEGDecompresser::begin_decoding( const Chunk& chunk )
{
  /* older versions of libjpeg did not use a const pointer
     in jpeg_mem_src's buffer argument */
  jpeg_mem_src(
    &decompresser_, const_cast<uint8_t*>( chunk.buffer() ), chunk.size() );

  if ( JPEG_HEADER_OK != jpeg_read_header( &decompresser_, true ) ) {
    throw runtime_error( "invalid JPEG" );
  }

  if ( decompresser_.jpeg_color_space != JCS_YCbCr ) {
    throw runtime_error( "not Y'CbCr" );
  }

  if ( toRGB_ ) {
    decompresser_.out_color_space = JCS_RGB;
  } else {
    decompresser_.out_color_space = JCS_YCbCr;
  }

  if ( decompresser_.num_components != 3 ) {
    throw runtime_error( "not 3 components" );
  }

  if ( not( decompresser_.comp_info[0].h_samp_factor == 2
            and decompresser_.comp_info[0].v_samp_factor == 1
            and decompresser_.comp_info[1].h_samp_factor == 1
            and decompresser_.comp_info[1].v_samp_factor == 1
            and decompresser_.comp_info[2].h_samp_factor == 1
            and decompresser_.comp_info[2].v_samp_factor == 1 )
       and not( decompresser_.comp_info[0].h_samp_factor == 2
               and decompresser_.comp_info[0].v_samp_factor == 2
               and decompresser_.comp_info[1].h_samp_factor == 1
               and decompresser_.comp_info[1].v_samp_factor == 2
               and decompresser_.comp_info[2].h_samp_factor == 1
               and decompresser_.comp_info[2].v_samp_factor == 2 ) ) {
    throw runtime_error( "not 4:2:2" );
  }

  if ( YUV_.has_value() ) {
    if ( YUV_->height() != height() or YUV_->width() != 3 * width() ) {
      throw runtime_error( "JPEG size changed" );
    }
  } else {
    YUV_.emplace( 3 * width(), height() );
    for ( unsigned int i = 0; i < height(); i++ ) {
      YUV_rows.emplace_back( &YUV_->at( 0, i ) );
    }
  }
}

unsigned int JPEGDecompresser::width() const
{
  return decompresser_.image_width;
}

unsigned int JPEGDecompresser::height() const
{
  return decompresser_.image_height;
}

void JPEGDecompresser::decode( BaseRaster& r )
{
  if ( r.display_height() != height() or r.display_width() != width() ) {
    throw runtime_error( "size mismatch" );
  }

  jpeg_start_decompress( &decompresser_ );

  while ( decompresser_.output_scanline < decompresser_.output_height ) {
    jpeg_read_scanlines(
      &decompresser_, &YUV_rows.at( decompresser_.output_scanline ), height() );
  }

  jpeg_finish_decompress( &decompresser_ );

  for ( size_t row = 0; row < height(); row++ ) {
    for ( size_t column = 0; column < width(); column++ ) {
      r.Y().at( column, row ) = YUV_->at( column * 3, row );
      if ( row < height() / image_ratio_ && column < width() / image_ratio_ ) {
        r.U().at( column, row )
          = YUV_->at( column * image_ratio_ * 3 + 1, row * image_ratio_ );
        r.V().at( column, row )
          = YUV_->at( column * image_ratio_ * 3 + 2, row * image_ratio_ );
      }
    }
  }
}
