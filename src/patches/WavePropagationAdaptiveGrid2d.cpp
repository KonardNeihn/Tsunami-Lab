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
    std::map<t_idx, std::array<t_idx, 4>> levelBounds;
    for (auto& [pos, level] : m_refinementMap) {
        if (level == 1) continue;
        auto [ix, iy] = pos;
        if (levelBounds.find(level) == levelBounds.end()) levelBounds[level] = {ix, iy, ix, iy};
        else {
            auto& bounds = levelBounds[level];
            bounds[0] = std::min(bounds[0], ix);
            bounds[1] = std::min(bounds[1], iy);
            bounds[2] = std::max(bounds[2], ix);
            bounds[3] = std::max(bounds[3], iy);
        }
    }
    for (auto& [level, bounds] : levelBounds) {
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
    t_real const* coarseH = m_coarseGrid->getHeight();
    t_real const* coarseHU = m_coarseGrid->getMomentumX();
    t_real const* coarseHV = m_coarseGrid->getMomentumY();
    WavePropagation2d* fineGrid = m_fineGrids[i_refinement];
    t_idx coarseStride = m_coarseGrid->getStride();

    for (auto& [coarsePos, level] : m_refinementMap) {
        if (level != i_refinement) continue;
        auto [coarseX, coarseY] = coarsePos;
        t_real h_coarse = coarseH[coarseY * coarseStride + coarseX];
        t_real hu_coarse = coarseHU[coarseY * coarseStride + coarseX];
        t_real hv_coarse = coarseHV[coarseY * coarseStride + coarseX];
        t_idx fineX0, fineY0;
        coarseToFineIndices(coarseX, coarseY, i_refinement, fineX0, fineY0);

        // set values for fine grid
        for (t_idx fi = 0; fi < i_refinement; fi++) {
            for (t_idx fj = 0; fj < i_refinement; fj++) {
                fineGrid->setHeight(fineX0 + fi, fineY0 + fj, h_coarse);
                fineGrid->setMomentumX(fineX0 + fi, fineY0 + fj, hu_coarse);
                fineGrid->setMomentumY(fineX0 + fi, fineY0 + fj, hv_coarse);
            }
        }
    }
}

void WavePropagationAdaptiveGrid2d::restrictBoundary(t_idx i_refinement) {
    t_real const* fineH = m_fineGrids[i_refinement]->getHeight();
    t_real const* fineHU = m_fineGrids[i_refinement]->getMomentumX();
    t_real const* fineHV = m_fineGrids[i_refinement]->getMomentumY();
    t_idx fineStride = m_fineGrids[i_refinement]->getStride();
    for (auto& [coarsePos, level] : m_refinementMap) {
        if (level != i_refinement) continue;
        auto [coarseX, coarseY] = coarsePos;
        t_idx fineX0, fineY0;
        coarseToFineIndices(coarseX, coarseY, i_refinement, fineX0, fineY0);
        t_real avgH = 0, avgHU = 0, avgHV = 0;
        for (t_idx fi = 0; fi < i_refinement; fi++) {
            for (t_idx fj = 0; fj < i_refinement; fj++) {
                t_idx fineIdx = (fineY0 + fj) * fineStride + (fineX0 + fi);
                avgH += fineH[fineIdx];
                avgHU += fineHU[fineIdx];
                avgHV += fineHV[fineIdx];
            }
        }
        t_real factor = 1.0 / (static_cast<t_real>(i_refinement) * i_refinement);
        m_coarseGrid->setHeight(coarseX, coarseY, avgH * factor);
        m_coarseGrid->setMomentumX(coarseX, coarseY, avgHU * factor);
        m_coarseGrid->setMomentumY(coarseX, coarseY, avgHV * factor);
    }
}

void WavePropagationAdaptiveGrid2d::timeStep(t_real i_scaling) {
    m_coarseGrid->setGhostCells(m_leftReflecting, m_rightReflecting, m_bottomReflecting, m_topReflecting);
    m_coarseGrid->timeStep(i_scaling);
    for (auto& [refinement, fineGrid] : m_fineGrids) {
        interpolateBoundaries(refinement);
        for (t_idx subcycle = 0; subcycle < refinement; subcycle++) {
            fineGrid->setGhostCells(m_leftReflecting, m_rightReflecting, m_bottomReflecting, m_topReflecting);
            fineGrid->timeStep(i_scaling);
        }
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

void tsunami_lab::patches::WavePropagationAdaptiveGrid2d::setHeight(t_idx i_ix, t_idx i_iy, t_real i_h) {
    // 1. Immer das Coarse-Grid aktualisieren (als Base-Layer)
    m_coarseGrid->setHeight(i_ix, i_iy, i_h);

    // 2. Falls refined, zusätzlich das Fine-Grid aktualisieren
    if (isRefined(i_ix, i_iy)) {
        t_idx refinement = getRefinement(i_ix, i_iy);
        WavePropagation2d* fineGrid = m_fineGrids[refinement];
        t_idx fineX0, fineY0;
        coarseToFineIndices(i_ix, i_iy, refinement, fineX0, fineY0);
        
        for (t_idx fi = 0; fi < refinement; fi++) {
            for (t_idx fj = 0; fj < refinement; fj++) {
                fineGrid->setHeight(fineX0 + fi, fineY0 + fj, i_h);
            }
        }
    }
}

void WavePropagationAdaptiveGrid2d::setMomentumX(t_idx i_ix, t_idx i_iy, t_real i_hu) {
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

void WavePropagationAdaptiveGrid2d::setBathymetry(t_idx i_ix, t_idx i_iy, t_real i_b) {
    m_coarseGrid->setBathymetry(i_ix, i_iy, i_b);

    if (isRefined(i_ix, i_iy)) {
        t_idx refinement = getRefinement(i_ix, i_iy);
        WavePropagation2d* fineGrid = m_fineGrids[refinement];
        t_idx fineX0, fineY0;
        coarseToFineIndices(i_ix, i_iy, refinement, fineX0, fineY0);
        for (t_idx fi = 0; fi < refinement; fi++)
            for (t_idx fj = 0; fj < refinement; fj++)
                fineGrid->setBathymetry(fineX0 + fi, fineY0 + fj, i_b);
    } else m_coarseGrid->setBathymetry(i_ix, i_iy, i_b);
}

  }
}