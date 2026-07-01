#include "WavePropagationAdaptiveGrid2d.h"
#include <cmath>
#include <algorithm>

namespace tsunami_lab {
  namespace patches {

WavePropagationAdaptiveGrid2d::WavePropagationAdaptiveGrid2d(t_idx i_nCellsX, t_idx i_nCellsY) 
    : m_nCellsX(i_nCellsX), m_nCellsY(i_nCellsY) {

    // coarse grid (nx x ny), these values are safed to NetCDF
    m_coarseGrid = new WavePropagation2d(i_nCellsX, i_nCellsY);
}

WavePropagationAdaptiveGrid2d::~WavePropagationAdaptiveGrid2d() {
    delete m_coarseGrid;
    for (auto& [level, grid] : m_fineGrids) delete grid;
}

void WavePropagationAdaptiveGrid2d::setRefinementMap(const std::map<std::pair<t_idx, t_idx>, t_idx>& i_refinementMap) {
    m_refinementMap = i_refinementMap;

    // Berechne die Bounds für jedes Level > 1
    std::map<t_idx, std::array<t_idx, 4>> levelBounds;
    for (auto& [pos, level] : m_refinementMap) {
        if (level == 1) continue;  // Level 1 = kein feines Gitter
        auto [ix, iy] = pos;

        // Aktualisiere die Bounds für dieses Level
        if (levelBounds.find(level) == levelBounds.end()) {
            levelBounds[level] = {ix, iy, ix, iy};  // {min_x, min_y, max_x, max_y}
        } else {
            auto& bounds = levelBounds[level];
            bounds[0] = std::min(bounds[0], ix);  // min_x
            bounds[1] = std::min(bounds[1], iy);  // min_y
            bounds[2] = std::max(bounds[2], ix);  // max_x
            bounds[3] = std::max(bounds[3], iy);  // max_y
        }
    }
    // Erstelle die feinen Gitter für jedes Level > 1
    for (auto& [level, bounds] : levelBounds) {
        // Berechne die Größe des feinen Gitters:
        t_idx fineNx = (bounds[2] - bounds[0] + 1) * level;
        t_idx fineNy = (bounds[3] - bounds[1] + 1) * level;

        m_fineGrids[level] = new WavePropagation2d(fineNx, fineNy);
        m_refinedBounds[level] = bounds;
    }
}

t_idx WavePropagationAdaptiveGrid2d::getRefinement(t_idx i_x, t_idx i_y) const {
    auto it = m_refinementMap.find({i_x, i_y});
    return it != m_refinementMap.end() ? it->second : 1;
}

bool WavePropagationAdaptiveGrid2d::isRefined(t_idx i_ix, t_idx i_iy) const {
    return getRefinement(i_ix, i_iy) > 1;
}

void WavePropagationAdaptiveGrid2d::coarseToFineIndices(t_idx i_coarseX, t_idx i_coarseY, t_idx i_refinement, t_idx& o_fineX, t_idx& o_fineY) const {
    auto it = m_refinedBounds.find(i_refinement);
    if (it == m_refinedBounds.end()) {
        o_fineX = 0;
        o_fineY = 0;
        return;
    }
    auto& bounds = it->second;
    o_fineX = (i_coarseX - bounds[0]) * i_refinement;
    o_fineY = (i_coarseY - bounds[1]) * i_refinement;
}

void WavePropagationAdaptiveGrid2d::interpolateBoundaries(t_idx i_refinement) {
    t_real const* coarseH  = m_coarseGrid->getHeight();
    t_real const* coarseB  = m_coarseGrid->getBathymetry();
    t_real const* coarseHU = m_coarseGrid->getMomentumX();
    t_real const* coarseHV = m_coarseGrid->getMomentumY();
    t_idx coarseStride     = m_coarseGrid->getStride();

    WavePropagation2d* fineGrid = m_fineGrids[i_refinement];
    //t_idx fineStride            = fineGrid->getStride();
    auto& bounds                = m_refinedBounds[i_refinement];

    // Instead of looping over the whole map, we only loop over the COARSE CELLS 
    // immediately surrounding the refined bounding box (the halo ring)
    for (t_idx cx = bounds[0] - 1; cx <= bounds[2] + 1; cx++) {
        for (t_idx cy = bounds[1] - 1; cy <= bounds[3] + 1; cy++) {
            
            // Skip if this coarse cell is actually INSIDE the fine grid interior
            if (cx >= bounds[0] && cx <= bounds[2] && cy >= bounds[1] && cy <= bounds[3]) {
                continue; 
            }

            t_idx coarseIdx  = cy * coarseStride + cx;
            t_real h_coarse  = coarseH[coarseIdx];
            t_real b_coarse  = coarseB[coarseIdx];
            t_real hu_coarse = coarseHU[coarseIdx];
            t_real hv_coarse = coarseHV[coarseIdx];
            t_real eta_coarse = h_coarse + b_coarse;

            // Map these surrounding coarse cells directly into the fine grid's GHOST layers
            t_idx fineX0 = (cx - bounds[0]) * i_refinement;
            t_idx fineY0 = (cy - bounds[1]) * i_refinement;

            for (t_idx fi = 0; fi < i_refinement; fi++) {
                for (t_idx fj = 0; fj < i_refinement; fj++) {
                    // This naturally targets indices like -1, -2 or fineNx, fineNx+1
                    t_idx fX = fineX0 + fi;
                    t_idx fY = fineY0 + fj;
                    
                    // Direct pointer write to fine grid ghost cells, bypassing standard setters
                    // assuming your WavePropagation2d exposes a way to write to ghost cells,
                    // or your setters accept negative/overflow boundary indices.
                    fineGrid->setBathymetry(fX, fY, b_coarse);
                    fineGrid->setHeight(fX, fY, std::max<t_real>(eta_coarse - b_coarse, 0.0));

                    t_real newHFine = std::max<t_real>(eta_coarse - b_coarse, 0.0);
                    if (newHFine <= 1e-7) {
                        fineGrid->setMomentumX(fX, fY, 0.0);
                        fineGrid->setMomentumY(fX, fY, 0.0);
                    } else {
                        fineGrid->setMomentumX(fX, fY, hu_coarse);
                        fineGrid->setMomentumY(fX, fY, hv_coarse);
                    }
                }
            }
        }
    }
}

void WavePropagationAdaptiveGrid2d::restrictBoundary(t_idx i_refinement) {
    // Feine Datenpointer laden
    t_real const* fineH  = m_fineGrids[i_refinement]->getHeight();
    t_real const* fineB  = m_fineGrids[i_refinement]->getBathymetry(); // NEU: Feine Bathymetrie
    t_real const* fineHU = m_fineGrids[i_refinement]->getMomentumX();
    t_real const* fineHV = m_fineGrids[i_refinement]->getMomentumY();
    t_idx fineStride     = m_fineGrids[i_refinement]->getStride();

    // Grobe Datenpointer für die korrekte Verrechnung der lokalen Bathymetrie laden
    t_real const* coarseB = m_coarseGrid->getBathymetry();             // NEU: Grobe Bathymetrie
    t_idx coarseStride    = m_coarseGrid->getStride();                 // NEU: Grober Stride für Indexierung

    for (auto& [coarsePos, level] : m_refinementMap) {
        if (level != i_refinement) continue;
        auto [coarseX, coarseY] = coarsePos;
        
        t_idx fineX0, fineY0;
        coarseToFineIndices(coarseX, coarseY, i_refinement, fineX0, fineY0);
        
        t_real avgEta = 0; // WICHTIG: Wir summieren hier eta = h + b auf, nicht h!
        t_real avgHU  = 0;
        t_real avgHV  = 0;
        
        // Schleife über alle feinen Zellen, die zu dieser einen groben Zelle gehören
        for (t_idx fi = 0; fi < i_refinement; fi++) {
            for (t_idx fj = 0; fj < i_refinement; fj++) {
                t_idx fineIdx = (fineY0 + fj) * fineStride + (fineX0 + fi);
                
                avgEta += (fineH[fineIdx] + fineB[fineIdx]); // η_fine = h_fine + b_fine
                avgHU  += fineHU[fineIdx];
                avgHV  += fineHV[fineIdx];
            }
        }
        
        t_real factor = 1.0 / (static_cast<t_real>(i_refinement) * i_refinement);
        
        // 1. Berechne die gemittelte absolute Wasseroberfläche (eta)
        t_real newEtaCoarse = avgEta * factor;
        
        // 2. Hole die exakte Bathymetrie der aktuellen Grobgitterzelle
        t_idx coarseIdx = coarseY * coarseStride + coarseX;
        t_real b_coarse = coarseB[coarseIdx];
        
        // 3. Berechne die neue grobe Wasserhöhe: h_coarse = eta_coarse - b_coarse
        // std::max schützt davor, dass h durch Rundungsfehler oder an steilen Küsten negativ wird
        t_real newHCoarse = std::max<t_real>(newEtaCoarse - b_coarse, 0.0);
        
        // Werte zurück auf das grobe Gitter schreiben
        m_coarseGrid->setHeight(coarseX, coarseY, newHCoarse);

        if (newHCoarse <= 1e-7) {
            m_coarseGrid->setMomentumX(coarseX, coarseY, 0.0);
            m_coarseGrid->setMomentumY(coarseX, coarseY, 0.0);
        } else {
            m_coarseGrid->setMomentumX(coarseX, coarseY, avgHU * factor);
            m_coarseGrid->setMomentumY(coarseX, coarseY, avgHV * factor);
        }
    }
}

void WavePropagationAdaptiveGrid2d::timeStep(t_real i_scaling) {
    // 1. Advance coarse grid
    m_coarseGrid->setGhostCells(m_leftReflecting, m_rightReflecting, m_bottomReflecting, m_topReflecting);
    m_coarseGrid->timeStep(i_scaling);

    // 2. Advance fine grids
    for (auto& [refinement, fineGrid] : m_fineGrids) {
        t_real fine_dt = i_scaling / static_cast<t_real>(refinement);

        for (t_idx subcycle = 0; subcycle < refinement; subcycle++) {
            // Re-interpolate coarse data into fine ghost cells EVERY subcycle step
            interpolateBoundaries(refinement);
            
            // CRITICAL: Only apply global boundary conditions if the fine grid actually touches the domain edge!
            auto& bounds = m_refinedBounds[refinement];
            bool left   = (bounds[0] == 0) ? m_leftReflecting   : false;
            bool right  = (bounds[2] == m_nCellsX - 1) ? m_rightReflecting  : false;
            bool bottom = (bounds[1] == 0) ? m_bottomReflecting : false;
            bool top    = (bounds[3] == m_nCellsY - 1) ? m_topReflecting    : false;
            
            fineGrid->setGhostCells(left, right, bottom, top);
            fineGrid->timeStep(fine_dt);
        }
        
        // 3. Average fine data back up to coarse grid
        restrictBoundary(refinement);
    }
}

void WavePropagationAdaptiveGrid2d::setGhostCells(bool i_leftReflecting, bool i_rightReflecting, bool i_bottomReflecting, bool i_topReflecting) {
    m_leftReflecting = i_leftReflecting;
    m_rightReflecting = i_rightReflecting;
    m_bottomReflecting = i_bottomReflecting;
    m_topReflecting = i_topReflecting;
}

t_idx WavePropagationAdaptiveGrid2d::getStride() { return m_coarseGrid->getStride(); }
t_real const* WavePropagationAdaptiveGrid2d::getHeight() { return m_coarseGrid->getHeight(); }
t_real const* WavePropagationAdaptiveGrid2d::getMomentumX() { return m_coarseGrid->getMomentumX(); }
t_real const* WavePropagationAdaptiveGrid2d::getMomentumY() { return m_coarseGrid->getMomentumY(); }
t_real const* WavePropagationAdaptiveGrid2d::getBathymetry() { return m_coarseGrid->getBathymetry(); }

void tsunami_lab::patches::WavePropagationAdaptiveGrid2d::setHeight(t_idx i_ix, t_idx i_iy, t_real i_h, setups::Setup* i_setup) {
    m_coarseGrid->setHeight(i_ix, i_iy, i_h);

    if (isRefined(i_ix, i_iy)) {
        t_idx refinement = getRefinement(i_ix, i_iy);
        WavePropagation2d* fineGrid = m_fineGrids[refinement];
        t_idx fineX0, fineY0;
        coarseToFineIndices(i_ix, i_iy, refinement, fineX0, fineY0);
        
        t_real fineCellSize = (1.0 / refinement);

        for (t_idx fi = 0; fi < refinement; fi++) {
            for (t_idx fj = 0; fj < refinement; fj++) {
                /*
                * -0.5 -> cancle out center shift from Setup and replace offset by fine offset = fineCellSize/2
                * fineCellSize * fi (or fj) is between 0 and 1 and represent the position of a fine cell inside its coarse cell
                */
                t_real x = i_ix - 0.5 + fineCellSize * fi + fineCellSize / 2;
                t_real y = i_iy - 0.5 + fineCellSize * fj + fineCellSize / 2;
                t_real height = i_setup->getHeight(x, y);
                if (std::isnan(height)) {
                    std::cerr << "CRITICAL: Height NaN detected at cell (" << i_ix << "|" << i_iy << ")" 
                            << " for coordinates x=" << x << ", y=" << y << std::endl;
                    exit(1); 
                }
                fineGrid->setHeight(fineX0 + fi, fineY0 + fj, height);
            }
        }
    }
}

void WavePropagationAdaptiveGrid2d::setMomentumX(t_idx i_ix, t_idx i_iy, t_real i_hu) {
    m_coarseGrid->setMomentumX(i_ix, i_iy, i_hu);

    if (isRefined(i_ix, i_iy)) {
        t_idx refinement = getRefinement(i_ix, i_iy);
        WavePropagation2d* fineGrid = m_fineGrids[refinement];
        t_idx fineX0, fineY0;
        coarseToFineIndices(i_ix, i_iy, refinement, fineX0, fineY0);
        for (t_idx fi = 0; fi < refinement; fi++)
            for (t_idx fj = 0; fj < refinement; fj++)
                fineGrid->setMomentumX(fineX0 + fi, fineY0 + fj, i_hu);
    } else m_coarseGrid->setMomentumX(i_ix, i_iy, i_hu);
}

void WavePropagationAdaptiveGrid2d::setMomentumY(t_idx i_ix, t_idx i_iy, t_real i_hv) {
    m_coarseGrid->setMomentumY(i_ix, i_iy, i_hv);

    if (isRefined(i_ix, i_iy)) {
        t_idx refinement = getRefinement(i_ix, i_iy);
        WavePropagation2d* fineGrid = m_fineGrids[refinement];
        t_idx fineX0, fineY0;
        coarseToFineIndices(i_ix, i_iy, refinement, fineX0, fineY0);
        for (t_idx fi = 0; fi < refinement; fi++)
            for (t_idx fj = 0; fj < refinement; fj++)
                fineGrid->setMomentumY(fineX0 + fi, fineY0 + fj, i_hv);
    } else m_coarseGrid->setMomentumY(i_ix, i_iy, i_hv);
}

void WavePropagationAdaptiveGrid2d::setBathymetry(t_idx i_ix, t_idx i_iy, t_real i_b, setups::Setup* i_setup) {
    m_coarseGrid->setBathymetry(i_ix, i_iy, i_b);

    if (isRefined(i_ix, i_iy)) {
        t_idx refinement = getRefinement(i_ix, i_iy);
        WavePropagation2d* fineGrid = m_fineGrids[refinement];
        t_idx fineX0, fineY0;
        coarseToFineIndices(i_ix, i_iy, refinement, fineX0, fineY0);

        t_real fineCellSize = (1.0 / refinement);

        for (t_idx fi = 0; fi < refinement; fi++)
            for (t_idx fj = 0; fj < refinement; fj++) {
                /*
                * -0.5 -> cancle out center shift from Setup and replace offset by fine offset = fineCellSize/2
                * fineCellSize * fi (or fj) is between 0 and 1 and represent the position of a fine cell inside its coarse cell
                */
                t_real x = i_ix - 0.5 + fineCellSize * fi + fineCellSize / 2;
                t_real y = i_iy - 0.5 + fineCellSize * fj + fineCellSize / 2;
                t_real bathy = i_setup->getBathymetry(x, y);
                if (std::isnan(bathy)) {
                    std::cerr << "CRITICAL: Bathymetry NaN detected at cell (" << i_ix << "|" << i_iy << ")" 
                            << " for coordinates x=" << x << ", y=" << y << std::endl;
                    exit(1); 
                }
                fineGrid->setBathymetry(fineX0 + fi, fineY0 + fj, bathy);
            }
    } else m_coarseGrid->setBathymetry(i_ix, i_iy, i_b);
}

void tsunami_lab::patches::WavePropagationAdaptiveGrid2d::exportUniformGrid(
        t_idx i_maxResolution,
    std::vector<t_real>& o_b,
    std::vector<t_real>& o_h,
    std::vector<t_real>& o_hu,
    std::vector<t_real>& o_hv
) const {

    t_idx nxOut = m_nCellsX * i_maxResolution;
    t_idx nyOut = m_nCellsY * i_maxResolution;

    o_b .assign(nxOut * nyOut, 0);
    o_h .assign(nxOut * nyOut, 0);
    o_hu.assign(nxOut * nyOut, 0);
    o_hv.assign(nxOut * nyOut, 0);

    //------------------------------------------------------------
    // 1. komplettes coarse grid aufblasen
    //------------------------------------------------------------

    auto const* b  = m_coarseGrid->getBathymetry();
    auto const* h  = m_coarseGrid->getHeight();
    auto const* hu = m_coarseGrid->getMomentumX();
    auto const* hv = m_coarseGrid->getMomentumY();

    t_idx coarseStride = m_coarseGrid->getStride();

    for (t_idx cy = 0; cy < m_nCellsY; ++cy) {
        for (t_idx cx = 0; cx < m_nCellsX; ++cx) {

            t_idx idx = cy * coarseStride + cx;

            for (t_idx fy = 0; fy < i_maxResolution; ++fy) {
                for (t_idx fx = 0; fx < i_maxResolution; ++fx) {

                    t_idx ox = cx * i_maxResolution + fx;
                    t_idx oy = cy * i_maxResolution + fy;

                    t_idx out = oy * nxOut + ox;

                    o_b[out]  = b[idx];
                    o_h[out]  = h[idx];
                    o_hu[out] = hu[idx];
                    o_hv[out] = hv[idx];
                }
            }
        }
    }

    //------------------------------------------------------------
    // 2. echte feine Daten überschreiben
    //------------------------------------------------------------

    for (auto const& [level, fineGrid] : m_fineGrids) {

        auto bounds = m_refinedBounds.at(level);

        auto const* fb  = fineGrid->getBathymetry();
        auto const* fh  = fineGrid->getHeight();
        auto const* fhu = fineGrid->getMomentumX();
        auto const* fhv = fineGrid->getMomentumY();

        t_idx fineStride = fineGrid->getStride();

        t_idx block = i_maxResolution / level;

        for (t_idx cy = bounds[1]; cy <= bounds[3]; ++cy) {
            for (t_idx cx = bounds[0]; cx <= bounds[2]; ++cx) {
                if (getRefinement(cx, cy) != level)
                    continue;

                t_idx fineX0, fineY0;
                coarseToFineIndices(cx, cy, level, fineX0, fineY0);

                for (t_idx fy = 0; fy < level; ++fy) {
                    for (t_idx fx = 0; fx < level; ++fx) {

                        t_idx fineIdx =
                            (fineY0 + fy) * fineStride +
                            (fineX0 + fx);

                        t_idx ox =
                            cx * i_maxResolution +
                            fx * block;

                        t_idx oy =
                            cy * i_maxResolution +
                            fy * block;

                        for (t_idx by = 0; by < block; ++by) {
                            for (t_idx bx = 0; bx < block; ++bx) {

                                t_idx out =
                                    (oy + by) * nxOut +
                                    (ox + bx);

                                o_b[out]  = fb[fineIdx];
                                o_h[out]  = fh[fineIdx];
                                o_hu[out] = fhu[fineIdx];
                                o_hv[out] = fhv[fineIdx];
                            }
                        }
                    }
                }
            }
        }
    }
}

}
}