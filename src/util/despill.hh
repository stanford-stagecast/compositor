/* -*-mode:c++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

#ifndef DESPILL_HH
#define DESPILL_HH

#include <vector>

#include "util/keying.hh"
#include "util/raster.hh"

class DespillOperation
{
private:
  double despill_factor_ { 0.5 };
  double color_balance_ { 0.5 };
  const KeyingOperation& keying_operation_;
  void process_pixel( RGBRaster& raster, int x, int y );

public:
  DespillOperation( const KeyingOperation& keying_operation );
  void set_despill_factor( const double despill_factor )
  {
    despill_factor_ = despill_factor;
  }
  void set_despill_balance( const double color_balance )
  {
    color_balance_ = color_balance;
  }
  void process_rows( RGBRaster& raster,
                     const uint16_t row_start_idx,
                     const uint16_t row_end_idx );
};

#endif /* DESPILL_HH */
