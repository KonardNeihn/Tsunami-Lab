#include "WavePropagationAdaptiveGrid2d.h"
#include <cmath>
#include <algorithm>
#include <cstdint>
#include <iostream>

namespace tsunami_lab {
  namespace patches {

using t_sidx = std::int64_t;

// Trockenschwelle 
static constexpr t_real DRY_TOL = 0.01;

WavePropagationAdaptiveGrid2d::WavePropagationAdaptiveGrid2d(t_idx i_nCellsX, t_idx i_nCellsY)
    : m_nCellsX(i_nCellsX), m_nCellsY(i_nCellsY) {
    m_coarseGrid = new WavePropagation2d(i_nCellsX, i_nCellsY);
}

WavePropagationAdaptiveGrid2d::~WavePropagationAdaptiveGrid2d() {
    delete m_coarseGrid;
    for (auto& [level, grid] : m_fineGrids) delete grid;
}

void WavePropagationAdaptiveGrid2d::setRefinementMap(const std::map<std::pair<t_idx, t_idx>, t_idx>& i_refinementMap) {
    m_refinementMap = i_refinementMap;

    static constexpr double CLIP_X = 0.02;
    static constexpr double CLIP_Y = 0.0;

    std::map<t_idx, std::array<t_idx, 4>> boxes;   // level -> {minX, minY, maxX, maxY}
    {
        std::map<t_idx, std::vector<t_idx>> xs, ys;
        for (auto const& [pos, level] : m_refinementMap) {
            if (level == 1) continue;
            xs[level].push_back(pos.first);
            ys[level].push_back(pos.second);
        }
        for (auto& [level, vx] : xs) {
            auto& vy = ys[level];
            std::sort(vx.begin(), vx.end());
            std::sort(vy.begin(), vy.end());

            std::size_t n  = vx.size();
            std::size_t lx = static_cast<std::size_t>(CLIP_X * n);
            std::size_t ly = static_cast<std::size_t>(CLIP_Y * n);

            boxes[level] = {vx[lx], vy[ly], vx[n - 1 - lx], vy[n - 1 - ly]};
        }
    }

    m_refinementMap.clear();
    for (auto const& [level, b] : boxes) {
        for (t_idx y = b[1]; y <= b[3]; ++y) {
            for (t_idx x = b[0]; x <= b[2]; ++x) {
                m_refinementMap[{x, y}] = level;
            }
        }
    }

    // bounds fuer jedes Level > 1 aus der neu aufgebauten Map
    std::map<t_idx, std::array<t_idx, 4>> levelBounds;
    for (auto const& [pos, level] : m_refinementMap) {
        if (level == 1) continue;
        auto [ix, iy] = pos;
        auto it = levelBounds.find(level);
        if (it == levelBounds.end()) {
            levelBounds[level] = {ix, iy, ix, iy};
        } else {
            auto& bounds = it->second;
            bounds[0] = std::min(bounds[0], ix);
            bounds[1] = std::min(bounds[1], iy);
            bounds[2] = std::max(bounds[2], ix);
            bounds[3] = std::max(bounds[3], iy);
        }
    }

    // feine Gitter fuer jedes Level > 1 erstellen
    for (auto& [level, bounds] : levelBounds) {
        t_idx fineNx = (bounds[2] - bounds[0] + 1) * level;
        t_idx fineNy = (bounds[3] - bounds[1] + 1) * level;

        m_fineGrids[level]     = new WavePropagation2d(fineNx, fineNy);
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

/**
 * Sichert den Zustand des groben Gitters (eta, h, hu, hv) inkl. des
 * Ghost-Rings VOR dem groben Zeitschritt. Wird fuer die lineare
 * Zeitinterpolation der feinen Randwerte waehrend des Subcyclings benoetigt.
 */
void WavePropagationAdaptiveGrid2d::snapshotCoarse() {
    t_sidx nx      = static_cast<t_sidx>(m_nCellsX);
    t_sidx ny      = static_cast<t_sidx>(m_nCellsY);
    t_sidx cStride = static_cast<t_sidx>(m_coarseGrid->getStride());

    t_real const* h  = m_coarseGrid->getHeight();
    t_real const* b  = m_coarseGrid->getBathymetry();
    t_real const* hu = m_coarseGrid->getMomentumX();
    t_real const* hv = m_coarseGrid->getMomentumY();

    std::size_t size       = static_cast<std::size_t>((nx + 2) * (ny + 2));
    t_sidx      snapStride = nx + 2;

    m_snapEta.resize(size);
    m_snapH.resize(size);
    m_snapHu.resize(size);
    m_snapHv.resize(size);

    for (t_sidx cy = -1; cy <= ny; cy++) {
        for (t_sidx cx = -1; cx <= nx; cx++) {
            t_sidx      src = cy * cStride + cx;
            std::size_t dst = static_cast<std::size_t>((cy + 1) * snapStride + (cx + 1));
            m_snapEta[dst] = h[src] + b[src];
            m_snapH[dst]   = h[src];
            m_snapHu[dst]  = hu[src];
            m_snapHv[dst]  = hv[src];
        }
    }
}

void WavePropagationAdaptiveGrid2d::interpolateBoundaries(t_idx i_refinement, t_real i_theta) {
    WavePropagation2d* fine = m_fineGrids[i_refinement];
    auto& bounds            = m_refinedBounds[i_refinement];

    t_sidx r   = static_cast<t_sidx>(i_refinement);
    t_sidx nxF = (static_cast<t_sidx>(bounds[2]) - bounds[0] + 1) * r;
    t_sidx nyF = (static_cast<t_sidx>(bounds[3]) - bounds[1] + 1) * r;

    t_sidx fStride = static_cast<t_sidx>(fine->getStride());
    t_sidx cStride = static_cast<t_sidx>(m_coarseGrid->getStride());

    t_sidx nx         = static_cast<t_sidx>(m_nCellsX);
    t_sidx ny         = static_cast<t_sidx>(m_nCellsY);
    t_sidx snapStride = nx + 2;

    static constexpr t_real G      = 9.81;
    static constexpr t_real FR_MAX = 0.1;
    static constexpr t_real W_EPS  = 1e-6;

    t_real* fH  = const_cast<t_real*>(fine->getHeight());
    t_real* fHu = const_cast<t_real*>(fine->getMomentumX());
    t_real* fHv = const_cast<t_real*>(fine->getMomentumY());
    t_real* fB  = const_cast<t_real*>(fine->getBathymetry());

    // grober Zustand
    t_real const* cH  = m_coarseGrid->getHeight();
    t_real const* cB  = m_coarseGrid->getBathymetry();
    t_real const* cHu = m_coarseGrid->getMomentumX();
    t_real const* cHv = m_coarseGrid->getMomentumY();

    auto clampX = [nx](t_sidx v) { return std::max<t_sidx>(-1, std::min<t_sidx>(v, nx)); };
    auto clampY = [ny](t_sidx v) { return std::max<t_sidx>(-1, std::min<t_sidx>(v, ny)); };

    auto fillGhostCell = [&](t_sidx gx, t_sidx gy) {
        // Zentrum der feinen Zelle in groben Indexkoordinaten
        t_real xc = static_cast<t_real>(bounds[0]) + (gx + static_cast<t_real>(0.5)) / r - static_cast<t_real>(0.5);
        t_real yc = static_cast<t_real>(bounds[1]) + (gy + static_cast<t_real>(0.5)) / r - static_cast<t_real>(0.5);

        t_sidx i0 = static_cast<t_sidx>(std::floor(xc));
        t_sidx j0 = static_cast<t_sidx>(std::floor(yc));
        t_real wx = xc - static_cast<t_real>(i0);
        t_real wy = yc - static_cast<t_real>(j0);

        t_sidx is[2] = {clampX(i0), clampX(i0 + 1)};
        t_sidx js[2] = {clampY(j0), clampY(j0 + 1)};
        t_real w[2][2] = {
            {(1 - wx) * (1 - wy), (1 - wx) * wy},
            {wx * (1 - wy),       wx * wy}
        };

        t_real bAcc = 0;
        for (int a = 0; a < 2; a++)
            for (int bIdx = 0; bIdx < 2; bIdx++)
                bAcc += w[a][bIdx] * cB[js[bIdx] * cStride + is[a]];
        t_real bGhost = bAcc;

        t_real etaOldAcc = 0, uOldAcc = 0, vOldAcc = 0, wOldSum = 0;
        t_real etaNewAcc = 0, uNewAcc = 0, vNewAcc = 0, wNewSum = 0;

        for (int a = 0; a < 2; a++) {
            for (int bIdx = 0; bIdx < 2; bIdx++) {
                t_sidx      cIdx = js[bIdx] * cStride + is[a];
                std::size_t sIdx = static_cast<std::size_t>((js[bIdx] + 1) * snapStride + (is[a] + 1));
                t_real      wgt  = w[a][bIdx];

                if (cH[cIdx] > DRY_TOL) {
                    etaNewAcc += wgt * (cH[cIdx] + cB[cIdx]);
                    uNewAcc   += wgt * (cHu[cIdx] / cH[cIdx]);
                    vNewAcc   += wgt * (cHv[cIdx] / cH[cIdx]);
                    wNewSum   += wgt;
                }
                if (m_snapH[sIdx] > DRY_TOL) {
                    etaOldAcc += wgt * m_snapEta[sIdx];
                    uOldAcc   += wgt * (m_snapHu[sIdx] / m_snapH[sIdx]);
                    vOldAcc   += wgt * (m_snapHv[sIdx] / m_snapH[sIdx]);
                    wOldSum   += wgt;
                }
            }
        }

        t_real etaOld, uOld, vOld, etaNew, uNew, vNew;
        if (wOldSum > W_EPS) {
            etaOld = etaOldAcc / wOldSum; uOld = uOldAcc / wOldSum; vOld = vOldAcc / wOldSum;
        } else {
            etaOld = bGhost; uOld = 0; vOld = 0;  
        }
        if (wNewSum > W_EPS) {
            etaNew = etaNewAcc / wNewSum; uNew = uNewAcc / wNewSum; vNew = vNewAcc / wNewSum;
        } else {
            etaNew = bGhost; uNew = 0; vNew = 0;
        }

        t_real eta = (1 - i_theta) * etaOld + i_theta * etaNew;
        t_real u   = (1 - i_theta) * uOld   + i_theta * uNew;
        t_real v   = (1 - i_theta) * vOld   + i_theta * vNew;

        t_real hGhost = std::max<t_real>(eta - bGhost, 0.0);

        t_sidx f = gy * fStride + gx;
        fB[f] = bGhost;
        fH[f] = hGhost;
        if (hGhost <= DRY_TOL) {
            fHu[f] = 0;
            fHv[f] = 0;
        } else {
            t_real speed = std::sqrt(u * u + v * v);
            t_real uMax  = FR_MAX * std::sqrt(G * hGhost);
            if (speed > uMax && speed > 0) {
                t_real s = uMax / speed;
                u *= s;
                v *= s;
            }
            fHu[f] = hGhost * u;
            fHv[f] = hGhost * v;
        }
    };

    bool skipLeft   = (bounds[0] == 0);
    bool skipRight  = (bounds[2] == m_nCellsX - 1);
    bool skipBottom = (bounds[1] == 0);
    bool skipTop    = (bounds[3] == m_nCellsY - 1);

    if (!skipBottom) for (t_sidx gx = -1; gx <= nxF; gx++) fillGhostCell(gx, -1);
    if (!skipTop)    for (t_sidx gx = -1; gx <= nxF; gx++) fillGhostCell(gx, nyF);
    if (!skipLeft)   for (t_sidx gy = -1; gy <= nyF; gy++) fillGhostCell(-1, gy);
    if (!skipRight)  for (t_sidx gy = -1; gy <= nyF; gy++) fillGhostCell(nxF, gy);
}

void WavePropagationAdaptiveGrid2d::restrictBoundary(t_idx i_refinement) {
    static constexpr t_real G = 9.81;
    static constexpr t_real FR_MAX = 0.1;

    t_real const* fineH  = m_fineGrids[i_refinement]->getHeight();
    t_real const* fineB  = m_fineGrids[i_refinement]->getBathymetry();
    t_real const* fineHU = m_fineGrids[i_refinement]->getMomentumX();
    t_real const* fineHV = m_fineGrids[i_refinement]->getMomentumY();
    t_idx fineStride     = m_fineGrids[i_refinement]->getStride();

    t_real const* coarseB = m_coarseGrid->getBathymetry();
    t_idx coarseStride    = m_coarseGrid->getStride();

    for (auto& [coarsePos, level] : m_refinementMap) {
        if (level != i_refinement) continue;
        auto [coarseX, coarseY] = coarsePos;

        t_idx fineX0, fineY0;
        coarseToFineIndices(coarseX, coarseY, i_refinement, fineX0, fineY0);

        // eta = h + b ueber nasse Subzellen mitteln 
        t_real avgEta = 0;
        t_real avgHU  = 0;
        t_real avgHV  = 0;
        t_idx  wetCount = 0;

        for (t_idx fi = 0; fi < i_refinement; fi++) {
            for (t_idx fj = 0; fj < i_refinement; fj++) {
                t_idx fineIdx = (fineY0 + fj) * fineStride + (fineX0 + fi);
                if (fineH[fineIdx] <= DRY_TOL) continue;

                avgEta += (fineH[fineIdx] + fineB[fineIdx]);
                avgHU  += fineHU[fineIdx];
                avgHV  += fineHV[fineIdx];
                wetCount++;
            }
        }

        t_idx coarseIdx = coarseY * coarseStride + coarseX;

        if (wetCount == 0) {
            m_coarseGrid->setHeight   (coarseX, coarseY, 0.0);
            m_coarseGrid->setMomentumX(coarseX, coarseY, 0.0);
            m_coarseGrid->setMomentumY(coarseX, coarseY, 0.0);
            continue;
        }

        t_real inv        = 1.0 / static_cast<t_real>(wetCount);
        t_real b_coarse   = coarseB[coarseIdx];
        t_real newHCoarse = std::max<t_real>(avgEta * inv - b_coarse, 0.0);

        m_coarseGrid->setHeight(coarseX, coarseY, newHCoarse);

        if (newHCoarse <= DRY_TOL) {
            m_coarseGrid->setMomentumX(coarseX, coarseY, 0.0);
            m_coarseGrid->setMomentumY(coarseX, coarseY, 0.0);
        } else {
            t_real hu = avgHU * inv;
            t_real hv = avgHV * inv;
            t_real u     = hu / newHCoarse;
            t_real v     = hv / newHCoarse;
            t_real speed = std::sqrt(u * u + v * v);
            t_real uMax  = FR_MAX * std::sqrt(G * newHCoarse);

            if (speed > uMax && speed > 0) {
                t_real s = uMax / speed;
                u *= s;
                v *= s;
            }

            m_coarseGrid->setMomentumX(coarseX, coarseY, newHCoarse * u);
            m_coarseGrid->setMomentumY(coarseX, coarseY, newHCoarse * v);
        }
    }
}

void WavePropagationAdaptiveGrid2d::timeStep(t_real i_scaling) {
    // 1. Randbedingungen des groben Gitters setzen
    m_coarseGrid->setGhostCells(m_leftReflecting, m_rightReflecting, m_bottomReflecting, m_topReflecting);
    snapshotCoarse();

    // 2. Grobes Gitter einen vollen Schritt vorwaerts
    m_coarseGrid->timeStep(i_scaling);
    m_coarseGrid->setGhostCells(m_leftReflecting, m_rightReflecting, m_bottomReflecting, m_topReflecting);

    // 3. Feine Gitter mit Subcycling
    for (auto& [refinement, fineGrid] : m_fineGrids) {
        auto& bounds = m_refinedBounds[refinement];
        bool left   = (bounds[0] == 0)             && m_leftReflecting;
        bool right  = (bounds[2] == m_nCellsX - 1) && m_rightReflecting;
        bool bottom = (bounds[1] == 0)             && m_bottomReflecting;
        bool top    = (bounds[3] == m_nCellsY - 1) && m_topReflecting;

        for (t_idx subcycle = 0; subcycle < refinement; subcycle++) {
            fineGrid->setGhostCells(left, right, bottom, top);

            t_real theta = static_cast<t_real>(subcycle) / static_cast<t_real>(refinement);
            interpolateBoundaries(refinement, theta);

            fineGrid->timeStep(i_scaling);
        }

        // 4. Feine Daten auf das grobe Gitter mitteln
        restrictBoundary(refinement);
    }
}

void WavePropagationAdaptiveGrid2d::setGhostCells(bool i_leftReflecting, bool i_rightReflecting, bool i_bottomReflecting, bool i_topReflecting) {
    m_leftReflecting   = i_leftReflecting;
    m_rightReflecting  = i_rightReflecting;
    m_bottomReflecting = i_bottomReflecting;
    m_topReflecting    = i_topReflecting;
}

void WavePropagationAdaptiveGrid2d::syncCoarseBathymetry() {
    for (auto& [level, fineGrid] : m_fineGrids) {
        auto& bounds = m_refinedBounds[level];

        t_real const* fb  = fineGrid->getBathymetry();
        t_real const* fh  = fineGrid->getHeight();
        t_real const* fhu = fineGrid->getMomentumX();
        t_real const* fhv = fineGrid->getMomentumY();
        t_idx fStride     = fineGrid->getStride();

        t_real invAll = 1.0 / static_cast<t_real>(level * level);

        for (t_idx cy = bounds[1]; cy <= bounds[3]; ++cy)
            for (t_idx cx = bounds[0]; cx <= bounds[2]; ++cx) {
                if (getRefinement(cx, cy) != level) continue;

                t_idx fx0, fy0;
                coarseToFineIndices(cx, cy, level, fx0, fy0);

                // Bathymetrie ueber alle Subzellen 
                t_real sB = 0;
                // eta nur ueber nasse Subzellen
                t_real sEta = 0, sHu = 0, sHv = 0;
                t_idx  wetCount = 0;

                for (t_idx fj = 0; fj < level; fj++)
                    for (t_idx fi = 0; fi < level; fi++) {
                        t_idx i = (fy0 + fj) * fStride + (fx0 + fi);
                        sB += fb[i];
                        if (fh[i] > DRY_TOL) {
                            sEta += fh[i] + fb[i];
                            sHu  += fhu[i];
                            sHv  += fhv[i];
                            wetCount++;
                        }
                    }

                t_real bCoarse = sB * invAll;
                m_coarseGrid->setBathymetry(cx, cy, bCoarse);

                if (wetCount == 0) {
                    m_coarseGrid->setHeight   (cx, cy, 0.0);
                    m_coarseGrid->setMomentumX(cx, cy, 0.0);
                    m_coarseGrid->setMomentumY(cx, cy, 0.0);
                    continue;
                }

                t_real invWet  = 1.0 / static_cast<t_real>(wetCount);
                t_real hCoarse = std::max<t_real>(sEta * invWet - bCoarse, 0.0);

                m_coarseGrid->setHeight(cx, cy, hCoarse);
                if (hCoarse <= DRY_TOL) {
                    m_coarseGrid->setMomentumX(cx, cy, 0.0);
                    m_coarseGrid->setMomentumY(cx, cy, 0.0);
                } else {
                    m_coarseGrid->setMomentumX(cx, cy, sHu * invWet);
                    m_coarseGrid->setMomentumY(cx, cy, sHv * invWet);
                }
            }
    }
}

t_idx WavePropagationAdaptiveGrid2d::getStride() { return m_coarseGrid->getStride(); }
t_real const* WavePropagationAdaptiveGrid2d::getHeight() { return m_coarseGrid->getHeight(); }
t_real const* WavePropagationAdaptiveGrid2d::getMomentumX() { return m_coarseGrid->getMomentumX(); }
t_real const* WavePropagationAdaptiveGrid2d::getMomentumY() { return m_coarseGrid->getMomentumY(); }
t_real const* WavePropagationAdaptiveGrid2d::getBathymetry() { return m_coarseGrid->getBathymetry(); }

void WavePropagationAdaptiveGrid2d::setHeight(t_idx i_ix, t_idx i_iy, t_real i_h, setups::Setup* i_setup) {
    m_coarseGrid->setHeight(i_ix, i_iy, i_h);

    if (isRefined(i_ix, i_iy)) {
        t_idx refinement            = getRefinement(i_ix, i_iy);
        WavePropagation2d* fineGrid = m_fineGrids[refinement];
        t_idx fineX0, fineY0;
        coarseToFineIndices(i_ix, i_iy, refinement, fineX0, fineY0);

        t_real fineCellSize = (1.0 / refinement);

        for (t_idx fi = 0; fi < refinement; fi++) {
            for (t_idx fj = 0; fj < refinement; fj++) {
                t_real x      = i_ix - 0.5 + fineCellSize * fi + fineCellSize / 2;
                t_real y      = i_iy - 0.5 + fineCellSize * fj + fineCellSize / 2;
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
        t_idx refinement            = getRefinement(i_ix, i_iy);
        WavePropagation2d* fineGrid = m_fineGrids[refinement];
        t_idx fineX0, fineY0;
        coarseToFineIndices(i_ix, i_iy, refinement, fineX0, fineY0);

        t_real const* cH     = m_coarseGrid->getHeight();
        t_idx         cStr   = m_coarseGrid->getStride();
        t_real        hCoar  = cH[i_iy * cStr + i_ix];
        t_real        u      = (hCoar > DRY_TOL) ? i_hu / hCoar : 0;

        t_real const* fH   = fineGrid->getHeight();
        t_idx         fStr = fineGrid->getStride();

        for (t_idx fi = 0; fi < refinement; fi++)
            for (t_idx fj = 0; fj < refinement; fj++) {
                t_real hFine = fH[(fineY0 + fj) * fStr + (fineX0 + fi)];
                t_real huFine = (hFine > DRY_TOL) ? hFine * u : 0;
                fineGrid->setMomentumX(fineX0 + fi, fineY0 + fj, huFine);
            }
    }
}

void WavePropagationAdaptiveGrid2d::setMomentumY(t_idx i_ix, t_idx i_iy, t_real i_hv) {
    m_coarseGrid->setMomentumY(i_ix, i_iy, i_hv);

    if (isRefined(i_ix, i_iy)) {
        t_idx refinement            = getRefinement(i_ix, i_iy);
        WavePropagation2d* fineGrid = m_fineGrids[refinement];
        t_idx fineX0, fineY0;
        coarseToFineIndices(i_ix, i_iy, refinement, fineX0, fineY0);

        t_real const* cH     = m_coarseGrid->getHeight();
        t_idx         cStr   = m_coarseGrid->getStride();
        t_real        hCoar  = cH[i_iy * cStr + i_ix];
        t_real        v      = (hCoar > DRY_TOL) ? i_hv / hCoar : 0;

        t_real const* fH   = fineGrid->getHeight();
        t_idx         fStr = fineGrid->getStride();

        for (t_idx fi = 0; fi < refinement; fi++)
            for (t_idx fj = 0; fj < refinement; fj++) {
                t_real hFine = fH[(fineY0 + fj) * fStr + (fineX0 + fi)];
                t_real hvFine = (hFine > DRY_TOL) ? hFine * v : 0;
                fineGrid->setMomentumY(fineX0 + fi, fineY0 + fj, hvFine);
            }
    }
}

void WavePropagationAdaptiveGrid2d::setBathymetry(t_idx i_ix, t_idx i_iy, t_real i_b, setups::Setup* i_setup) {
    m_coarseGrid->setBathymetry(i_ix, i_iy, i_b);

    if (isRefined(i_ix, i_iy)) {
        t_idx refinement            = getRefinement(i_ix, i_iy);
        WavePropagation2d* fineGrid = m_fineGrids[refinement];
        t_idx fineX0, fineY0;
        coarseToFineIndices(i_ix, i_iy, refinement, fineX0, fineY0);

        t_real fineCellSize = (1.0 / refinement);

        for (t_idx fi = 0; fi < refinement; fi++)
            for (t_idx fj = 0; fj < refinement; fj++) {
                t_real x     = i_ix - 0.5 + fineCellSize * fi + fineCellSize / 2;
                t_real y     = i_iy - 0.5 + fineCellSize * fj + fineCellSize / 2;
                t_real bathy = i_setup->getBathymetry(x, y);
                if (std::isnan(bathy)) {
                    std::cerr << "CRITICAL: Bathymetry NaN detected at cell (" << i_ix << "|" << i_iy << ")"
                              << " for coordinates x=" << x << ", y=" << y << std::endl;
                    exit(1);
                }
                fineGrid->setBathymetry(fineX0 + fi, fineY0 + fj, bathy);
            }
    }
}

void WavePropagationAdaptiveGrid2d::exportUniformGrid(
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

                    t_idx ox  = cx * i_maxResolution + fx;
                    t_idx oy  = cy * i_maxResolution + fy;
                    t_idx out = oy * nxOut + ox;

                    o_b[out]  = b[idx];
                    o_h[out]  = h[idx];
                    o_hu[out] = hu[idx];
                    o_hv[out] = hv[idx];
                }
            }
        }
    }

    for (auto const& [level, fineGrid] : m_fineGrids) {

        auto bounds = m_refinedBounds.at(level);

        auto const* fb  = fineGrid->getBathymetry();
        auto const* fh  = fineGrid->getHeight();
        auto const* fhu = fineGrid->getMomentumX();
        auto const* fhv = fineGrid->getMomentumY();

        t_idx fineStride = fineGrid->getStride();
        t_idx block      = i_maxResolution / level;

        for (t_idx cy = bounds[1]; cy <= bounds[3]; ++cy) {
            for (t_idx cx = bounds[0]; cx <= bounds[2]; ++cx) {
                if (getRefinement(cx, cy) != level)
                    continue;

                t_idx fineX0, fineY0;
                coarseToFineIndices(cx, cy, level, fineX0, fineY0);

                for (t_idx fy = 0; fy < level; ++fy) {
                    for (t_idx fx = 0; fx < level; ++fx) {

                        t_idx fineIdx = (fineY0 + fy) * fineStride + (fineX0 + fx);

                        t_idx ox = cx * i_maxResolution + fx * block;
                        t_idx oy = cy * i_maxResolution + fy * block;

                        for (t_idx by = 0; by < block; ++by) {
                            for (t_idx bx = 0; bx < block; ++bx) {

                                t_idx out = (oy + by) * nxOut + (ox + bx);

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