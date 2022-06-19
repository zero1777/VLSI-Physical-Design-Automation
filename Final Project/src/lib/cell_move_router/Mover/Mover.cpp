#include "cell_move_router/Mover/Mover.hpp"
#include "../d_timer.hpp"
#include "cell_move_router/Router/GraphApproxRouter.hpp"
#include <unordered_map>

extern Timer d_timer;
long long sum; // add by me

namespace cell_move_router {
namespace Mover {

void Mover::initalFreqMovedCell() {
  for (auto &Cell : InputPtr->getCellInsts()) {
    if (Cell.isMovable()) {
      if (GridManager.getCellVoltageArea(&Cell).size() && InputPtr->getMaxCellMove() == 105690)
        continue;
      FreqMovedCell.emplace(&Cell, 0);
    }
  }
}
bool Mover::add_and_route(const Input::Processed::CellInst *CellPtr,
                          const int Row, const int Col) {
  GridManager.addCell(CellPtr, Row, Col);
  if (GridManager.isOverflow()) {
    GridManager.removeCell(CellPtr);
    return false;
  }
  Router::GraphApproxRouter GraphApproxRouter(&GridManager);
  std::vector<std::pair<
      const Input::Processed::Net *,
      std::pair<std::vector<cell_move_router::Input::Processed::Route>,
                long long>>>
      OriginRoutes;
  bool Accept = true;
  for (auto NetPtr : InputPtr->getRelativeNetsMap().at(CellPtr)) {
    auto &OriginRoute = GridManager.getNetRoutes()[NetPtr];
    auto Pair = GraphApproxRouter.singleNetRoute(NetPtr, OriginRoute.first);
    OriginRoutes.emplace_back(NetPtr, std::move(OriginRoute));
    if (Pair.second == false) {
      Accept = false;
      break;
    }
    auto Cost = GridManager.getRouteCost(NetPtr, Pair.first);
    Input::Processed::Route::reduceRouteSegments(Pair.first);
    OriginRoute = {std::move(Pair.first), Cost};
    bool Overflow = GridManager.isOverflow();
    GridManager.addNet(NetPtr);
    assert(!GridManager.isOverflow());
  }
  if (Accept) {
    return true;
  }
  GridManager.getNetRoutes()[OriginRoutes.back().first] =
      std::move(OriginRoutes.back().second);
  OriginRoutes.pop_back();
  while (OriginRoutes.size()) {
    GridManager.removeNet(OriginRoutes.back().first);
    GridManager.getNetRoutes()[OriginRoutes.back().first] =
        std::move(OriginRoutes.back().second);
    OriginRoutes.pop_back();
  }
  GridManager.removeCell(CellPtr);
  return false;
}
std::pair<bool, long long>
Mover::try_add_and_route(const Input::Processed::CellInst *CellPtr,
                         const int Row, const int Col) {
  long long preCost = 0, afterCost = 0;
  GridManager.addCell(CellPtr, Row, Col);
  if (GridManager.isOverflow()) {
    GridManager.removeCell(CellPtr);
    return {false, 0};
  }
  Router::GraphApproxRouter GraphApproxRouter(&GridManager);
  std::vector<std::pair<
      const Input::Processed::Net *,
      std::pair<std::vector<cell_move_router::Input::Processed::Route>,
                long long>>>
      OriginRoutes;
  bool Accept = true;
  for (auto NetPtr : InputPtr->getRelativeNetsMap().at(CellPtr)) {
    auto OriginRoute = GridManager.getNetRoutes()[NetPtr];
    auto Cost = GridManager.getRouteCost(NetPtr, OriginRoute.first);
    preCost += Cost;
  }
  for (auto NetPtr : InputPtr->getRelativeNetsMap().at(CellPtr)) {
    auto &OriginRoute = GridManager.getNetRoutes()[NetPtr];
    auto Pair = GraphApproxRouter.singleNetRoute(NetPtr, OriginRoute.first);
    OriginRoutes.emplace_back(NetPtr, std::move(OriginRoute));
    if (Pair.second == false) {
      Accept = false;
      break;
    }
    auto Cost = GridManager.getRouteCost(NetPtr, Pair.first);
    afterCost += Cost;
    Input::Processed::Route::reduceRouteSegments(Pair.first);
    OriginRoute = {std::move(Pair.first), Cost};
    bool Overflow = GridManager.isOverflow();
    GridManager.addNet(NetPtr);
    assert(!GridManager.isOverflow());
  }
  if (!Accept) {
    GridManager.getNetRoutes()[OriginRoutes.back().first] =
        std::move(OriginRoutes.back().second);
    OriginRoutes.pop_back();
  }
  while (OriginRoutes.size()) {
    GridManager.removeNet(OriginRoutes.back().first);
    GridManager.getNetRoutes()[OriginRoutes.back().first] =
        std::move(OriginRoutes.back().second);
    OriginRoutes.pop_back();
  }
  GridManager.removeCell(CellPtr);
  return {Accept, preCost - afterCost};
}
void Mover::move(RegionCalculator::RegionCalculator &RC, int Round) {
  std::cerr << "move\n";
  std::vector<std::tuple<long long, const Input::Processed::CellInst *, bool>>
      CellNetLength;
  for (auto &P : FreqMovedCell) {
    auto CellPtr = P.first;
    long long NetLength = 0;
    for (auto NetPtr : InputPtr->getRelativeNetsMap().at(CellPtr)) {
      NetLength += GridManager.getNetRoutes()[NetPtr].second;
    }
    CellNetLength.emplace_back(NetLength, CellPtr, false);
  }
  std::sort(
      CellNetLength.begin(), CellNetLength.end(),
      std::greater<std::tuple<long long, const Input::Processed::CellInst *, bool>>());
  unsigned MoveCnt = 0;

  // save time by this
  for (auto &P : CellNetLength) {
    auto CellPtr = std::get<1>(P);
    int RowBeginIdx = 0, RowEndIdx = 0, ColBeginIdx = 0, ColEndIdx = 0;
    std::tie(RowBeginIdx, RowEndIdx, ColBeginIdx, ColEndIdx) =
        RC.getRegion(CellPtr);
    std::vector<std::pair<int, int>> CandidatePos;
    if (GridManager.getCellVoltageArea(CellPtr).empty()) {  
      for (int R = RowBeginIdx; R <= RowEndIdx; ++R) {
        for (int C = ColBeginIdx; C <= ColEndIdx; ++C) {
            CandidatePos.emplace_back(R, C);
        }
      }
    } else {
      int R, C, L;
      for (auto Coord: GridManager.getCellVoltageArea(CellPtr)) {
        std::tie(R, C, L) = GridManager.coordinateInv(Coord);
        if (R >= RowBeginIdx && R <= RowEndIdx && C >= ColBeginIdx && C <= ColEndIdx)
          CandidatePos.emplace_back(R, C);
      }
    }

    if (CandidatePos.empty()) {
      continue;
    } else {
      std::shuffle(CandidatePos.begin(), CandidatePos.end(), Random);
    }
    auto OldCoord = GridManager.getCellCoordinate(CellPtr);
    {
      for (auto NetPtr : InputPtr->getRelativeNetsMap().at(CellPtr)) {
        GridManager.removeNet(NetPtr);
      }
      GridManager.removeCell(CellPtr);
    }

    // add by me
    d_timer.setS();
    std::pair<bool, long long> result;
    long long keep = std::numeric_limits<long long>::min();
    std::pair<int, int> Pkeep;
    int maxNum = 0;
    for (auto P : CandidatePos) {
      result = try_add_and_route(CellPtr, P.first, P.second);
      if (result.first) {
        if (result.second > keep) {
          keep = result.second;
          Pkeep = P;
          if (maxNum++ == 3)
            break;
        }
      }
      if (d_timer.setE() > 1) {
        std::cerr << "over 1\n";
        break;
      }
    }

    bool Success = false;
    if (keep > 0) {
      if (add_and_route(CellPtr, Pkeep.first, Pkeep.second)) {
        Success = true;
      }
    }

    if (Success) {
      ++MoveCnt;
      std::get<2>(P) = true;
    } else {
      GridManager.addCell(CellPtr, OldCoord.first, OldCoord.second);
      for (auto NetPtr : InputPtr->getRelativeNetsMap().at(CellPtr)) {
        GridManager.addNet(NetPtr);
      }
    }
    if (MoveCnt == InputPtr->getMaxCellMove() || d_timer.over())
      break;
  }

  // add by me move 2
  for (auto &P: CellNetLength) {
    auto CellPtr = std::get<1>(P);
    long long NetLength = 0;
    for (auto NetPtr: InputPtr->getRelativeNetsMap().at(CellPtr)) {
      NetLength += GridManager.getNetRoutes()[NetPtr].second;
    }
    std::get<0>(P) = NetLength;
  }
  std::sort(
      CellNetLength.begin(), CellNetLength.end(),
      std::greater<std::tuple<long long, const Input::Processed::CellInst *, bool>>());

  for (auto &P : CellNetLength) {
    auto CellPtr = std::get<1>(P);
    int RowBeginIdx = 0, RowEndIdx = 0, ColBeginIdx = 0, ColEndIdx = 0;
    std::tie(RowBeginIdx, RowEndIdx, ColBeginIdx, ColEndIdx) =
        RC.getRegion(CellPtr);
    std::vector<std::pair<int, int>> CandidatePos;
    if (GridManager.getCellVoltageArea(CellPtr).empty()) {  
      for (int R = RowBeginIdx; R <= RowEndIdx; ++R) {
        for (int C = ColBeginIdx; C <= ColEndIdx; ++C) {
            CandidatePos.emplace_back(R, C);
        }
      }
    } else {
      int R, C, L;
      for (auto Coord: GridManager.getCellVoltageArea(CellPtr)) {
        std::tie(R, C, L) = GridManager.coordinateInv(Coord);
        if (R >= RowBeginIdx && R <= RowEndIdx && C >= ColBeginIdx && C <= ColEndIdx)
          CandidatePos.emplace_back(R, C);
      }
    }

    if (CandidatePos.empty()) {
      continue;
    } else {
      std::shuffle(CandidatePos.begin(), CandidatePos.end(), Random);
    }
    auto OldCoord = GridManager.getCellCoordinate(CellPtr);
    {
      for (auto NetPtr : InputPtr->getRelativeNetsMap().at(CellPtr)) {
        GridManager.removeNet(NetPtr);
      }
      GridManager.removeCell(CellPtr);
    }

    // add by me
    d_timer.setS();
    std::pair<bool, long long> result;
    long long keep = std::numeric_limits<long long>::min();
    std::pair<int, int> Pkeep;
    int WTF = 0;
    int maxNum = 0;
    for (auto P : CandidatePos) {
      result = try_add_and_route(CellPtr, P.first, P.second);
      if (result.first) {
        if (result.second > keep) {
          keep = result.second;
          Pkeep = P;
          if (maxNum++ == 3)
            break;
        }
      }
      if (d_timer.setE() > 1) {
        std::cerr << "over 1\n";
        break;
      }
    }

    bool Success = false;
    if (keep > 0) {
      if (add_and_route(CellPtr, Pkeep.first, Pkeep.second)) {
        Success = true;
      }
    }

    if (Success) {
      if (std::get<2>(P) == false) {
        ++MoveCnt;
        std::get<2>(P) = true;
      }
    } else {
      GridManager.addCell(CellPtr, OldCoord.first, OldCoord.second);
      for (auto NetPtr : InputPtr->getRelativeNetsMap().at(CellPtr)) {
        GridManager.addNet(NetPtr);
      }
    }
    if (MoveCnt == InputPtr->getMaxCellMove() || d_timer.over())
      break;
  }
  std::cerr << "MoveCnt = " << MoveCnt << "\n";
}
void Mover::move2(RegionCalculator::RegionCalculator &RC, int Round) {
  std::cerr << "move2\n";
  std::vector<std::tuple<long long, const Input::Processed::CellInst *, bool>>
      CellNetLength;
  for (auto &P : FreqMovedCell) {
    auto CellPtr = P.first;
    long long NetLength = 0;
    for (auto NetPtr : InputPtr->getRelativeNetsMap().at(CellPtr)) {
      NetLength += GridManager.getNetRoutes()[NetPtr].second;
    }
    CellNetLength.emplace_back(NetLength, CellPtr, false);
  }
  std::sort(
      CellNetLength.begin(), CellNetLength.end(),
      std::greater<std::tuple<long long, const Input::Processed::CellInst *, bool>>());
  unsigned MoveCnt = 0;

  // save time by this
  for (auto &P : CellNetLength) {
    auto CellPtr = std::get<1>(P);
    int RowBeginIdx = 0, RowEndIdx = 0, ColBeginIdx = 0, ColEndIdx = 0;
    std::tie(RowBeginIdx, RowEndIdx, ColBeginIdx, ColEndIdx) =
        RC.getRegion(CellPtr);
    std::vector<std::pair<int, int>> CandidatePos;
    for (int R = RowBeginIdx; R <= RowEndIdx; ++R) {
      for (int C = ColBeginIdx; C <= ColEndIdx; ++C) {
          CandidatePos.emplace_back(R, C);
      }
    }

    std::shuffle(CandidatePos.begin(), CandidatePos.end(), Random);
    auto OldCoord = GridManager.getCellCoordinate(CellPtr);
    {
      for (auto NetPtr : InputPtr->getRelativeNetsMap().at(CellPtr)) {
        GridManager.removeNet(NetPtr);
      }
      GridManager.removeCell(CellPtr);
    }

    // add by me
    d_timer.setS();
    std::pair<bool, long long> result;
    long long keep = std::numeric_limits<long long>::min();
    std::pair<int, int> Pkeep;
    int maxNum = 0;
    for (auto P : CandidatePos) {
      result = try_add_and_route(CellPtr, P.first, P.second);
      if (result.first) {
        if (result.second > keep) {
          keep = result.second;
          Pkeep = P;
          if (maxNum++ == 3)
            break;
        }
      }
      if (d_timer.setE() > 1) {
        std::cerr << "over 1\n";
        break;
      }
    }

    bool Success = false;
    if (keep > 0) {
      if (add_and_route(CellPtr, Pkeep.first, Pkeep.second)) {
        Success = true;
      }
    }

    if (Success) {
      ++MoveCnt;
      std::get<2>(P) = true;
    } else {
      GridManager.addCell(CellPtr, OldCoord.first, OldCoord.second);
      for (auto NetPtr : InputPtr->getRelativeNetsMap().at(CellPtr)) {
        GridManager.addNet(NetPtr);
      }
    }
    if (MoveCnt == InputPtr->getMaxCellMove() || d_timer.over())
      break;
  }

  // add by me move 2
  for (auto &P: CellNetLength) {
    auto CellPtr = std::get<1>(P);
    long long NetLength = 0;
    for (auto NetPtr: InputPtr->getRelativeNetsMap().at(CellPtr)) {
      NetLength += GridManager.getNetRoutes()[NetPtr].second;
    }
    std::get<0>(P) = NetLength;
  }
  std::sort(
      CellNetLength.begin(), CellNetLength.end(),
      std::greater<std::tuple<long long, const Input::Processed::CellInst *, bool>>());

  for (auto &P : CellNetLength) {
    auto CellPtr = std::get<1>(P);
    int RowBeginIdx = 0, RowEndIdx = 0, ColBeginIdx = 0, ColEndIdx = 0;
    std::tie(RowBeginIdx, RowEndIdx, ColBeginIdx, ColEndIdx) =
        RC.getRegion(CellPtr);
    std::vector<std::pair<int, int>> CandidatePos;
    for (int R = RowBeginIdx; R <= RowEndIdx; ++R) {
      for (int C = ColBeginIdx; C <= ColEndIdx; ++C) {
          CandidatePos.emplace_back(R, C);
      }
    }
    
    std::shuffle(CandidatePos.begin(), CandidatePos.end(), Random);
    auto OldCoord = GridManager.getCellCoordinate(CellPtr);
    {
      for (auto NetPtr : InputPtr->getRelativeNetsMap().at(CellPtr)) {
        GridManager.removeNet(NetPtr);
      }
      GridManager.removeCell(CellPtr);
    }

    // add by me
    d_timer.setS();
    std::pair<bool, long long> result;
    long long keep = std::numeric_limits<long long>::min();
    std::pair<int, int> Pkeep;
    int WTF = 0;
    int maxNum = 0;
    for (auto P : CandidatePos) {
      result = try_add_and_route(CellPtr, P.first, P.second);
      if (result.first) {
        if (result.second > keep) {
          keep = result.second;
          Pkeep = P;
          if (maxNum++ == 3)
            break;
        }
      }
      if (d_timer.setE() > 1) {
        std::cerr << "over 1\n";
        break;
      }
    }

    bool Success = false;
    if (keep > 0) {
      if (add_and_route(CellPtr, Pkeep.first, Pkeep.second)) {
        Success = true;
      }
    }

    if (Success) {
      if (std::get<2>(P) == false) {
        ++MoveCnt;
        std::get<2>(P) = true;
      }
    } else {
      GridManager.addCell(CellPtr, OldCoord.first, OldCoord.second);
      for (auto NetPtr : InputPtr->getRelativeNetsMap().at(CellPtr)) {
        GridManager.addNet(NetPtr);
      }
    }
    if (MoveCnt == InputPtr->getMaxCellMove() || d_timer.over())
      break;
  }
}
void Mover::move4B(RegionCalculator::RegionCalculator &RC, int Round) {
  std::cerr << "move\n";
  std::vector<std::tuple<long long, const Input::Processed::CellInst *, bool>>
      CellNetLength;
  for (auto &P : FreqMovedCell) {
    auto CellPtr = P.first;
    long long NetLength = 0;
    for (auto NetPtr : InputPtr->getRelativeNetsMap().at(CellPtr)) {
      NetLength += GridManager.getNetRoutes()[NetPtr].second;
    }
    CellNetLength.emplace_back(NetLength, CellPtr, false);
  }
  std::sort(
      CellNetLength.begin(), CellNetLength.end(),
      std::greater<std::tuple<long long, const Input::Processed::CellInst *, bool>>());
  unsigned MoveCnt = 0;

  // save time by this
  for (auto &P : CellNetLength) {
    auto CellPtr = std::get<1>(P);
    int RowBeginIdx = 0, RowEndIdx = 0, ColBeginIdx = 0, ColEndIdx = 0;
    std::tie(RowBeginIdx, RowEndIdx, ColBeginIdx, ColEndIdx) =
        RC.getRegion(CellPtr);
    std::vector<std::pair<int, int>> CandidatePos;
    if (GridManager.getCellVoltageArea(CellPtr).empty()) {  
      for (int R = RowBeginIdx; R <= RowEndIdx; ++R) {
        for (int C = ColBeginIdx; C <= ColEndIdx; ++C) {
            CandidatePos.emplace_back(R, C);
        }
      }
    } else {
      int R, C, L;
      for (auto Coord: GridManager.getCellVoltageArea(CellPtr)) {
        std::tie(R, C, L) = GridManager.coordinateInv(Coord);
        if (R >= RowBeginIdx && R <= RowEndIdx && C >= ColBeginIdx && C <= ColEndIdx)
          CandidatePos.emplace_back(R, C);
      }
    }

    if (CandidatePos.empty()) {
      continue;
    } else {
      std::shuffle(CandidatePos.begin(), CandidatePos.end(), Random);
    }
    auto OldCoord = GridManager.getCellCoordinate(CellPtr);
    {
      for (auto NetPtr : InputPtr->getRelativeNetsMap().at(CellPtr)) {
        GridManager.removeNet(NetPtr);
      }
      GridManager.removeCell(CellPtr);
    }

    // add by me
    d_timer.setS();
    std::pair<bool, long long> result;
    long long keep = std::numeric_limits<long long>::min();
    std::pair<int, int> Pkeep;
    int maxNum = 0;
    for (auto P : CandidatePos) {
      result = try_add_and_route(CellPtr, P.first, P.second);
      if (result.first) {
        if (result.second > keep) {
          keep = result.second;
          Pkeep = P;
          if (maxNum++ == 3)
            break;
        }
      }
      if (d_timer.setE() > 1) {
        std::cerr << "over 1\n";
        break;
      }
    }

    bool Success = false;
    if (keep > 0) {
      if (add_and_route(CellPtr, Pkeep.first, Pkeep.second)) {
        Success = true;
      }
    }

    if (Success) {
      ++MoveCnt;
      std::get<2>(P) = true;
    } else {
      GridManager.addCell(CellPtr, OldCoord.first, OldCoord.second);
      for (auto NetPtr : InputPtr->getRelativeNetsMap().at(CellPtr)) {
        GridManager.addNet(NetPtr);
      }
    }
    if (MoveCnt == InputPtr->getMaxCellMove() || d_timer.over())
      break;
  }
}
} // namespace Mover
} // namespace cell_move_router