/**
 * @author Alexander Breuer (alex.breuer AT uni-jena.de)
 *
 * @section DESCRIPTION
 * IO-routines for writing a snapshot as Comma Separated Values (CSV).
 **/
#ifndef TSUNAMI_LAB_IO_CSV
#define TSUNAMI_LAB_IO_CSV

#include "../constants.h"
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

namespace tsunami_lab {
  namespace io {
    class Csv;
  }
}

class tsunami_lab::io::Csv {
  public:
    /**
     * Writes the data as CSV to the given stream.
     *
     * @param i_dxy cell width in x- and y-direction.
     * @param i_nx number of cells in x-direction.
     * @param i_ny number of cells in y-direction.
     * @param i_stride stride of the data arrays in y-direction (x is assumed to be stride-1).
     * @param i_h water height of the cells; optional: use nullptr if not required.
     * @param i_hu momentum in x-direction of the cells; optional: use nullptr if not required.
     * @param i_hv momentum in y-direction of the cells; optional: use nullptr if not required.
     * @param io_stream stream to which the CSV-data is written.
     **/
    static void write( t_real               i_dxy,
                       t_idx                i_nx,
                       t_idx                i_ny,
                       t_idx                i_stride,
                       t_real       const * i_b,
                       t_real       const * i_h,
                       t_real       const * i_hu,
                       t_real       const * i_hv,
                       std::ostream       & io_stream );

    /**
     * Reads a column from a CSV file.
     *
     * @param i_filename name of the CSV file.
     * @param i_columnIndex index of the column to read.
     * @return vector containing the values from the specified column.
     **/
    static std::vector<tsunami_lab::t_real> read( const std::string & i_filename,
                                     t_idx               i_columnIndex );

    static void writeStationData(t_real i_waterHeight,
      t_real i_momentumX,
      t_real i_momentumY,
      std::ostream       & io_stream
    );
};

#endif