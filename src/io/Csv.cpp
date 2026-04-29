/**
 * @author Alexander Breuer (alex.breuer AT uni-jena.de)
 *
 * @section DESCRIPTION
 * IO-routines for writing a snapshot as Comma Separated Values (CSV).
 **/
#include "Csv.h"

void tsunami_lab::io::Csv::write( t_real               i_dxy,
                                  t_idx                i_nx,
                                  t_idx                i_ny,
                                  t_idx                i_stride,
                                  t_real       const * i_b,
                                  t_real       const * i_h,
                                  t_real       const * i_hu,
                                  t_real       const * i_hv,
                                  std::ostream       & io_stream ) {
  // write the CSV header
  io_stream << "x,y,bathymetry";
  if( i_h  != nullptr ) io_stream << ",height";
  if( i_hu != nullptr ) io_stream << ",momentum_x";
  if( i_hv != nullptr ) io_stream << ",momentum_y";
  io_stream << "\n";

  // iterate over all cells
  for( t_idx l_iy = 0; l_iy < i_ny; l_iy++ ) {
    for( t_idx l_ix = 0; l_ix < i_nx; l_ix++ ) {
      // derive coordinates of cell center
      t_real l_posX = (l_ix + 0.5) * i_dxy;
      t_real l_posY = (l_iy + 0.5) * i_dxy;

      t_idx l_id = l_iy * i_stride + l_ix;

      // write data
      io_stream << l_posX << "," << l_posY;
      if( i_b  != nullptr ) io_stream << "," << i_b[l_id];
      else io_stream << ",0";
      if( i_h  != nullptr ) io_stream << "," << i_h[l_id];
      if( i_hu != nullptr ) io_stream << "," << i_hu[l_id];
      if( i_hv != nullptr ) io_stream << "," << i_hv[l_id];
      io_stream << "\n";
    }
  }
  io_stream << std::flush;
}

std::vector<tsunami_lab::t_real> tsunami_lab::io::Csv::read( const std::string & i_filename,
                                                                 t_idx               i_columnIndex ) {
  std::vector<tsunami_lab::t_real> columnData;

  // Check if csv file can be opend 
  std::ifstream file(i_filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << i_filename << std::endl;
        return columnData;
    }

    std::string line;

    // go through every line of the file
    while (std::getline(file, line)) {
      std::stringstream stringStream(line);
      std::string cell;
      t_idx currentColumnIndex = 0;

      // go through every cell of the line, HAVE TO BE SEPERATED BY "," !
      while (std::getline(stringStream, cell, ',')) {
        if (i_columnIndex == currentColumnIndex) {
          columnData.push_back(std::stod(cell)); // convert string to double and add to vector
          break; 
        }
        currentColumnIndex++;
      }
    }
    return columnData;
}

tsunami_lab::t_real* vectorToArray(const std::vector<tsunami_lab::t_real>& vec) {
    tsunami_lab::t_real* arr = new tsunami_lab::t_real[vec.size()];

    for (std::size_t i = 0; i < vec.size(); i++) {
        arr[i] = vec[i];
    }

    return arr;
}