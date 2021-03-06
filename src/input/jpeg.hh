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

#ifndef JPEG_HH
#define JPEG_HH

#include <cstdio>
#include <cstdlib>
#include <jpeglib.h>
#include <optional>

#include "util/2d.hh"
#include "util/chunk.hh"
#include "util/raster.hh"

class JPEGDecompresser
{
  jpeg_decompress_struct decompresser_ {};
  jpeg_error_mgr error_manager_ {};

  static void error( const j_common_ptr cinfo );

  std::optional<TwoD<uint8_t>> YUV_ {};
  std::vector<uint8_t*> YUV_rows {};
  bool toRGB_ { false };
  uint8_t image_ratio_ { 2 };

public:
  JPEGDecompresser();
  ~JPEGDecompresser();

  void begin_decoding( const Chunk& chunk );

  void decode( BaseRaster& r );

  RGBRaster load_image( const std::string& image_name );

  void set_output_rgb()
  {
    toRGB_ = true;
    image_ratio_ = 2 - toRGB_;
  }

  unsigned int width() const;
  unsigned int height() const;
};

#endif /* JPEG_HH */
