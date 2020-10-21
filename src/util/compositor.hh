/* -*-mode:c++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

#ifndef COMPOSITOR_HH
#define COMPOSITOR_HH

#include "input/jpeg.hh"
#include "util/raster.hh"

class Compositor
{
private:
  JPEGDecompresser jpegdec_ {};
  RGBRaster background_;

public:
  Compositor( const std::string& image_name );
  void composite( RGBRaster& raster );
};

#endif /* COMPOSITOR_HH */
