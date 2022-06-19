#pragma once
#include "cell_move_router/Grid/GridManager.hpp"
#include "cell_move_router/Input/Processed/Input.hpp"
#include "cell_move_router/RegionCalculator/RegionCalculator.hpp"
#include <random>
#include <unordered_map>

namespace cell_move_router {
namespace Mover {

class Mover {
  Grid::GridManager &GridManager;
  const Input::Processed::Input *InputPtr;
  std::default_random_engine Random;
  std::unordered_map<const Input::Processed::CellInst *, int> FreqMovedCell;

  void initalFreqMovedCell();

public:
  Mover(Grid::GridManager &GridManager)
      : GridManager(GridManager), InputPtr(GridManager.getInputPtr()) {
    Random.seed(7122);
    initalFreqMovedCell();
  }
  bool add_and_route(const Input::Processed::CellInst *CellPtr, const int Row,
                     const int Col);
  std::pair<bool, long long> try_add_and_route(const Input::Processed::CellInst *CellPtr, const int Row,
                     const int Col); // add by me
  void move(RegionCalculator::RegionCalculator &RC, int Round);
  void move2(RegionCalculator::RegionCalculator &RC, int Round);
  void move4B(RegionCalculator::RegionCalculator &RC, int Round);
};

} // namespace Mover
} // namespace cell_move_router